#include<string>
#include<thread>
#include<iostream>
#include<cassert>

#if defined(__linux__)
#include<pthread.h>
#endif

#include<htserver/utils/thread_utils.h>

int main(){
    std::cout<<"1: main thread id: "<< htserver::GetThreadId()<<std::endl;
    std::cout<<"2: main thread name: "<<htserver::GetThreadName()<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));

    htserver::SetThreadName("main_thread");
    std::cout<<"3.main thread name: "<<htserver::GetThreadName()<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    htserver::SetThreadName("不经意间，梨云梦远");
    std::cout<<"4.main thread name: "<<htserver::GetThreadName()<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    htserver::SetThreadName("但远山长，云山乱，晓山侵");
    std::cout<<"5.main thread name: "<<htserver::GetThreadName()<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::thread worker([](){
        std::cout<<"6.main thread id: "<<htserver::GetThreadId()<<std::endl;
        std::cout<<"7.main thread name: "<<htserver::GetThreadName()<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));

        htserver::SetThreadName("son-thread-long-name");
        std::cout << "8. son thread name: " << htserver::GetThreadName() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));

        //htserver::SetThreadName("");
        std::cout << "9. son thread name: " << htserver::GetThreadName() << std::endl;
    });
    worker.join();

    return 0;
}
//top -H -p <pid>