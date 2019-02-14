include(add_link_options)

if(MSVC)
    # cmake adds /stack parameter, we don't want 10 MB of stack!
    # therefore we override all parameters by hand
    # without this parameter, qdb does not work
    if(QDB_CPU_IS_X86)
        if(QDB_CPU_ARCH_IS_64BIT)
            add_link_options(WITH_STATIC
                /MACHINE:X64
            )
        elseif(QDB_CPU_ARCH_IS_32BIT)
            add_link_options(WITH_STATIC
                /MACHINE:X86
            )
            add_link_options(
                /LARGEADDRESSAWARE
            )
        else()
            message(FATAL_ERROR "Unknown x86 machine architecture!")
        endif()
    elseif(QDB_CPU_IS_ARM)
        message(STATUS "ARM machine architecture")
    else()
        message(FATAL_ERROR "Unknown machine architecture!")
    endif()

    add_link_options(
        /NXCOMPAT
        /INCREMENTAL:NO
        /stack:2097152
    )

    # https://docs.microsoft.com/en-us/cpp/build/reference/functionpadmin-create-hotpatchable-image
    if(QDB_CPU_IS_ARM)
        add_link_options(
            /FUNCTIONPADMIN:6 # non-x86 targets need a value. Use 6 as a guess.
        )
    else()
        add_link_options(
            /FUNCTIONPADMIN
        )
    endif()

    add_link_options(CONFIGURATION Debug
        /DEBUG
    )

    add_link_options(CONFIGURATION Release
        /OPT:REF
        /RELEASE
        /DYNAMICBASE
    )

    add_link_options(CONFIGURATION RelWithDebInfo
        /DEBUG
        /PROFILE
        /DYNAMICBASE:NO
        /INCREMENTAL:NO
    )
endif()

if(CLANG)
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        add_link_options(
            -L/usr/local/lib
        )
    endif()

    add_link_options(
        -lc++
        #-lc++abi # We do not need libc++abi on macOS.
    )
endif()

if(CLANG AND NOT APPLE)
    add_link_options(
        -Qunused-arguments
        -Wl,--gc-sections
    )
endif()

# let's help the user and have a look in our lib directories for missing libraries
# we don't use CMake built-in mechanism for Rpath as it is inconsistent from one UNIX to the other
if(CMAKE_COMPILER_IS_GNUCXX)
    if(NOT QDB_NO_STATIC_LIBSTDCXX)
        add_link_options(
            -static-libgcc
            -static-libstdc++
        )
    endif()
    add_link_options(
        # -Wl,-s <-- do not get rid of symbols because we need them to print a decent stacktrace
        -Wl,--gc-sections # remove dead code
    )
    if(QDB_ENABLE_COVERAGE)
        add_link_options(CONFIGURATION Debug
            --coverage
        )
    endif()
endif()

if(APPLE)
    # Prevent ranlib warning
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
endif()

# https://gist.github.com/kwk/4171e37f4bcdf7705329
add_link_options(CONFIGURATION Sanitize OPTIONS ${COMPILE_FLAGS_SANITIZE_FOR_LINKER})
add_link_options(CONFIGURATION ASan     OPTIONS ${COMPILE_FLAGS_ASAN_FOR_LINKER})
add_link_options(CONFIGURATION LSan     OPTIONS ${COMPILE_FLAGS_LSAN_FOR_LINKER})
add_link_options(CONFIGURATION MSan     OPTIONS ${COMPILE_FLAGS_MSAN_FOR_LINKER})
add_link_options(CONFIGURATION TSan     OPTIONS ${COMPILE_FLAGS_TSAN_FOR_LINKER})
add_link_options(CONFIGURATION UBSan    OPTIONS ${COMPILE_FLAGS_UBSAN_FOR_LINKER})
