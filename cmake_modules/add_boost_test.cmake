include(qdb_add_executable)

function(add_boost_test_executable NAME COMPONENT)
    qdb_add_executable(${NAME}
        ${ARGN}
        $<TARGET_OBJECTS:teamcity_boost>
    )

    target_link_libraries(${NAME}
        boost_test
    )

    if($<CONFIG:SanitizeValgrind>)
        add_test(
            NAME ${NAME}
            COMMAND
                valgrind>
                $<$<PLATFORM_ID:Darwin>:--dsymutil=yes>
                --gen-suppressions=all
                ./${NAME}
                --log_level=test_suite
                --report_level=detailed
                --build_info=yes
                --detect_memory_leaks=0
        )
    else()
        add_test(
            NAME ${NAME}
            COMMAND
                ${NAME}
                --log_level=test_suite
                --report_level=detailed
                --build_info=yes
                --detect_memory_leaks=0
        )
    endif()

    install(TARGETS ${NAME}
        RUNTIME DESTINATION bin COMPONENT ${COMPONENT}
        LIBRARY DESTINATION lib COMPONENT ${COMPONENT}
        ARCHIVE DESTINATION lib COMPONENT ${COMPONENT}
    )
endfunction()
