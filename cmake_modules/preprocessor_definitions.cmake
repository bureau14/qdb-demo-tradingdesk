add_compile_options(
    -DGTEST_LANG_CXX11=1
    -DBOOST_RESULT_OF_USE_DECLTYPE
    -DBOOST_SPIRIT_USE_PHOENIX_V3

    -DBOOST_MOVE_USE_STANDARD_LIBRARY_MOVE=1

    -DBOOST_ASIO_NO_THROW=1
    -DBOOST_ASIO_HAS_MOVE=1

    -DBOOST_THREAD_DONT_PROVIDE_DEPRECATED_FEATURES_SINCE_V3_0_0=1

    -DRAPIDJSON_HAS_STDSTRING=1
    -DRAPIDJSON_HAS_CXX11_RVALUE_REFS=1

    -DTBB_IMPLEMENT_CPP0X=0
    -DTBB_PREVIEW_MEMORY_POOL=1

    -DSODIUM_STATIC=1

    -DBOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS=1
    -DNAMESPACE_FOR_HASH_FUNCTIONS=farmhash

    $<$<NOT:$<CONFIG:Debug>>:-DNDEBUG>
    $<$<CONFIG:Debug>:-DDEBUG=1>
    $<$<CONFIG:Debug>:-D_DEBUG=1>
)

if (QDB_CPU_ARCH_IS_64BIT)
    add_definitions(/DQDB_ARCH_64BIT=1)
endif()

if (QDB_CPU_ARCH_IS_32BIT)
    add_definitions(/DQDB_ARCH_32BIT=1)
endif()

if(QDB_CPU_ARCHITECTURE_CORE2)
    add_compile_options(
        -DBOOST_SIMD_ASSUME_SSSE3
        -DQDB_DISABLE_SIMD=1
    )
else()
    add_compile_options(
        -DBOOST_SIMD_ASSUME_SSE4_2
    )
endif()

if(QDB_THROW_FROM_SIGHANDLER)
    add_compile_options(
      -DQDB_THROW_FROM_SIGHANDLER
    )
endif()

if(MSVC)
    add_definitions(
        /DBOOST_ALL_NO_LIB

        /D__TBB_NO_IMPLICIT_LINKAGE=1
        /D__TBBMALLOC_NO_IMPLICIT_LINKAGE=1

        /D_WINDOWS
        /DWIN32
        /DWIN32_LEAN_AND_MEAN
        /DVC_EXTRALEAN

        /DWINVER=0x0600
        /D_WIN32_WINNT=0x0600
        /DNTDDI_VERSION=0x06000000
        /DBOOST_USE_WINAPI_VERSION=0x0600

        /DPSAPI_VERSION=1
        /DNOMINMAX=1
        /D_CRT_SECURE_NO_WARNINGS
        /D_SCL_SECURE_NO_WARNINGS
        /D_SECURE_SCL=$<CONFIG:Debug>
        /D_HAS_ITERATOR_DEBUGGING=$<CONFIG:Debug>

        # error C2338: You've instantiated std::aligned_storage<Len, Align> with an extended alignment
        # (in other words, Align > alignof(max_align_t)).
        # Before VS 2017 15.8, the member type would non-conformingly have an alignment of only alignof(max_align_t).
        # VS 2017 15.8 was fixed to handle this correctly, but the fix inherently changes layout and breaks binary compatibility
        # (*only* for uses of aligned_storage with extended alignments).
        # Please define either (1) _ENABLE_EXTENDED_ALIGNED_STORAGE to acknowledge that you understand this message and that you actually want a type with an extended alignment,
        # or (2) _DISABLE_EXTENDED_ALIGNED_STORAGE to silence this message and get the old non-conformant behavior.
        /D_ENABLE_EXTENDED_ALIGNED_STORAGE
        /D_HAS_AUTO_PTR_ETC=1 # Boost 1.65.1 does not support this.
        /D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1
        /D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING=1
        /D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING=1
        /D_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING=1
    )
    if(${MSVC_VERSION} VERSION_LESS 1910)
        add_definitions(
            /D_ENABLE_ATOMIC_ALIGNMENT_FIX
        )
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -DPIC
    )
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(
        -DPIC
        $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG=1>
        -DBOOST_NO_AUTO_PTR
    )
endif()

add_compile_options(
    $<$<CONFIG:Debug>:-DQDB_BUILD_TYPE_DEBUG=1>
    $<$<CONFIG:Release>:-DQDB_BUILD_TYPE_RELEASE=1>
    $<$<CONFIG:RelWithDebInfo>:-DQDB_BUILD_TYPE_RELWITHDEBINFO=1>
    $<$<CONFIG:Sanitize>:-DQDB_BUILD_TYPE_SANITIZE=1>
    $<$<CONFIG:ASan>:-DQDB_BUILD_TYPE_ASAN=1>
    $<$<CONFIG:LSan>:-DQDB_BUILD_TYPE_LSAN=1>
    $<$<CONFIG:MSan>:-DQDB_BUILD_TYPE_MSAN=1>
    $<$<CONFIG:TSan>:-DQDB_BUILD_TYPE_TSAN=1>
    $<$<CONFIG:UBSan>:-DQDB_BUILD_TYPE_UBSAN=1>
    $<$<CONFIG:SanitizeValgrind>:-DQDB_BUILD_TYPE_SANITIZEVALGRIND=1>
)
