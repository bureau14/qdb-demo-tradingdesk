if(MSVC)
    add_compile_options(
        /wd4722 # destructor never returns, potential memory leak
    )
endif()

if(CLANG)
    add_compile_options(
        -Wno-expansion-to-defined # macro expansion producing 'defined' has undefined behavior
    )
endif()

add_subdirectory(boost)
add_subdirectory(brigand)
add_subdirectory(fmt)
add_subdirectory(tbb)
