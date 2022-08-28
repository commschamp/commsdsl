#!/bin/bash

if [ -z "${CC}" -o -z "$CXX" ]; then
    echo "ERROR: Compilers are not provided"
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$( dirname ${SCRIPT_DIR} )
BUILD_DIR="${ROOT_DIR}/build.${CC}"
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug -DCOMMSDSL_BUILD_UNIT_TESTS=ON -DCOMMSDSL_TEST_BUILD_DOC=ON \
    -DCOMMSDSL_BUILD_COMMSDSL2TEST=ON -DCOMMSDSL_BUILD_COMMSDSL2TOOLS_QT=ON -DCOMMSDSL_BUILD_COMMSDSL2SWIG=ON "$@"
