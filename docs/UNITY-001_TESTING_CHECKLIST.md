# UNITY-001: Windows Testing Checklist

Quick checklist for Windows P/Invoke testing.

## Prerequisites

- [ ] Windows machine or VM available
- [ ] Unity Editor 2021.3+ installed
- [ ] Visual Studio 2022 (optional, for local builds)

---

## Step 1: Obtain Windows DLL

### From CI/CD (Recommended)

- [ ] Navigate to https://github.com/yogt1984/SpectraCore/actions
- [ ] Find latest workflow run: "Complete Unity integration with CI/CD pipeline"
- [ ] Verify workflow completed successfully (all green checkmarks)
- [ ] Scroll to bottom → Artifacts section
- [ ] Download `spectra-windows` artifact (~100 KB)
- [ ] Extract `spectra.dll` from .zip file

**Verify:**
```powershell
dir spectra.dll
# Should be ~100 KB, PE32+ executable
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
  │       └── Windows/
  │           └── x86_64/
  └── Tests/
      └── Editor/
  ```

- [ ] Copy Unity integration files:
  ```powershell
  # From SpectraCore repository
  xcopy /E /I unity\Runtime Assets\SpectraCore\Runtime
  xcopy /E /I unity\Editor Assets\SpectraCore\Editor
  xcopy /E /I unity\Tests Assets\SpectraCore\Tests
  ```

- [ ] Copy Windows DLL:
  ```powershell
  copy spectra.dll Assets\SpectraCore\Runtime\Plugins\Windows\x86_64\
  ```

**Verify in Unity:**
- [ ] Open Unity Editor
- [ ] No errors in Console
- [ ] DLL visible in Project window
- [ ] Select DLL → Inspector shows:
  - Platform: Windows ✓
  - CPU: x86_64 ✓

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 3: Run P/Invoke Binding Tests

- [ ] Open Test Runner: Window → General → Test Runner (Ctrl+Alt+T)
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
```powershell
cd path\to\SpectraCore

# Compile test program
csc /out:PInvokeTest.exe unity\Tests\StandalonePInvokeTest.cs

# Copy DLL to same directory
copy spectra.dll .
```

**Run:**
```powershell
.\PInvokeTest.exe
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

## Step 6: Test Editor Tools

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

## Step 7: Performance Testing

### FFT Performance

Create test script:

```csharp
using UnityEngine;
using Spectra;
using System.Diagnostics;

public class FFTBenchmark : MonoBehaviour
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
        }
    }
}
```

**Expected performance:**
- [ ] 1000 iterations in < 500ms
- [ ] Average < 0.5ms per FFT
- [ ] Suitable for real-time audio

**Actual result:** _________ ms

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 8: Build Verification

Test Windows standalone build:

- [ ] File → Build Settings
- [ ] Platform: Windows
- [ ] Architecture: x86_64
- [ ] Click "Build"
- [ ] Run .exe file
- [ ] Verify:
  - [ ] Runs without errors
  - [ ] DLL loads correctly
  - [ ] No missing DLL errors
  - [ ] Audio processing works

Status: ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Troubleshooting Quick Reference

### DllNotFoundException
```powershell
# Check DLL exists
dir Assets\SpectraCore\Runtime\Plugins\Windows\x86_64\spectra.dll

# Verify platform settings
# Unity: Select DLL → Inspector → Platform: Windows, CPU: x86_64
```

### BadImageFormatException
```powershell
# Verify DLL is 64-bit
dumpbin /headers spectra.dll | findstr machine
# Should show: x64
```

### Tests Fail
```powershell
# Check Unity Console for errors
# Verify DLL exports
dumpbin /exports spectra.dll | findstr spectra_
```

**Detailed troubleshooting:** See `UNITY-001_WINDOWS_TESTING_GUIDE.md`

---

## Success Criteria

**UNITY-001 considered COMPLETE when all checked:**

- [ ] Windows DLL obtained and verified
- [ ] Unity Editor loads DLL without errors
- [ ] All 36 P/Invoke binding tests pass ✓
- [ ] All 35 Component tests pass ✓
- [ ] Standalone P/Invoke test passes (6/6) ✓
- [ ] Signal Inspector tool works on Windows ✓
- [ ] Filter Designer tool works on Windows ✓
- [ ] FFT performance acceptable (< 1ms per 1024-point FFT) ✓
- [ ] Windows standalone build succeeds ✓
- [ ] No platform-specific errors or warnings ✓

**Total score:** ____ / 10

---

## Final Steps

After all tests pass:

1. **Create completion report:**
   ```powershell
   # Copy template
   copy UNITY-001_COMPLETION_TEMPLATE.md UNITY-001_COMPLETION.md
   # Fill in test results, timings, screenshots
   ```

2. **Update integration summary:**
   - Edit `UNITY_INTEGRATION_SUMMARY.md`
   - Change Windows status from ⏳ to ✅

3. **Commit results:**
   ```bash
   git add UNITY-001_COMPLETION.md UNITY_INTEGRATION_SUMMARY.md
   git commit -m "Complete UNITY-001: Windows P/Invoke testing

   - All 71 Unity tests passing
   - Standalone test verified
   - Performance: [X]ms per 1024-FFT
   - Build: Windows x64 standalone verified"
   git push
   ```

4. **Proceed to UNITY-002 (macOS testing)**

---

## Quick Commands Reference

```powershell
# Download artifact (with GitHub CLI)
gh run download --name spectra-windows

# Build locally
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# Copy to Unity
copy build\native\Release\spectra.dll Assets\SpectraCore\Runtime\Plugins\Windows\x86_64\

# Compile standalone test
csc /out:PInvokeTest.exe unity\Tests\StandalonePInvokeTest.cs

# Run standalone test
.\PInvokeTest.exe

# Verify DLL
dumpbin /headers spectra.dll | findstr machine
dumpbin /exports spectra.dll
```

---

## Estimated Time

- Download DLL: 5 min
- Setup Unity: 15 min (if Unity already installed)
- Run all tests: 5 min
- Validation: 10 min
- **Total: ~35 minutes**

---

**Current Status:** ⏳ Waiting for CI/CD artifacts (Windows DLL)

**CI/CD Progress:** Check at https://github.com/yogt1984/SpectraCore/actions
