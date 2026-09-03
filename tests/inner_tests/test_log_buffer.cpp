#include<array>
#include<atomic>
#include<chrono>
#include<cstddef>   //std::szie_t
#include<cstdint>
#include<iomanip>   //setw
#include<iostream>
#include<sstream>   //std::stringstream
#include<string>
#include<string_view>

#include "logger/buffer.h"
#include "logger/buffer_config.h"

#include<htserver/macros.h>

constexpr int RUNS=7;
std::uint64_t g_target=0;
std::atomic<std::uint64_t> BENCHMARK_SINK{0};   //编译编译器优化

std::uint64_t Observe(std::string_view value)noexcept{
    //强制CPU去动buffer，否则会被编译器优化删除
    return static_cast<std::uint64_t>(value.size())+
            static_cast<unsigned char>(value.front())+
            static_cast<unsigned char>(value[value.size()/2])+
            static_cast<unsigned char>(value.back());
}

std::uint64_t WriteWithSmallStreamBuffer(std::string_view message){
    //这里不采用全局单例，是因为全局单例下需要clear，有好事开销，而且全局单例下，一直单例所存的缓存行
    //也就是堆CPU来说就是热行，这会导致写入速度偏快，不是真实反馈

    //再就是栈分配更加快
    htserver::detail::InlineBuffer<htserver::detail::LOG_MESSAGE_INLINE_CAPACITY> buffer;
    htserver::detail::SmallStreamBuffer<htserver::detail::LOG_MESSAGE_INLINE_CAPACITY> stream_buffer(buffer);
    std::ostream stream(&stream_buffer);
    stream << message;
    return Observe(buffer.view());
}

std::uint64_t WriteWithStringStream(std::string_view message){
    std::stringstream stream;
    stream << message;
    return Observe(stream.view());
}

template<typename Operation>
std::chrono::nanoseconds BenchmarkWrites(Operation&& operation){
    std::uint64_t checksum=0;
    const auto begin=std::chrono::steady_clock::now();
    for(std::uint64_t iter=0;iter<g_target;iter++){
        checksum+=operation();
    }
    const auto end=std::chrono::steady_clock::now();
    BENCHMARK_SINK.fetch_xor(checksum,std::memory_order_relaxed);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin);
}

template<typename Benchmark>
double Run(Benchmark&& benchmark){
    long double total_nanoseconds=0.0L;
    for(int run=0;run<RUNS;run++){
        total_nanoseconds+=static_cast<long double>(benchmark().count());
    }
    return static_cast<double>(total_nanoseconds)/static_cast<long double>(RUNS);
}


void PrintResult(std::string_view name,double total_nanoseconds){
    std::cout<< std::left <<std::setw(30)<<name<<std::right<<std::right<<
            total_nanoseconds<< " ns \ttotal,"<<
            total_nanoseconds/static_cast<double>(g_target)<<" ns/op\n";

}

int main(int grac,char** grav){
    if(grac<2){
        std::cerr<<"Usage: ./test_log_buffer count\n";
        return 1;
    }
    g_target=std::stoull(grav[1]);
    std::cout<<std::fixed<<std::setprecision(2);
    std::cout<<"operations:"<<g_target<<", average of "<<RUNS<<"runs.\n";
    constexpr std::array<std::size_t,6>MESSAGE_SIZE={50,100,200,500,1000,2000};
    for(const std::size_t message_size:MESSAGE_SIZE){
        std::string message(message_size,'x');
        const double small_stream_buffer_nanoseconds=Run([&](){
            return BenchmarkWrites([&](){return WriteWithSmallStreamBuffer(message);});
        });
        const double stringstream_nanoseconds=Run([&](){
            return BenchmarkWrites([&](){return WriteWithStringStream(message);});
        });
        std::cout<<"message:"<<message_size<<" character"<<std::endl;
        PrintResult("SmallStreamBuffer", small_stream_buffer_nanoseconds);
        PrintResult("stringstream",stringstream_nanoseconds);

        std::cout<<"speedup: "<<stringstream_nanoseconds/small_stream_buffer_nanoseconds<<"x\n\n";
    }
    return 0;
}