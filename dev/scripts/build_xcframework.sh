#!/usr/bin/env bash
set -e

ROOT=$(git rev-parse --show-toplevel)
cd "$ROOT" || exit 1

NAME="Cube"
VERSION=$(cut -d'-' -f1 "$ROOT/VERSION")

CUBE_CPP="amalg/cube_amalg.min.cpp"
SWIFT_DIR="bindings/swift"
SWIFT_SRC="$SWIFT_DIR/Cube.swift"
MODULEMAP="$SWIFT_DIR/module.modulemap"
APINOTES="$SWIFT_DIR/Cube.apinotes"
HEADERS_DIR="include/cube"

MIN_MACOS="10.15"
MIN_IOS="13.0"

BUILD_DIR="build-xcframework"
DIST_DIR="dist"

echo "==> Generating headers via CMake..."
cmake -G Ninja -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release > /dev/null

GEN_INCLUDE_DIR="$BUILD_DIR/include/cube"
FLAT_HEADERS="$BUILD_DIR/Headers"
rm -rf "$FLAT_HEADERS" && mkdir -p "$FLAT_HEADERS"
cp "$GEN_INCLUDE_DIR"/*.h "$FLAT_HEADERS/" 2>/dev/null || true

build_platform() {
    local platform=$1
    local archs=$2
    local minver=$3
    local os=$4
    local sim=$5

    local build_dir="$BUILD_DIR/${platform}"
    local fw_dir="$build_dir/${NAME}.framework"
    local ver_dir="$fw_dir/Versions/A"

    echo "==> Building $NAME.framework for $platform ($archs)"
    rm -rf "$fw_dir"
    mkdir -p "$ver_dir/Headers" "$ver_dir/Modules"
    mkdir -p "$ver_dir/Modules/$NAME.swiftmodule"

    # copy headers, modulemap and apinotes
    cp "$FLAT_HEADERS"/*.h "$ver_dir/Headers/"
    cp "$MODULEMAP" "$ver_dir/Modules/module.modulemap"
    if [ -f "$APINOTES" ]; then
        cp "$APINOTES" "$ver_dir/Headers/$NAME.apinotes"
    else
        echo "  ->Warning: No apinotes found, skipping."
    fi

    # create symlinks
    ln -sf A "$fw_dir/Versions/Current"
    ln -sf Versions/Current/Headers "$fw_dir/Headers"
    ln -sf Versions/Current/Modules "$fw_dir/Modules"

    local sdk_path=$(xcrun --sdk $platform --show-sdk-path)
    local slice_bins=()

    IFS=';' read -ra ARCH_ARRAY <<< "$archs"
    for arch in "${ARCH_ARRAY[@]}"; do
        local target_triple="${arch}-apple-${os}${minver}${sim}"
        local module_name="${arch}-apple-${os}${sim}"
        local slice_obj="$build_dir/${NAME}_${arch}.o"
        local slice_bin="$build_dir/${NAME}_${arch}.slice"

        echo "  -> Compiling C++ for $target_triple"
        xcrun --sdk $platform clang++ \
            -c -std=c++17 -O3 -fPIC \
            -target $target_triple  \
            -o "$slice_obj" \
            "$CUBE_CPP"

        echo "  -> Compiling Swift for $target_triple"
        xcrun --sdk $platform swiftc \
            -emit-library -emit-module -enable-library-evolution \
            -target $target_triple \
            -emit-module-path "$ver_dir/Modules/${NAME}.swiftmodule/${module_name}.swiftmodule" \
            -emit-module-interface-path "$ver_dir/Modules/${NAME}.swiftmodule/${module_name}.swiftinterface" \
            -import-underlying-module -module-name "$NAME" \
            -sdk "$sdk_path" -F "$fw_dir/.." \
            -Xlinker -install_name -Xlinker "@rpath/${NAME}.framework/${NAME}" \
            -lc++ \
            -o "$slice_bin" \
            "$SWIFT_SRC" "$slice_obj"

        slice_bins+=("$slice_bin")
    done

    # Lipo merging
    echo "  -> Lipo merging for $platform"
    if [ ${#slice_bins[@]} -eq 1 ]; then
        mv "${slice_bins[0]}" "$ver_dir/$NAME"
    else
        xcrun lipo -create "${slice_bins[@]}" -output "$ver_dir/$NAME"
    fi
    ln -sf Versions/Current/$NAME "$fw_dir/$NAME"
    rm -f $slice_bins
}

# ==============================================================================
# Build Multi-Platform XCFramework
# ==============================================================================
mkdir -p "$DIST_DIR"

build_platform "macosx"          "arm64;x86_64" "$MIN_MACOS" "macos" ""
build_platform "iphoneos"        "arm64"        "$MIN_IOS"   "ios"   ""
build_platform "iphonesimulator" "arm64;x86_64" "$MIN_IOS"   "ios"   "-simulator"

XCFRAMEWORK_DIR="$BUILD_DIR/${NAME}.xcframework"
rm -rf "$XCFRAMEWORK_DIR"
echo "==> Creating XCFramework at $XCFRAMEWORK_DIR..."
xcodebuild -create-xcframework \
    -framework "$BUILD_DIR/macosx/${NAME}.framework" \
    -framework "$BUILD_DIR/iphoneos/${NAME}.framework" \
    -framework "$BUILD_DIR/iphonesimulator/${NAME}.framework" \
    -output "$XCFRAMEWORK_DIR"

# ==============================================================================
# Package
# ==============================================================================
ZIP_FILE="$DIST_DIR/${NAME}.xcframework.zip"
echo "==> Packaging XCFramework..."
rm -f "$ZIP_FILE" && mkdir -p "$DIST_DIR"
ditto -c -k --sequesterRsrc --keepParent "$XCFRAMEWORK_DIR" "$ZIP_FILE"
echo "All done! Output: $ZIP_FILE"
