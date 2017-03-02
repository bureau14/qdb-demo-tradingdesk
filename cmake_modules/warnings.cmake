if(MSVC)
    # we get rid of the following warnings as they are most always false positive
    add_compile_options(
        /WX     # Treats all compiler warnings as errors
        /W4
        # Disable warnings:
        /wd4127 # conditional expression is constant
        /wd4244 # 'var': conversion from 'type' to 'type', possible loss of data
        /wd4503 # 'var': decorated name length exceeded, name was truncated
        /wd4505 # unreferenced local function has been removed
        /wd4456 # declaration of 'variable' hides previous local declaration
        /wd4459 # declaration of 'variable' hides global declaration
        /wd4592 # symbol will be dynamically initialized (implementation limitation)
        # Treat the following warnings as errors:
        /we4013 # 'function' undefined; assuming extern returning int
    )
endif()

if(CLANG)
    if (WIN32)
        set(C_CXX_FLAGS "-Xclang -pedantic -Xclang -Wno-language-extension-token")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_CXX_FLAGS}")
    else()
        add_compile_options(
            -pedantic
        )
    endif()

    add_compile_options(
        -Werror     # Treats all compiler warnings as errors
        -Wno-unknown-warning-option
        -Wall
        -Wno-char-subscripts # => using char for subscripts can be a problem?
        -Wno-delete-non-virtual-dtor # => otherwise we get a lot of error for shared_ptr
        -Wno-nested-anon-types
        -Wno-parentheses-equality # => no warning if there are too many parentheses
        -Wno-undefined-var-template # instantiation of variable 'var' required here, but no definition is available
        -Wno-unused-function
        -Wno-unused-variable # => seh_translator
    )

    if(NOT APPLE AND CLANG_35_OR_GREATER)
        add_compile_options(
            -Wno-tautological-undefined-compare # => 'this' can be actually null due to casting
        )
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    # we don't use -Wall because some -Wunused produce too many false positives
    # because the third party libraries we use simply don't pay attention to it (especially leveldb)
    add_compile_options(
        -Werror     # Treats all compiler warnings as errors
        -Waddress
        -Warray-bounds
        -Wchar-subscripts
        -Wcomment
        -Wenum-compare
        -Wformat=2
        -Wignored-qualifiers
        -Wmissing-braces
        -Wmissing-field-initializers
        -Wmissing-include-dirs
        -Wno-missing-include-dirs
        -Wparentheses
        -Wreturn-type
        -Wsequence-point
        -Wsign-compare
        -Wstrict-aliasing
        -Wstrict-overflow=1
        -Wswitch
        -Wtrigraphs
        -Wtype-limits
        -Wuninitialized
        -Wunused-but-set-parameter
        -Wunused-label
        -Wvolatile-register-var
    )

    # Necessary for GCC 5+
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-placement-new") # C++ only
endif()
