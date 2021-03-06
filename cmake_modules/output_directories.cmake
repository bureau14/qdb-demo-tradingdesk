if(QDB_CPU_ARCH_IS_64BIT)
    set(QDB_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin64")
else()
    set(QDB_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif()

set(QDB_DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/doc")

if(MSVC)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${QDB_BIN_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${QDB_BIN_DIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${QDB_BIN_DIR}")
    set(QDB_OUTPUT_DIRECTORY           "${QDB_BIN_DIR}/${CMAKE_BUILD_TYPE}")
else()
    if("${CMAKE_BUILD_TYPE}" STREQUAL "")
        message(WARNING "CMAKE_BUILD_TYPE is empty, weird things can happen!")
    endif()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${QDB_BIN_DIR}/${CMAKE_BUILD_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${QDB_BIN_DIR}/${CMAKE_BUILD_TYPE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${QDB_BIN_DIR}/${CMAKE_BUILD_TYPE}")
    set(QDB_OUTPUT_DIRECTORY           "${QDB_BIN_DIR}/${CMAKE_BUILD_TYPE}")
endif()
