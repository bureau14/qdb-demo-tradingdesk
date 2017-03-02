include(add_link_options)

set(CMAKE_EXE_LINKER_FLAGS_SANITIZE    "" CACHE STRING "Flags used by the linker for executables during sanitizer builds." FORCE)
set(CMAKE_MODULE_LINKER_FLAGS_SANITIZE "" CACHE STRING "Flags used by the linker for modules during sanitizer builds." FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_SANITIZE "" CACHE STRING "Flags used by the linker for shared libraries during sanitizer builds." FORCE)
set(CMAKE_STATIC_LINKER_FLAGS_SANITIZE "" CACHE STRING "Flags used by the linker for static libraries during sanitizer builds." FORCE)

if(MSVC)
    # cmake adds /stack parameter, we don't want 10 MB of stack!
    # therefore we override all parameters by hand
    # without this parameter, qdb does not work
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
        message(FATAL_ERROR "Unknown machine architecture!")
    endif()

    add_link_options(
        /NXCOMPAT
        /FUNCTIONPADMIN
        /INCREMENTAL:NO
        /stack:2097152
    )

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
    if(NOT CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        add_link_options(
            -lc++
            -lc++abi
        )
    endif()
endif()

if(CLANG AND NOT APPLE)
    add_link_options(
        -Qunused-arguments
        -Wl,--gc-sections
    )

    add_link_options(CONFIGURATION Release
        -Wl,-s # strip symbol because we can't print a stacktrace with Clang
    )
endif()

# let's help the user and have a look in our lib directories for missing libraries
# we don't use CMake built-in mechanism for Rpath as it is inconsistent from one UNIX to the other
if(CMAKE_COMPILER_IS_GNUCXX)
    add_link_options(
        -static-libgcc
        -static-libstdc++
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
    set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
endif()

# https://gist.github.com/kwk/4171e37f4bcdf7705329
add_link_options(CONFIGURATION Sanitize OPTIONS ${COMPILE_FLAGS_SANITIZE_FOR_LINKER})
if(CMAKE_COMPILER_IS_GNUCXX)
    add_link_options(CONFIGURATION Sanitize
        #-lasan
        -static-libasan
        #-static-libstdc++
    )
endif()
