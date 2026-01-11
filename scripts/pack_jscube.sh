#!/usr/bin/env bash
set -e

ROOT_DIR=$(cd "$(dirname "$0")/../" && pwd)

cd "$ROOT_DIR/wasm"

npm install && npm run build

PKG_FILE=$(npm pack)

mv "$PKG_FILE" "$ROOT_DIR/dist/"