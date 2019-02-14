include(cpu_arch)
include(join)

message(STATUS "CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION}")

set(COMMON_SANITIZE_FLAGS " -g -fno-omit-frame-pointer -fno-optimize-sibling-calls ")
set(CMAKE_C_FLAGS_SANITIZE "" CACHE STRING "Flags used by the C compiler during sanitizer builds." FORCE)
set(CMAKE_C_FLAGS_SANITIZEVALGRIND "" CACHE STRING "Flags used by the C compiler during sanitizer builds using Valgrind." FORCE)
set(CMAKE_C_FLAGS_ASAN "" CACHE STRING "Flags used by the C compiler during ASan builds." FORCE)
set(CMAKE_C_FLAGS_LSAN "" CACHE STRING "Flags used by the C compiler during LSan builds." FORCE)
set(CMAKE_C_FLAGS_MSAN "" CACHE STRING "Flags used by the C compiler during LSan builds." FORCE)
set(CMAKE_C_FLAGS_TSAN "" CACHE STRING "Flags used by the C compiler during TSan builds." FORCE)
set(CMAKE_C_FLAGS_UBSAN "" CACHE STRING "Flags used by the C compiler during UBSan builds." FORCE)
set(CMAKE_CXX_FLAGS_SANITIZE "" CACHE STRING "Flags used by the C++ compiler during sanitizer builds." FORCE)
set(CMAKE_CXX_FLAGS_SANITIZEVALGRIND "" CACHE STRING "Flags used by the C++ compiler during sanitizer builds using Valgrind." FORCE)
set(CMAKE_CXX_FLAGS_ASAN "" CACHE STRING "Flags used by the C++ compiler during ASan builds." FORCE)
set(CMAKE_CXX_FLAGS_LSAN "" CACHE STRING "Flags used by the C++ compiler during LSan builds." FORCE)
set(CMAKE_CXX_FLAGS_MSAN "" CACHE STRING "Flags used by the C++ compiler during LSan builds." FORCE)
set(CMAKE_CXX_FLAGS_TSAN "" CACHE STRING "Flags used by the C++ compiler during TSan builds." FORCE)
set(CMAKE_CXX_FLAGS_UBSAN "" CACHE STRING "Flags used by the C++ compiler during UBSan builds." FORCE)

