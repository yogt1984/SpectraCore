# Cross-Platform Build Guide

This guide explains how to build SpectraCore for Windows and macOS from a Linux development machine.

## Quick Summary

| Platform | Build from Linux? | Test from Linux? | Recommended Approach |
|----------|-------------------|------------------|---------------------|
| **Linux** | ✅ Yes (native) | ✅ Yes | Build and test locally |
| **Windows** | ✅ Yes (cross-compile) | ❌ No | Build on Linux, test on Windows VM/machine |
| **macOS** | ⚠️ Difficult | ❌ No | Use GitHub Actions or Mac hardware |
| **iOS** | ❌ No | ❌ No | Requires macOS with Xcode |
| **Android** | ✅ Yes (NDK) | ⚠️ Partial | Build on Linux, test on device/emulator |
| **WebGL** | ✅ Yes (Emscripten) | ⚠️ Browser | Build on Linux, test in browser |

## Building for Windows from Linux

### Prerequisites

Install MinGW-w64 cross-compiler:

```bash
sudo apt-get update
sudo apt-get install mingw-w64 cmake build-essential
```

Verify installation:
```bash
x86_64-w64-mingw32-g++ --version
```

### Method 1: Using the Build Script (Easiest)

```bash
cd /home/onat/SpectraCore
./build-windows-from-linux.sh
```

This script will:
1. Check for MinGW installation
2. Configure CMake with Windows toolchain
3. Build `spectra.dll`
4. Copy to `unity/Runtime/Plugins/Windows/x86_64/`

### Method 2: Manual CMake Build

```bash
cd native
mkdir build-windows
cd build-windows

# Configure for Windows
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF

# Build
cmake --build . -j$(nproc)

# Output: lib/spectra.dll
```

### Output Files

After building:
- **Library:** `native/build-windows/lib/spectra.dll`
- **Size:** ~500-600 KB
- **Architecture:** x86_64 (64-bit)

### Testing on Windows

**You cannot test P/Invoke on Linux.** You need:

1. **Option A: Windows Machine/VM**
   - Transfer `spectra.dll` to Windows PC
   - Install Unity Editor
   - Copy DLL to Unity project: `Assets/Plugins/Windows/x86_64/`
   - Run Unity Test Runner

2. **Option B: Wine (Limited)**
   - Wine can run some .NET/Mono apps
   - Unity Editor may not work properly
   - **Not recommended for production testing**

3. **Option C: Remote Windows Access**
   - Use RDP/VNC to access Windows machine
   - Run Unity Editor remotely
   - Transfer DLL via network share

### Known Issues

**MinGW Compatibility:**
- Some C++17 features may behave differently
- Thread-local storage works differently
- Suggested: Test thoroughly on real Windows

**Missing Dependencies:**
If build fails with missing headers:
```bash
sudo apt-get install mingw-w64-tools
```

## Building for macOS from Linux

### The Problem

**Cross-compiling for macOS from Linux is legally problematic:**

1. **Apple SDK Required**
   - macOS SDK is only legally available on macOS
   - Xcode license restricts use to Apple hardware
   - Distributing macOS SDK violates Apple EULA

2. **OSXCross Exists But...**
   - Requires macOS SDK (legal gray area)
   - Complex setup (hours of configuration)
   - May produce binaries that don't work
   - Apple could change this at any time

3. **Code Signing**
   - macOS libraries need to be signed
   - Requires Apple Developer account
   - Cannot sign from Linux

### Recommended Approaches

**Option 1: GitHub Actions (Best for Linux-only dev)**

Use the provided workflow:
```bash
# Workflow is at: .github/workflows/ci.yml

# Push to GitHub and it will:
# - Build on macOS runner (free)
# - Run tests on macOS
# - Produce libspectra.dylib
# - Upload as artifact
```

**Benefits:**
- Free macOS build environment
- Automated on every commit
- Produces universal binary (Intel + Apple Silicon)
- No Mac hardware needed

**Option 2: Mac Hardware**

Borrow/rent a Mac:
```bash
# On the Mac:
cd native
cmake -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build

# Output: build/lib/libspectra.dylib
```

**Option 3: macOS VM (Dubious Legality)**

Running macOS in VirtualBox/VMware on non-Apple hardware violates Apple EULA.

**Not recommended for commercial projects.**

