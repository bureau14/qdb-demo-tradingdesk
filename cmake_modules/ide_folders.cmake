macro(set_ide_folder args)
    set(folder ${ARGV0})
    foreach(project ${ARGN})
        # message("${folder} <- ${project}")
        if(TARGET ${project})
            set_target_properties(${project} PROPERTIES FOLDER ${folder})
        endif()
    endforeach()
endmacro()


set_ide_folder("thirdparty"
    benchmark
    brigand
    farmhash
    fmt
    linenoise
    lz4
    rapidjson
    robin_hood
    rxterm
    skein
    zlibstatic
)

if(WIN32)
    set_ide_folder("thirdparty"
        ms_win_service
    )
endif()

set_ide_folder("thirdparty/boost"
    boost_filesystem
    boost_program_options
    boost_regex
    boost_serialization
    boost_system
    boost_test
    teamcity_boost
)

set_ide_folder("thirdparty/rocksdb"
    rocksdb
)

set_ide_folder("thirdparty/rocksdb/thirdparty"
    gtest
)

set_ide_folder("thirdparty/tbb"
    tbb
    tbbmalloc
    tbbmalloc_proxy
)
