#ifndef HTSERVER_INCLUDE_HTSERVER_SCHEDULER_H
#define HTSERVER_INCLUDE_HTSERVER_SCHEDULER_H

#include<atomic>
#include<condition_variable>
#include<concepts>
#include<functional>
#include<iosfwd>
#include<list>
#include<memory>
#include<mutex>
#include<string>
#include<thread>
#include<type_traits>
#include<utility>
#include<variant>
#include<vector>

#include<htserver/export.h>
#include<htserver/fiber.h>
#include<htserver/types.h>

namespace htserver{

class HTSERVER_API Scheduler{
public:
    using ptr = std::shared_ptr<Scheduler>;
    using ThreadId = uint64_t;

    static constexpr ThreadId ANY_THREAD = 0;

    explicit Scheduler(std::size_t threads,bool use_caller = true,const std::string& name ="");

    virtual ~Scheduler();

    Scheduler(const Scheduler&) =delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler& operator=(Scheduler&&) =delete;

    [[nodiscard]] const std::string& getName()const noexcept {return m_name;}

    [[nodiscard]] static Scheduler* GetThis() noexcept;

    [[nodiscard]] static Fiber* GetMainFiber() noexcept;

    void start();

    void stop();

    template<typename FiberOrCallback>
    void addTask(FiberOrCallback&& task, ThreadId thread_id = ANY_THREAD){
        bool need_tickle = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            need_tickle = scheduleNoLock(std::forward<FiberOrCallback>(task),thread_id);
        }
        if(need_tickle){
            tickle();
        }
    }

    template<typename InputIterator>
    void addTask(InputIterator begin,InputIterator end){
        bool need_tickle = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            while(begin!=end){
                need_tickle = scheduleNoLock(*begin,ANY_THREAD) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle){
            tickle();
        }
    }

    // 切换到指定线程执行
    void switchTo(ThreadId thread_id = ANY_THREAD);

    std::ostream& dump(std::ostream& output) const;

protected:
    virtual void tickle();

    void run();

    [[nodiscard]] virtual bool canStop();

    //无任务时空闲处理
    virtual void idle();

    void setThis()noexcept;

    [[nodiscard]] bool hasIdleThreads() const noexcept{
        return m_idleThreadCount.load(std::memory_order_acquire);
    }

private:
    using Callback = std::function<void()>;
    using Work = std::variant<std::monostate,Fiber::ptr,Callback>;

    struct Task{
        Work work;
        ThreadId threadId{ANY_THREAD};

        Task() = default;

        Task(Fiber::ptr fiber, ThreadId thread_id)
            : work(std::move(fiber))
            , threadId(thread_id)
        {}

        Task(Callback callback, ThreadId thread_id)
            : work(std::move(callback))
            , threadId(thread_id)
        {}

        [[nodiscard]] Fiber::ptr* fiber() noexcept{ return std::get_if<Fiber::ptr>(&work); } 
        [[nodiscard]] Callback* callback() noexcept { return std::get_if<Callback>(&work); }

        [[nodiscard]] bool valid() const noexcept{
            if(const auto* value = std::get_if<Fiber::ptr>(&work)){
                return static_cast<bool> (*value);
            }
            if(const auto* value = std::get_if<Callback>(&work)){
                return static_cast<bool> (*value);
            }
            return false;
        }
    };

    struct FetchResult{
        Task task;
        bool tickleOthers{false};
        bool retryPending{false};
    };

    [[nodiscard]] static Task MakeTask(Fiber::ptr fiber,ThreadId thread_id){
        return Task(std::move(fiber),thread_id);
    }

    template<typename Callable>
    requires std::invocable<std::decay_t<Callable>&> && std::constructible_from<Callback,Callable>
    [[nodiscard]] static Task MakeTask(Callable&& callback,ThreadId thread_id){
        return Task(Callback(std::forward<Callable>(callback)),thread_id);
    } 

    template<typename FiberOrCallback>
    bool scheduleNoLock(FiberOrCallback&& task ,ThreadId thread_id){
        Task t = MakeTask(std::forward<FiberOrCallback>(task),thread_id);
        if(!t.valid()){
            return false;
        }
        const bool need_tickle = m_tasks.empty() || hasIdleThreads();
        m_tasks.emplace_back(std::move(t));
        return need_tickle;
    }

    [[nodiscard]] FetchResult fetchTask(ThreadId current_thread);

    [[nodiscard]] bool hasRunnableTaskNoLock(ThreadId current_thread)const;

    [[nodiscard]] bool canStopNoLock()const noexcept;

    void taskFinished() noexcept;

    void reportFiberFailure(const Fiber::ptr& fiber) const noexcept;

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    std::vector<std::thread>m_threads;
    std::list<Task> m_tasks;
    Fiber::ptr m_rootFiber;
    std::string m_name;
    bool m_useCaller{false};
    bool m_started{false};
    std::size_t m_registeredWorkers{0};

protected: 
    std::vector<ThreadId> m_threadIds;
    std::size_t m_threadCount{0};
    std::atomic<std::size_t> m_activeThreadCount{0};
    std::atomic<std::size_t>m_idleThreadCount{0};
    std::atomic<bool> m_stopping{true};
    std::atomic<bool> m_autoStop{false};
    ThreadId m_rootThreadId{ANY_THREAD};
};

class HTSERVER_API SchedulerSwitcher final{
public:
    explicit SchedulerSwitcher(Scheduler* target = nullptr);
    ~SchedulerSwitcher() noexcept;

    SchedulerSwitcher(const SchedulerSwitcher&) = delete;
    SchedulerSwitcher& operator=(const SchedulerSwitcher&) = delete;
    SchedulerSwitcher(SchedulerSwitcher&&) = delete;
    SchedulerSwitcher& operator=(SchedulerSwitcher&&) = delete;

private:
    Scheduler* m_caller{nullptr};
};

} // namespace htserver


#endif // HTSERVER_INCLUDE_HTSERVER_SCHEDULER_H
