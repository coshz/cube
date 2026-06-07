#!/usr/bin/env bash
set -e

export BUILD_SDIST=true
ROOT_DIR=$(cd "$(dirname "$0")/../" && pwd)

cd $ROOT_DIR/scripts

sh ./build_libcube.sh package
sh ./pack_jscube.sh
sh ./pack_pycube.sh