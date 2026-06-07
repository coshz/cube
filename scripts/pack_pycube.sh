#!/usr/bin/env bash
set -e

ROOT_DIR="$(dirname "$0")/.."
PY_SRC_DIR="$ROOT_DIR/python"

DIST_DIR="${1:-$ROOT_DIR/dist}"
BUILD_SDIST=${BUILD_SDIST:-false}

mkdir -p "$DIST_DIR"

export PYCUBE_LIB_DIR="${PYCUBE_LIB_DIR:-$ROOT_DIR/dist/_LIB}"

main() {
    if [[ "$BUILD_SDIST" == "true" ]]; then 
        echo "Building sdist..."
        python -m build "$PY_SRC_DIR/" --sdist --outdir "$DIST_DIR/"
    fi

    echo "Building wheel (using binaries from $PYCUBE_LIB_DIR)"
    python -m build "$ROOT_DIR/python" --wheel --outdir "$DIST_DIR/"

    echo "pycube build completed successfully!"
}

main "$@"