## GitHub Actions: The Complete Solution

### Setup

The workflow file is already created at:
```
.github/workflows/ci.yml
```

### How It Works

1. **Push code to GitHub**
   ```bash
   git add .
   git commit -m "Update native library"
   git push
   ```

2. **GitHub Actions automatically:**
   - Builds on Ubuntu (Linux)
   - Builds on Windows Server
   - Builds on macOS
   - Runs tests on all platforms
   - Creates downloadable artifacts

3. **Download artifacts:**
   - Go to Actions tab on GitHub
   - Click on your workflow run
   - Download artifacts (DLL, SO, DYLIB)

### Artifact Structure

```
SpectraCore-Unity-Package/
├── Runtime/
│   └── Plugins/
│       ├── Linux/x86_64/libspectra.so
│       ├── Windows/x86_64/spectra.dll
│       └── macOS/libspectra.dylib
```

### Testing with GitHub Actions

**For building:** ✅ Fully automated

**For Unity P/Invoke testing:**
- Still requires manual testing
- Download artifacts from GitHub
- Test in Unity on each platform

**Future enhancement:**
Could add Unity Test Runner to GitHub Actions, but requires Unity license activation in CI.

## Platform-Specific Notes

### Linux (Native)

**Already working:**
```bash
cd native
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build  # All tests pass
```

**Unity testing:**
- Copy `libspectra.so` to `unity/Runtime/Plugins/`
- Unity Editor on Linux (if supported by your Unity version)
- Or use standalone test: `mono StandalonePInvokeTest.exe`

### Windows (Cross-compiled)

**Building from Linux:**
```bash
./build-windows-from-linux.sh
```

**Output:** `spectra.dll` for Windows x64

**Testing requirements:**
- Windows 10/11
- Unity Editor 2021.3+ (Windows version)
- .NET Framework 4.7.1+

**Transfer DLL:**
```bash
# Via SCP to Windows machine
scp native/build-windows/lib/spectra.dll user@windows-pc:~/

# Or use USB drive, network share, etc.
```

### macOS (Use CI or Mac)

**Building:**
- Recommended: GitHub Actions
- Alternative: Actual Mac hardware

**Testing:**
- Requires actual Mac (physical or cloud)
- macOS 10.15+ (Catalina)
- Unity Editor 2021.3+ (macOS version)

**Universal Binary:**
```bash
# On Mac, build for both Intel and Apple Silicon:
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```

## Unity P/Invoke Testing Checklist

Once you have the platform-specific libraries:

### On Windows

1. ✅ Copy `spectra.dll` to project
2. ✅ Open Unity Editor
3. ✅ Check DLL is recognized (Inspector → Plugins)
4. ✅ Open Test Runner (Window → General → Test Runner)
5. ✅ Run Edit Mode tests:
   - `SpectraNativeTests` (38 tests)
   - `SpectraComponentTests` (41 tests)
6. ✅ Check for errors in Console

### On macOS

1. ✅ Copy `libspectra.dylib` to project
2. ✅ Open Unity Editor
3. ✅ Check library is recognized
4. ✅ Run Test Runner
5. ✅ Test on both Intel and Apple Silicon Macs if possible

### Common Issues

**"DllNotFoundException":**
- Library not in Plugins folder
- Wrong architecture (x86 vs x64)
- Missing dependencies (MSVC runtime on Windows)

**"EntryPointNotFoundException":**
- Function signature mismatch
- Name mangling issue (should use `extern "C"`)
- Wrong calling convention

**Crashes on load:**
- ABI mismatch
- Incompatible compiler flags
- Thread-local storage issues (MinGW)

## CI/CD Strategy

### For Solo Developers (Linux Primary)

**Build phase:**
```yaml
1. Develop on Linux
2. Test on Linux
3. Push to GitHub
4. GitHub Actions builds all platforms
5. Download artifacts
```

**Test phase:**
```yaml
1. Manual testing on Windows VM (once per release)
2. Manual testing on borrowed Mac (once per release)
3. Or recruit beta testers on those platforms
```

### For Teams

**Automated:**
- All platforms build via CI
- Linux tests run automatically
- Artifacts uploaded

**Manual:**
- Windows developer tests Windows build
- macOS developer tests macOS build
- Report results in issue tracker

## Cost Analysis

