if(CLANG)
    add_compile_options(
        -Wno-conversion
    #    -Wno-expansion-to-defined # macro expansion producing 'defined' has undefined behavior
        -Wno-shorten-64-to-32
        -Wno-sign-conversion
    )
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(
        -Wno-conversion
        -Wno-deprecated-declarations
        -Wno-double-promotion
        -Wno-duplicated-branches   # GCC 7+
        -Wno-format-nonliteral
        -Wno-sign-conversion
    )
endif()

add_subdirectory(boost)
add_subdirectory(brigand)
add_subdirectory(fmt)
add_subdirectory(robin-hood-hashing)
add_subdirectory(rxterm)
add_subdirectory(tbb)
