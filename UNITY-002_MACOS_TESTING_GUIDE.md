# UNITY-002: macOS P/Invoke Testing Guide

**Task ID:** UNITY-002
**Status:** ⏳ IN PROGRESS
**Platform:** macOS (Universal Binary: x86_64 + arm64)
**Prerequisites:** macOS machine, Unity Editor 2021.3+

---

## Objective

Verify that the SpectraCore P/Invoke bindings work correctly on macOS by:
1. Obtaining the macOS native library (libspectra.dylib)
2. Running Unity Editor tests
3. Running standalone P/Invoke tests
4. Validating on both Intel and Apple Silicon Macs

---

## Part 1: Obtain macOS Library

### Option A: Download from GitHub Actions (Recommended)

The CI/CD workflow builds a universal binary that works on both Intel and Apple Silicon Macs.

**Steps:**

1. **Wait for workflow completion:**
   - Navigate to: https://github.com/yogt1984/SpectraCore/actions
   - Find the latest workflow run: "Complete Unity integration with CI/CD pipeline"
   - Wait for all jobs to complete (green checkmarks)
   - Expected time: 10-15 minutes

2. **Download macOS artifact:**
   - Scroll to bottom of workflow run page
   - Find artifact: `libspectra-macos` (~250 KB)
   - Click to download (downloads as .zip file)

3. **Extract the library:**
   ```bash
   # On macOS
   unzip libspectra-macos.zip
   # Should contain: libspectra.dylib
   ```

4. **Verify universal binary:**
   ```bash
   file libspectra.dylib
   # Expected: Mach-O universal binary with 2 architectures
   # [x86_64: Mach-O 64-bit dynamically linked shared library x86_64]
   # [arm64: Mach-O 64-bit dynamically linked shared library arm64]

   lipo -info libspectra.dylib
   # Expected: Architectures in the fat file: libspectra.dylib are: x86_64 arm64
   ```

### Option B: Build Locally on macOS

**Requirements:**
- Xcode Command Line Tools
- CMake 3.15+
- Git

**Install dependencies:**
```bash
# Install Xcode Command Line Tools (if not already installed)
xcode-select --install

# Install CMake (via Homebrew)
brew install cmake

# Or download CMake from: https://cmake.org/download/
```

**Build Steps:**

```bash
# Clone repository (if not already done)
git clone https://github.com/yogt1984/SpectraCore.git
cd SpectraCore

# Configure for universal binary (x86_64 + arm64)
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

# Build
cmake --build build

# Library location
ls -lh build/native/libspectra.dylib
```

**Expected output:**
```
libspectra.dylib - ~250 KB universal binary
```

**Verify architectures:**
```bash
lipo -info build/native/libspectra.dylib
# Should show: x86_64 arm64
```

---

## Part 2: Setup Unity Project on macOS

### Install Unity

1. **Download Unity Hub:**
   - https://unity.com/download

2. **Install Unity Editor 2021.3 LTS or newer:**
   - Open Unity Hub
   - Go to Installs tab
   - Add: Unity 2021.3.x LTS (or newer)
   - Include macOS Build Support

3. **Create or Open Project:**

   **Option 1: Import SpectraCore package**
   ```bash
   # Create new Unity project via Unity Hub
   # Then copy SpectraCore Unity integration
   cd /path/to/SpectraCore
   cp -r unity/* "/path/to/UnityProject/Assets/SpectraCore/"
   ```

   **Option 2: Use Unity Package (from CI/CD)**
   - Download `SpectraCore-Unity-Package` artifact from GitHub Actions
   - Extract SpectraCore-Unity.tar.gz
   - Import into Unity project

### Install macOS Library

**Copy library to Unity Plugins folder:**

```bash
# Navigate to Unity project
cd "/path/to/UnityProject"

# Create Plugins directory structure
mkdir -p "Assets/SpectraCore/Runtime/Plugins/macOS"

# Copy the library
cp /path/to/libspectra.dylib "Assets/SpectraCore/Runtime/Plugins/macOS/"
```

**Verify in Unity:**
1. Open Unity Editor
2. Navigate to: `Assets/SpectraCore/Runtime/Plugins/macOS/`
3. Select `libspectra.dylib`
4. In Inspector, verify:
   - ✅ Platform: macOS
   - ✅ CPU: Universal (or x86_64 + arm64)
   - ✅ Load on startup: checked

---

## Part 3: Run Unity Editor Tests

### Open Test Runner

1. **Open Test Runner window:**
   - Menu: Window → General → Test Runner
   - Or: Cmd+T (if shortcut is configured)