| Approach | Cost | Time | Reliability |
|----------|------|------|-------------|
| **GitHub Actions** | Free (2000 min/month) | Automated | High |
| **Windows VM** | ~$50/year (Azure, AWS) | Medium | High |
| **Mac Mini** | ~$600 one-time | Low | High |
| **macOS cloud** | ~$1-2/hour | Medium | Medium |
| **OSXCross** | Free | High (setup) | Low (legal issues) |

**Recommendation for this project:**
1. Use GitHub Actions for building (free, automated)
2. Test on Windows VM for UNITY-001 (~$5/month)
3. Skip macOS testing for now or recruit Mac beta tester
4. Document "Windows tested, macOS build available but untested"

## Practical Workflow for UNITY-001/002

### Immediate Action (From Linux)

1. **Set up Windows cross-compilation:**
   ```bash
   sudo apt-get install mingw-w64
   ./build-windows-from-linux.sh
   ```

2. **Verify Windows DLL:**
   ```bash
   file native/build-windows/lib/spectra.dll
   # Should say: "PE32+ executable (DLL) (console) x86-64"
   ```

3. **Set up GitHub Actions:**
   ```bash
   git add .github/workflows/ci.yml
   git commit -m "Add multi-platform CI"
   git push
   ```

4. **Download artifacts from GitHub Actions:**
   - Wait for build to complete (~5-10 minutes)
   - Download Windows DLL
   - Download macOS DYLIB

### Testing Phase (Requires Other Hardware)

**UNITY-001 (Windows):**
- Rent Windows VM on Azure/AWS ($5 test credit)
- Install Unity Hub + Unity 2021.3 LTS
- Transfer DLL + Unity package
- Run Test Runner
- Document results

**UNITY-002 (macOS):**
- Use GitHub Actions to build (free)
- Find someone with Mac to test
- Or mark as "built but untested"
- Or delay until you have Mac access

## Summary

**What you CAN do from Linux:**
- ✅ Build for Linux (native) - Working now
- ✅ Build for Windows (cross-compile) - Ready to use
- ✅ Build for macOS (via GitHub Actions) - Workflow provided
- ✅ Run Linux tests locally
- ✅ Automate all builds via CI

**What you CANNOT do from Linux:**
- ❌ Test Unity P/Invoke on Windows
- ❌ Test Unity P/Invoke on macOS
- ❌ Run Unity Editor on Linux (limited support)

**Best approach for UNITY-001/002:**
1. Build Windows DLL using provided script
2. Use GitHub Actions for macOS build
3. Get ~1 hour access to Windows VM for testing
4. Document macOS as "built but untested" if no Mac available
5. Mark tasks as "partially complete" with artifacts ready

**This approach:**
- Costs: $0-10 (one-time Windows VM rental)
- Time: 2-4 hours total
- Gives you: Production-ready DLLs for all platforms
- Limitation: Manual testing still needed for non-Linux

---

## Building for iOS (Mobile)

### Requirements

**Hardware:**
- Mac computer (MacBook, iMac, Mac Mini, Mac Studio)
- **Cannot build from Linux** (requires Xcode)

**Software:**
- macOS 11.0 (Big Sur) or later
- Xcode 13.0 or later
- CMake 3.20+
- Xcode Command Line Tools

### Installation

1. **Install Xcode:**
   - Download from Mac App Store
   - Or download from https://developer.apple.com/xcode/

2. **Install Command Line Tools:**
   ```bash
   xcode-select --install
   ```

3. **Verify installation:**
   ```bash
   xcodebuild -version
   # Should show: Xcode 14.x or later
   ```

### Building for iOS Device (arm64)

**Method 1: Using Build Script (Recommended)**

```bash
cd native
./scripts/build_ios.sh
```

**Method 2: Manual Build**

```bash
cd native

# Configure for iOS device
cmake -B build-ios -G Xcode \
    -DCMAKE_TOOLCHAIN_FILE=cmake/ios.toolchain.cmake \
    -DPLATFORM=OS64 \
    -DCMAKE_BUILD_TYPE=Release \
    -DSPECTRA_BUILD_TESTS=OFF

# Build
cmake --build build-ios --config Release

# Output: build-ios/lib/Release/libspectra.a
```

### Building for iOS Simulator

**For Intel Macs:**
```bash
PLATFORM=SIMULATOR64 ./scripts/build_ios.sh
```

