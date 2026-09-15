set(HTSERVER_THIRD_PARTY_DIR "${PROJECT_SOURCE_DIR}/third_party" CACHE PATH "third-party source")
set(HTSERVER_THIRD_PARTY_OUTPUT_DIR "${HTSERVER_THIRD_PARTY_DIR}/output" CACHE PATH "compiled third-party output")
set(HTSERVER_THIRD_PARTY_BUILD_TYPE Release CACHE STRING "third-party build-type")
set_property(CACHE HTSERVER_THIRD_PARTY_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)

option(HTSERVER_UPDATE_SUBMODULES "Initialize missing third-party git submodule" ON)
option(HTSERVER_REUSE_INSTALLED_DEPENDENCIES "Reuse complatible dependencies from thirdparty/output" ON)

# 子模块哨兵文件
set(_htserver_required_submodules
    "third_party/yaml-cpp|CMakeLists.txt"
    "third_party/abseil-cpp|CMakeLists.txt"
    "third_party/protobuf|CMakeLists.txt"
    "third_party/rapidjson|include/rapidjson/document.h"
    "third_party/tinyxml2|CMakeLists.txt"
    "third_party/kcp|ikcp.c"
    "third_party/zlib|CMakeLists.txt"
)

find_package(Git QUIET)

function(htserver_init_third_party_submodules)
    set(missing_submodules)
    foreach(entry IN LISTS _htserver_required_submodules)
        string(REPLACE "|" ";" parts "${entry}")
        list(GET parts 0 submodules_path)
        list(GET parts 1 sentinel)
        if(NOT EXISTS "${PROJECT_SOURCE_DIR}/${submodule_path}/${sentinel}")
        list(APPEND missing_submodules "${submodule_path}")
        endif()
    endforeach()
    
    if(NOT missing_submodules)
        return()
    endif()

    if(NOT HTSERVER_UPDATE_SUBMODULES)
        message(FATAL_ERROR 
            "Missing third-party source trees: ${missing_submodule}\n"
            "Enable HTSERVER_UPDATE_SUBMODULES or run : \n"
            "git submodule update --init --resoursive")
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
        message(FATAL_ERROR 
            "Missing third-party source trees: ${missing_submodules}\n"
            "The source tree is not a git checkout, so CMake cannot initialize them.")
    endif()

    if(NOT Git_FOUND)
        message(FATAL_ERROR "Git is required to initialize missing submodules.")
    endif()
    
    message(STATUS "Initializing missing git submodules: ${missing_submodules}")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" submodule update --init --recursive -- ${missing_submodules}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        RESULT_VARIABLE submodule_result
        OUTPUT_VARIABLE submodule_output
        ERROR_VARIABLE submodule_error
    )

    if(NOT submodule_result EQUAL 0)
        message(FATAL_ERROR 
            "git submodule update failed (exit=${submodel_result}):\n"
            "${submodule_output}${submodule_error}")
    endif()
    
    foreach(entry IN LISTS _htserver_required_submodules)
        string(REPLACE "|" ";" parts "${entry}")
        if(NOT Git_FOUND)
        message(FATAL_ERROR "Git is required to initialize missing submodules.")
    endif()
    
    foreach(entry IN LISTS _pans_required_submodules)
        string(REPLACE "|" ";" parts "${entry}")
        list(GET parts 0 submodule_path)
        list(GET parts 1 sentinel)
        if(NOT EXISTS "${PROJECT_SOURCE_DIR}/${submodule_path}/${sentinel}")
            message(FATAL_ERROR
                "Required third-party source is still missing: "
                "${submodule_path}/${sentinel}")
        endif()
    endforeach()
endfunction()

htserver_init_third_party_submodules()

# 获取子模块commit ID 函数
function(htserver_get_source_revision source_dir output_varible)
    if(Git_FOUND AND EXISTS "${source_dir}/.git")
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" -C "${source_dir}" rev-parse HEAD
            RESULT_VARIABLE revision_result
            OUTPUT_VARIABLE revisioin
            ERROR_VARIABLE revision_error
            OUTPUT_STRIP_TRAILING_WHITESPACE 
        )
        if(NOT revision_result EQUAL 0)
            message(FATAL_ERROR "Failed to read third-party revision in ${source_dir}: ${revision_error}")
        endif()
    else()
        file(SHA256 "${source_dir}/CMakeLists.txt" revision)
    endif()
    set(${output_variable} "${revision}" PARENT_SCOPE)
endfunction()


