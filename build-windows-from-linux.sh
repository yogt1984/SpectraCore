#!/bin/bash
# Build SpectraCore for Windows from Linux using MinGW cross-compiler

set -e  # Exit on error

echo "================================================"
echo "Building SpectraCore for Windows (from Linux)"
echo "================================================"

# Check if MinGW is installed
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "ERROR: MinGW-w64 not installed"
    echo ""
    echo "Install with:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install mingw-w64 cmake"
    exit 1
fi

echo "✓ MinGW-w64 found: $(x86_64-w64-mingw32-g++ --version | head -1)"

# Navigate to native directory
cd "$(dirname "$0")/native"

# Create build directory
BUILD_DIR="build-windows"
echo ""
echo "Creating build directory: $BUILD_DIR"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake with MinGW toolchain
echo ""
echo "Configuring CMake for Windows cross-compilation..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_VERBOSE_MAKEFILE=ON

# Build
echo ""
echo "Building native library..."
cmake --build . --config Release -j$(nproc)

# Check if DLL was created
if [ -f "lib/spectra.dll" ]; then
    echo ""
    echo "================================================"
    echo "✓ SUCCESS: Windows DLL built successfully"
    echo "================================================"
    echo "Output: $(pwd)/lib/spectra.dll"
    ls -lh lib/spectra.dll

    # Copy to Unity Plugins
    UNITY_PLUGINS="../unity/Runtime/Plugins/Windows/x86_64"
    mkdir -p "$UNITY_PLUGINS"
    cp lib/spectra.dll "$UNITY_PLUGINS/"
    echo ""
    echo "✓ Copied to: $UNITY_PLUGINS/spectra.dll"

    echo ""
    echo "NEXT STEPS:"
    echo "1. Transfer spectra.dll to a Windows machine"
    echo "2. Place it in Unity project: Assets/Plugins/Windows/x86_64/"
    echo "3. Run Unity Editor on Windows"
    echo "4. Run the test suite: Unity Test Runner → Edit Mode Tests"
else
    echo ""
    echo "================================================"
    echo "✗ ERROR: DLL was not created"
    echo "================================================"
    exit 1
fi
