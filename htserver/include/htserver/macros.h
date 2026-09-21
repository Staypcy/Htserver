#ifndef HTSERVER_INCLUDE_HTSERVER_MACROS_H
#define HTSERVER_INCLUDE_HTSERVER_MACROS_H

#include<cassert>

#include<htserver/logger/log.h>
#include<htserver/utils/system_utils.h>

#define HTSERVER_ASSERT(x) \
    if(!(x))[[unlikely]]\
    { \
        HTSERVER_LOG(HTSERVER_LOG_ROOT())<<__FILE__<<":"<<__LINE__<<"ASSERT FAILED: "<<#x<<"\nStacktrace:\n"<<htserver::GetBacktrace(); \
        assert(x); \
    }else{}

#define HTSERVER_ASSERT2(x,w) \
    if(!(x))[[unlikely]] \
    { \
        HTSERVER_LOG(HTSERVER_LOG_ROOT())<<__FILE__<<__LINE__<<"Assert "<<#x<<" failed. ["<<w<<"].\nStacktrace :\n"<<htserver::GetBacktrace(); \
        assert(x); \
    }else{}

#define ASSERT_RETVAL(x,val) \
    do{\
        if(x)[[unlikely]] break;\
        HTSERVER_ASSERT(x); \
        return val;\
    }while(0);

#define ASSERT_RETVAL2(x,val,info) \
    do{\
        if(x)[[unlikely]] break;\
        HTSERVER_ASSERT2(x,info); \
        return val;\
    }while(0);

#define ASSERT_RETNONE(x) \
    do{\
        if(x)[[unlikely]] break;\
        HTSERVER_ASSERT(x); \
        return ;\
    }while(0);

#define ASSERT_RETNONE2(x,info) \
    do{\
        if(x)[[unlikely]] break;\
        HTSERVER_ASSERT2(x,info); \
        return ;\
    }while(0);

#define ASSERT_NOEFFECT(x)\
    do{\
        if(x)[[unlikely]] break;\
        HTSERVER_ASSERT(x);\
    }while(0)

#define ASSERT_NOEFFECT2(x,info)\
    do{\
        if(x)[[unlikely]] break;\
        HTSERVER_ASSERT2(x,info);\
    }while(0)

#define ASSERT_CONTINUE(x)\
    if(!(x))[[unlikely]]{\
        HTSERVER_ASSERT(x);\
        continue;\
    }else{}

#define ASSERT_CONTINUE2(x,info)\
    if(!(x))[[unlikely]]{\
        HTSERVER_ASSERT2(x,info);\
        continue;\
    }else{}
 
#define ASSERT_BREAK(x)\
    if(!(x))[[unlikely]]{\
        HTSERVER_ASSERT(x);\
        break;\
    }else{}

#define ASSERT_BREAK2(x,info)\
    if(!(x)){\
        HTSERVER_ASSERT2(x,info);\
        break;\
    }else{}

#endif