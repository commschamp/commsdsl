#!/bin/bash

if [ -z "${CC}" -o -z "${CXX}" ]; then
    echo "ERROR: Compilers are not provided"
    exit 1
fi

if [ -z "${COMMON_BUILD_TYPE}" ]; then
    echo "ERROR: Build type is not provided"
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$( dirname ${SCRIPT_DIR} )
if [ -z "${BUILD_DIR}" ]; then
    BUILD_DIR="${ROOT_DIR}/build.${CC}.${COMMON_BUILD_TYPE}"
fi

CXX_STANDARD_PARAM=
if [ -n "${COMMON_CXX_STANDARD}" ]; then
    CXX_STANDARD_PARAM="-DCMAKE_CXX_STANDARD=${COMMON_CXX_STANDARD}"
fi

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=${COMMON_BUILD_TYPE} \
    -DCOMMSDSL_USE_CCACHE=ON -DCOMMSDSL_BUILD_UNIT_TESTS=ON -DCOMMSDSL_TEST_BUILD_DOC=ON \
    -DCOMMSDSL_BUILD_COMMSDSL2TEST=ON -DCOMMSDSL_BUILD_COMMSDSL2TOOLS_QT=ON -DCOMMSDSL_BUILD_COMMSDSL2SWIG=ON \
    -DCOMMSDSL_BUILD_COMMSDSL2EMSCRIPTEN=ON ${CXX_STANDARD_PARAM} "$@"

 
