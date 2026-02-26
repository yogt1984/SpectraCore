#!/bin/bash
# Build script for Android platform
# Requires: Android NDK installed and NDK environment variable set

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Configuration
ABI="${ABI:-arm64-v8a}"  # arm64-v8a, armeabi-v7a, x86_64, x86
API_LEVEL="${API_LEVEL:-21}"  # Android 5.0 minimum
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="$PROJECT_ROOT/build-android-${ABI}"

echo "=================================================="
echo "Building SpectraCore for Android"
echo "=================================================="
echo "ABI: $ABI"
echo "API Level: $API_LEVEL"
echo "Build Type: $BUILD_TYPE"
echo "Build Directory: $BUILD_DIR"
echo "=================================================="

# Check for Android NDK
if [ -z "$ANDROID_NDK" ] && [ -z "$NDK" ]; then
    echo "ERROR: Android NDK not found"
    echo "Set ANDROID_NDK or NDK environment variable to NDK root directory"
    echo ""
    echo "Example:"
    echo "  export ANDROID_NDK=/path/to/android-ndk-r26b"
    echo ""
    echo "Download from: https://developer.android.com/ndk/downloads"
    exit 1
fi

# Use NDK variable if ANDROID_NDK not set
if [ -z "$ANDROID_NDK" ]; then
    export ANDROID_NDK="$NDK"
fi

# Verify NDK toolchain file exists
NDK_TOOLCHAIN="$ANDROID_NDK/build/cmake/android.toolchain.cmake"
if [ ! -f "$NDK_TOOLCHAIN" ]; then
    echo "ERROR: NDK toolchain file not found at: $NDK_TOOLCHAIN"
    echo "Check ANDROID_NDK path: $ANDROID_NDK"
    exit 1
fi

echo ""
echo "Using NDK: $ANDROID_NDK"

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure with CMake
echo ""
echo "Configuring CMake for Android ($ABI)..."
cmake -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_TOOLCHAIN" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="android-$API_LEVEL" \
    -DANDROID_STL="c++_shared" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DSPECTRA_BUILD_TESTS=OFF \
    -DSPECTRA_BUILD_BENCHMARKS=OFF

# Build
echo ""
echo "Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

# Output location
SHARED_LIB="$BUILD_DIR/lib/libspectra.so"

if [ -f "$SHARED_LIB" ]; then
    echo ""
    echo "=================================================="
    echo "Build successful!"
    echo "Output: $SHARED_LIB"
    ls -lh "$SHARED_LIB"
    echo "=================================================="

    # Show binary info
    if command -v file &> /dev/null; then
        echo ""
        echo "Binary info:"
        file "$SHARED_LIB"
    fi

    # Copy to Unity Plugins directory
    UNITY_PLUGIN_DIR="$PROJECT_ROOT/../unity/Runtime/Plugins/Android/libs/$ABI"
    mkdir -p "$UNITY_PLUGIN_DIR"
    cp "$SHARED_LIB" "$UNITY_PLUGIN_DIR/"
    echo ""
    echo "Copied to Unity: $UNITY_PLUGIN_DIR/libspectra.so"
else
    echo ""
    echo "ERROR: Build failed - output file not found"
    exit 1
fi

echo ""
echo "Done!"
echo ""
echo "To build for other ABIs, run:"
echo "  ABI=armeabi-v7a ./scripts/build_android.sh"
echo "  ABI=x86_64 ./scripts/build_android.sh"
