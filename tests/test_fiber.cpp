#include <atomic>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <htserver/fiber.h>
#include <htserver/macros.h>

namespace {
    
void TestFiber1(){
    std::vector<int>steps;
    htserver::Fiber::ptr fiber = std::make_shared<htserver::Fiber>([&](){
        ASSERT_RETNONE2(htserver::Fiber::Current() == fiber.get(),"Current() did not return running fiber");
        ASSERT_RETNONE2(htserver::Fiber::GetThis() == fiber ,"GetThis() did not preserve shared ownership"); 
        ASSERT_RETNONE2(htserver::Fiber::GetFiberId() == fiber->getId(), "running fiber id mismatch");
    
        steps.push_back(1);
        htserver::Fiber::YieldToReady();
        steps.push_back(3);
    });

    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::INIT, "new fiber must start in INIT");
    fiber->resume();
    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::READY, "yielded fiber must be READY");
    steps.push_back(2);
    fiber->resume();
    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::TERM, "completed fiber must be TERM");
    ASSERT_RETNONE2((steps == std::vector<int>{1,2,3}), "fiber execution order mismatch");
    ASSERT_RETNONE2(htserver::Fiber::Current() == nullptr, "caller must not be reported as a fiber");
    ASSERT_RETNONE2(htserver::Fiber::GetFiberId() == 0, "caller fiber id must be zero");
}

void TestFiber2(){
    int executions = 0;
    auto fiber = std::make_shared<htserver::Fiber>([&](){
        executions++;
        htserver::Fiber::YieldToHold(htserver::Fiber::WaitReason::IO);
        executions++;
    });

    fiber->resume();
    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::WAITING, "held fiber must be WAITING");
    ASSERT_RETNONE2(fiber->getWaitReason() == htserver::Fiber::WaitReason::IO, "wait reason was not retained");
    bool rejected = false;

    try{
        fiber->resume();
    }catch(const std::logic_error& ){
        rejected =true;
    }

    ASSERT_RETNONE2(rejected,"WAITING fiber resumed without wake()");

    std::atomic<int> successful_wakes{0};
    std::vector<std::thread>wake_thread;
    for(int index = 0 ;index<8;index++){ 
        wake_thread.emplace_back([&]() {
            if (fiber->wake())
            {
                successful_wakes.fetch_add(1, std::memory_order_relaxed);
            }
        });
    } 
    for(std::thread& thread : wake_thread){
        thread.join();
    }
    ASSERT_RETNONE2(successful_wakes.load(std::memory_order_relaxed) == 1, "never!absolutely never!");
    ASSERT_RETNONE2(fiber->getWaitReason() == htserver::Fiber::WaitReason::None, "wake() must clear wait reason");
    fiber->resume();
    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::TERM, "woken fiber did not terminate");
    ASSERT_RETNONE2(executions == 2, "woken fiber did not resume at suspension point"); 
}

void TestFiber3(){
    std::shared_ptr<htserver::Fiber> fiber = std::make_shared<htserver::Fiber>(
        [](){
          throw std::runtime_error("fiber failure");
        }
    );

    const std::uint64_t id = fiber->getId();
    fiber->resume();
    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::EXCEPT, "throwing fiber must be EXCEPT");
    ASSERT_RETNONE2(fiber->getException() != nullptr, "fiber exception was not retained");
    bool rethrown = false;
    try{
        fiber->rethrowIfFailed();
    }catch(const std::runtime_error& exception){
        rethrown = std::string(exception.what()) == "fiber failure"; 
    }
    ASSERT_RETNONE2(rethrown, "original fiber exception was not rethrown");
    bool reset_flag = false;
    fiber->reset([&](){reset_flag = true;});
    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::INIT, "reset fiber must return to INIT");
    ASSERT_RETNONE2(fiber->getException() == nullptr, "reset fiber retained old exception");
    ASSERT_RETNONE2(fiber->getId() == id, "reset must preserve fiber identity");
    fiber->resume();
    ASSERT_RETNONE2(reset_flag && fiber->getState() == htserver::Fiber::State::TERM, "reset task did not complete");
}

