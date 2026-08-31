#file(GLOB_RECURSE HTSERVER_SOURCES CONFIGURE_DEPENDS *.cpp *.cc *.c)
#file(GLOB_RECURSE HTSERVER_HEADERS CONFIGURE_DEPENDS *.h *.hpp)
file(GLOB_RECURSE HTSERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)
file(GLOB_RECURSE HTSERVER_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

#后续的头文件等使用set手动管理
#set(HTSERVER_SOURCES )
#set(HTSERVER_HEADERS )

add_library(htserver STATIC ${HTSERVER_SOURCES} ${HTSERVER_HEADERS})


target_include_directories(htserver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(htserver PUBLIC htserver_options)#继承所有的编译选项

target_compile_options(htserver PRIVATE
    #主库需要严格的编译警告
    #将警告视为错误（主库严格要求）
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Werror>
    # -Wno-deprecated-declarations
)


install(TARGETS htserver
    EXPORT htserverTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${HTSERVER_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ht
)