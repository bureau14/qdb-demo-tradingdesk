cmake_minimum_required(VERSION 3.10)
project(demo-tradingdesk)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

include(policies NO_POLICY_SCOPE)
include(build_types) # Must be before linker and compiler flags.
include(clang)
include(cpu_arch)
include(warnings)
include(preprocessor_definitions)
include(compiler_flags)
include(linker_flags)
include(binutils)
include(realtime)
include(add_boost_test)
include(options)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_VERSION VERSION_LESS 3.4)
    message(WARNING "CMake < 3.4 will not add manifest files to Windows binaries")
endif()

# don't try to test the compiler
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# we only want Debug, release and release with debug info (for profiling), and Sanitize
if(CMAKE_CONFIGURATION_TYPES)
    if(CMAKE_BUILD_TYPE)
        set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE STRING "Configuration types" FORCE)
    else()
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;Sanitize" CACHE STRING "Configuration types" FORCE)
    endif()
endif()

include(qdb_version)

set(CMAKE_DEBUG_POSTFIX "d")

include(output_directories)

if(NOT WIN32)
    find_library(Pthread_LIBRARY pthread)
    # find library can be problematic with stdc++ which is why we hardwire the link
else()
    set(Pthread_LIBRARY "")
    set(Dynload_LIBRARY "")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_library(Dynload_LIBRARY dl)
else()
    set(Dynload_LIBRARY "")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    find_library(Kvm_LIBRARY kvm)
else()
    set(Kvm_LIBRARY)
endif()

find_package(Doxygen)

find_library(QDB_API NAMES qdb_api
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/qdb/lib ${CMAKE_CURRENT_SOURCE_DIR}/qdb/bin)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}    
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/boost
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/fmt
    ${CMAKE_CURRENT_SOURCE_DIR}/qdb/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(thirdparty)
add_subdirectory(src)

include(cpack_config)
include(ide_folders)
