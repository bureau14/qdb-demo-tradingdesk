add_executable(nasdaq_exec
    itch_exec.hpp
    itch_messages.hpp
    itch_status.hpp
    nasdaq_exec.cpp
)

target_link_libraries(nasdaq_exec
    utils

    ${QDB_API}
    rxterm
    robin_hood
    fmt
    tbb

    boost_filesystem
    boost_date_time
    boost_program_options
    boost_system

    brigand
)

set_target_properties(nasdaq_exec PROPERTIES
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
)

