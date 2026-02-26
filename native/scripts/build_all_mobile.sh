#!/bin/bash
# Build script for all mobile platforms
# Builds: iOS (device + simulator), Android (arm64-v8a + armeabi-v7a)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=================================================="
echo "Building SpectraCore for ALL Mobile Platforms"
echo "=================================================="

# Check platform
if [[ "$OSTYPE" == "darwin"* ]]; then
    HAS_MACOS=true
else
    HAS_MACOS=false
fi

# iOS builds (macOS only)
if [ "$HAS_MACOS" = true ]; then
    echo ""
    echo ">>> Building iOS (Device - arm64)"
    PLATFORM=OS64 "$SCRIPT_DIR/build_ios.sh"

    echo ""
    echo ">>> Building iOS (Simulator - arm64)"
    PLATFORM=SIMULATORARM64 "$SCRIPT_DIR/build_ios.sh"
else
    echo ""
    echo "SKIPPED: iOS builds (requires macOS)"
fi

# Android builds (Linux, macOS, Windows with WSL)
if [ -n "$ANDROID_NDK" ] || [ -n "$NDK" ]; then
    echo ""
    echo ">>> Building Android (arm64-v8a)"
    ABI=arm64-v8a "$SCRIPT_DIR/build_android.sh"

    echo ""
    echo ">>> Building Android (armeabi-v7a)"
    ABI=armeabi-v7a "$SCRIPT_DIR/build_android.sh"

    # Optional: x86_64 for emulator
    # echo ""
    # echo ">>> Building Android (x86_64 - emulator)"
    # ABI=x86_64 "$SCRIPT_DIR/build_android.sh"
else
    echo ""
    echo "SKIPPED: Android builds (ANDROID_NDK not set)"
    echo "Download NDK: https://developer.android.com/ndk/downloads"
    echo "Then: export ANDROID_NDK=/path/to/ndk"
fi

echo ""
echo "=================================================="
echo "Mobile builds complete!"
echo "=================================================="
echo ""
echo "Output locations:"
echo "  iOS:     unity/Runtime/Plugins/iOS/libspectra.a"
echo "  Android: unity/Runtime/Plugins/Android/libs/{abi}/libspectra.so"
echo ""
echo "Next steps:"
echo "  1. Create Unity .meta files for platform import settings"
echo "  2. Test in Unity Editor"
echo "  3. Deploy to physical device for testing"
