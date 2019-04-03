add_executable(generator
    generator.cpp
)

target_link_libraries(generator
    utils
    fmt

    ${QDB_API}

    boost_date_time
    boost_program_options
    boost_system
)

set_target_properties(generator PROPERTIES
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
)

