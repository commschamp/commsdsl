#!/bin/bash

export CC=clang
export CXX=clang++
export COMMON_BUILD_TYPE=Release

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
${SCRIPT_DIR}/env_dev.sh "$@"
