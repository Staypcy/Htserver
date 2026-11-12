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

} // namespace 

int main(){
    TestFunc1();
    return EXIT_SUCCESS;
}