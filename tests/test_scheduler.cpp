#include<atomic>
#include<chrono>
#include<cstdlib>
#include<future>
#include<sstream>
#include<stdexcept>
#include<string>
#include<thread>
#include<vector>

#include<htserver/scheduler.h>
#include<htserver/utils/thread_utils.h>

namespace {
using namespace std::chrono_literals;

std::atomic<int> g_functionCounter{0};

void FunctionTask(){
    g_functionCounter.fetch_add(1,std::memory_order_relaxed);
}

void Check(bool condition,const char* message){
    if(!condition){
        throw std::runtime_error(message);
    }
}

template<typename Future>
void WaitReady(Future& future,const char* message){
    if(future.wait_for(5s) != std::future_status::ready){
        throw std::runtime_error(message);
    }
}

void TestFunc1(){
    std::atomic<int>callback_count{0};
    std::atomic<int> fiber_steps{0};
    std::promise<void> fiber_done;
    std::future<void> fiber_done_future = fiber_done.get_future();

    htserver::Scheduler scheduler(3,false,"main");
    scheduler.start();
    scheduler.addTask(&FunctionTask);

    for(int index= 0;index<64;index++){
        scheduler.addTask([&callback_count](){
            Check(htserver::Scheduler::GetThis() != nullptr ,"callback lost its scheduler context");
            callback_count.fetch_add(1,std::memory_order_relaxed);
        });
    }

    scheduler.addTask([&](){
        for(int step = 0;step<3;step++){
            fiber_steps.fetch_add(1,std::memory_order_relaxed);
            if(step!=2){
                htserver::Fiber::YieldToReady();
            }
        }
        fiber_done.set_value();
    });
    WaitReady(fiber_done_future,"READY fiber was not rescheduled");
    scheduler.stop();
    Check(callback_count.load(std::memory_order_relaxed) == 64, "scheduler lost callbacks");
    Check(fiber_steps.load(std::memory_order_relaxed) == 3, "READY fiber ran the wrong number of times");
    Check(g_functionCounter.load(std::memory_order_relaxed) == 1, "legacy function-pointer task did not execute");
}

void TestFunc2(){
    std::promise<htserver::Fiber::ptr>suspended;
    auto suspended_future = suspended.get_future();
    std::promise<void> resumed;
    auto resumed_future = resumed.get_future();
    
    htserver::Scheduler scheduler(2,false,"main");
    scheduler.start();
    scheduler.addTask([&](){
        suspended.set_value(htserver::Fiber::GetThis());
        htserver::Fiber::YieldToHold(htserver::Fiber::WaitReason::External);
        resumed.set_value();
    });

    WaitReady(suspended_future, "fiber did not publish its waiting handle");
    htserver::Fiber::ptr fiber = suspended_future.get();
    const auto waiting_deadline = std::chrono::steady_clock::now() + 5s;
    while(fiber->getState() != htserver::Fiber::State::WAITING && std::chrono::steady_clock::now() < waiting_deadline){
        std::this_thread::sleep_for(100us);     
    }
    Check(fiber->getState() == htserver::Fiber::State::WAITING, "fiber did not enter WAITING");
    scheduler.addTask(fiber);
    WaitReady(resumed_future, "explicitly scheduled WAITING fiber did not resume");
    scheduler.stop();
    Check(fiber->getState() == htserver::Fiber::State::TERM, "resumed fiber did not terminate");
}

void TestFunc3(){
    htserver::Scheduler scheduler(2,false,"main");
    scheduler.start();
    std::promise<htserver::Scheduler::ThreadId> selected_thread;
    auto selected_thread_future = selected_thread.get_future();
    scheduler.addTask([&selected_thread](){
        selected_thread.set_value(htserver::GetThreadId());
    });
    WaitReady(selected_thread_future, "could not discover a scheduler worker thread");

    const htserver::Scheduler::ThreadId thread_id = selected_thread_future.get();
    constexpr int TASK_COUNT = 32;
    std::atomic<int>completed{0};
    std::promise<void>all_done;
    auto all_done_future = all_done.get_future();

    for(int index = 0;index<TASK_COUNT;index++){
        scheduler.addTask([&](){
            Check(htserver::GetThreadId() == thread_id, "thread-affine task ran on the wrong worker");
            if(completed.fetch_add(1,std::memory_order_acq_rel) +1 == TASK_COUNT){
                all_done.set_value();
            }
        },thread_id);
    }
    WaitReady(all_done_future, "thread-affine tasks timed out");
    scheduler.stop();
    std::promise<void> restarted;
    auto restarted_future = restarted.get_future();
    scheduler.start();
    scheduler.addTask([&restarted](){ restarted.set_value();});
    WaitReady(restarted_future, "restarted scheduler did not execute tasks");
    scheduler.stop();
}

void TestFunc4(){
    htserver::Scheduler first(1,false,"first");
    htserver::Scheduler second(1,false,"second");

    std::promise<void> finished;
    auto finished_future = finished.get_future();
    std::atomic<uint64_t> first_thread{0};
    std::atomic<uint64_t> second_thread{0};
    std::atomic<uint64_t> returned_thread{0};

    first.start();
    second.start();
    first.addTask([&](){
        first_thread.store(htserver::GetThreadId(),std::memory_order_relaxed);
        {
            htserver::SchedulerSwitcher switcher(&second);
            Check(htserver::Scheduler::GetThis() == &second, "SchedulerSwitcher did not enter target scheduler");
            second_thread.store(htserver::GetThreadId(),std::memory_order_relaxed);
        }
        Check(htserver::Scheduler::GetThis() == &first, "SchedulerSwitcher did not return to source scheduler");
        returned_thread.store(htserver::GetThreadId(),std::memory_order_relaxed);
        finished.set_value();
    });
    WaitReady(finished_future, "SchedulerSwitcher migration timed out");
    first.stop();
    second.stop();
    Check(first_thread.load(std::memory_order_relaxed) != second_thread.load(std::memory_order_relaxed), "fiber did not migrate to the second scheduler");
    Check(first_thread.load(std::memory_order_relaxed) == returned_thread.load(std::memory_order_relaxed), "fiber did not return to the first scheduler thread");
}

void TestFunc5(){
    htserver::Scheduler scheduler(1,true,"caller");
    std::atomic<int> executions{0};
    std::vector<std::function<void()>>callbacks;
    callbacks.emplace_back([&]() { executions.fetch_add(1, std::memory_order_relaxed); });
    callbacks.emplace_back([]() { throw std::runtime_error("expected scheduler test failure"); });
    callbacks.emplace_back([&]() { executions.fetch_add(1, std::memory_order_relaxed); });
    Check(htserver::Scheduler::GetThis() == &scheduler, "caller scheduler was not installed on its construction thread");
    Check(htserver::Scheduler::GetMainFiber() != nullptr, "caller scheduler did not expose its root fiber");
    scheduler.start();
    scheduler.addTask(callbacks.begin(),callbacks.end());
    scheduler.stop();
    Check(executions.load(std::memory_order_relaxed) == 2, "exception stopped caller scheduler task draining");
}

} // namespace 

int main(){
    TestFunc1();
    TestFunc2();
    TestFunc3();
    TestFunc4();
    TestFunc5();
    return EXIT_SUCCESS;
}