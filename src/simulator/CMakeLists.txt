add_executable(simulator
    broker.hpp
    dow_jones.hpp
    dow_jones.cpp
    index.cpp
    index.hpp
    products.hpp
    quote.cpp
    quote.hpp
    quote_generator.hpp
    simulator.cpp
    trade.cpp
    trade.hpp
    trader.hpp
)

target_link_libraries(simulator
    utils
    fmt

    ${QDB_API}

    boost_program_options
    boost_system
)

set_target_properties(simulator PROPERTIES
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
)
