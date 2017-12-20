if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    find_package(Git)
endif()

set(QDB_VERSION_PREFIX "2.3.0") # the default

string(TIMESTAMP QDB_COPYRIGHT_YEAR "%Y")

if(NOT QDB_VERSION)

    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        if(GIT_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE QDB_VERSION)
        else()
            message(WARNING "Git not installed, can't find branch name")
            set(QDB_VERSION "unknown")
        endif()
    else()
        message(WARNING "Not a Git repository, can't find branch name")
            set(QDB_VERSION "unknown")
    endif()
endif()

string(STRIP ${QDB_VERSION} QDB_VERSION)

if(QDB_VERSION MATCHES ${QDB_VERSION_PREFIX})
    set(QDB_VERSION ${QDB_VERSION_PREFIX})
else()
    # otherwise use the prefix as is
    # don't put a - in the version prefix, rpm will refuse it
    # for example 2.0.0-case1028 is an invalid version
    # but 2.0.0case1028 is valid
    set(QDB_VERSION "${QDB_VERSION_PREFIX}${QDB_VERSION}")
endif()

if(NOT QDB_BUILD_REFERENCE)
    set(QDB_BUILD_REFERENCE "*** unknown build ***")
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        if(GIT_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} log -1 "--format=format:%h %ai"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE QDB_BUILD_REFERENCE)
        else()
            message(WARNING "Git not installed, can't find commit hash")
        endif()
    else()
        message(WARNING "Not a Git repository, can't find commit hash")
    endif()
endif()
