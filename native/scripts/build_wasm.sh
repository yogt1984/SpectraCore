#!/bin/bash
# Build script for WebAssembly/WASM platform using Emscripten
# Requires: Emscripten SDK installed and activated

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Configuration
BUILD_TYPE="${BUILD_TYPE:-MinSizeRel}"  # MinSizeRel, Release, Debug
BUILD_DIR="$PROJECT_ROOT/build-wasm"

echo "=================================================="
echo "Building SpectraCore for WebAssembly"
echo "=================================================="
echo "Build Type: $BUILD_TYPE"
echo "Build Directory: $BUILD_DIR"
echo "=================================================="

# Check for Emscripten
if ! command -v emcc &> /dev/null; then
    echo "ERROR: Emscripten not found"
    echo ""
    echo "Install Emscripten SDK:"
    echo "  git clone https://github.com/emscripten-core/emsdk.git"
    echo "  cd emsdk"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    echo "  source ./emsdk_env.sh"
    echo ""
    echo "Then run this script again."
    exit 1
fi

# Show Emscripten version
echo ""
echo "Emscripten version:"
emcc --version | head -1

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure with Emscripten CMake wrapper
echo ""
echo "Configuring CMake for WebAssembly..."
emcmake cmake -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/wasm.toolchain.cmake" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DSPECTRA_BUILD_TESTS=OFF \
    -DSPECTRA_BUILD_BENCHMARKS=OFF

# Build
echo ""
echo "Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

# Output location
WASM_LIB="$BUILD_DIR/lib/libspectra.wasm"
JS_GLUE="$BUILD_DIR/lib/libspectra.js"

if [ -f "$WASM_LIB" ]; then
    echo ""
    echo "=================================================="
    echo "Build successful!"
    echo "=================================================="
    ls -lh "$WASM_LIB"

    if [ -f "$JS_GLUE" ]; then
        ls -lh "$JS_GLUE"
    fi

    echo "=================================================="

    # Show sizes
    echo ""
    echo "File sizes:"
    WASM_SIZE=$(stat -f%z "$WASM_LIB" 2>/dev/null || stat -c%s "$WASM_LIB" 2>/dev/null || echo "unknown")
    echo "  WASM: $WASM_SIZE bytes (~$((WASM_SIZE / 1024)) KB)"

    if [ -f "$JS_GLUE" ]; then
        JS_SIZE=$(stat -f%z "$JS_GLUE" 2>/dev/null || stat -c%s "$JS_GLUE" 2>/dev/null || echo "unknown")
        echo "  JS:   $JS_SIZE bytes (~$((JS_SIZE / 1024)) KB)"
    fi

    # Estimate compressed size (gzip typically achieves 3-4× compression for WASM)
    if [ "$WASM_SIZE" != "unknown" ]; then
        COMPRESSED_EST=$((WASM_SIZE / 3))
        echo ""
        echo "Estimated gzip size: ~$((COMPRESSED_EST / 1024)) KB"
    fi

    # Copy to Unity Plugins directory
    UNITY_PLUGIN_DIR="$PROJECT_ROOT/../unity/Runtime/Plugins/WebGL"
    mkdir -p "$UNITY_PLUGIN_DIR"
    cp "$WASM_LIB" "$UNITY_PLUGIN_DIR/"

    if [ -f "$JS_GLUE" ]; then
        cp "$JS_GLUE" "$UNITY_PLUGIN_DIR/"
    fi

    echo ""
    echo "Copied to Unity: $UNITY_PLUGIN_DIR/"
else
    echo ""
    echo "ERROR: Build failed - output file not found"
    echo "Expected: $WASM_LIB"
    exit 1
fi

echo ""
echo "Next steps:"
echo "  1. Run setup_meta_files.sh to create Unity .meta files"
echo "  2. Open Unity project and verify WebGL plugin import"
echo "  3. Build Unity project for WebGL target"
echo "  4. Test in browser (Chrome, Firefox, Safari, Edge)"
echo ""
echo "Build profiles:"
echo "  BUILD_TYPE=MinSizeRel ./scripts/build_wasm.sh  # Smallest size (~200KB)"
echo "  BUILD_TYPE=Release ./scripts/build_wasm.sh     # Balanced (~300KB)"
echo "  BUILD_TYPE=Debug ./scripts/build_wasm.sh       # Debug symbols (~800KB)"
echo ""
echo "Done!"
