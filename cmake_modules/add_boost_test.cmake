include(ide_folders)

function(add_boost_test_executable NAME)
    add_executable(${NAME}
        ${ARGN}
        $<TARGET_OBJECTS:teamcity_boost>
    )

    set_target_properties(${NAME} PROPERTIES
        DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

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
                --detect_memory_leaks=1
        )
    else()
        add_test(
            NAME ${NAME}
            COMMAND
                ${NAME}
                --log_level=test_suite
                --report_level=detailed
                --build_info=yes
                --detect_memory_leaks=1
        )
    endif()

    install(TARGETS ${NAME}
        RUNTIME DESTINATION bin COMPONENT tests
        LIBRARY DESTINATION lib COMPONENT tests
        ARCHIVE DESTINATION lib COMPONENT tests
    )
endfunction()
