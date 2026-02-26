# iOS Native Plugin

This directory contains the native iOS static library for SpectraCore.

## Building

From the `native/` directory, run:

```bash
# Build for iOS device (arm64)
./scripts/build_ios.sh

# Or build for simulator
PLATFORM=SIMULATORARM64 ./scripts/build_ios.sh
```

## Output

- **libspectra.a** - Static library for iOS (arm64)

## Unity Import Settings

The `.meta` file for `libspectra.a` should have these settings:

- **Platform:** iOS
- **CPU:** ARM64
- **Plugin Type:** Static (.a)

## Testing

1. Build the iOS library using the script above
2. Open Unity project
3. File → Build Settings → iOS
4. Build and run on physical device or simulator
5. Test filter creation and audio processing

## Platform-Specific Notes

- iOS requires **static libraries** (.a), not dynamic libraries (.dylib)
- Bitcode is disabled (deprecated since iOS 14)
- Minimum deployment target: iOS 12.0
- Architecture: ARM64 only (iPhone 5s and newer)

## Troubleshooting

**Build failed on macOS:**
- Ensure Xcode is installed
- Run `xcode-select --install` if needed
- Verify Xcode Command Line Tools: `xcode-select -p`

**Unity import error:**
- Check that `libspectra.a` has correct .meta file settings
- Verify platform is set to "iOS" in import settings
- Ensure architecture is set to "ARM64"
