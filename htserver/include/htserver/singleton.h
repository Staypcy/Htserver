#ifndef HTSERVER_INCLUDE_HTSERVER_SINGLETON_H
#define HTSERVER_INCLUDE_HTSERVER_SINGLETON_H

namespace htserver
{

template<typename T>
class Singleton{
public:
    static T& Istance(){
        static T instance;
        return instance;
    }

    Singleton(const Singleton&)=delete;
    Singleton(Singleton&&)=delete;
    Singleton& operator=(const Singleton&)=delete;
    Singleton& operator=(Singleton&&)=delete;
protected:
    Singleton()=default;
    ~Singleton()=default;
};
} // namespace htserver

#endif