#!/usr/bin/env bash
set -e
BUILD_SDIST=${BUILD_SDIST:false}

ROOT_DIR="$(dirname "$0")/.."
LIB_COLLECTION="$ROOT_DIR/dist/_LIB"
PY_LIB_DIR="$ROOT_DIR/python/pycube"

if [[ "$BUILD_SDIST" == "true" ]]; then 
    echo "Building sdist..."
    python -m build "$ROOT_DIR/python" --sdist --outdir "$ROOT_DIR/dist"
fi

rm -f "$PY_LIB_DIR"/*.so "$PY_LIB_DIR"/*.dylib "$PY_LIB_DIR"/*.dll

if [ -d "$LIB_COLLECTION" ] && [ "$(ls -A "$LIB_COLLECTION")" ]; then
    echo "Injecting library files from $LIB_COLLECTION into $PY_LIB_DIR..."
    cp "$LIB_COLLECTION"/* "$PY_LIB_DIR/"
else
    echo "Warning: No binaries found in $LIB_COLLECTION. Wheel will be empty of native libs!"
fi

python -m build "$ROOT_DIR/python" --wheel --outdir "$ROOT_DIR/dist"

rm -rf "$LIB_COLLECTION"