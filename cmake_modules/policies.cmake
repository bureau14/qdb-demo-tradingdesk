if(POLICY CMP0066)
    cmake_policy(SET CMP0066 NEW) # Honor per-config flags in try_compile() source-file signature.
endif()

if(POLICY CMP0067)
    cmake_policy(SET CMP0067 NEW) # Honor language standard in try_compile() source-file signature.
endif()
