if(CLANG)
    add_compile_options(
        -Wno-overloaded-virtual # 'function' hides overloaded virtual function
    )
endif()

add_definitions(
    -DBOOST_FILESYSTEM_STATIC_LINK=1
)

add_library(boost_date_time
    src/gregorian/date_generators.cpp
    src/gregorian/greg_month.cpp
    src/gregorian/greg_weekday.cpp
    src/gregorian/gregorian_types.cpp
    src/posix_time/posix_time_types.cpp
)
target_link_libraries(boost_date_time
    PUBLIC boost_system
)