# UNITY-002: macOS Testing Checklist

Quick checklist for macOS P/Invoke testing.

## Prerequisites

- [ ] macOS machine available (Intel or Apple Silicon)
- [ ] Unity Editor 2021.3+ installed
- [ ] Xcode Command Line Tools (optional, for local builds)

---

## Step 1: Obtain macOS Library

### From CI/CD (Recommended)

- [ ] Navigate to https://github.com/yogt1984/SpectraCore/actions
- [ ] Find latest workflow run: "Complete Unity integration with CI/CD pipeline"
- [ ] Verify workflow completed successfully (all green checkmarks)
- [ ] Scroll to bottom → Artifacts section
- [ ] Download `libspectra-macos` artifact (~250 KB)
- [ ] Extract `libspectra.dylib` from .zip file

**Verify:**
```bash
file libspectra.dylib
# Should show: Mach-O universal binary with 2 architectures

lipo -info libspectra.dylib
# Should show: Architectures: x86_64 arm64
```

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 2: Setup Unity Project

- [ ] Open Unity Hub
- [ ] Create new 3D project (or open existing)
- [ ] Create folder structure:
  ```
  Assets/SpectraCore/
  ├── Runtime/
  │   └── Plugins/
  │       └── macOS/
  └── Tests/
      └── Editor/
  ```

- [ ] Copy Unity integration files:
  ```bash
  # From SpectraCore repository
  cp -r unity/Runtime Assets/SpectraCore/Runtime
  cp -r unity/Editor Assets/SpectraCore/Editor
  cp -r unity/Tests Assets/SpectraCore/Tests
  ```

- [ ] Copy macOS library:
  ```bash
  cp libspectra.dylib Assets/SpectraCore/Runtime/Plugins/macOS/
  ```

**Verify in Unity:**
- [ ] Open Unity Editor
- [ ] No errors in Console
- [ ] Library visible in Project window
- [ ] Select library → Inspector shows:
  - Platform: macOS ✓
  - CPU: Universal (or x86_64 + ARM64) ✓

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 3: Run P/Invoke Binding Tests

- [ ] Open Test Runner: Window → General → Test Runner
- [ ] Switch to "EditMode" tab
- [ ] Expand "SpectraNativeTests" category

**Run all tests:**
- [ ] Click "Run All" button
- [ ] Wait for completion (~30 seconds)

**Verify results:**
```
Expected: 36/36 tests passed

Test groups:
- [ ] Version info (1 test) ✓
- [ ] FFT operations (5 tests) ✓
- [ ] Window functions (3 tests) ✓
- [ ] STFT operations (5 tests) ✓
- [ ] Filter design (7 tests) ✓
- [ ] Filter application (4 tests) ✓
- [ ] Correlation (2 tests) ✓
- [ ] Resampling (2 tests) ✓
- [ ] Onset detection (2 tests) ✓
- [ ] Advanced filters (3 tests) ✓
- [ ] PSD (1 test) ✓
- [ ] Hilbert (2 tests) ✓
```

**All tests pass:** ✅ Yes | ❌ No (see troubleshooting)

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 4: Run Component Tests

Still in Test Runner:

- [ ] Expand "SpectraComponentTests" category
- [ ] Click "Run All"
- [ ] Wait for completion (~45 seconds)

**Verify results:**
```
Expected: 35/35 tests passed

Test groups:
- [ ] DSP filter design (4 tests) ✓
- [ ] DSP filter application (2 tests) ✓
- [ ] DSP analysis (4 tests) ✓
- [ ] DSP utilities (3 tests) ✓
- [ ] FFTAnalyzer (3 tests) ✓
- [ ] STFTAnalyzer (4 tests) ✓
- [ ] StreamingIIRFilter (3 tests) ✓
- [ ] OnsetDetector (3 tests) ✓
- [ ] Frequency response (6 tests) ✓
```

**All tests pass:** ✅ Yes | ❌ No (see troubleshooting)

**Total Unity tests:** 71/71 ✓

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 5: Run Standalone P/Invoke Test

Test outside Unity to verify .NET interop.

**Compile:**
```bash
cd /path/to/SpectraCore

# Compile test program (using Mono)
mcs -out:PInvokeTest.exe unity/Tests/StandalonePInvokeTest.cs

# Copy library to same directory
cp libspectra.dylib .
```