**For Apple Silicon Macs (M1/M2):**
```bash
PLATFORM=SIMULATORARM64 ./scripts/build_ios.sh
```

### Platform Options

| Platform | Architecture | Target |
|----------|--------------|--------|
| `OS64` | arm64 | iPhone/iPad devices |
| `SIMULATOR64` | x86_64 | Simulator on Intel Macs |
| `SIMULATORARM64` | arm64 | Simulator on Apple Silicon Macs |

### Output Files

- **Library:** `libspectra.a` (static library)
- **Location:** `unity/Runtime/Plugins/iOS/`
- **Size:** ~400-500 KB
- **Architecture:** ARM64

### Unity Integration

1. **Copy library to Unity:**
   ```bash
   # Build script does this automatically
   cp build-ios/lib/Release/libspectra.a unity/Runtime/Plugins/iOS/
   ```

2. **Set up .meta file:**
   ```bash
   cd unity/Runtime/Plugins
   ./setup_meta_files.sh
   ```

3. **Verify in Unity:**
   - Open Unity project
   - Select `libspectra.a` in Project window
   - Inspector should show:
     - Platform: iOS
     - CPU: ARM64
     - Plugin Type: Static

### Testing on iOS

**Option 1: Physical Device (Recommended)**
1. Connect iPhone/iPad via USB
2. In Unity: File → Build Settings → iOS
3. Click "Build and Run"
4. Xcode will launch and deploy to device
5. Test app on device

**Option 2: Simulator**
1. In Unity: File → Build Settings → iOS
2. Check "Development Build"
3. Build project
4. Open `.xcodeproj` in Xcode
5. Select iOS Simulator as target
6. Run in simulator

### iOS-Specific Notes

**Static vs Shared Library:**
- iOS requires **static libraries** (.a files)
- Unlike Linux/Android which use shared (.so) files
- Unity handles linking automatically

**Code Signing:**
- Real device testing requires Apple Developer account ($99/year)
- Simulator testing is free
- Unity manages signing for built apps

**Deployment Target:**
- Minimum: iOS 12.0
- Recommended: iOS 13.0+ (wider API support)
- Set in CMake: `CMAKE_OSX_DEPLOYMENT_TARGET`

**Architecture Support:**
- ARM64 only (iPhone 5s and newer, 2013+)
- No 32-bit ARM support (deprecated by Apple)

### Common iOS Build Issues

**"No such file or directory: ios.toolchain.cmake":**
```bash
# Make sure you're in native/ directory
cd native
./scripts/build_ios.sh
```

**"xcodebuild: command not found":**
```bash
# Install Xcode Command Line Tools
xcode-select --install
```

**"Code signing required":**
- Simulator builds don't require signing
- Device builds need Apple Developer account
- Or use automatic signing in Xcode

**"Undefined symbols for architecture arm64":**
- Missing library dependencies
- Check that all C++ symbols are exported
- Verify `extern "C"` wrapping in headers

---

## Building for Android (Mobile)

### Requirements

**Platforms:** Linux, macOS, or Windows

**Software:**
- Android NDK r21 or later (r26 recommended)
- CMake 3.20+
- Java JDK 8+ (for Android tools)

### Installing Android NDK

**Option 1: Android Studio (Easiest)**
1. Install Android Studio
2. Open SDK Manager
3. SDK Tools tab → Check "NDK (Side by side)"
4. Click "Apply" to download

**Option 2: Command Line Download**
```bash
# Download NDK
cd ~
wget https://dl.google.com/android/repository/android-ndk-r26b-linux.zip
unzip android-ndk-r26b-linux.zip

# Set environment variable
export ANDROID_NDK=~/android-ndk-r26b

# Add to .bashrc for persistence
echo 'export ANDROID_NDK=~/android-ndk-r26b' >> ~/.bashrc
```

**Verify installation:**
```bash
$ANDROID_NDK/build/cmake/android.toolchain.cmake
# Should exist
```

### Building for Android

**Method 1: Using Build Script (Recommended)**

```bash
cd native

# Build for arm64-v8a (64-bit ARM, priority)
ABI=arm64-v8a ./scripts/build_android.sh

# Build for armeabi-v7a (32-bit ARM, compatibility)
ABI=armeabi-v7a ./scripts/build_android.sh

# Build for x86_64 (emulator)
ABI=x86_64 ./scripts/build_android.sh
```

