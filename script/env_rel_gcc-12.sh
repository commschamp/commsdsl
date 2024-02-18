#!/bin/bash

export CC=gcc-12
export CXX=g++-12
export COMMON_BUILD_TYPE=Release

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
${SCRIPT_DIR}/env_dev.sh "$@"
