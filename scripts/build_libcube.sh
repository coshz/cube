#!/usr/bin/env bash
set -e

PLATFORM_TAG="${PLATFORM_TAG:-$(uname -s | tr '[:upper:]' '[:lower:]')}"
ROOT_DIR=$(cd "$(dirname "$0")/../" && pwd)
BUILD_DIR="$ROOT_DIR/build"
DIST_DIR="$ROOT_DIR/dist"
VERSION="$(cat $ROOT_DIR/VERSION)"
ARCHIVE="cube.sdk.${PLATFORM_TAG}-$VERSION"
LIB_COLLECTION="$DIST_DIR/_LIB" # for pycube
INSTALL_PREFIX="${INSTALL_PREFIX:-$DIST_DIR}"

build_all() {
    rm -rf "$BUILD_DIR" && mkdir -p "$BUILD_DIR"
    
    echo "Configuring CMake build into $BUILD_DIR..."
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
        -S $ROOT_DIR -B $BUILD_DIR

    echo "Starting build..."
    cmake --build $BUILD_DIR --config Release --parallel
}

install() {
    build_all

    local PREFIX="${1:-$INSTALL_PREFIX}"
    mkdir -p "$PREFIX"

    #  Install the artifacts
    echo "Coping files to: $PREFIX"
    cmake --install $BUILD_DIR --prefix "$PREFIX"
}

package() {
    build_all

    local PACK_DIR="${1:-$DIST_DIR}"
    local STAGING="$PACK_DIR/$ARCHIVE"

    mkdir -p "$STAGING"
    mkdir -p "$LIB_COLLECTION"

    cmake --install $BUILD_DIR --prefix "$STAGING"

    CANDIDATES=(
        "$STAGING/lib"/libcube.so
        "$STAGING/lib"/libcube.dylib
        "$STAGING/bin"/cube.dll
    )
    LIB_TO_COPY=()
    for file in "${CANDIDATES[@]}"; do
        if [ -f "$file" ]; then
            LIB_TO_COPY+=("$file")
        fi
    done 
    if [ ${#LIB_TO_COPY[@]} -eq 0 ]; then
        echo "ERROR: no libraries found in staging!"
        exit 1
    fi
    echo "Collecting libraries ${LIB_TO_COPY[*]} into $LIB_COLLECTION..."
    cp -L ${LIB_TO_COPY[@]} "$LIB_COLLECTION/"

    echo "Packaging SDK into $PACK_DIR/${ARCHIVE}.tgz ..." 
    tar -czf "$PACK_DIR/${ARCHIVE}.tgz" -C "$STAGING" .

    echo "Cleaning staging directory: $STAGING" 
    rm -rf "$STAGING"
}

clean() {
    rm -rf "$BUILD_DIR"
    rm -rf "$DIST_DIR"
}

main() {
    case "$1" in 
        install)    install "$2" ;;
        package)    package "$2" ;;
        clean)      clean ;;
        *) 
            echo "Usage: $0 [install|package|clean] [prefix]"
            exit 1 ;;
    esac
}

main "$@"