#!/bin/bash
# Build script for iOS platform
# Requires: macOS with Xcode installed

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Configuration
PLATFORM="${PLATFORM:-OS64}"  # OS64, SIMULATOR64, SIMULATORARM64
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="$PROJECT_ROOT/build-ios-${PLATFORM}"

echo "=================================================="
echo "Building SpectraCore for iOS"
echo "=================================================="
echo "Platform: $PLATFORM"
echo "Build Type: $BUILD_TYPE"
echo "Build Directory: $BUILD_DIR"
echo "=================================================="

# Check if on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "ERROR: iOS builds require macOS with Xcode"
    exit 1
fi

# Check for Xcode
if ! command -v xcodebuild &> /dev/null; then
    echo "ERROR: Xcode not found. Install from Mac App Store."
    exit 1
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure with CMake
echo ""
echo "Configuring CMake for iOS ($PLATFORM)..."
cmake -B "$BUILD_DIR" -G Xcode \
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/ios.toolchain.cmake" \
    -DPLATFORM="$PLATFORM" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DSPECTRA_BUILD_TESTS=OFF \
    -DSPECTRA_BUILD_BENCHMARKS=OFF

# Build
echo ""
echo "Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -- -quiet

# Output location
STATIC_LIB="$BUILD_DIR/lib/$BUILD_TYPE/libspectra.a"

if [ -f "$STATIC_LIB" ]; then
    echo ""
    echo "=================================================="
    echo "Build successful!"
    echo "Output: $STATIC_LIB"
    ls -lh "$STATIC_LIB"
    echo "=================================================="

    # Show architecture
    echo ""
    echo "Architecture info:"
    lipo -info "$STATIC_LIB"

    # Copy to Unity Plugins directory
    UNITY_PLUGIN_DIR="$PROJECT_ROOT/../unity/Runtime/Plugins/iOS"
    mkdir -p "$UNITY_PLUGIN_DIR"
    cp "$STATIC_LIB" "$UNITY_PLUGIN_DIR/"
    echo ""
    echo "Copied to Unity: $UNITY_PLUGIN_DIR/libspectra.a"
else
    echo ""
    echo "ERROR: Build failed - output file not found"
    exit 1
fi

echo ""
echo "Done!"
