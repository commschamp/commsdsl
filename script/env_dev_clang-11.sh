#!/bin/bash

export CC=clang-11
export CXX=clang++-11

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${SCRIPT_DIR}/env_dev.sh "$@"
