#!/bin/sh -eux

if [[ $# -ne 1 ]] ; then
    >&2 echo "Usage: $0 <new_version>"
    exit 1
fi

INPUT_VERSION=$1; shift

MAJOR_VERSION=${INPUT_VERSION%%.*}
WITHOUT_MAJOR_VERSION=${INPUT_VERSION#${MAJOR_VERSION}.}
MINOR_VERSION=${WITHOUT_MAJOR_VERSION%%.*}
WITHOUT_MINOR_VERSION=${INPUT_VERSION#${MAJOR_VERSION}.${MINOR_VERSION}.}
PATCH_VERSION=${WITHOUT_MINOR_VERSION%%.*}

XYZ_VERSION="${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}"

cd $(dirname -- $0)
cd ${PWD}/../..

# set(QDB_VERSION_PREFIX "2.2.0") # the default
sed -i -e 's/set(QDB_VERSION_PREFIX *"[^"]*")/set(QDB_VERSION_PREFIX "'"${XYZ_VERSION}"'")/' cmake_modules/qdb_version.cmake