2. **Enable EditMode tests:**
   - Click "EditMode" tab
   - Tests should appear in list

### Run P/Invoke Binding Tests

**Test file:** `SpectraNativeTests.cs` (36 tests)

**Run all tests:**
1. Click "Run All" button
2. Wait for completion (~30 seconds)
3. Verify all tests pass (green checkmarks)

**Expected results:**
```
✅ SpectraNative/VersionInfo_ReturnsNonNull
✅ SpectraNative/FFT_CreateAndDestroy_HandlesValid
✅ SpectraNative/FFT_ForwardTransform_ProducesSpectrum
✅ SpectraNative/STFT_CreateAndDestroy_HandlesValid
✅ SpectraNative/FilterDesign_Butter_ValidCoefficients
✅ SpectraNative/FilterDesign_Cheby1_ValidCoefficients
✅ SpectraNative/FilterDesign_Cheby2_ValidCoefficients
✅ SpectraNative/FilterDesign_Ellip_ValidCoefficients
... (36 tests total)

Total: 36 tests passed, 0 failed
Time: ~30 seconds
```

### Run Component Tests

**Test file:** `SpectraComponentTests.cs` (35 tests)

**Run all tests:**
1. Stay in EditMode tab
2. Expand "SpectraComponentTests" group
3. Click "Run All"
4. Verify all pass

**Expected results:**
```
✅ DSP/Butter_DesignsValidFilter
✅ DSP/Cheby1_DesignsValidFilter
✅ DSP/LFilter_FiltersSignal
✅ DSP/FiltFilt_ZeroPhaseFilters
✅ FFTAnalyzer/CreateAndDispose_NoErrors
✅ STFTAnalyzer/CreateAndDispose_NoErrors
✅ STFTAnalyzer/StreamingMode_Works
✅ OnsetDetector/CreateAndDispose_NoErrors
... (35 tests total)

Total: 35 tests passed, 0 failed
Time: ~45 seconds
```

### Total Test Results

**Expected:**
- ✅ 36 P/Invoke tests passed
- ✅ 35 Component tests passed
- ✅ **71 total tests passed**
- ❌ 0 tests failed

**Test on both architectures (if possible):**
- Intel Mac: Tests run natively on x86_64 slice
- Apple Silicon Mac: Tests run natively on arm64 slice
- Universal binary works on both automatically

---

## Part 4: Run Standalone P/Invoke Test

This test runs outside Unity to verify P/Invoke works in standalone .NET applications.

**Test file:** `unity/Tests/StandalonePInvokeTest.cs`

### Compile and Run with Mono

**Requirements:**
- Mono runtime (usually pre-installed on macOS)
- Or .NET SDK

**Steps:**

```bash
# Navigate to project
cd /path/to/SpectraCore

# Check if Mono is available
which mono
# If not installed: brew install mono

# Compile test program
mcs -out:PInvokeTest.exe unity/Tests/StandalonePInvokeTest.cs

# Copy library to same directory
cp libspectra.dylib .

# Run test
mono PInvokeTest.exe
```

**Expected output:**
```
=== SpectraCore P/Invoke Standalone Test ===

[1/6] Testing version info...
  Version: 1.0.0
  ✓ PASSED

[2/6] Testing error handling...
  No errors initially
  ✓ PASSED

[3/6] Testing FFT creation...
  FFT handle: 0x00000123456789AB
  ✓ PASSED

[4/6] Testing FFT transform...
  Computed 129 spectrum values
  ✓ PASSED

[5/6] Testing filter design...
  Filter coefficients: 5 numerator, 5 denominator
  ✓ PASSED

[6/6] Testing library path...
  Can load libspectra.dylib from current directory
  ✓ PASSED

=== All Tests Passed! ===
macOS P/Invoke bindings are working correctly.
```

### Compile and Run with .NET

**Alternative using modern .NET:**

```bash
# Create temporary project
mkdir StandaloneTest
cd StandaloneTest

# Initialize .NET project
dotnet new console

# Copy test code
cp ../unity/Tests/StandalonePInvokeTest.cs Program.cs

# Copy library
cp ../libspectra.dylib .

# Run
dotnet run
```

---

## Part 5: Validation Checklist

### Library Validation

- [ ] File exists: `libspectra.dylib`
- [ ] File size: ~250 KB (acceptable range: 200-300 KB)
- [ ] File type: Mach-O universal binary
- [ ] Contains x86_64 architecture
- [ ] Contains arm64 architecture
- [ ] Can be loaded by Unity
- [ ] No dependency errors

