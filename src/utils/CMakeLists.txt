set(FILES
    gregorian.hpp
    time.cpp
    time.hpp
    timespec.hpp
    timestamp_unit.hpp
    utils.cpp
    utils.hpp
    version.hpp
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
