#!/bin/bash
# Build native library for all platforms

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
NATIVE_DIR="$PROJECT_ROOT/native"
OUTPUT_DIR="$PROJECT_ROOT/unity/Runtime/Plugins"

echo "Building SpectraCore native library..."
echo "Project root: $PROJECT_ROOT"

# Detect platform
case "$(uname -s)" in
    Linux*)     HOST_PLATFORM=Linux;;
    Darwin*)    HOST_PLATFORM=macOS;;
    MINGW*|MSYS*|CYGWIN*) HOST_PLATFORM=Windows;;
    *)          HOST_PLATFORM=Unknown;;
esac

echo "Host platform: $HOST_PLATFORM"

build_native() {
    local build_dir="$1"
    local cmake_args="$2"
    local output_subdir="$3"

    echo "Building: $build_dir"
    cd "$NATIVE_DIR"

    cmake -B "$build_dir" $cmake_args -DCMAKE_BUILD_TYPE=Release -DSPECTRA_BUILD_TESTS=OFF
    cmake --build "$build_dir" --config Release

    mkdir -p "$OUTPUT_DIR/$output_subdir"

    # Copy library
    if [ -d "$build_dir/lib" ]; then
        cp -v "$build_dir/lib/"* "$OUTPUT_DIR/$output_subdir/" 2>/dev/null || true
    fi
    if [ -d "$build_dir/Release" ]; then
        cp -v "$build_dir/Release/"* "$OUTPUT_DIR/$output_subdir/" 2>/dev/null || true
    fi
}

# Build for host platform
case "$HOST_PLATFORM" in
    Linux)
        build_native "build-linux" "" "Linux/x86_64"
        ;;
    macOS)
        build_native "build-macos" "-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64" "macOS"

        # Build for iOS if Xcode is available
        if command -v xcodebuild &> /dev/null; then
            echo "Building for iOS..."
            build_native "build-ios" "-DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=arm64" "iOS"
        fi
        ;;
    Windows)
        build_native "build-windows" "-A x64" "Windows/x86_64"
        ;;
esac

# Build for Android if NDK is available
if [ -n "$ANDROID_NDK_HOME" ] || [ -n "$ANDROID_NDK" ]; then
    NDK_PATH="${ANDROID_NDK_HOME:-$ANDROID_NDK}"
    TOOLCHAIN="$NDK_PATH/build/cmake/android.toolchain.cmake"

    if [ -f "$TOOLCHAIN" ]; then
        echo "Building for Android..."

        build_native "build-android-arm64" \
            "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24" \
            "Android/libs/arm64-v8a"

        build_native "build-android-armv7" \
            "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN -DANDROID_ABI=armeabi-v7a -DANDROID_PLATFORM=android-24" \
            "Android/libs/armeabi-v7a"
    fi
fi

echo "Build complete!"
echo "Output: $OUTPUT_DIR"