**Verify with Terminal:**
```bash
# Check file type
file libspectra.dylib

# Check architectures
lipo -info libspectra.dylib

# Check dependencies
otool -L libspectra.dylib
# Should only depend on system libraries

# Check exports
nm -gU libspectra.dylib | grep spectra_
# Should show ~50+ spectra_* functions
```

### Unity Test Validation

- [ ] Unity Editor opens without errors
- [ ] Library loads on Unity startup
- [ ] No P/Invoke errors in Console
- [ ] Test Runner shows all 71 tests
- [ ] All 36 P/Invoke tests pass
- [ ] All 35 Component tests pass
- [ ] No red errors in Console
- [ ] Tests complete in < 2 minutes

### Standalone Test Validation

- [ ] StandalonePInvokeTest.cs compiles without errors
- [ ] Executable runs without crashes
- [ ] All 6 test sections pass
- [ ] FFT operations work
- [ ] Filter design works
- [ ] No DllNotFoundException
- [ ] No marshaling errors

### Architecture-Specific Validation

**On Intel Mac (x86_64):**
- [ ] Unity tests pass
- [ ] Standalone test passes
- [ ] Uses x86_64 slice of universal binary

**On Apple Silicon Mac (arm64):**
- [ ] Unity tests pass (Rosetta not required!)
- [ ] Standalone test passes
- [ ] Uses arm64 slice of universal binary
- [ ] Native performance (no emulation)

### Functional Validation

Test each major feature works:

1. **FFT:**
   ```csharp
   using (var fft = new FFTAnalyzer(256))
   {
       float[] signal = new float[256];
       float[] spectrum = fft.GetMagnitudeSpectrum(signal);
       Debug.Log($"Spectrum size: {spectrum.Length}"); // Should be 129
   }
   ```

2. **Filter Design:**
   ```csharp
   var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
   Debug.Log($"Filter coefficients: {b.Length}b, {a.Length}a"); // Should be 5, 5
   ```

3. **STFT:**
   ```csharp
   using (var stft = new STFTAnalyzer(512, 256, WindowType.Hann))
   {
       float[] signal = new float[1024];
       var (real, imag, frames) = stft.Analyze(signal);
       Debug.Log($"Frames: {frames}"); // Should be > 0
   }
   ```

---

## Part 6: Performance Testing

### Benchmark FFT Performance

```csharp
using UnityEngine;
using Spectra;
using System.Diagnostics;

public class MacPerformanceTest : MonoBehaviour
{
    void Start()
    {
        TestFFTPerformance();
        DetectArchitecture();
    }

    void TestFFTPerformance()
    {
        int iterations = 1000;
        using (var fft = new FFTAnalyzer(1024))
        {
            float[] signal = new float[1024];

            var sw = Stopwatch.StartNew();
            for (int i = 0; i < iterations; i++)
            {
                float[] spectrum = fft.GetMagnitudeSpectrum(signal);
            }
            sw.Stop();

            Debug.Log($"FFT: {iterations} iterations in {sw.ElapsedMilliseconds}ms");
            Debug.Log($"Average: {sw.ElapsedMilliseconds / (float)iterations:F2}ms per FFT");
        }
    }

    void DetectArchitecture()
    {
        string arch = System.Runtime.InteropServices.RuntimeInformation.ProcessArchitecture.ToString();
        Debug.Log($"Running on: {arch}");
        Debug.Log($"macOS version: {SystemInfo.operatingSystem}");
    }
}
```

**Expected performance:**
- **Intel Mac:** ~0.5-1.0ms per 1024-point FFT
- **Apple Silicon:** ~0.3-0.5ms per 1024-point FFT (potentially faster due to ARM architecture)

### Memory Leak Test

```csharp
using UnityEngine;
using Spectra;

public class MacMemoryLeakTest : MonoBehaviour
{
    void Start()
    {
        long startMemory = GC.GetTotalMemory(true);

        // Create and destroy many analyzers
        for (int i = 0; i < 10000; i++)
        {
            using (var fft = new FFTAnalyzer(512))
            {
                float[] spectrum = fft.GetMagnitudeSpectrum(new float[512]);
            }
        }

        GC.Collect();
        GC.WaitForPendingFinalizers();

        long endMemory = GC.GetTotalMemory(true);
        long leaked = endMemory - startMemory;

        Debug.Log($"Memory leaked: {leaked / 1024}KB");
        // Should be < 100KB
    }
}
```

---

## Part 7: Troubleshooting

### Issue: DllNotFoundException

