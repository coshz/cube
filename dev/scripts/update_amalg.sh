#!/usr/bin/env bash
set -euo pipefail

FORCE=false
TARGET_VERSION=""
ROOT=$(git rev-parse --show-toplevel)

while getopts "fv:h" opt; do
  case $opt in
    f) FORCE=true ;;
    v) TARGET_VERSION="$OPTARG" ;;
    h|*) echo "Usage: $0 [-f] [-v <version>]"; exit 1 ;;
  esac
done

if [ -z "$TARGET_VERSION" ]; then
    TARGET_VERSION=$(cat "$ROOT/VERSION" | xargs)
fi

AMALG_CPP="$ROOT/amalg/cube_amalg.cpp"
CURRENT_VERSION=""

if [ -f "$AMALG_CPP" ]; then
    CURRENT_VERSION=$(grep -m1 "Version:" "$AMALG_CPP" | sed 's/.*Version:[[:space:]]*//' | xargs || true)
fi

if [ "$FORCE" = true ] || [ "$CURRENT_VERSION" != "$TARGET_VERSION" ] || [ ! -f "$AMALG_CPP" ]; then
    echo "🚀 Updating amalgamation to version $TARGET_VERSION..."
    rm -f "$ROOT/amalg"/*
    make -f "$ROOT/dev/scripts/mk/gen_amalg.mk" VERSION="$TARGET_VERSION"
    echo "✅ Amalgamation updated."
else
    echo "⏭️ Amalgamation is already at version $TARGET_VERSION. Skipping."
fi
