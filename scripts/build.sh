#!/bin/bash
set -e 

ROOT_DIR="$(cd $(dirname $(dirname $0)) && pwd)"
BUILD_DIR="$ROOT_DIR/build"
INSTALL_PREFIX="${INSTALL_PREFIX:-$ROOT_DIR/dist}"

# Re-build and Install project
build_and_install() {
    # 1. Clean the build directory to ensure a fresh build
    if [ -d "$BUILD_DIR" ]; then
        echo "Removing previous build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    fi
    
    # 2. Configure CMake
    echo "Configuring CMake build into $BUILD_DIR..."
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
        -S $ROOT_DIR -B $BUILD_DIR
    
    # Check if cmake succeeded
    if [ ! -d "$BUILD_DIR" ]; then
        echo "Error: CMake configuration failed. Exiting."
        exit 1
    fi
    
    cd "$BUILD_DIR"
    
    # 3. Build the project
    # Use -j$(nproc) for parallel compilation to speed up the build
    echo "Starting build..."
    make -j4
    
    # 4. Install the artifacts
    echo "Coping files to: $INSTALL_PREFIX"
    make install
    
    cd .. # Return to the project root
    echo "Artifacts available in: $INSTALL_PREFIX"
}

main() {
    if [ $# -ne 0 ]; then 
        INSTALL_PREFIX="$1" 
        if [ -e "$INSTALL_PREFIX" ] && [ ! -d "$INSTALL_PREFIX" ]; then
            echo "Error: '$INSTALL_PREFIX' exists but is not a directory."
            exit 1
        fi
        if [ ! -e "$INSTALL_PREFIX" ]; then
            mkdir -p "$INSTALL_PREFIX" || {
                echo "Error: Failed to create directory '$INSTALL_PREFIX'."
                exit 1
            }
        fi
    fi
    build_and_install
}

main "$@"