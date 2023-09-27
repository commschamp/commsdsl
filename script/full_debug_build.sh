#!/bin/bash

if [ -z "${CC}" -o -z "$CXX" ]; then
    echo "ERROR: Compilers are not provided"
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$( dirname ${SCRIPT_DIR} )
export BUILD_DIR="${ROOT_DIR}/build.full.${CC}"
export COMMON_INSTALL_DIR=${BUILD_DIR}/install
export COMMON_BUILD_TYPE=Debug
export EXTERNALS_DIR=${ROOT_DIR}/externals
mkdir -p ${BUILD_DIR}

${SCRIPT_DIR}/prepare_externals.sh

cd ${BUILD_DIR}
cmake .. -DCMAKE_PREFIX_PATH=${COMMON_INSTALL_DIR} \
    -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug -DCOMMSDSL_BUILD_UNIT_TESTS=ON -DCOMMSDSL_TEST_BUILD_DOC=ON \
    -DCOMMSDSL_BUILD_COMMSDSL2TEST=ON -DCOMMSDSL_BUILD_COMMSDSL2TOOLS_QT=ON -DCOMMSDSL_BUILD_COMMSDSL2SWIG=ON \
    -DCOMMSDSL_BUILD_COMMSDSL2EMSCRIPTEN=ON "$@"

procs=$(nproc)
if [ -n "${procs}" ]; then
    procs_param="--parallel ${procs}"
fi

cmake --build ${BUILD_DIR} --config ${COMMON_BUILD_TYPE} --target install ${procs_param}