void TestFiber4(){
    bool invalid_size_rejected = false;
    try{
        htserver::Fiber::Options invalid_options;
        invalid_options.stackSize = -1;
        auto invalid = std::make_shared<htserver::Fiber>([](){},invalid_options);
        static_cast<void>(invalid);
    }catch(const std::invalid_argument&){
        invalid_size_rejected = true;
    }

    ASSERT_RETNONE2(invalid_size_rejected, "invalid stack size was accepted");
    htserver::Fiber::Options options;
    options.stackSize = htserver::Fiber::DEFAULT_STACK_SIZE;
    options.guardPage = true;
    auto fiber = std::make_shared<htserver::Fiber>([]() {}, options);
    ASSERT_RETNONE2(fiber->getStackSize() == options.stackSize, "custom stack size was not retained");
    ASSERT_RETNONE2(!fiber->hasGuardPage(), "guard-page option was not retained");
    fiber->resume();
    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::TERM, "custom-stack fiber did not terminate");
}

void TestFiber5(){
    std::vector<int>steps;
    htserver::Fiber::ptr outer= std::make_shared<htserver::Fiber>([&](){
        steps.push_back(1);
        htserver::Fiber::ptr inner = std::make_shared<htserver::Fiber>([&](){
            steps.push_back(3);
            htserver::Fiber::YieldToReady();
            steps.push_back(5);
        });

        htserver::Fiber::YieldToReady();
        inner->resume();
        ASSERT_RETNONE2(htserver::Fiber::Current() == outer.get() ,"nested yield did not restore outer fiber");
        steps.push_back(4);
        inner->resume();
        ASSERT_RETNONE2(inner->getState() == htserver::Fiber::State::TERM, "nested fiber did not terminate");
        steps.push_back(6);
    });
}

struct DestructionProbe{
    explicit DestructionProbe(std::atomic<int>& destructions)
        : count(destructions)
    {}
    ~DestructionProbe(){
        count.fetch_add(1,std::memory_order_relaxed);
    }
    std::atomic<int>& count;
};

void TestFiber6(){
    std::atomic<int> destructions{0};
    htserver::Fiber::ptr fiber = std::make_shared<htserver::Fiber>([&](){
        DestructionProbe probe(destructions);
        htserver::Fiber::YieldToHold();
    });
    fiber->resume();
    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::WAITING,"fiber did not suspend before destruction");
    fiber.reset();
    ASSERT_RETNONE2(destructions.load(std::memory_order_relaxed) == 1, "fatal : fiber did not unwind stack objects");
}


void TestFiber7(){
    std::thread::id first_thread;
    std::thread::id second_thread;
    htserver::Fiber::ptr fiber = std::make_shared<htserver::Fiber>([&](){
        first_thread = std::this_thread::get_id();
        htserver::Fiber::YieldToReady();
        second_thread = std::this_thread::get_id();
    });
    fiber->resume();
    std::thread worker([fiber](){ fiber->resume(); });
    worker.join();
    ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::TERM, "migrated fiber did not terminate");
    ASSERT_RETNONE2(first_thread != second_thread, "fiber did not resume on the worker thread");
}

void TestFiber8(){
    for(int i =0;i<i<2000;++i){
        std::atomic<bool> entered{false};
        std::atomic<bool> completed{false};
        
        htserver::Fiber::ptr fiber =std::make_shared<htserver::Fiber>([&](){
            entered.store(true,std::memory_order_release);
            htserver::Fiber::YieldToReady();
            completed.store(true,std::memory_order_release);
        });
        std::thread first_resume([fiber](){fiber->resume();});
        while(!entered.load(std::memory_order_acquire)){
            std::this_thread::yield();
        }      
        while(fiber->getState() != htserver::Fiber::State::READY){
            std::this_thread::yield();
        }
        fiber->resume();
        first_resume.join();
        ASSERT_RETNONE2(completed.load(std::memory_order_acquire), "concurrent context handoff lost execution");
        ASSERT_RETNONE2(fiber->getState() == htserver::Fiber::State::TERM, "concurrent context handoff did not terminate");
    }
}
}

int main(){
    const std::uint64_t baseline = htserver::Fiber::TotalFibers();

    TestFiber1();
    TestFiber2();
    TestFiber3();
    TestFiber4();
    TestFiber5();
    TestFiber6();
    TestFiber7();
    TestFiber8();

    ASSERT_RETVAL2(htserver::Fiber::TotalFibers() == baseline, EXIT_FAILURE, "fiber lifetime counter leaked");
    return EXIT_SUCCESS;
}


