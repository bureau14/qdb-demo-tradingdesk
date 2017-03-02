# are we compiling 32-bit or 64-bit?
if(MSVC AND NOT ${CMAKE_GENERATOR} MATCHES "Win64")
    set(QDB_CPU_ARCH "32bit")
    set(QDB_CPU_ARCH_IS_32BIT "YES")
else()
    set(QDB_CPU_ARCH "64bit")
    set(QDB_CPU_ARCH_IS_64BIT "YES")
endif()