**Method 2: Build All ABIs at Once**

```bash
cd native
./scripts/build_all_mobile.sh
```

**Method 3: Manual CMake Build**

```bash
cd native

# Configure for arm64-v8a
cmake -B build-android-arm64 \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    -DANDROID_STL=c++_shared \
    -DCMAKE_BUILD_TYPE=Release \
    -DSPECTRA_BUILD_TESTS=OFF

# Build
cmake --build build-android-arm64

# Output: build-android-arm64/lib/libspectra.so
```

### Android ABI Support

| ABI | Architecture | API Level | Notes |
|-----|--------------|-----------|-------|
| **arm64-v8a** | 64-bit ARM | 21+ (Android 5.0) | **Priority** - 2020+ devices |
| **armeabi-v7a** | 32-bit ARM | 16+ (Android 4.1) | Legacy compatibility |
| **x86_64** | 64-bit Intel | 21+ | Emulator, rare devices |
| **x86** | 32-bit Intel | 16+ | Old emulator (optional) |

**Production Recommendation:**
- Build `arm64-v8a` + `armeabi-v7a`
- Skip x86/x86_64 unless testing on emulator

### Output Files

Directory structure:
```
unity/Runtime/Plugins/Android/libs/
├── arm64-v8a/
│   └── libspectra.so  (~500 KB)
├── armeabi-v7a/
│   └── libspectra.so  (~450 KB)
└── x86_64/
    └── libspectra.so  (~520 KB)
```

### Unity Integration

1. **Build libraries:**
   ```bash
   cd native
   ./scripts/build_all_mobile.sh
   ```

2. **Set up .meta files:**
   ```bash
   cd ../unity/Runtime/Plugins
   ./setup_meta_files.sh
   ```

3. **Verify in Unity:**
   - Select each `libspectra.so` in Project window
   - Inspector settings:
     - Platform: Android
     - CPU: ARM64 (for arm64-v8a) or ARMv7 (for armeabi-v7a)

### Testing on Android

**Option 1: Physical Device**
1. Enable Developer Options on Android device
2. Enable USB Debugging
3. Connect via USB
4. In Unity: File → Build Settings → Android
5. Click "Build and Run"
6. App deploys to device automatically

**Option 2: Android Emulator**
1. Open Android Studio
2. AVD Manager → Create Virtual Device
3. Choose device (Pixel 5 recommended)
4. System Image: x86_64 with Google APIs
5. Start emulator
6. In Unity: Build and Run
7. App deploys to emulator

**Emulator Performance:**
- x86_64 emulator is fast on Intel hosts
- ARM emulation is slow (use ARM device if possible)

### Android-Specific Notes

**Minimum API Level:**
- Set to API 21 (Android 5.0 Lollipop)
- Covers 95%+ of active Android devices
- Can lower to API 16 if needed (broader support)

**C++ STL:**
- Uses `c++_shared` (LLVM libc++)
- Automatically included by Unity
- No manual linking required

**NEON SIMD:**
- Enabled for armeabi-v7a (ARM SIMD instructions)
- Improves performance on 32-bit ARM
- Configured in `cmake/platforms/Android.cmake`

**Library Size:**
- arm64-v8a: ~500 KB
- armeabi-v7a: ~450 KB (smaller, 32-bit)
- Compressed in APK: ~200 KB per ABI

### Common Android Build Issues

**"ANDROID_NDK is not set":**
```bash
export ANDROID_NDK=/path/to/android-ndk-r26b
# Or
export NDK=/path/to/android-ndk-r26b
```

**"android.toolchain.cmake not found":**
```bash
# Verify NDK path
ls $ANDROID_NDK/build/cmake/android.toolchain.cmake

# If missing, re-download NDK
```

**"Unsupported Android ABI":**
```bash
# Valid ABIs: arm64-v8a, armeabi-v7a, x86_64, x86
ABI=arm64-v8a ./scripts/build_android.sh
```

**Unity "Unable to find libspectra.so":**
- Check .meta files have correct ABI settings
- Verify directory structure: `Android/libs/{abi}/libspectra.so`
- Run `./setup_meta_files.sh` to regenerate meta files

**"DllNotFoundException" on Android device:**
- Library not included in build
- Check Unity Build Settings → Player Settings → Other Settings
- Ensure target ABI is selected (ARMv7, ARM64, or both)

