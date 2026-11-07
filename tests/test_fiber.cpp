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


}
