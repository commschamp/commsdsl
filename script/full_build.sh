#!/bin/bash

if [ -z "${CC}" -o -z "$CXX" ]; then
    echo "ERROR: Compilers are not provided"
    exit 1
fi

if [ -z "${COMMON_BUILD_TYPE}" ]; then
    echo "ERROR: Build type is not provided"
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$( dirname ${SCRIPT_DIR} )
export BUILD_DIR="${ROOT_DIR}/build.full.${CC}.${COMMON_BUILD_TYPE}"
export COMMON_INSTALL_DIR=${BUILD_DIR}/install
export EXTERNALS_DIR=${ROOT_DIR}/externals
export COMMON_USE_CCACHE=ON
mkdir -p ${BUILD_DIR}

if [ -z "${COMMON_CXX_STANDARD}" ]; then
    export COMMON_CXX_STANDARD="17"
fi

${SCRIPT_DIR}/prepare_externals.sh 
${SCRIPT_DIR}/env_dev.sh -DCMAKE_PREFIX_PATH=${COMMON_INSTALL_DIR} "$@"

procs=$(nproc)
if [ -n "${procs}" ]; then
    procs_param="--parallel ${procs}"
fi

cmake --build ${BUILD_DIR} --config ${COMMON_BUILD_TYPE} --target install ${procs_param}
