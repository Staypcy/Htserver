#ifndef HTSERVER_INCLUDE_HTSERVER_EXPORT_H
#define HTSERVER_INCLUDE_HTSERVER_EXPORT_H


#if defined(_win32) && defined(HTSERVER_SHARED_LIBRARY)
#if defined(HTSERVER_BUILDING_LIBRARY)
#define HTSERVER_API __declspec(dllexport)
#else
#define HTSERVER_API __declspec(dllimport)
#endif


#elif defined(__GNUC__) && defined(HTSERVER_SHARED_LIBRARY)
//编译动态库
#define HTSERVER_API __attribute__((visibility("default")))
#else
#define HTSERVER_API
//编译静态库
#endif

#endif // HTSERVER_INCLUDE_HTSERVER_EXPORT_H