**Run:**
```bash
mono PInvokeTest.exe
```

**Expected output:**
```
=== SpectraCore P/Invoke Standalone Test ===

[1/6] Testing version info... ✓ PASSED
[2/6] Testing error handling... ✓ PASSED
[3/6] Testing FFT creation... ✓ PASSED
[4/6] Testing FFT transform... ✓ PASSED
[5/6] Testing filter design... ✓ PASSED
[6/6] Testing library path... ✓ PASSED

=== All Tests Passed! ===
```

**Verify:**
- [ ] Program runs without crashes
- [ ] All 6 tests pass
- [ ] No DllNotFoundException
- [ ] No marshaling errors

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 6: Verify Universal Binary

- [ ] Check architectures:
  ```bash
  lipo -info libspectra.dylib
  # Should show: x86_64 arm64
  ```

- [ ] Check file format:
  ```bash
  file libspectra.dylib
  # Should show: Mach-O universal binary with 2 architectures
  ```

- [ ] Check exports:
  ```bash
  nm -gU libspectra.dylib | grep spectra_ | wc -l
  # Should show: ~50+ functions
  ```

- [ ] Check dependencies:
  ```bash
  otool -L libspectra.dylib
  # Should only show system libraries
  ```

**Architecture used:**
- [ ] Intel Mac: Uses x86_64 slice automatically
- [ ] Apple Silicon: Uses arm64 slice automatically
- [ ] Rosetta not required ✓

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 7: Test Editor Tools

### Signal Inspector

- [ ] Open: Window → Spectra → Signal Inspector
- [ ] Load test audio file (WAV/MP3)
- [ ] Verify displays:
  - [ ] Waveform shows correctly
  - [ ] Spectrum shows correctly
  - [ ] Spectrogram renders
- [ ] Test export:
  - [ ] Export Waveform CSV ✓
  - [ ] Export Spectrum CSV ✓
  - [ ] Export Spectrogram CSV ✓

### Filter Designer

- [ ] Open: Window → Spectra → Filter Designer
- [ ] Select filter type: Butterworth
- [ ] Adjust order slider
- [ ] Adjust cutoff frequency
- [ ] Verify:
  - [ ] Bode plot updates in real-time
  - [ ] Magnitude plot shows
  - [ ] Phase plot shows
- [ ] Export:
  - [ ] Copy coefficients to clipboard ✓
  - [ ] Export to C# class file ✓

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 8: Performance Testing

### FFT Performance

Create test script:

```csharp
using UnityEngine;
using Spectra;
using System.Diagnostics;

public class MacFFTBenchmark : MonoBehaviour
{
    void Start()
    {
        using (var fft = new FFTAnalyzer(1024))
        {
            float[] signal = new float[1024];
            var sw = Stopwatch.StartNew();

            for (int i = 0; i < 1000; i++)
                fft.GetMagnitudeSpectrum(signal);

            sw.Stop();
            UnityEngine.Debug.Log($"1000 FFTs in {sw.ElapsedMilliseconds}ms");
            UnityEngine.Debug.Log($"Average: {sw.ElapsedMilliseconds / 1000.0:F3}ms");
        }

        string arch = System.Runtime.InteropServices.RuntimeInformation.ProcessArchitecture.ToString();
        UnityEngine.Debug.Log($"Architecture: {arch}");
    }
}
```

**Expected performance:**
- [ ] Intel Mac: 1000 iterations in < 1000ms (< 1ms avg)
- [ ] Apple Silicon: 1000 iterations in < 500ms (< 0.5ms avg)
- [ ] Suitable for real-time audio

**Actual result:** _________ ms (Architecture: _________)

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 9: Build Verification

Test macOS standalone build:

- [ ] File → Build Settings
- [ ] Platform: macOS
- [ ] Architecture: Apple Silicon, Intel, or Universal
- [ ] Click "Build"
- [ ] Run .app file
- [ ] Verify:
  - [ ] Runs without errors
  - [ ] Library loads correctly
  - [ ] No missing library errors
  - [ ] Audio processing works

**Test on different architectures (if available):**
- [ ] Intel Mac build works
- [ ] Apple Silicon build works
- [ ] Universal build works on both

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Troubleshooting Quick Reference

### DllNotFoundException
```bash
# Check library exists
ls -la Assets/SpectraCore/Runtime/Plugins/macOS/libspectra.dylib

# Verify platform settings
# Unity: Select library → Inspector → Platform: macOS, CPU: Universal
```

