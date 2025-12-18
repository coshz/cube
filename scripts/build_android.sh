ANDROID_ABI="arm64-v8a"
ANDROID_PLATFORM="android-26"
NDK_ROOT="~/Library/Android/sdk/ndk/25.1.8937393"
ROOT_DIR="$(cd $(dirname $(dirname $0)) && pwd)"
BUILD_DIR="$ROOT_DIR/build/android/${ANDROID_ABI}"

if [ ! -d "${BUILD_DIR}" ]; then 
    mkdir -p "${BUILD_DIR}"
fi

cmake \
    -DCMAKE_TOOLCHAIN_FILE="${NDK_ROOT}/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="${ANDROID_ABI}" \
    -DANDROID_PLATFORM="${ANDROID_PLATFORM}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DANDROID_STL=c++_shared \
    -S "${ROOT_DIR}" -B "${BUILD_DIR}"

cmake --build "${BUILD_DIR}" --target cube
