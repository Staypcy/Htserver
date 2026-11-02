#include<atomic>
#include<mutex>
#include<stdexcept>
#include<utility>

#include<boost/context/detail/exception.hpp>
#include<boost/context/fiber.hpp>
#include<boost/context/fixedsize_stack.hpp>
#include<boost/context/protected_fixedsize_stack.hpp>
#include<boost/context/stack_traits.hpp>

#include<htserver/fiber.h>

namespace htserver{

namespace {

std::atomic<uint64_t>g_nextFiberId{0};
std::atomic<uint64_t>g_fiberCount{0};

thread_local Fiber* g_currentFiber = nullptr;

// 利用RAII机制来切换运行时协程，便于维护当前协程和上一个协程的切换关系
class CurrentFiberGuard final{ 
public:
    explicit CurrentFiberGuard(Fiber* current)noexcept
        :m_previous(g_currentFiber)
    {
        g_currentFiber = current;
    }

    ~CurrentFiberGuard()noexcept{
        g_currentFiber = m_previous;
    }

    CurrentFiberGuard(const CurrentFiberGuard&) = delete;
    CurrentFiberGuard& operator=(const CurrentFiberGuard&) =delete;

private:
    Fiber* m_previous;
};

void ValidateStackSize(std::size_t stack_size){
    const std::size_t minimum = boost::context::stack_traits::minimum_size();
    if(stack_size<minimum){
        throw std::invalid_argument("fiber stack size is smaller than Boost.Context minimum");
    }
    if(!boost::context::stack_traits::is_unbounded() && stack_size > boost::context::stack_traits::maximum_size())
    {
        throw std::invalid_argument("fiber stack size exceeds Boost.Context maximum");
    }
}
} // namespace 

struct Fiber::Impl{
    Impl(uint64_t fiber_id, Options fiber_options)
        :id(fiber_id)
        ,options(fiber_options)
    {}

