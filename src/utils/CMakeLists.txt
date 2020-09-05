set(FILES
    file_mapping.hpp
    file_stream.hpp
    gregorian.hpp
    humanize_number.cpp
    humanize_number.hpp
    make_array.hpp
    mktime.cpp
    mktime.hpp
    stringify.hpp
    stringify.cpp
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
