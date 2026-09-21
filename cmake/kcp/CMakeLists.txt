cmake_minimum_required(VERSION 3.10)

project(htserver-kcp LANGUAGES C)

include(GNUInstallDirs)

if(NOT HTSERVER_KCP_SOURCE_DIR)
    message(FATAL_ERROR "HTSERVER_KCP_SOURCE_DIR is required")
endif()

if(NOT EXISTS "${HTSERVER_KCP_SOURCE_DIR}/ikcp.c" OR
    NOT EXISTS "${HTSERVER_KCP_SOURCE_DIR}/ikcp.h"
)
    message(FATAL_ERROR "Invalid kcp source directory: ${HTSERVER_KCP_SOURCE_DIR}" )
endif()

add_library(kcp STATIC "${HTSERVER_KCP_SOURCE_DIR}/ikcp.c")
add_library(kcp::kcp ALIAS kcp)

target_include_directories(kcp PUBLIC 
    $<BUILD_INTERFACE:${HTSERVER_KCP_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(FILES "${HTSERVER_KCP_SOURCE_DIR}/ikcp.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS kcp EXPORT kcp-targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(EXPORT kcp-targets 
    FILE kcp-config.cmake
    NAMESPACE kcp::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/kcp"
)
