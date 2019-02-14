set(CMAKE_FOLDER "thirdparty/boost")

if(MSVC)
    add_compile_options(
        # Disable warnings:
        /wd4267 # 'var': conversion from 'size_t' to 'type', possible loss of data
        /wd4458 # declaration of args hides class member
        /wd4189 # r local variable is initialized but not referenced
    )
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CLANG)
    add_compile_options(
        -Wno-unused-variable
    )
endif()

add_subdirectory(libs/date_time)
add_subdirectory(libs/filesystem)
add_subdirectory(libs/program_options)
add_subdirectory(libs/regex)
add_subdirectory(libs/serialization)
add_subdirectory(libs/system)
add_subdirectory(libs/test)