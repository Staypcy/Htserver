add_library(htserver_options INTERFACE)

target_compile_options(htserver_options INTERFACE
    #GCC/Clang通用选项: -Wall基础警告 -Wextra额外警告 -Wpedantic严格遵循标准 -fno-strict-aliasing禁止严格别名优化
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
    -Wall 
    -Wextra 
    -Wpedantic 
    -fno-strict-aliasing
    -Wno-builtin-macro-redefined
    >
)

target_compile_options(htserver_options INTERFACE
    #-fPIC: 编译位置与代码无关(仅Linux)
    $<$<AND:$<PLATFORM_ID:Linux>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-fPIC>
)

target_link_options(htserver_options INTERFACE
    #-rdynamic: 栈回溯需要,动态符号表,快速查找错误(仅Linux)
    $<$<AND:$<PLATFORM_ID:Linux>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-rdynamic>
)

target_compile_definitions(htserver_options INTERFACE
    $<$<CONFIG:Debug>:HTSERVER_DEBUG>   #添加预编译宏HTSERVER_DEBUG
)

target_compile_options(htserver_options INTERFACE
    $<$<CONFIG:Debug>:-O0 -g3 -ggdb>
)

# ===== Release/RelWithDebInfo 共用选项 =====
target_compile_options(htserver_options INTERFACE
    $<$<CONFIG:Release>:-DNDEBUG -O2 -fno-omit-frame-pointer>
    $<$<CONFIG:RelWithDebInfo>:-DNDEBUG -O2 -g -fno-omit-frame-pointer>
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
