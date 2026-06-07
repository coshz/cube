#!/usr/bin/env bash
set -e

ROOT_DIR=$(cd "$(dirname "$0")/../" && pwd)
DIST_DIR="${1:-$ROOT_DIR/dist}"

mkdir -p "$DIST_DIR"

cd "$ROOT_DIR/wasm"

npm install && npm run build

PKG_FILE=$(npm pack)

mv "$PKG_FILE" "$DIST_DIR/"