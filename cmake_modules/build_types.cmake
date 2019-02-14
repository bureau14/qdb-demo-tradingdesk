set(QDB_RELEASE_BUILD_TYPES
    #MinSizeRel
    RelWithDebInfo
    Release
)

set(QDB_ALLOWED_BUILD_TYPES
    ${QDB_RELEASE_BUILD_TYPES}
    Debug
    Sanitize
    SanitizeValgrind
    ASan
    LSan
    MSan
    TSan
    UBSan
)

# Check if multi-config generator
if(CMAKE_CONFIGURATION_TYPES) # GENERATOR_IS_MULTICONFIG
    set(CMAKE_CONFIGURATION_TYPES ${QDB_ALLOWED_BUILD_TYPES}
        CACHE STRING "Configuration types" FORCE)
else()
    set_property(CACHE CMAKE_BUILD_TYPE
        PROPERTY STRINGS "${QDB_ALLOWED_BUILD_TYPES}"
    )
    if(NOT CMAKE_BUILD_TYPE)
        message(FATAL_ERROR "Empty CMAKE_BUILD_TYPE")
    elseif(NOT CMAKE_BUILD_TYPE IN_LIST QDB_ALLOWED_BUILD_TYPES)
        message(FATAL_ERROR "Invalid CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

function(add_build_type BUILD_TYPE)
    string(TOUPPER "${BUILD_TYPE}" UPPER_BUILD_TYPE)

    set(CMAKE_EXE_LINKER_FLAGS_${UPPER_BUILD_TYPE}    "" CACHE STRING "Flags used by the linker for executables during ${BUILD_TYPE} builds." FORCE)
    set(CMAKE_MODULE_LINKER_FLAGS_${UPPER_BUILD_TYPE} "" CACHE STRING "Flags used by the linker for modules during ${BUILD_TYPE} builds." FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_${UPPER_BUILD_TYPE} "" CACHE STRING "Flags used by the linker for shared libraries during ${BUILD_TYPE} builds." FORCE)
    set(CMAKE_STATIC_LINKER_FLAGS_${UPPER_BUILD_TYPE} "" CACHE STRING "Flags used by the linker for static libraries during ${BUILD_TYPE} builds." FORCE)
endfunction()

add_build_type(Sanitize)
add_build_type(SanitizeValgrind)
add_build_type(ASan)
add_build_type(LSan)
add_build_type(MSan)
add_build_type(TSan)
add_build_type(UBSan)
