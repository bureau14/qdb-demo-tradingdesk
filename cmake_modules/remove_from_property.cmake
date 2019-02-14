include(CMakeParseArguments)

function(remove_from_property)
    cmake_parse_arguments(REMOVE_FROM_PROPERTY "" "PROPERTY" "OPTIONS" ${ARGN})
    if(NOT REMOVE_FROM_PROPERTY_OPTIONS)
        message(SEND_ERROR "remove_from_property() called without any options")
        return()
    endif()

    if(NOT REMOVE_FROM_PROPERTY_PROPERTY)
        message(SEND_ERROR "remove_from_property() called without any property")
        return()
    endif()

    get_directory_property(VALUE ${REMOVE_FROM_PROPERTY_PROPERTY})

    message(STATUS "Before: ${REMOVE_FROM_PROPERTY_PROPERTY} = ${VALUE}")
    foreach(OPT ${REMOVE_FROM_PROPERTY_OPTIONS})
        list(REMOVE_ITEM VALUE "${OPT}" VALUE)
    endforeach()

    set_property(DIRECTORY
        PROPERTY ${REMOVE_FROM_PROPERTY_PROPERTY}
        ${VALUE}
    )
    message(STATUS "After : ${REMOVE_FROM_PROPERTY_PROPERTY} = ${VALUE}")

    get_directory_property(DIR_PROPERTY_OPTIONS COMPILE_OPTIONS)
endfunction()
