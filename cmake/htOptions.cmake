add_library(htserver_options INTERFACE)

target_compile_options(htserver_options INTERFACE
    #GCC/Clang通用选项
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>::
        -Wall
        -Wextra         #额外警告
        -Wpedantic      #严格遵循标准
        -fno-strict-aliasing   #禁止严格别名优化 
    >
)

target_compile_options(htserver_options INTERFACE
    $<$<AND:$<PLATFORM_ID:linux>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:
        -fPIC       #编译位置与代码无关
    >
)

target_link_options(htserver_options INTERFACE
    $<$<AND:$<PLATFORM_ID:Linux>,<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:
        -rdynamic   #栈回溯需要，动态符号表，快速查找错误
    >
)

target_compile_definitions(htserver_options INTERFACE 
    $<$<CONFIG:Debug>:HTSERVER_DEBUG>   #添加预编译宏HTSERVER_DEBUG
)

target_compile_options(htserver_options INTERFACE
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-ggdb>
)

# ===== Release/RelWithDebInfo 共用选项 =====
target_compile_options(htserver_options INTERFACE
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-fno-omit-frame-pointer>

    $<$<CONFIG:RelWithDebInfo>:-DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O2>
    $<$<CONFIG:RelWithDebInfo>:-g>
    $<$<CONFIG:RelWithDebInfo>:-fno-omit-frame-pointer>
)

option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(htserver_options INTERFACE
        $<$<CONFIG:Debug>:--coverage>
    )
    target_link_options(htserver_options INTERFACE
        $<$<CONFIG:Debug>:--coverage>
    )
endif()