**Error:**
```
DllNotFoundException: Unable to load DLL 'spectra': The specified module could not be found.
```

**Solutions:**

1. **Verify library location:**
   ```bash
   ls -la "Assets/SpectraCore/Runtime/Plugins/macOS/libspectra.dylib"
   ```

2. **Check library naming:**
   - Should be: `libspectra.dylib` (with "lib" prefix)
   - Unity automatically adds "lib" prefix when loading

3. **Verify platform settings:**
   - Select library in Unity
   - Inspector → Platform: macOS
   - CPU: Universal or both x86_64 + ARM64

4. **Check library permissions:**
   ```bash
   chmod +x Assets/SpectraCore/Runtime/Plugins/macOS/libspectra.dylib
   ```

5. **Verify code signing (macOS 10.15+):**
   ```bash
   codesign -s - --force libspectra.dylib
   ```

### Issue: BadImageFormatException

**Error:**
```
BadImageFormatException: An attempt was made to load a program with an incorrect format.
```

**Cause:** Architecture mismatch or corrupted binary

**Solutions:**

1. **Verify universal binary:**
   ```bash
   file libspectra.dylib
   # Must show: Mach-O universal binary

   lipo -info libspectra.dylib
   # Must show: x86_64 arm64
   ```

2. **Rebuild for universal binary:**
   ```bash
   cmake -B build \
     -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
   cmake --build build
   ```

3. **Check if library is stripped:**
   ```bash
   nm -gU libspectra.dylib | grep spectra_
   # Should show exports
   ```

### Issue: EntryPointNotFoundException

**Error:**
```
EntryPointNotFoundException: Unable to find an entry point named 'spectra_fft_create' in DLL 'spectra'.
```

**Solutions:**

1. **Verify library exports:**
   ```bash
   nm -gU libspectra.dylib | grep spectra_
   # Should list all spectra_* functions
   ```

2. **Check for name mangling:**
   - C++ functions must be `extern "C"`
   - Verify in `spectra_abi.h`:
   ```cpp
   #ifdef __cplusplus
   extern "C" {
   #endif
   ```

3. **Rebuild if exports missing**

### Issue: Library Not Loading on Apple Silicon

**Error:**
```
DllNotFoundException on M1/M2 Mac
```

**Solutions:**

1. **Verify arm64 slice exists:**
   ```bash
   lipo -info libspectra.dylib
   # Must include: arm64
   ```

2. **Extract and test arm64 slice:**
   ```bash
   lipo libspectra.dylib -thin arm64 -output libspectra_arm64.dylib
   file libspectra_arm64.dylib
   ```

3. **Ensure Unity is running natively (not via Rosetta):**
   - Right-click Unity in Finder → Get Info
   - Uncheck "Open using Rosetta"

### Issue: Code Signing Problems (macOS 10.15+)

**Error:**
```
Code signature invalid or not present
```

**Solutions:**

1. **Ad-hoc sign the library:**
   ```bash
   codesign -s - --force libspectra.dylib
   ```

2. **For distribution, proper signing:**
   ```bash
   codesign -s "Developer ID Application: Your Name" libspectra.dylib
   ```

3. **Allow unsigned libraries (development only):**
   ```bash
   sudo spctl --master-disable
   # Warning: Security risk, only for development
   ```

### Issue: Slow Performance

**Expected:**
- Intel Mac: FFT (1024) < 1ms
- Apple Silicon: FFT (1024) < 0.5ms

**Solutions:**

1. **Verify Release build:**
   ```bash
   # Library should be from Release, not Debug
   ls -lh build/native/libspectra.dylib
   ```

2. **Check Activity Monitor:**
   - Look for high CPU usage
   - Verify correct architecture is running

3. **Profile in Unity:**
   - Window → Analysis → Profiler
   - Check "Deep Profile"
   - Identify bottlenecks

4. **For Apple Silicon, ensure native execution:**
   ```bash
   # Check if running via Rosetta
   sysctl sysctl.proc_translated
   # Should return 0 (native) not 1 (Rosetta)
   ```

---

## Part 8: Success Criteria

**UNITY-002 is considered COMPLETE when:**

- [x] macOS library obtained (from CI/CD or local build)
- [ ] Library verified as universal binary (x86_64 + arm64)
- [ ] Unity Editor opens with library loaded
- [ ] All 36 P/Invoke tests pass
- [ ] All 35 Component tests pass
- [ ] Standalone test compiles and runs successfully
- [ ] All 6 standalone test sections pass
- [ ] FFT performance acceptable (< 1ms on Intel, < 0.5ms on Apple Silicon)
- [ ] No memory leaks detected
- [ ] Filter Designer tool works on macOS
- [ ] Signal Inspector tool works on macOS
- [ ] No platform-specific errors

