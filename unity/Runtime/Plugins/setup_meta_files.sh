#!/bin/bash
# Setup Unity .meta files for native plugins (mobile & web)
# Run this after building the native libraries

set -e

PLUGINS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Setting up Unity .meta files for native plugins..."
echo "Plugins directory: $PLUGINS_DIR"
echo ""

# iOS
IOS_LIB="$PLUGINS_DIR/iOS/libspectra.a"
IOS_META_TEMPLATE="$PLUGINS_DIR/iOS/libspectra.a.meta.template"
IOS_META="$PLUGINS_DIR/iOS/libspectra.a.meta"

if [ -f "$IOS_LIB" ]; then
    if [ -f "$IOS_META_TEMPLATE" ]; then
        cp "$IOS_META_TEMPLATE" "$IOS_META"
        echo "✓ Created iOS meta file: $IOS_META"
    else
        echo "⚠ iOS template not found: $IOS_META_TEMPLATE"
    fi
else
    echo "⊘ iOS library not found (skip): $IOS_LIB"
fi

# Android arm64-v8a
ANDROID_ARM64_LIB="$PLUGINS_DIR/Android/libs/arm64-v8a/libspectra.so"
ANDROID_ARM64_META_TEMPLATE="$PLUGINS_DIR/Android/libs/arm64-v8a/libspectra.so.meta.template"
ANDROID_ARM64_META="$PLUGINS_DIR/Android/libs/arm64-v8a/libspectra.so.meta"

if [ -f "$ANDROID_ARM64_LIB" ]; then
    if [ -f "$ANDROID_ARM64_META_TEMPLATE" ]; then
        cp "$ANDROID_ARM64_META_TEMPLATE" "$ANDROID_ARM64_META"
        echo "✓ Created Android arm64-v8a meta file"
    else
        echo "⚠ Android arm64-v8a template not found"
    fi
else
    echo "⊘ Android arm64-v8a library not found (skip)"
fi

# Android armeabi-v7a
ANDROID_ARMV7_LIB="$PLUGINS_DIR/Android/libs/armeabi-v7a/libspectra.so"
ANDROID_ARMV7_META_TEMPLATE="$PLUGINS_DIR/Android/libs/armeabi-v7a/libspectra.so.meta.template"
ANDROID_ARMV7_META="$PLUGINS_DIR/Android/libs/armeabi-v7a/libspectra.so.meta"

if [ -f "$ANDROID_ARMV7_LIB" ]; then
    if [ -f "$ANDROID_ARMV7_META_TEMPLATE" ]; then
        cp "$ANDROID_ARMV7_META_TEMPLATE" "$ANDROID_ARMV7_META"
        echo "✓ Created Android armeabi-v7a meta file"
    else
        echo "⚠ Android armeabi-v7a template not found"
    fi
else
    echo "⊘ Android armeabi-v7a library not found (skip)"
fi

# Android x86_64
ANDROID_X64_LIB="$PLUGINS_DIR/Android/libs/x86_64/libspectra.so"
ANDROID_X64_META_TEMPLATE="$PLUGINS_DIR/Android/libs/x86_64/libspectra.so.meta.template"
ANDROID_X64_META="$PLUGINS_DIR/Android/libs/x86_64/libspectra.so.meta"

if [ -f "$ANDROID_X64_LIB" ]; then
    if [ -f "$ANDROID_X64_META_TEMPLATE" ]; then
        cp "$ANDROID_X64_META_TEMPLATE" "$ANDROID_X64_META"
        echo "✓ Created Android x86_64 meta file"
    else
        echo "⚠ Android x86_64 template not found"
    fi
else
    echo "⊘ Android x86_64 library not found (skip)"
fi

# WebGL WASM
WEBGL_WASM="$PLUGINS_DIR/WebGL/libspectra.wasm"
WEBGL_WASM_META_TEMPLATE="$PLUGINS_DIR/WebGL/libspectra.wasm.meta.template"
WEBGL_WASM_META="$PLUGINS_DIR/WebGL/libspectra.wasm.meta"

if [ -f "$WEBGL_WASM" ]; then
    if [ -f "$WEBGL_WASM_META_TEMPLATE" ]; then
        cp "$WEBGL_WASM_META_TEMPLATE" "$WEBGL_WASM_META"
        echo "✓ Created WebGL WASM meta file"
    else
        echo "⚠ WebGL WASM template not found"
    fi
else
    echo "⊘ WebGL WASM library not found (skip)"
fi

# WebGL JS
WEBGL_JS="$PLUGINS_DIR/WebGL/libspectra.js"
WEBGL_JS_META_TEMPLATE="$PLUGINS_DIR/WebGL/libspectra.js.meta.template"
WEBGL_JS_META="$PLUGINS_DIR/WebGL/libspectra.js.meta"

if [ -f "$WEBGL_JS" ]; then
    if [ -f "$WEBGL_JS_META_TEMPLATE" ]; then
        cp "$WEBGL_JS_META_TEMPLATE" "$WEBGL_JS_META"
        echo "✓ Created WebGL JS meta file"
    else
        echo "⚠ WebGL JS template not found"
    fi
else
    echo "⊘ WebGL JS library not found (skip)"
fi

echo ""
echo "Done! Unity will recognize these plugins on next project refresh."
echo ""
echo "Next steps:"
echo "  1. Open Unity project"
echo "  2. Wait for import/compilation"
echo "  3. Check Console for any import errors"
echo "  4. Verify plugin settings in Inspector"
