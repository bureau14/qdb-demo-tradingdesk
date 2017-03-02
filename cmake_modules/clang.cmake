if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CLANG TRUE)

    if(NOT APPLE AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.5.O")
        set(CLANG_35_OR_GREATER TRUE)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        # we need to tell clang to link against lcxxrt otherwise exceptions won't work
        # we prefer to include our clang libraries because we might have a different Clang version than the FreeBSD on which qdb
        # is used
        # starting with FreeBSD 10 it will be much more simple as FreeBSD AMD64 will be fully clangized
        # we don't include libgcc_s and libstdc++ because we use the system's default
        find_file(CLANG_RT libcxxrt.so.1 PATHS /usr/local/lib/ /usr/lib/ /lib/ ENV LIB)
        get_filename_component(ACTUAL_CLANG_RT ${CLANG_RT} REALPATH)
        find_file(CLANG_STDCPP libc++.so.1 PATHS /usr/local/lib/ /usr/lib/ /lib/ ENV LIB)
        get_filename_component(ACTUAL_STDCPP ${CLANG_STDCPP} REALPATH)
        set(CLANG_ADDITIONAL_LIBS ${ACTUAL_CLANG_RT} ${ACTUAL_STDCPP})

        install(PROGRAMS
            ${CLANG_ADDITIONAL_LIBS}
            DESTINATION lib
            COMPONENT c-api)

        install(PROGRAMS
            ${CLANG_ADDITIONAL_LIBS}
            DESTINATION lib
            COMPONENT server)

        install(PROGRAMS
            ${CLANG_ADDITIONAL_LIBS}
            DESTINATION lib
            COMPONENT utils)
    endif()
endif()
