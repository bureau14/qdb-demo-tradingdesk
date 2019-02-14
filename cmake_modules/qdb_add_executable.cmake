function(qdb_add_executable TARGET)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(AE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${TARGET} ${AE_UNPARSED_ARGUMENTS})

    # CMAKE_<CONFIG>_POSTFIX variable is used for non-executable targets by default.
    set_target_properties(${TARGET} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
endfunction()

function(qdb_add_executable_install_release TARGET)
    set(options)
    set(oneValueArgs COMPONENT)
    set(multiValueArgs)
    cmake_parse_arguments(AE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    qdb_add_executable(${TARGET} ${AE_UNPARSED_ARGUMENTS})

    # Package for release builds only.
    install(TARGETS ${TARGET}
        RUNTIME DESTINATION bin CONFIGURATIONS ${QDB_RELEASE_BUILD_TYPES} COMPONENT ${AE_COMPONENT}
        LIBRARY DESTINATION lib CONFIGURATIONS ${QDB_RELEASE_BUILD_TYPES} COMPONENT ${AE_COMPONENT}
        ARCHIVE DESTINATION lib CONFIGURATIONS ${QDB_RELEASE_BUILD_TYPES} COMPONENT ${AE_COMPONENT}
    )
endfunction()
