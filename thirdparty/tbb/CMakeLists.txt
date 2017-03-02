cmake_minimum_required(VERSION 2.8)

include_directories(
    src
	src/rml/include
	build/vs2010
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

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTBB_USE_DEBUG")

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_definitions(-DDO_ITT_NOTIFY)
endif()

if(MSVC)

	enable_language(ASM_MASM)

	add_definitions(/DUSE_WINTHREAD=1 /D__TBB_SOURCE_DIRECTLY_INCLUDED=1 /W0)

	set_source_files_properties(${TBB_ASM_FILES_32BIT} PROPERTIES
		COMPILE_FLAGS "/safeseh")

	set_source_files_properties(${TBB_ASM_FILES_64BIT} PROPERTIES
		COMPILE_FLAGS "/DEM64T=1"
	    COMPILE_FLAGS_DEBUG "/DUSE_FRAME_POINTER")

	if(QDB_CPU_ARCH STREQUAL "32bit")
		set(TBB_SOURCE_FILES ${TBB_CXX_FILES} ${TBB_ASM_FILES_32BIT})
	elseif(QDB_CPU_ARCH STREQUAL "64bit")
		set(TBB_SOURCE_FILES ${TBB_CXX_FILES} ${TBB_ASM_FILES_64BIT})
	else()
		message(FATAL_ERROR "QDB_CPU_ARCH is not defined!")
	endif()

else(MSVC)

	set(TBB_SOURCE_FILES ${TBB_CXX_FILES})

	add_definitions(-DUSE_PTHREAD=1 -D__TBB_SOURCE_DIRECTLY_INCLUDED=1 -w)

endif(MSVC)

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

if(NOT CMAKE_SYSTEM_NAME MATCHES "FreeBSD") # tbbmalloc_proxy is not supported on FreeBSD
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