### BadImageFormatException
```bash
# Verify universal binary
lipo -info libspectra.dylib
# Should show: x86_64 arm64

# Check file format
file libspectra.dylib
# Should show: Mach-O universal binary
```

### Tests Fail
```bash
# Check Unity Console for errors
# Verify library exports
nm -gU libspectra.dylib | grep spectra_
```

### Code Signing Issues (macOS 10.15+)
```bash
# Ad-hoc sign the library
codesign -s - --force libspectra.dylib
```

**Detailed troubleshooting:** See `UNITY-002_MACOS_TESTING_GUIDE.md`

---

## Success Criteria

**UNITY-002 considered COMPLETE when all checked:**

- [ ] macOS library obtained and verified
- [ ] Universal binary confirmed (x86_64 + arm64)
- [ ] Unity Editor loads library without errors
- [ ] All 36 P/Invoke binding tests pass ✓
- [ ] All 35 Component tests pass ✓
- [ ] Standalone P/Invoke test passes (6/6) ✓
- [ ] Signal Inspector tool works on macOS ✓
- [ ] Filter Designer tool works on macOS ✓
- [ ] FFT performance acceptable ✓
- [ ] macOS standalone build succeeds ✓
- [ ] No platform-specific errors or warnings ✓

**Bonus (if multiple Macs available):**
- [ ] Tested on Intel Mac
- [ ] Tested on Apple Silicon Mac
- [ ] Performance benchmarks for both architectures

**Total score:** ____ / 11 (or ____ / 13 with bonus)

---

## Architecture Detection

**Current Mac:**
```bash
# Check architecture
uname -m
# Intel: x86_64
# Apple Silicon: arm64

# Check Unity architecture
# In Unity Console, the performance test script shows architecture
```

**Which slice is used:**
- Intel Mac → Uses x86_64 slice
- Apple Silicon → Uses arm64 slice
- Automatic selection, no configuration needed

---

## Final Steps

After all tests pass:

1. **Create completion report:**
   ```bash
   # Copy template
   cp UNITY-002_COMPLETION_TEMPLATE.md UNITY-002_COMPLETION.md
   # Fill in test results, timings, architecture info, screenshots
   ```

2. **Update integration summary:**
   - Edit `UNITY_INTEGRATION_SUMMARY.md`
   - Change macOS status from ⏳ to ✅
   - Note which architectures were tested

3. **Commit results:**
   ```bash
   git add UNITY-002_COMPLETION.md UNITY_INTEGRATION_SUMMARY.md
   git commit -m "docs: complete UNITY-002 macOS P/Invoke testing

   - All 71 Unity tests passing
   - Standalone test verified
   - Architecture: [Intel/Apple Silicon/Both]
   - Performance: [X]ms per 1024-FFT
   - Build: macOS standalone verified"
   git push
   ```

4. **All platform testing complete!**
   - ✅ Linux
   - ✅ Windows
   - ✅ macOS

---

## Quick Commands Reference

```bash
# Download artifact (with GitHub CLI)
gh run download --name libspectra-macos

# Build locally
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build

# Copy to Unity
cp build/native/libspectra.dylib Assets/SpectraCore/Runtime/Plugins/macOS/

# Compile standalone test
mcs -out:PInvokeTest.exe unity/Tests/StandalonePInvokeTest.cs

# Run standalone test
mono PInvokeTest.exe

# Verify library
file libspectra.dylib
lipo -info libspectra.dylib
otool -L libspectra.dylib
nm -gU libspectra.dylib | grep spectra_
```

---

## Estimated Time

- Download library: 5 min
- Setup Unity: 15 min (if Unity already installed)
- Run all tests: 5 min
- Validation: 10 min
- **Total: ~35 minutes**

---

## Platform-Specific Notes

### Intel Mac
- Uses x86_64 slice
- Performance similar to Linux
- Well-tested platform

### Apple Silicon Mac
- Uses arm64 slice natively
- Potentially better performance
- No Rosetta translation needed
- Unity 2021.2+ runs natively

---

**Current Status:** ⏳ Waiting for CI/CD artifacts (macOS library)

**CI/CD Progress:** Check at https://github.com/yogt1984/SpectraCore/actions

**After completion:** Follow this checklist to validate macOS build!