### Multi-ABI Builds

**Why build multiple ABIs?**
- arm64-v8a: Modern devices (2020+), faster, 64-bit
- armeabi-v7a: Older devices (2015-2020), wider compatibility

**APK size impact:**
- Each ABI adds ~200 KB compressed
- arm64 + armv7: ~400 KB total
- Users download all ABIs (or use App Bundle for split delivery)

**Recommendation:**
- Development: arm64-v8a only (faster builds)
- Production: arm64-v8a + armeabi-v7a (broader support)

### Performance Notes

**Real-time Audio (44.1 kHz):**
- Tested on Pixel 5 (arm64-v8a): ✅ Real-time capable
- Filter processing: <1ms per 1024-sample buffer
- Sufficient for real-time audio applications

**Battery Impact:**
- DSP operations are CPU-intensive
- Use `ProcessorAffinity` to avoid P-cores on big.LITTLE
- Profile with Android Studio Profiler

**Thermal Throttling:**
- Extended DSP processing generates heat
- Monitor `ThermalManager` API
- Reduce processing if device overheats

---

## WebGL/WASM Support

### Requirements

**Platform:** Linux, macOS, or Windows

**Software:**
- Emscripten SDK (emsdk)
- CMake 3.20+
- Python 3.6+
- Node.js 12+ (for testing)

### Installing Emscripten

```bash
# Clone emsdk repository
cd ~
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install latest
./emsdk install latest
./emsdk activate latest

# Set up environment (add to .bashrc for persistence)
source ./emsdk_env.sh

# Verify
emcc --version
# Should show: emcc (Emscripten) 3.1.x or later
```

### Building for WebGL

**Method 1: Using Build Script (Coming in v1.2.1)**

```bash
cd native
./scripts/build_wasm.sh
```

**Method 2: Manual Build**

```bash
cd native

# Configure with Emscripten
emcmake cmake -B build-wasm \
    -DCMAKE_BUILD_TYPE=Release \
    -DSPECTRA_BUILD_TESTS=OFF \
    -DSPECTRA_BUILD_BENCHMARKS=OFF

# Build
cmake --build build-wasm

# Output: build-wasm/lib/libspectra.wasm
```

### Optimization Flags

**For size (recommended for web):**
```bash
emcmake cmake -B build-wasm \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DCMAKE_C_FLAGS="-Oz" \
    -DCMAKE_CXX_FLAGS="-Oz"
```

**For performance:**
```bash
emcmake cmake -B build-wasm \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-O3 -msimd128" \
    -DCMAKE_CXX_FLAGS="-O3 -msimd128"
```

### Output Files

- **libspectra.wasm** - WebAssembly binary (~200-300 KB)
- **libspectra.js** - JavaScript glue code (~50 KB)

### Unity WebGL Integration

Unity handles WASM integration differently:

```csharp
// Platform-conditional import
#if UNITY_WEBGL && !UNITY_EDITOR
    [DllImport("__Internal")]  // Use __Internal for WebGL
#else
    [DllImport("spectra")]
#endif
private static extern int spectra_butter(/* ... */);
```

### WebGL-Specific Notes

**Performance:**
- ~50-70% of native speed (acceptable for real-time DSP)
- WASM SIMD improves to ~80-90% (with `-msimd128`)

**Browser Support:**
- Chrome/Edge: Full support (WASM SIMD enabled)
- Firefox: Full support
- Safari: Full support (iOS 15+)

**Memory:**
- WASM heap limited to 2-4 GB (plenty for DSP)
- Can grow dynamically

**Threading:**
- Limited in WebGL (Web Workers available but complex)
- Single-threaded processing recommended

### Testing WebGL Build

**Option 1: Unity WebGL Build**
1. In Unity: File → Build Settings → WebGL
2. Build and run
3. Test in browser

**Option 2: Standalone HTML Test**
```bash
# Create test HTML
emcc test.cpp -o test.html \
    -Lbuild-wasm/lib -lspectra

# Serve locally
python3 -m http.server 8000

# Open in browser
# http://localhost:8000/test.html
```

---

## Cross-Platform Build Summary

### Build Matrix

