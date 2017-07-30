set(FILES
    time.hpp
    time.cpp
    timespec.hpp
    timestamp_unit.hpp
    utils.hpp
    utils.cpp
)

if(WIN32)
    list(APPEND FILES
        precise_time.cpp
        precise_time.hpp
    )
endif()

add_library(utils STATIC
    ${FILES}
)