    uint64_t id;
    Options options;
    Callback callback;
    std::atomic<State>state{State::INIT};
    std::atomic<State>suspendState{State::INIT};
    std::atomic<WaitReason>waitReason{WaitReason::None};
    std::atomic<bool> parked{true};
    mutable std::mutex exceptionMutex;
    std::exception_ptr exception;
    boost::context::fiber caller;
    boost::context::fiber context;
};

Fiber::Fiber(Callback callback)
    : Fiber(std::move(callback),Options{})
{    
}

Fiber::Fiber(Callback callback,Options options)
    :m_impl(std::make_unique<Impl>(g_nextFiberId.fetch_add(1,std::memory_order_relaxed) + 1 , options))
{
    ValidateStackSize(options.stackSize);
    initializeContext(std::move(callback));
    g_fiberCount.fetch_add(1,std::memory_order_relaxed);
}

Fiber::~Fiber()noexcept{
    if(g_currentFiber == this){
        std::terminate();
    }
    g_fiberCount.fetch_add(1,std::memory_order_relaxed);
}

void Fiber::initializeContext(Callback callback){
    if(!callback){
        throw std::invalid_argument("fiber callback must not be empty");
    }

    auto entry = [this](boost::context::fiber&& caller) -> boost::context::fiber{
        m_impl->caller = std::move(caller);
        run();
        return std::move(m_impl->caller);  
    };

    boost::context::fiber new_context;
    if(m_impl->options.guardPage){
        new_context = boost::context::fiber(std::allocator_arg,boost::context::protected_fixedsize_stack(m_impl->options.stackSize),std::move(entry));
    }
    else{
        new_context = boost::context::fiber(std::allocator_arg,boost::context::fixedsize_stack(m_impl->options.stackSize),std::move(entry));
    }

    {
        std::lock_guard lock(m_impl->exceptionMutex);
        m_impl->exception = nullptr;
    }

    m_impl->callback = std::move(callback);
    m_impl->caller = {};
    m_impl->context = std::move(new_context);
    m_impl->waitReason.store(WaitReason::None,std::memory_order_relaxed);
    m_impl->parked.store(true,std::memory_order_relaxed);
    m_impl->suspendState.store(State::INIT,std::memory_order_relaxed);
    m_impl->state.store(State::INIT,std::memory_order_release);
}

Fiber::State Fiber::resume()
{
    State state = m_impl->state.load(std::memory_order_acquire);
    if(state == State::INIT){
        if(!m_impl->state.compare_exchange_strong(state,State::READY,std::memory_order_acq_rel,std::memory_order_acquire))
        {
            throw std::logic_error("Fiber state changed concurrently while becoming ready");
        }
        state = State::READY;
    }

    if(state != State::READY){
        throw std::logic_error("only INIT or READY fiber can be resumed");
    }

    if(!m_impl->state.compare_exchange_strong(state,State::EXEC,std::memory_order_acq_rel,std::memory_order_acquire)){
        throw std::logic_error("fiber was resumed concurrently");
    }

    while(!m_impl->parked.load(std::memory_order_acquire)){
        m_impl->parked.wait(false,std::memory_order_acquire);
    }

    if(!m_impl->context){
        m_impl->suspendState.store(State::EXCEPT,std::memory_order_relaxed);
        m_impl->state.store(State::EXCEPT,std::memory_order_release);
        throw std::logic_error("fiber has no resumable Boost.Context State.");
    }

    m_impl->parked.store(false,std::memory_order_relaxed);
    try{
        CurrentFiberGuard current_guard(this);
        m_impl->context = std::move(m_impl->context).resume();
    }catch(...){
        if(m_impl->state.load(std::memory_order_acquire) == State::EXEC){
            {
                std::lock_guard lock(m_impl->exceptionMutex);
                m_impl->exception = std::current_exception();
            }
            m_impl->suspendState.store(State::EXCEPT,std::memory_order_relaxed);
            m_impl->state.store(State::EXCEPT,std::memory_order_release);
        }
        m_impl->parked.store(true,std::memory_order_release);
        m_impl->parked.notify_all();
        throw;
    }
    const State suspend_state = m_impl->suspendState.load(std::memory_order_acquire);
    m_impl->parked.store(true,std::memory_order_release);
    return suspend_state;
}

void Fiber::yield(State target, WaitReason reason)
{
    m_impl->waitReason.store(reason,std::memory_order_relaxed);
    State expect = State::EXEC;
    if(!m_impl->state.compare_exchange_strong(expect,target,std::memory_order_acq_rel,std::memory_order_acquire)){
        m_impl->waitReason.store(WaitReason::None,std::memory_order_relaxed);
        throw std::logic_error("only running fiber can yield");
    }
    m_impl->suspendState.store(target,std::memory_order_release);
    if(!m_impl->caller){
        m_impl->state.store(State::EXCEPT,std::memory_order_release);
        throw std::logic_error("fiber has no caller context to yiled to");
    }
    m_impl->caller = std::move(m_impl->caller).resume();
}

void Fiber::reset(Callback callback){
    const State state = m_impl->state.load(std::memory_order_acquire);
    if(state != State::TERM && state != State::EXCEPT){
        throw std::logic_error("fiber can only be reset after TERM or EXCEPT");
    }
    if(g_currentFiber == this){
        throw std::logic_error("running fiber cannot reset itself");
    }
    initializeContext(std::move(callback));
}

bool Fiber::wake() noexcept{
    if(m_impl->state.load(std::memory_order_acquire) != State::WAITING){
        return false;
    }

    m_impl->waitReason.store(WaitReason::None,std::memory_order_relaxed);
    State except = State::WAITING;
    if(!m_impl->state.compare_exchange_strong(except,State::READY,std::memory_order_release,std::memory_order_relaxed))
    {
        return false;
    }
    return true;
}

uint64_t Fiber::getId()const noexcept{
    return m_impl->id;
}

Fiber::State Fiber::getState()const noexcept{
    return m_impl->state.load(std::memory_order_acquire);
}

Fiber::WaitReason Fiber::getWaitReason()const noexcept{
    return m_impl->waitReason.load(std::memory_order_acquire);
}

std::size_t Fiber::getStackSize()const noexcept{
    return m_impl->options.stackSize;
}

bool Fiber::hasGuardPage() const noexcept {
    return m_impl->options.guardPage;
}
 
std::exception_ptr Fiber::getException() const noexcept{
    std::lock_guard lock(m_impl->exceptionMutex);
    return m_impl->exception;
}

void Fiber::rethrowIfFailed()const{
    if(std::exception_ptr exception = getException()){
        std::rethrow_exception(exception);
    }
}

Fiber* Fiber::Current()noexcept{
    return g_currentFiber;
}

Fiber::ptr Fiber::GetThis(){
    Fiber* current = Current();
    if(current == nullptr)
    {
        return nullptr;
    }
    ptr owner = current->weak_from_this().lock();
    if(!owner)
    {
        throw std::logic_error("current fiber is not owned by std::shared_ptr");
    }
    return owner;
}

uint64_t Fiber::GetFiberId()noexcept{
    return g_currentFiber == nullptr ? 0 : g_currentFiber->getId();
}

uint64_t Fiber::TotalFibers()noexcept{
    return g_fiberCount.load(std::memory_order_relaxed);
}

void Fiber::YieldToReady(){
    if(g_currentFiber == nullptr){
        throw std::logic_error("YieldToReady must be called inside a fiber");
    }
    g_currentFiber->yield(State::READY,WaitReason::None);
}

void Fiber::YieldToHold(WaitReason reason){
    if(g_currentFiber == nullptr){
        throw std::logic_error("YieldToHold must be called inside a fiber");
    }
    if(reason == WaitReason::None){
        throw std::logic_error("waiting fiber requires a wait reason");
    }
    g_currentFiber->yield(State::WAITING,reason);
}

void Fiber::run(){
    try{
        m_impl->callback();
        m_impl->callback = nullptr;
        m_impl->waitReason.store(WaitReason::None,std::memory_order_relaxed);
        m_impl->suspendState.store(State::TERM,std::memory_order_relaxed);
        m_impl->state.store(State::TERM,std::memory_order_release);
    }catch(const boost::context::detail::forced_unwind&){
        throw;
    }catch(...){
        {
            std::lock_guard lock(m_impl->exceptionMutex);
            m_impl->exception = std::current_exception();
        }
        m_impl->callback = nullptr;
        m_impl->waitReason.store(WaitReason::None,std::memory_order_relaxed);
        m_impl->suspendState.store(State::EXCEPT,std::memory_order_relaxed);
        m_impl->state.store(State::EXCEPT,std::memory_order_release);
    }
}

std::string_view Fiber::StateName(State state) noexcept{
    switch (state)
    {
#define XX(name) case State::name : return #name;
        XX(INIT)
        XX(READY)
        XX(EXEC)
        XX(WAITING)
        XX(TERM)
        XX(EXCEPT)
#undef XX
    }
    return "UNKNOWN";
}

std::string_view Fiber::WaitReasonName(WaitReason reason)noexcept{
    switch (reason)
    {
#define XX(name, str) case WaitReason::name : return str;
        XX(None,      "NONE")
        XX(IO,        "IO")
        XX(Timer,     "TIMER")
        XX(Semaphore, "SEMAPHORE")
        XX(External,  "EXTERNAL")
#undef XX
    }
    return "UNKNOWN";
}

} // namespace htserver
