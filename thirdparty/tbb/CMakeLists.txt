cmake_minimum_required(VERSION 2.8)

set(CMAKE_FOLDER "thirdparty/tbb")

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(
        # GCC 6.X, dead store elimination (DSE) optimises away memset in allocate_scheduler, causes run-time errors
        # https://software.intel.com/en-us/node/641654
        -flifetime-dse=1
    )
endif()

include_directories(SYSTEM
    src
    src/rml/include
    build/vs2013
)

set(TBB_CXX_FILES
    src/old/concurrent_queue_v2.cpp
    src/old/concurrent_vector_v2.cpp
    src/old/spin_rw_mutex_v2.cpp
    src/old/task_v2.cpp
    src/rml/client/rml_tbb.cpp
    src/tbb/arena.cpp
    src/tbb/cache_aligned_allocator.cpp
    src/tbb/concurrent_hash_map.cpp
    src/tbb/concurrent_monitor.cpp
    src/tbb/concurrent_queue.cpp
    src/tbb/concurrent_vector.cpp
    src/tbb/condition_variable.cpp
    src/tbb/critical_section.cpp
    src/tbb/dynamic_link.cpp
    src/tbb/governor.cpp
    src/tbb/itt_notify.cpp
    src/tbb/market.cpp
    src/tbb/mutex.cpp
    src/tbb/observer_proxy.cpp
    src/tbb/pipeline.cpp
    src/tbb/private_server.cpp
    src/tbb/queuing_mutex.cpp
    src/tbb/queuing_rw_mutex.cpp
    src/tbb/reader_writer_lock.cpp
    src/tbb/recursive_mutex.cpp
    src/tbb/scheduler.cpp
    src/tbb/semaphore.cpp
    src/tbb/spin_mutex.cpp
    src/tbb/spin_rw_mutex.cpp
    src/tbb/task.cpp
    src/tbb/task_group_context.cpp
    src/tbb/tbb_main.cpp
    src/tbb/tbb_misc.cpp
    src/tbb/tbb_misc_ex.cpp
    src/tbb/tbb_statistics.cpp
    src/tbb/tbb_thread.cpp
    src/tbb/x86_rtm_rw_mutex.cpp
)

set(TBB_ASM_FILES_32BIT
    src/tbb/ia32-masm/atomic_support.asm
    src/tbb/ia32-masm/itsx.asm
    src/tbb/ia32-masm/lock_byte.asm
)

set(TBB_ASM_FILES_64BIT
    src/tbb/intel64-masm/atomic_support.asm
    src/tbb/intel64-masm/intel64_misc.asm
    src/tbb/intel64-masm/itsx.asm
)

string(APPEND CMAKE_CXX_FLAGS_DEBUG " -DTBB_USE_DEBUG")

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions(-DDO_ITT_NOTIFY)
endif()

add_definitions(-D__TBB_SOURCE_DIRECTLY_INCLUDED=1)

set(TBB_SOURCE_FILES ${TBB_CXX_FILES})

if(MSVC)
    enable_language(ASM_MASM)

    add_definitions(-DUSE_WINTHREAD=1)
    add_compile_options(/W1)

    set_property(SOURCE ${TBB_ASM_FILES_32BIT}
        PROPERTY COMPILE_FLAGS
        "/safeseh"
    )

    set_property(SOURCE ${TBB_ASM_FILES_64BIT}
        PROPERTY COMPILE_FLAGS
        "-DEM64T=1 $<$<CONFIG:Debug>:-DUSE_FRAME_POINTER>"
    )

    if(QDB_CPU_ARCH MATCHES "32bit")
        list(APPEND TBB_SOURCE_FILES ${TBB_ASM_FILES_32BIT})
    elseif(QDB_CPU_ARCH MATCHES "64bit")
        list(APPEND TBB_SOURCE_FILES ${TBB_ASM_FILES_64BIT})
    else()
        message(FATAL_ERROR "QDB_CPU_ARCH is not defined!")
    endif()
else()
    add_definitions(-DUSE_PTHREAD=1)
    add_compile_options(-w)
endif()

add_library(tbb ${TBB_SOURCE_FILES})

target_link_libraries(tbb
    ${Dynload_LIBRARY}
    ${Realtime_LIBRARY}
)

target_include_directories(tbb
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(tbb
    PRIVATE
    -D__TBB_BUILD=1
)

# -------------------------------------------------------------------------------------------------------

set(TBB_MALLOC_CXX_FILES
    src/tbbmalloc/backend.cpp
    src/tbbmalloc/backref.cpp
    src/tbbmalloc/frontend.cpp
    src/tbbmalloc/large_objects.cpp
    src/tbbmalloc/tbbmalloc.cpp
)

if(QDB_ENABLE_TBB_ALLOCATOR AND NOT CMAKE_SYSTEM_NAME MATCHES "FreeBSD") # tbbmalloc_proxy is not supported on FreeBSD
    list(APPEND TBB_MALLOC_CXX_FILES
        src/tbbmalloc/proxy.cpp
        src/tbbmalloc/tbb_function_replacement.cpp
    )
endif()

add_library(tbbmalloc ${TBB_MALLOC_CXX_FILES})

target_link_libraries(tbbmalloc
    tbb
    ${Pthread_LIBRARY}
)

target_compile_definitions(tbbmalloc
    PRIVATE
    -D__TBBMALLOC_BUILD=1
)
