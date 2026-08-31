#include<iostream>
#include<thread>
#include<vector>
#include<mutex>
#include<chrono>
#include<atomic>

std::atomic<int> gs_your_money=0;
int thread_count=10;    //线程数
int N=1000000;      //每个线程每次存入的钱


void add_money(){
    for(int i=0;i<N;i++){
        ++gs_your_money;
        //所以线程安全本质上就是将并行变成穿行？
    }
}

int main(){
    std::vector<std::thread>threads;
    
    for(int i=0;i<thread_count;i++){
        threads.emplace_back(add_money);
    }

    for(auto& t:threads){
        t.join();
    }
    std::cout<<"--------------------:"<<gs_your_money<<std::endl;

    return 0;
}