function(htserver_add_cmake_dependency)
    # 解析函数命名参数
    cmake_parse_arguments(PARSE_ARGV 0 dep ""
    "NAME;SOURCE_DIR;SOURCE_REVISION"
    "CMAKE_ARGS;SENTINELS;DEPENDS;KEY_MATERIAL")

    if(NOT dep_NAME OR NOT dep_SOURCE_DIR OR NOT dep_SOURCE_REVISION)
        message(FATAL_ERROR "pans_add_cmake_dependency requires NAME, SOURCE_DIR and SOURCE_REVISION")
    endif()

    if(NOT dep_SENTINELS)
        message(FATAL_ERROR "pans_add_cmake_dependency(${dep_NAME}) requires SENTINELS")
    endif()

    string(CONCAT cache_material
        "${dep_SOURCE_REVISION};${CMAKE_C_COMPILER};${CMAKE_C_COMPILER_ID};"
        "${CMAKE_C_COMPILER_VERSION};${CMAKE_CXX_COMPILER};${CMAKE_CXX_COMPILER_ID};"
        "${CMAKE_CXX_COMPILER_VERSION};${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR};"
        "${CMAKE_GENERATOR};${CMAKE_GENERATOR_PLATFORM};${CMAKE_GENERATOR_TOOLSET};"
        "${CMAKE_OSX_ARCHITECTURES};${CMAKE_OSX_DEPLOYMENT_TARGET};"
        "${CMAKE_MSVC_RUNTIME_LIBRARY};${PANS_THIRD_PARTY_BUILD_TYPE};static;"
        "${dep_CMAKE_ARGS};${dep_KEY_MATERIAL}")

    # 保持第三方库ABI一致
    string(SH256 cache_key "${cache_material}")
    string(SUBSTRING "${cache_key}" 0 16 cache_key_short)

    set(install_dir "${HTSERVER_THIRD_PARTY_OUTPUT_DIR}/${dep_NAME}/${cache_key_short}")
    set(build_dir "${HTSERVER_THIRD_PARTY_OUTPUT_DIR}/build/${dep_NAME}-${cache_key_short}")
    set(cache_stamp "${install_dir}/.htserver-cache-${cache_key}")
    
    set(installed_outputs "${cache_stamp}")
    set(cache_complete TRUE)
    foreach(sentinel IN LISTS dep_SENTINELS)
        if(_IS_ABSOLUTE "${sentinel}")
            set(sentinel_path "${sentinel}")
        else()
            set(sentinel_path "${install_dir}/${sentinel}")
        endif()
        list(APPEND installed_outputs "${sentinel_path}")
        if(NOT EXISTS "${sentinel_path}")
            set(cache_complete FALSE)
        endif()
    endforeach()
    
    if(NOT EXISTS "${cache_stamp}")
        set(cache_complete FALSE)
    endif()

    set(configure_command "${CMAKE_COMMAND}" -S "${dep_SOURCE_DIR}" -B "${build_dir}" -G "${CMAKE_GENERATOR}")

    if(CMAKE_GENERATOR_PLATFORM)
        list(APPEND configure_command -T "${CMAKE_GENERATOR_TOOLSET}")
    endif()

    list(APPEND configure_command 
        "-DCMAKE_INSTALL_PREFIX=${install_dir}"
        "-DCMAKE_INSTALL_LINDIR=lib"
        "-DCMAKE_BUILD_TYPE=${HTSERVER_THIRD_PARTY_BUILD_TYPE}"
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
        "-DBUILD_TESTING=OFF"
    )

    if(CMAKE_C_COMPILER)
        list(APPEND configure_command "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
    endif()
    if(CMAKE_CXX_COMPILER)
        list(APPEND configure_command "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
    endif()
    if(CMAKE_TOOLCHAIN_FILE)
        list(APPEND configure_command "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    endif()
    if(CMAKE_MSVC_RUNTIME_LIBRARY)
        list(APPEND configure_command "-DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}")
    endif()
    if(CMAKE_OSX_ARCHITECTURES)
        list(APPEND configure_command "-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}")
    endif()
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
        list(APPEND configure_command "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
    list(APPEND configure_command ${dep_CMAKE_ARGS})

    set(build_commands
        COMMAND ${configure_command}
        COMMAND "${CMAKE_COMMAND}" --build "${build_dir}" --config "${PANS_THIRD_PARTY_BUILD_TYPE}" --parallel "${CMAKE_BUILD_PARALLEL_LEVEL}"
        COMMAND "${CMAKE_COMMAND}" --install "${build_dir}" --config "${PANS_THIRD_PARTY_BUILD_TYPE}"
        COMMAND "${CMAKE_COMMAND}" "-DPANS_DEPS_STAMP_FILE=${cache_stamp}" "-DPANS_DEPS_STAMP_VALUE=${cache_key}" -P "${PROJECT_SOURCE_DIR}/cmake/writeDependencyStamp.cmake"
    )

    set(ready_target "htserver_dep_${dep_NAME}_ready")
    string(REPLACE "-" "_" ready_target "${ready_target}")

    if(HTSERVER_REUSE_INSTALLED_DEPENDENCIES)
        add_custom_command(
            OUTPUT ${installed_output}
            ${build_commands}
            COMMENT "Building and installing third-party dependency: ${dep_NAME}"
            VERBATIM 
            COMMAND_EXPAND_LISTS
        )
        add_custom_target("${ready_target}" DEPENDS ${installed_outputs})
    else()
        add_custom_target("${ready_target}"
            ${build_commands}
            COMMENT "Rebuilding third-party dependency: ${dep_NAME}"
            VERBATIM
            COMMAND_EXPAND_LISTS
        )
    endif()

    if(dep_DEPENDS)
        add_dependencies("${ready_target}" ${dep_DEPENDS})
    endif()

    if(cache_complete AND HTSERVER_REUSE_INSTALLED_DEPENDENCIES)
        message(STATUS "${dep_NAME}: reusing ${install_dir}")
    else()
        message(STATUS "${dep_NAME}: build scheduled; output ${install_dir}")
    endif()

    string(TOUPPER "${dep_NAME}" variable_prefix)
    string(REPLACE "-" "_" variable_prefix "${variable_prefix}")

    set("PANS_${variable_prefix}_INSTALL_DIR" "${install_dir}" PARENT_SCOPE)
    set("PANS_${variable_prefix}_CACHE_KEY" "${cache_key}" PARENT_SCOPE)
    set("PANS_${variable_prefix}_CACHE_COMPLETE" "${cache_complete}" PARENT_SCOPE)
    set("PANS_${variable_prefix}_DEPENDENCY_TARGET" "${ready_target}" PARENT_SCOPE)
endfunction()


set(_htserver_static_library "${CMAKE_STATIC_LIBRARY_PREFIX}@NAME${CMAKE_STATIC_LIBRARY_SUFFIX}")

# -----------abseil-cpp----------------
set(_htserver_absl_source "${HTSERVER_THIRD_PARTY_DIR}/abseil-cpp")
htserver_get_source_revision("${_htserver_absl_source}" _htserver_apsl_revision)
string(REPLACE "@NAME" "absl_base" _htserver_absl_library "${_htserver_static_library}")
htserver_add_cmake_dependency(
    NAME absl-cpp 
    SOURCE_DIR "${_htserver_absl_source}"
    SOURCE_REVISION "${_htserver_absl_revision}"
    CMAKE_ARGS 
        "-DBUILD_SHARED_LIBS=OFF"
        "-DABEL_ENABLE_INSTALL=ON"
        "-DABSL_PROPAGATE_CXX_STD=ON"
        "-DABSL_BUILD_TESTING=OFF"
    SENTINELS
        "lib/cmake/absl/abslConfig.cmake"
        "include/absl/base/config.h"
        "lib/${_pans_absl_library}"
)

# --------------zlib----------------
set(_htserver_zlib_source "${HTSERVER_THIRD_PARTY_DIR}/zlib")
htserver_get_source_revision("${_htserver_zlib_source}" _htserver_zlib_revision)
string(REPLACE "@NAME" "zlib" _htserver_zlib_library "${_htserver_zlib_library}")
if(MSVC)
    string(REPLACE "@NAME" "zlibstatic" _htserver_zlib_library "${_htserver_zlib_library}")
endif()
htserver_add_cmake_dependency(
    NAME zlib
    SOURCE_DIR "${_htserver_zlib_source}"
    SOURCE_REVISION "${_htserver_zlib_revision}"
    CMAKE_ARGS 
        "-DZLIB_BUILD_TESTING=OFF"
        "-DZLIB_BUILD_SHARED=OFF"
        "-DZLIB_BUILD_STATIC=ON"
        "-DZLIB_INSTALL=ON"
    SENTINELS
        "lib/cmake/zlib/ZLIBConfig.cmake"
        "include/zlib.h"
        "lib/${_htserver_zlib_library}"
)

# ------------yaml-cpp--------------
set(_htserver_yaml_source "${HTSERVER_THIRD_PARTY_DIR}/yaml-cpp")
htserver_get_source_revision("${_htserver_yaml_source}" htserver_yaml_revision)
string(REPLACE "@NAME" "yaml" _htserver_yaml_library "${_htserver_yaml_library}")
htserver_add_cmake_dependency(
    NAME yaml
    SOURCE_DIR "${_htserver_yaml_source}"
    SOURCE_REVISION "${_htserver_yaml_revision}"
    CMAKE_ARGS 
        "-DBUILD_SHARED_LIBS=OFF"
        "-DYAML_BUILD_SHARED_LIBS=OFF"
        "-DYAML_CPP_BUILD_TESTS=OFF"
        "-DYAML_CPP_BUILD_TOOLS=OFF"
        "-DYAML_CPP_FORMAT_SOURCE=OFF"
        "-DYAML_CPP_INSTALL=ON"
        "-DYAML_CPP_DISABLE_UNINSTALL=ON"
    SENTINELS
        "lib/cmake/yaml-cpp/yaml-cpp-config.cmake"
        "include/yaml-cpp/yaml.h"
        "lib/${_htserver_yaml_library}"
)

# ------------tinyxml2--------------
set(_htserver_tinyxml2_source "${HTSERVER_THIRD_PARTY_DIR}/tinyxml2")
pans_get_source_revision("${_htserver_tinyxml2_source}" _htserver_tinyxml2_revision)
string(REPLACE "@NAME@" "tinyxml2" _htserver_tinyxml2_library "${_htserver_static_library}")
pans_add_cmake_dependency(
    NAME tinyxml2
    SOURCE_DIR "${_htserver_tinyxml2_source}"
    SOURCE_REVISION "${_htserver_tinyxml2_revision}"
    CMAKE_ARGS
        "-DBUILD_SHARED_LIBS=OFF"
        "-Dtinyxml2_SHARED_LIBS=OFF"
        "-Dtinyxml2_BUILD_TESTING=OFF"
    SENTINELS
        "lib/cmake/tinyxml2/tinyxml2-config.cmake"
        "include/tinyxml2.h"
        "lib/${_htserver_tinyxml2_library}"
)

# --------------kcp-----------------
set(_htserver_kcp_source "${HTSERVER_THIRD_PARTY_DIR}/kcp")
pans_get_source_revision("${_htserver_kcp_source}" _htserver_kcp_revision)
file(SHA256 "${PROJECT_SOURCE_DIR}/cmake/kcp/CMakeLists.txt" _htserver_kcp_wrapper_hash)
string(REPLACE "@NAME@" "kcp" _htserver_kcp_library "${_htserver_static_library}")
pans_add_cmake_dependency(
    NAME kcp
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/cmake/kcp"
    SOURCE_REVISION "${_htserver_kcp_revision}"
    KEY_MATERIAL "wrapper=${_htserver_kcp_wrapper_hash}"
    CMAKE_ARGS
        "-DPANS_KCP_SOURCE_DIR=${_htserver_kcp_source}"
        "-DBUILD_SHARED_LIBS=OFF"
    SENTINELS
        "lib/cmake/kcp/kcp-config.cmake"
        "include/ikcp.h"
        "lib/${_htserver_kcp_library}"
)
# -------------protobuf----------------
set(_htserver_protobuf_source "${HTSERVER_THIRD_PARTY_DIR}/protobuf")
pans_get_source_revision("${_htserver_protobuf_source}" _htserver_protobuf_revision)
string(REPLACE "@NAME@" "protobuf" _htserver_protobuf_library "${_htserver_static_library}")
if(MSVC)
    string(REPLACE "@NAME@" "libprotobuf" _htserver_protobuf_library "${_htserver_static_library}")
endif()
set(_htserver_protoc "bin/protoc${CMAKE_EXECUTABLE_SUFFIX}")
htserver_add_cmake_dependency(
    NAME protobuf
    SOURCE_DIR "${_htserver_protobuf_source}"
    SOURCE_REVISION "${_htserver_protobuf_revision}"
    KEY_MATERIAL
        "abseil=${HTSERVER_ABSEIL_CPP_CACHE_KEY}"
        "zlib=${HTSERVER_ZLIB_CACHE_KEY}"
    DEPENDS 
        "${HTSERVER_ABSEIL_CPP_DEPENDENCY_TARGET}"
        "${HTSERVER_ZLIB_DEPENDENCY_TARGET}"
    CMAKE_ARGS
        "-DCMAKE_CXX_STANDARD=20"
        "-DBUILD_SHARED_LIBS=OFF"
        "-Dprotobuf_BUILD_SHARED_LIBS=OFF"
        "-Dprotobuf_BUILD_TESTS=OFF"
        "-Dprotobuf_BUILD_CONFORMANCE=OFF"
        "-Dprotobuf_BUILD_EXAMPLES=OFF"
        "-Dprotobuf_LOCAL_DEPENDENCIES_ONLY=ON"
        "-Dprotobuf_WITH_ZLIB=ON"
        "-Dabsl_DIR=${HTSERVER_ABSEIL_CPP_INSTALL_DIR}/lib/cmake/absl"
        "-DZLIB_ROOT=${HTSERVER_ZLIB_INSTALL_DIR}"
        "-DZLIB_INCLUDE_DIR=${HTSERVER_ZLIB_INSTALL_DIR}/include"
        "-DZLIB_LIBRARY=${HTSERVER_ZLIB_INSTALL_DIR}/lib/${_htserver_zlib_library}"
    SENTINELS
        "lib/cmake/protobuf/protobuf-config.cmake"
        "include/google/protobuf/message.h"
        "lib/${_htserver_protobuf_library}"
        "${_htserver_protoc}"
)

# RapidJson是only-head库，无需生成output缓存
if(NOT TARGET rapidjson::rapidjson)
    add_library(rapidjson::rapidjson INTERFACE IMPORTED GLOBAL)
    set_target_properties(rapidjson::rapidjson PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${HTSERVER_THIRD_PARTY_DIR}/rapidjson/include"
    )
endif()
message(STATUS "rapidjson: header-only; using ${HTSERVER_THIRD_PARTY_DIR}/rapidjson/include")

add_custom_target(htserver_third_party_dependencies ALL)
add_dependencies(htserver_third_party_dependencies
    "${HTSERVER_ABSEIL_CPP_DEPENDENCY_TARGET}"
    "${HTSERVER_ZLIB_DEPENDENCY_TARGET}"
    "${HTSERVER_YAML_CPP_DEPENDENCY_TARGET}"
    "${HTSERVER_TINYXML2_DEPENDENCY_TARGET}"
    "${HTSERVER_KCP_DEPENDENCY_TARGET}"
    "${HTSERVER_PROTOBUF_DEPENDENCY_TARGET}"
)


# -------- htserver当前直接依赖-----------
find_package(Threads REQUIRED)


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_module(HTSERVER REQUIRED IMPORTED_TARGET libdw)
endif()

if(HTSERVER_YAML_CPP_COMPLETE)
    find_package(yaml-cpp CONFIG REQUIRED PATHS "${HTSERVER_YAML_CPP_INSTALL_DIR}" NO_DEFAULT_PATH)
else()
    file(MAKE_DIRECTORY "${HTSERVER_YAML_CPP_INSTALL_DIR}/include")
    add_library(yaml-cpp::yaml-cpp STATIC IMPORTED GLOBAL)
    set_target_properties(yaml-cpp::yaml-cpp EXPORT_PROPERTIES
        IMPORTED_LOCATION "${HTSERVER_YAML_CPP_INSTALL_DIR}/lib/_htserver_yaml_library"
        INTERFACE_COMPILE_DEFINITIONS YAML_CPP_STATIC_DEFINE
        INTERFACE_INCLUDE_DIRECTORIES "${HTSERVER_YAML_CPP_INSTALL_DIR}/include"
    )
endif()

foreach(_htserver_dependency_install_dir IN LISTS
    "${HTSERVER_YAML_CPP_INSTALL_DIR}"
    "${HTSERVER_ABSEIL_CPP_INSTALL_DIR}"
    "${HTSERVER_ZLIB_INSTALL_DIR}"
    "${HTSERVER_PROTOBUF_INSTALL_DIR}"
    "${HTSERVER_TINYXML2_INSTALL_DIR}"
    "${HTSERVER_KCP_INSTALL_DIR}"
)
    install(
        DIRECTORY "${_htserver_dependency_install_dir}/"
        DESTINATION "."
        PATTERN ".htserver-cache-*" EXCLUDE
    )    
endforeach()

install(
    DIRECTORY "${HTSERVER_THIRD_PARTY_DIR}/rapidjson/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

unset(_htserver_required_submodules)
unset(_htserver_static_library)