**Additional validation:**
- [ ] Tested on Intel Mac (if available)
- [ ] Tested on Apple Silicon Mac (if available)
- [ ] Example projects run on macOS
- [ ] OnAudioFilterRead integration works
- [ ] Microphone input works
- [ ] Build for macOS standalone succeeds

---

## Part 9: Architecture-Specific Notes

### Intel Mac (x86_64)

**Characteristics:**
- Runs x86_64 slice of universal binary
- Similar performance to Linux
- Well-tested architecture

**Performance expectations:**
- FFT (1024-point): 0.5-1.0ms
- STFT: 2-4ms per frame
- Filter: 1-2ms per 1000 samples

### Apple Silicon (arm64)

**Characteristics:**
- Runs arm64 slice natively
- Potentially better performance (ARM architecture)
- Energy efficient
- No Rosetta translation needed with universal binary

**Performance expectations:**
- FFT (1024-point): 0.3-0.5ms (potentially faster)
- STFT: 1-3ms per frame
- Filter: 0.5-1.5ms per 1000 samples

**Unity on Apple Silicon:**
- Unity 2021.2+ supports Apple Silicon natively
- Universal binary works seamlessly
- No Rosetta required

---

## Part 10: Documentation Updates

After successful testing, update these files:

1. **UNITY_INTEGRATION_SUMMARY.md:**
   ```markdown
   ### Platform Compatibility

   - ✅ Linux (x86_64) - Native library built and tested
   - ✅ Windows (x86_64) - P/Invoke bindings verified
   - ✅ macOS (Universal) - Tested on Intel and Apple Silicon ← UPDATE
   ```

2. **Create UNITY-002_COMPLETION.md:**
   - Test results summary (Intel and/or Apple Silicon)
   - Performance metrics for each architecture
   - Any macOS-specific notes
   - Screenshots of passing tests

3. **README.md updates:**
   - Add macOS to supported platforms
   - Note universal binary support
   - Mention Apple Silicon native support

---

## Part 11: Next Steps

After completing UNITY-002:

1. **Commit test results:**
   ```bash
   git add UNITY-002_COMPLETION.md
   git commit -m "docs: complete UNITY-002 macOS P/Invoke testing

   - All 71 Unity tests passing on macOS
   - Standalone P/Invoke test verified
   - Performance benchmarks recorded
   - Tested on [Intel/Apple Silicon/Both]
   - Universal binary validated"
   git push
   ```

2. **Update task tracking:**
   - Mark UNITY-002 as complete
   - All major platform testing complete

3. **Optional: Create macOS-specific examples:**
   - Core Audio integration
   - Audio Unit plugins
   - macOS-specific audio features

---

## Quick Reference Commands

**Download from CI/CD:**
```bash
# Using GitHub CLI (if installed)
gh run download --name libspectra-macos

# Or download manually from:
# https://github.com/yogt1984/SpectraCore/actions
```

**Build locally:**
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build
```

**Copy to Unity:**
```bash
cp build/native/libspectra.dylib "Assets/SpectraCore/Runtime/Plugins/macOS/"
```

**Run tests:**
- Unity: Window → General → Test Runner → Run All
- Standalone: `mcs -out:PInvokeTest.exe StandalonePInvokeTest.cs && mono PInvokeTest.exe`

**Verify library:**
```bash
file libspectra.dylib
lipo -info libspectra.dylib
otool -L libspectra.dylib
nm -gU libspectra.dylib | grep spectra_
```

---

## Support

**Need help?**

1. Check error message in Unity Console
2. Verify library is in correct location
3. Check this troubleshooting guide
4. Review SpectraNative.cs for correct marshaling
5. Test with standalone P/Invoke test first

**Common resources:**
- Unity macOS docs: https://docs.unity3d.com/Manual/PluginsForDesktop.html
- Apple Developer: https://developer.apple.com/
- SpectraCore API docs: `docs/API_Reference.md`

---

## Estimated Time

- **Library download/build:** 5-15 minutes
- **Unity setup:** 10-20 minutes (if installing Unity)
- **Running tests:** 2-5 minutes
- **Validation:** 10-15 minutes
- **Total:** 30-60 minutes

**If you already have Unity installed and library from CI/CD:** ~15 minutes

---

**Status:** Ready for macOS testing once CI/CD artifacts are available (currently building)
