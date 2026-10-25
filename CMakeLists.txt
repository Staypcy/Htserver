cmake_minimum_required(VERSION 3.25)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set("/usr/bin/gcc" CACHE FILEPATH "C complier" FORCE)
    set("/usr/bin/g++" CACHE FILEPATH "C++ complier" FORCE)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(
    htserver 
    VERSION 0.1.0
    LANGUAGES CXX C        
)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)


option(HTSERVER_BUILD_EXAMPLES "build examples" ON)
option(HTSERVER_BUILD_TESTS "build tests" ON)
option(HTSERVER_BUILD_TOOLS "build tools" ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")#输出日志

cmake_host_system_information(RESULT CPU_NUMS QUERY NUMBER_OF_LOGICAL_CORES)
message(STATUS "CPU NUMS on current server is: ${CPU_NUMS}")
math(EXPR JOBS "${CPU_NUMS} + ${CPU_NUMS} / 2")
set(CMAKE_BUILD_PARALLEL_LEVEL ${JOBS} CACHE STRING "并行编译线程数")
message(STATUS "Complier default work jobs: ${CMAKE_BUILD_PARALLEL_LEVEL}")


include(cmake/htOptions.cmake)
include(cmake/htserverDependencies.cmake)
include(cmake/functions.cmake)

add_subdirectory(cmake/boost-context)
add_subdirectory(htserver)
if(HTSERVER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if(HTSERVER_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

set(HTSERVER_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/htserver")

configure_package_config_file(
    cmake/htserverConfig.cmake.in 
    "${CMAKE_CURRENT_BINARY_DIR}/htserverConfig.cmake"
    INSTALL_DESTINATION "${HTSERVER_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/pansConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(
    FILES 
        "${CMAKE_CURRENT_BINARY_DIR}/htserverConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/htserverConfigVersion.cmake"
    DESTINATION "${HTSERVER_INSTALL_CMAKEDIR}"
)

install(
    EXPORT htserverTargets
    FILE htserverTargets.cmake 
    NAMESPACE htserver::
    DESTINATION "${HTSERVER_INSTALL_CMAKEDIR}"
)