if(WIN32)
    if(CLANG)
        add_compile_options(
            -DBOOST_SP_USE_STD_ATOMIC # HACK for mangling problem in Boost https://llvm.org/bugs/show_bug.cgi?id=25384
            # -Xclang -fms-extensions
            -Xclang -fms-compatibility-version=18
            #-Xclang -Wno-error=inconsistent-dllimport
            -Xclang -Wno-inconsistent-dllimport
            #-Xclang -Wno-error=pedantic
            -Xclang -Wno-pedantic
            -Xclang -Wno-microsoft-enum-value
            #-Xclang -Wno-error=unknown-pragmas
            -Xclang -Wno-unknown-pragmas
            -Xclang -Wno-error=vla-extension
        )
    endif()

    # clang-cl defines _MSC_VER and so MSVC==true
    if(MSVC AND NOT CLANG)
        add_compile_options(
            /bigobj       # increases the number of sections that an object file can contain.
            /MP4          # 4 parallel compilations
        )

        if(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 19.14)
            add_compile_options(
                /permissive-  # set standard-conformance mode
            )
            #if(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 19.16.27025)
                add_compile_options(
                    /Zc:twoPhase- # turn off two-phase lookup for now, as it's broken on MSVC 2017 15.3-15.9.4.
                )
            #endif()
        endif()

        if(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 19.15.26608)
            add_compile_options(
                $<$<CONFIG:Debug>:/JMC> # enable Just My Code
            )
        endif()
    endif()

    add_compile_options(
        /volatile:iso # use ISO volatile behavior for better performance
        /Gy           # Allows the compiler to package individual functions in the form of packaged functions (COMDATs).
        /Zc:wchar_t   # Parse wchar_t as a built-in type according to the C++ standard
        /EHa          # The exception-handling model that catches both asynchronous (structured) and synchronous (C++) exceptions.
        /GR           # Enable Run-Time Type Information
        /GF           # Eliminate Duplicate Strings

        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:/Zi> # Produces a program database (PDB) that contains type information and symbolic debugging information for use with the debugger

        # Debug: Turns off all optimizations in the program and speeds compilation.
        # Non-debug: Selects full optimization.
        $<IF:$<CONFIG:Debug>,/Od,/Ox>

        # Debug: Disables inline expansion, which is on by default.
        # Non-debug: Expands functions marked as inline or __inline and any other function that the compiler chooses
        $<IF:$<CONFIG:Debug>,/Ob0,/Ob2>

        $<$<NOT:$<CONFIG:Debug>>:/Oi>  # Replaces some function calls with intrinsic or otherwise special forms of the function that help your application run faster.
        $<$<NOT:$<CONFIG:Debug>>:/Ot>  # Maximizes the speed of EXEs and DLLs by instructing the compiler to favor speed over size.
        $<$<NOT:$<CONFIG:Debug>>:/Oy>  # Suppresses creation of frame pointers on the call stack.
        $<$<NOT:$<CONFIG:Debug>>:/GS->  # Suppresses Buffer Security Check
        # Some of our machines don't support AVX, note that by default, SSE2 will be enabled
        # $<$<NOT:$<CONFIG:Debug>>:/arch:AVX>

        $<$<CONFIG:Debug>:/RTC1>       # Enable the run-time error checks feature, in conjunction with the runtime_checks pragma.
        /MT$<$<CONFIG:Debug>:d>        # /MT : Causes the application to use the multithread, static version of the run-time library.
                                       # /MTd: Defines _DEBUG and _MT. This option also causes the compiler to place the library name LIBCMTD.lib into the .obj file so that the linker will use LIBCMTD.lib to resolve external symbols.
    )

    if(NOT CLANG)
        add_compile_options(
            /F2097152    # Sets the program stack size in bytes.
        )
    endif()

    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19")
        add_compile_options(
            /Zc:throwingNew # turn on optimizations due to the conforming new operator
        )
    endif()

    # for hiredis
    set(CMAKE_C_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_C_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif()

if(CLANG OR CMAKE_COMPILER_IS_GNUCXX)
    if(QDB_CPU_IS_X86)
        # GCC documentation: core2: Intel Core2 CPU with 64-bit extensions, MMX, SSE, SSE2, SSE3 and SSSE3 instruction set support.
        if(QDB_CPU_ARCHITECTURE_CORE2)
            add_compile_options(
                -march=core2
            )
        else()
            add_compile_options(
                # for the moment we target Westmere architecture or later (2008+)
                # this enables MMX, SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, POPCNT, AES and PCLMUL
                # if we want AVX we need to target sandy bridge whose processors were released in 2011
                -march=westmere
            )
        endif()
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fcxx-exceptions")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++") # tbb will not work under clang with libstdc++
    endif()

    add_compile_options(
        -fcolor-diagnostics # force color output on ninja
        -fmacro-backtrace-limit=0
        $<$<CONFIG:Release>:-O3>
        # already added by CMake:
        #$<$<CONFIG:Debug>:-fno-assume-sane-operator-new>
        #$<$<CONFIG:Debug>:-fno-inline-functions>
        #$<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        #$<$<CONFIG:Debug>:-fno-optimize-sibling-calls>
        #$<$<CONFIG:Debug>:-g>

    )
    if(QDB_ENABLE_COVERAGE)
        add_compile_options(
            $<$<CONFIG:Debug>:-fprofile-instr-generate>
            $<$<CONFIG:Debug>:-fcoverage-mapping>
        )
    endif()

    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fexceptions")
    else()
        add_compile_options(
            -fexceptions
            -ftemplate-backtrace-limit=0
            -fvisibility-inlines-hidden
            -fvisibility=hidden
        )
    endif()

    if(APPLE)
        set(COMPILE_FLAGS_SANITIZE_FOR_LINKER
            -fsanitize-undefined-trap-on-error
            -fsanitize=alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,null,object-size,return,shift,signed-integer-overflow,unreachable,vla-bound,undefined-trap,vla-bound
            #-fsanitize-coverage=edge
        )
    elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(COMPILE_FLAGS_SANITIZE_FOR_LINKER
            -fsanitize=address,integer
            #-fsanitize-coverage=edge
        )
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    # if we're using GCC, we statically link against libc++ and libgcc

    # let's pimp GCC a bit
    # -flto is problematic, don't use it - it does not compile correctly on FreeBSD even with gcc 4.6.1
    # you're going to ignore the above message and add it anyway, break the build and remove it
    # every time you do this please increase this counter : 4
    # we need -fnon-call-exceptions to catch SIGSEGV
    # we also need -fasynchronous-unwind-tables for asynchronous signals
    # we disable stack protector for now but we need to benchmark the impact on performance
    # stack protection should harden the security
    # -fdata-sections -ffunction-sections enables us to remove dead code in the linking phase
    # to avoid symbols clashing and to preven our dynamic library to expose tons of symbols
    # we have a map file, for debugging and to be able to print a decent stack trace we need symbols
    # if we wanted to hide all symbols we would use -fvisibility=hidden and -fvisibility-inlines-hidden
    # -ftrack-macro-expansion=0 or prepare for epic compile times
    add_compile_options(
        -fdata-sections
        -ffunction-sections
        -fno-stack-protector
        -fnon-call-exceptions
        -ftrack-macro-expansion=0
        $<$<CONFIG:Debug>:-fno-eliminate-unused-debug-types>
        $<$<CONFIG:Debug>:-ggdb>
    )
    if(QDB_CPU_IS_X86)
        add_compile_options(
            -m64
            $<$<NOT:$<CONFIG:Debug>>:-mfpmath=sse>
        )
    endif()

    if(QDB_ENABLE_COVERAGE)
        add_compile_options(
            $<$<CONFIG:Debug>:--coverage>
        )
    endif()

    set(COMPILE_FLAGS_SANITIZE_FOR_LINKER
        -fsanitize=address
        -fsanitize=undefined
        # -fsanitize-coverage=edge
    )
