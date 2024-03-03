#!/bin/bash

export CC=gcc
export CXX=g++
export COMMON_BUILD_TYPE=Debug

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${SCRIPT_DIR}/env_dev.sh "$@"
