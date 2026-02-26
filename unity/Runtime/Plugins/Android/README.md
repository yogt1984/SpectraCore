# Android Native Plugin

This directory contains native Android libraries for SpectraCore across multiple ABIs.

## Building

From the `native/` directory, run:

```bash
# Build for arm64-v8a (64-bit ARM, most modern devices)
ABI=arm64-v8a ./scripts/build_android.sh

# Build for armeabi-v7a (32-bit ARM, legacy support)
ABI=armeabi-v7a ./scripts/build_android.sh

# Build for x86_64 (Android emulator)
ABI=x86_64 ./scripts/build_android.sh

# Or build all at once
./scripts/build_all_mobile.sh
```

## Requirements

- **Android NDK** r21 or later
- Set environment variable: `export ANDROID_NDK=/path/to/ndk`
- Download: https://developer.android.com/ndk/downloads

## Output Structure

```
Android/
└── libs/
    ├── arm64-v8a/
    │   └── libspectra.so
    ├── armeabi-v7a/
    │   └── libspectra.so
    └── x86_64/
        └── libspectra.so
```

## ABI Support

| ABI | Architecture | Support |
|-----|--------------|---------|
| **arm64-v8a** | 64-bit ARM | Primary (2020+ devices) |
| **armeabi-v7a** | 32-bit ARM | Legacy (wider compatibility) |
| **x86_64** | 64-bit Intel | Emulator only |
| **x86** | 32-bit Intel | Legacy emulator (optional) |

**Recommendation:** Build arm64-v8a + armeabi-v7a for production.

## Unity Import Settings

Each `.so` file should have a `.meta` file with:

- **Platform:** Android
- **CPU:** Specific to ABI (ARM64, ARMv7, etc.)
- **Plugin Type:** Native (.so)

## Testing

1. Build for desired ABIs
2. Open Unity project
3. File → Build Settings → Android
4. Build and run on physical device or emulator
5. Test filter creation and audio processing

## Platform-Specific Notes

- **Minimum API Level:** Android 5.0 (API 21)
- **STL:** c++_shared (included automatically)
- **NEON:** Enabled for armeabi-v7a (ARM SIMD)
- **Dynamic Library:** Android uses .so (shared object), not .a

## Troubleshooting

**NDK not found:**
```bash
export ANDROID_NDK=/path/to/android-ndk-r26b
# Or
export NDK=/path/to/android-ndk-r26b
```

**Build errors:**
- Verify NDK version (r21-r26 supported)
- Check toolchain file exists: `$ANDROID_NDK/build/cmake/android.toolchain.cmake`
- Ensure CMake 3.20+ installed

**Unity import issues:**
- Verify .meta files have correct ABI settings
- Check directory structure matches Unity requirements
- Ensure platform is set to "Android" in import settings

**Device compatibility:**
- arm64-v8a: Android 5.0+ on 64-bit ARM devices
- armeabi-v7a: Android 4.1+ on 32-bit ARM devices
- x86_64: Emulator or rare x86 Android devices