| Platform | Build From | Script | Output | Size |
|----------|------------|--------|--------|------|
| Linux | Linux | `cmake -B build` | `libspectra.so` | ~560 KB |
| Windows | Linux | `build-windows-from-linux.sh` | `spectra.dll` | ~500 KB |
| macOS | macOS or CI | GitHub Actions | `libspectra.dylib` | ~480 KB |
| iOS | macOS | `scripts/build_ios.sh` | `libspectra.a` | ~420 KB |
| Android arm64 | Linux/macOS/Win | `scripts/build_android.sh` | `libspectra.so` | ~500 KB |
| Android armv7 | Linux/macOS/Win | `scripts/build_android.sh` | `libspectra.so` | ~450 KB |
| WebGL | Linux/macOS/Win | `emcmake cmake` | `libspectra.wasm` | ~250 KB |

### Recommended Workflow

**For comprehensive release:**
```bash
# 1. Build Linux (native)
cd native
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 2. Build Windows (cross-compile)
./build-windows-from-linux.sh

# 3. Build Android (all ABIs)
./scripts/build_all_mobile.sh

# 4. Build WebGL
emcmake cmake -B build-wasm -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build-wasm

# 5. Build iOS and macOS (requires macOS)
# - Use GitHub Actions or physical Mac
# - Or defer until Mac access available
```

**Setup Unity meta files:**
```bash
cd ../unity/Runtime/Plugins
./setup_meta_files.sh
```

---

## Platform Testing Checklist

### Desktop Platforms
- [ ] Linux: Build and test locally
- [ ] Windows: Build on Linux, test on Windows VM/PC
- [ ] macOS: Build via CI or Mac, test on Mac hardware

### Mobile Platforms
- [ ] iOS: Build on Mac, test on iPhone/iPad (physical device recommended)
- [ ] iOS Simulator: Test on Xcode simulator (free, no Apple Developer account needed)
- [ ] Android arm64: Build on Linux, test on modern Android device
- [ ] Android armv7: Test on older Android device (optional)
- [ ] Android Emulator: Test on x86_64 emulator (development only)

### Web Platform
- [ ] WebGL: Build with Emscripten, test in Unity WebGL build
- [ ] Chrome/Edge: Test real-time audio processing
- [ ] Firefox: Verify WASM compatibility
- [ ] Safari: Test on macOS/iOS (iOS 15+)

### Performance Validation
- [ ] Real-time capability (44.1 kHz): All platforms
- [ ] Filter accuracy: Cross-validate with MATLAB
- [ ] Memory usage: Profile on mobile devices
- [ ] Battery impact: Monitor on iOS/Android
- [ ] Thermal behavior: Long-running tests on mobile

---

## Troubleshooting Multi-Platform Builds

### General Issues

**CMake configuration fails:**
```bash
# Clear cache
rm -rf build/CMakeCache.txt
cmake -B build --fresh
```

**Linker errors:**
- Check all dependencies are available for target platform
- Verify toolchain file is correct
- Ensure C++ standard is consistent (C++17)

### Platform-Specific Issues

**iOS: "xcodebuild: error: SDK \"iphoneos\" cannot be located":**
```bash
# Check Xcode installation
xcode-select -p

# Reset Xcode path
sudo xcode-select --switch /Applications/Xcode.app
```

**Android: "ld: cannot find -lstdc++":**
- Update NDK to r21 or later
- Check `ANDROID_STL` is set to `c++_shared`

**WebGL: "error: undefined symbol: __cxa_thread_atexit":**
- Disable thread-local storage for WASM
- Use `-s DISABLE_EXCEPTION_CATCHING=1`

### Unity Integration Issues

**"DllNotFoundException" on specific platform:**
1. Check library exists in correct Plugins subdirectory
2. Verify .meta file platform settings
3. Check Architecture matches (x64, ARM64, etc.)
4. Ensure Library is marked as "Compatible with platform"

**Inspector shows "Incompatible plugin":**
- Click on plugin asset
- Platform settings should match library type
- iOS: Must be .a (static), Platform: iOS, CPU: ARM64
- Android: Must be .so (shared), Platform: Android, CPU: per-ABI

---

## Future Platform Support

Planned for v1.3+:
- tvOS (Apple TV)
- Consoles (PlayStation, Xbox, Switch - requires dev kits)
- Raspberry Pi (ARM Linux)

Document version: v1.2.0 - Mobile & Web platforms added