endif()

if(CLANG OR CMAKE_COMPILER_IS_GNUCXX)
    set(COMPILE_FLAGS_ASAN_FOR_LINKER
        -fsanitize=address
    )
    set(COMPILE_FLAGS_LSAN_FOR_LINKER
        -fsanitize=leak
    )
    set(COMPILE_FLAGS_MSAN_FOR_LINKER
        -fsanitize=memory
    )
    set(COMPILE_FLAGS_TSAN_FOR_LINKER
        -fsanitize=thread
    )
    set(COMPILE_FLAGS_UBSAN_FOR_LINKER
        -fsanitize=undefined
    )

    join(COMPILE_FLAGS_ASAN_FOR_LINKER " " ${COMPILE_FLAGS_ASAN_FOR_LINKER})
    join(COMPILE_FLAGS_LSAN_FOR_LINKER " " ${COMPILE_FLAGS_LSAN_FOR_LINKER})
    join(COMPILE_FLAGS_MSAN_FOR_LINKER " " ${COMPILE_FLAGS_MSAN_FOR_LINKER})
    join(COMPILE_FLAGS_TSAN_FOR_LINKER " " ${COMPILE_FLAGS_TSAN_FOR_LINKER})
    join(COMPILE_FLAGS_UBSAN_FOR_LINKER " " ${COMPILE_FLAGS_UBSAN_FOR_LINKER})
    join(COMPILE_FLAGS_SANITIZE_FOR_LINKER " " ${COMPILE_FLAGS_SANITIZE_FOR_LINKER})

    set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_ASAN} ${COMPILE_FLAGS_ASAN_FOR_LINKER} -O1 ${COMMON_SANITIZE_FLAGS}")
    set(CMAKE_CXX_FLAGS_LSAN "${CMAKE_CXX_FLAGS_LSAN} ${COMPILE_FLAGS_LSAN_FOR_LINKER} -O1 ${COMMON_SANITIZE_FLAGS}")
    set(CMAKE_CXX_FLAGS_MSAN "${CMAKE_CXX_FLAGS_MSAN} ${COMPILE_FLAGS_MSAN_FOR_LINKER} -O1 ${COMMON_SANITIZE_FLAGS}")
    set(CMAKE_CXX_FLAGS_TSAN "${CMAKE_CXX_FLAGS_TSAN} ${COMPILE_FLAGS_TSAN_FOR_LINKER} -O1 ${COMMON_SANITIZE_FLAGS}")
    set(CMAKE_CXX_FLAGS_UBSAN "${CMAKE_CXX_FLAGS_UBSAN} ${COMPILE_FLAGS_UBSAN_FOR_LINKER} -O1 ${COMMON_SANITIZE_FLAGS}")
    set(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} ${COMPILE_FLAGS_SANITIZE_FOR_LINKER} -O1 ${COMMON_SANITIZE_FLAGS}")
    set(CMAKE_CXX_FLAGS_SANITIZEVALGRIND "${CMAKE_CXX_FLAGS_SANITIZEVALGRIND} -O2 ${COMMON_SANITIZE_FLAGS}")
endif()