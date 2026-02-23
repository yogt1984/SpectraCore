# Cross-Platform Build Guide

This guide explains how to build SpectraCore for Windows and macOS from a Linux development machine.

## Quick Summary

| Platform | Build from Linux? | Test from Linux? | Recommended Approach |
|----------|-------------------|------------------|---------------------|
| **Linux** | ✅ Yes (native) | ✅ Yes | Build and test locally |
| **Windows** | ✅ Yes (cross-compile) | ❌ No | Build on Linux, test on Windows VM/machine |
| **macOS** | ⚠️ Difficult | ❌ No | Use GitHub Actions or Mac hardware |

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
# Workflow is at: .github/workflows/build-multiplatform.yml

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
.github/workflows/build-multiplatform.yml
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
   git add .github/workflows/build-multiplatform.yml
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
