add_library(robin_hood STATIC
    rh/robin_hood.h
    rh/robin_hood.cpp
)

target_include_directories(robin_hood
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)