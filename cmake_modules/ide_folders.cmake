macro(set_ide_folder args)
    set(folder ${ARGV0})
    foreach(project ${ARGN})
        # message("${folder} <- ${project}")
        if(TARGET ${project})
            set_target_properties(${project} PROPERTIES FOLDER ${folder})
        endif()
    endforeach()
endmacro()

set_ide_folder("api"
    qdb_api
    qdb_api_static
    qdb_capi_doc
)

set_ide_folder("apps/qdbd"
    qdb_daemon
    qdbd
    qdbd_generate_config
)

if(WIN32)
    set_ide_folder("apps/qdbd"
        qdb_service
    )
endif()

set_ide_folder("apps/qdb_httpd"
    qdb_http_server
    qdb_httpd
    qdb_httpd_config
    qdb_httpd_generate_config
    qdb_www
)

if(WIN32)
    set_ide_folder("apps/qdb_httpd"
        qdb_http_service
    )
endif()

set_ide_folder("apps/tools"
    qdb_bench
    qdb_dbtool
    qdb_generate_stats
    qdb_license_check
    qdb_license_gen
    qdb_max_conn
    qdb_protocol_doc
    qdb_stress
)

set_ide_folder("apps/qdbsh"
    qdb_shell
    qdbsh
)

set_ide_folder("tests/performance"
    qdb_self_bench
)

set_ide_folder("tests"
    qdb_regression_test
    qdb_self_bench
    qdb_test_utils
    qdb_test_utils_cluster
    qdb_test_utils_kernel
    qdb_content_generator
)

set_ide_folder("libs"
    qdb_chord
    qdb_client
    qdb_config
    qdb_crypto
    qdb_id
    qdb_json
    qdb_kernel
    qdb_license
    qdb_log
    qdb_network
    qdb_persistence
    qdb_protocol
    qdb_serialization
    qdb_sys
    qdb_thread_hook
    qdb_util
    qdb_version
)

set_ide_folder("thirdparty"
    benchmark
    brigand
    farmhash
    fmt
    linenoise
    lz4
    rapidjson
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
