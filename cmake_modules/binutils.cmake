include(cpu_arch)

function(replace_binutil VARIABLE FILE)
    if(EXISTS ${FILE})
        message(STATUS "${VARIABLE}: using ${FILE} instead ${${VARIABLE}}")
        set(${VARIABLE} ${FILE})
    else()
        message(STATUS "${VARIABLE}: keeping ${${VARIABLE}} because ${FILE} doesn't exist")
    endif()
endfunction()

if(CMAKE_COMPILER_IS_GNUCXX)
    if(NOT CMAKE_CROSSCOMPILING)
        # Workaround for CentOS:
        # use the "local" ar and ranlib because we have old crap in the default path on Linux/GCC
        replace_binutil(CMAKE_AR     "/usr/local/bin/ar")
        replace_binutil(CMAKE_RANLIB "/usr/local/bin/ranlib")
        replace_binutil(CMAKE_STRIP  "/usr/local/bin/strip")
    endif()

    message(STATUS "CMAKE_AR=${CMAKE_AR}")
    message(STATUS "CMAKE_LINKER=${CMAKE_LINKER}")
    message(STATUS "CMAKE_RANLIB=${CMAKE_RANLIB}")
    message(STATUS "CMAKE_STRIP=${CMAKE_STRIP}")
endif()
