# Windows Validation Requirements

**Task:** UNITY-001
**Purpose:** Manual validation of SpectraCore on Windows (optional)
**Status:** CI/CD validates automatically; manual testing is for additional verification

---

## Overview

The CI/CD workflow automatically validates Windows builds by:
- ✅ Compiling native library with MSVC 2022
- ✅ Running 68 native C++ tests
- ✅ Generating Windows DLL artifact

**Manual validation is optional** but provides additional confidence for:
- Unity Editor integration
- Editor tools (Signal Inspector, Filter Designer)
- Performance benchmarking
- Real-world usage scenarios

---

## Prerequisites

### Required

- **Operating System:** Windows 10/11 (64-bit)
- **.NET Framework:** 4.7.2 or newer (included with Windows 10/11)
- **Windows DLL:** Download from CI/CD artifacts

### Optional (for Unity testing)

- **Unity Editor:** 2021.3 LTS or newer
- **Unity Hub:** Latest version
- **Disk Space:** ~5 GB for Unity + project

### Optional (for local building)

- **Visual Studio:** 2022 Community Edition or higher
- **CMake:** 3.15 or newer
- **Git:** For cloning repository

---

## Quick Start (Minimal Validation)

**Time:** 5-10 minutes
**Requirement:** Any Windows machine with .NET

### Step 1: Download Windows DLL

1. Go to: https://github.com/yogt1984/SpectraCore/actions
2. Click on latest successful workflow run
3. Scroll to "Artifacts" section at bottom
4. Download: `spectra-windows` (~100 KB .zip file)
5. Extract `spectra.dll` from the zip

### Step 2: Run Automated Test Script

```powershell
# Clone repository (if not already done)
git clone https://github.com/yogt1984/SpectraCore.git
cd SpectraCore

# Copy downloaded DLL to repository
copy path\to\spectra.dll .

# Run automated test script
test-windows.bat spectra.dll
```

**Expected Output:**
```
========================================
  SpectraCore Windows Testing Script
  UNITY-001: P/Invoke Testing
========================================

[1/6] Verifying DLL...                [PASS]
[2/6] Checking DLL architecture...    [PASS]
[3/6] Checking DLL exports...         [PASS]
[4/6] Running standalone P/Invoke...  [PASS]
[5/6] Unity testing...                [SKIP] (no project specified)
[6/6] Performance test...             [PASS]

========================================
  Testing Summary
========================================
  All standalone tests passed!
```

### Step 3: Verify Results

- ✅ All 6 standalone tests pass
- ✅ FFT performance < 1ms per 1024-point transform
- ✅ No DllNotFoundException
- ✅ No marshaling errors

**If successful:** Windows validation complete! ✅

---

## Full Validation (with Unity Editor)

**Time:** 45-60 minutes
**Requirement:** Windows + Unity Editor

### Step 1: Install Unity

1. **Download Unity Hub:**
   - https://unity.com/download

2. **Install Unity Editor:**
   - Open Unity Hub
   - Go to "Installs" tab
   - Click "Add"
   - Select Unity 2021.3 LTS (or newer)
   - Include modules:
     - ✅ Windows Build Support (IL2CPP)
     - ✅ Windows Build Support (Mono)

### Step 2: Setup Unity Project

```powershell
# Navigate to repository
cd path\to\SpectraCore

# Run setup script with Unity project path
test-windows.bat spectra.dll "C:\UnityProjects\SpectraTest"

# Or manually:
# 1. Create new Unity project
# 2. Copy unity/* to Assets/SpectraCore/
# 3. Copy spectra.dll to Assets/SpectraCore/Runtime/Plugins/Windows/x86_64/
```

### Step 3: Run Unity Editor Tests

1. **Open Unity project**
   - Unity Hub → Projects → Open
   - Select your project folder

2. **Open Test Runner**
   - Menu: Window → General → Test Runner
   - Or press: Ctrl+Alt+T

3. **Run EditMode tests**
   - Click "EditMode" tab
   - Click "Run All" button
   - Wait for completion (~1-2 minutes)

**Expected Results:**
```
✅ SpectraNativeTests:     36/36 passed
✅ SpectraComponentTests:  35/35 passed
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ Total:                  71/71 passed
```

### Step 4: Test Editor Tools

**Signal Inspector:**
1. Menu: Window → Spectra → Signal Inspector
2. Load a WAV/MP3 audio file
3. Verify displays:
   - ✅ Waveform renders correctly
   - ✅ Spectrum shows frequency content
   - ✅ Spectrogram displays time-frequency
4. Test exports:
   - ✅ Export Waveform CSV
   - ✅ Export Spectrum CSV
   - ✅ Export Spectrogram CSV

**Filter Designer:**
1. Menu: Window → Spectra → Filter Designer
2. Select filter type: Butterworth
3. Adjust order slider (1-10)
4. Adjust cutoff frequency
5. Verify:
   - ✅ Bode plot updates in real-time
   - ✅ Magnitude response shows correctly
   - ✅ Phase response shows correctly
6. Test export:
   - ✅ Copy coefficients to clipboard
   - ✅ Export to C# class file

### Step 5: Performance Benchmarking

Create test script in Unity:

```csharp
using UnityEngine;
using Spectra;
using System.Diagnostics;

public class WindowsPerformanceTest : MonoBehaviour
{
    void Start()
    {
        BenchmarkFFT();
        BenchmarkSTFT();
        BenchmarkFilter();
    }

    void BenchmarkFFT()
    {
        using (var fft = new FFTAnalyzer(1024))
        {
            float[] signal = new float[1024];
            var sw = Stopwatch.StartNew();

            for (int i = 0; i < 1000; i++)
                fft.GetMagnitudeSpectrum(signal);

            sw.Stop();
            UnityEngine.Debug.Log($"FFT: {sw.ElapsedMilliseconds}ms for 1000 iterations");
            UnityEngine.Debug.Log($"Average: {sw.ElapsedMilliseconds / 1000.0:F3}ms per FFT");
        }
    }

    void BenchmarkSTFT()
    {
        using (var stft = new STFTAnalyzer(512, 256, WindowType.Hann))
        {
            float[] signal = new float[10240];
            var sw = Stopwatch.StartNew();

            for (int i = 0; i < 100; i++)
            {
                var (real, imag, frames) = stft.Analyze(signal);
            }

            sw.Stop();
            UnityEngine.Debug.Log($"STFT: {sw.ElapsedMilliseconds}ms for 100 iterations");
        }
    }

    void BenchmarkFilter()
    {
        var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
        float[] signal = new float[1000];

        var sw = Stopwatch.StartNew();
        for (int i = 0; i < 1000; i++)
        {
            DSP.LFilter(b, a, signal);
        }
        sw.Stop();

        UnityEngine.Debug.Log($"Filter: {sw.ElapsedMilliseconds}ms for 1000 iterations");
    }
}
```

**Expected Performance:**
- FFT (1024-point): < 1.0ms per transform
- STFT (512-point): < 5.0ms per batch
- Filter (1000 samples): < 2.0ms

### Step 6: Build Windows Standalone

1. **Open Build Settings:**
   - Menu: File → Build Settings

2. **Configure build:**
   - Platform: Windows
   - Architecture: x86_64
   - Scripting Backend: IL2CPP or Mono

3. **Build and run:**
   - Click "Build and Run"
   - Choose output folder
   - Wait for build completion
   - Executable runs automatically

4. **Verify:**
   - ✅ Application launches
   - ✅ No DLL errors
   - ✅ Audio processing works
   - ✅ No console errors

---

## Validation Checklist

### Minimal Validation (Recommended)

- [ ] Windows DLL downloaded from CI/CD
- [ ] `test-windows.bat` executed successfully
- [ ] All 6 standalone tests passed
- [ ] Performance acceptable (< 1ms FFT)
- [ ] No errors reported

**If all checked:** Windows validation complete! ✅

### Full Validation (Comprehensive)

**Standalone Tests:**
- [ ] DLL verification passed
- [ ] Architecture check passed (x64)
- [ ] Export verification passed
- [ ] Standalone P/Invoke test: 6/6 passed
- [ ] Performance benchmark acceptable

**Unity Editor Tests:**
- [ ] Unity project setup completed
- [ ] P/Invoke binding tests: 36/36 passed
- [ ] Component tests: 35/35 passed
- [ ] Total: 71/71 tests passed

**Editor Tools:**
- [ ] Signal Inspector opens and works
- [ ] Signal Inspector exports CSV correctly
- [ ] Filter Designer opens and works
- [ ] Filter Designer exports coefficients

**Performance:**
- [ ] FFT (1024-point) < 1ms
- [ ] STFT acceptable for real-time
- [ ] Filter processing < 2ms

**Build:**
- [ ] Windows standalone build succeeds
- [ ] Executable runs without errors
- [ ] Audio processing functional in build

**If all checked:** Comprehensive validation complete! ✅

---

## Troubleshooting

### Common Issues

**1. DllNotFoundException**

**Symptoms:**
```
DllNotFoundException: Unable to load DLL 'spectra'
```

**Solutions:**
- Verify DLL is in: `Assets/SpectraCore/Runtime/Plugins/Windows/x86_64/`
- Check filename is exactly: `spectra.dll` (not `libspectra.dll`)
- Verify DLL platform settings in Unity Inspector:
  - Platform: Windows
  - CPU: x86_64

**2. BadImageFormatException**

**Symptoms:**
```
BadImageFormatException: An attempt was made to load a program with an incorrect format
```

**Cause:** Architecture mismatch (32-bit vs 64-bit)

**Solutions:**
- Verify DLL is 64-bit:
  ```powershell
  dumpbin /headers spectra.dll | findstr machine
  # Should show: x64
  ```
- Ensure Unity Editor is 64-bit (Unity 2021+ is always 64-bit)

**3. EntryPointNotFoundException**

**Symptoms:**
```
EntryPointNotFoundException: Unable to find entry point 'spectra_fft_create'
```

**Solutions:**
- Verify DLL exports:
  ```powershell
  dumpbin /exports spectra.dll | findstr spectra_
  # Should show ~50+ exported functions
  ```
- Rebuild DLL if exports are missing

**4. Tests Fail with AccessViolationException**

**Symptoms:**
```
AccessViolationException: Attempted to read or write protected memory
```

**Causes:**
- Incorrect marshaling
- Buffer size mismatch
- Using disposed handle

**Solutions:**
- Check array sizes match expected values
- Verify using `using` statements for IDisposable objects
- See `SpectraNative.cs` for correct marshaling examples

**5. Slow Performance**

**Expected:**
- FFT (1024-point): < 1ms
- If slower, check:

**Solutions:**
- Verify using Release build (not Debug):
  ```powershell
  # When building locally
  cmake --build build --config Release
  ```
- Close other applications
- Check Task Manager for CPU usage
- Profile in Unity Profiler

**For more troubleshooting:** See `UNITY-001_WINDOWS_TESTING_GUIDE.md`

---

## Success Criteria

### Minimal (Required)
- ✅ CI/CD Windows build passes (automatic)
- ✅ Standalone P/Invoke test passes (6/6)

### Standard (Recommended)
- Above +
- ✅ Unity Editor tests pass (71/71)
- ✅ Performance acceptable

### Comprehensive (Full Validation)
- Above +
- ✅ Editor tools functional
- ✅ Windows standalone build succeeds
- ✅ Example projects work

---

## Recording Results

### Use Completion Template

1. **Copy template:**
   ```powershell
   copy UNITY-001_COMPLETION_TEMPLATE.md UNITY-001_COMPLETION.md
   ```

2. **Fill in results:**
   - Test results (pass/fail counts)
   - Performance metrics
   - Screenshots
   - Issues encountered

3. **Commit results:**
   ```bash
   git add UNITY-001_COMPLETION.md
   git commit -m "docs: complete UNITY-001 Windows validation"
   git push
   ```

---

## Time Estimates

### Minimal Validation
- Download DLL: 2 min
- Run test script: 3 min
- Verify results: 2 min
- **Total: ~10 minutes**

### Standard Validation
- Above: 10 min
- Install Unity: 20 min (first time)
- Setup project: 10 min
- Run Unity tests: 5 min
- **Total: ~45 minutes**

### Comprehensive Validation
- Above: 45 min
- Test editor tools: 10 min
- Performance benchmarks: 5 min
- Build standalone: 5 min
- Fill completion report: 10 min
- **Total: ~75 minutes**

---

## When to Perform Manual Validation

### You Should Validate If:
- Deploying to Windows users
- Using editor tools on Windows
- Need performance benchmarks for Windows
- Encountered Windows-specific issues
- Want comprehensive verification

### You Can Skip If:
- CI/CD tests pass (68/68 native tests)
- Only targeting Linux/macOS
- Trust automated CI/CD validation
- Time constraints

**Remember:** CI/CD already validates core functionality!

---

## Resources

### Documentation
- **Comprehensive Guide:** `UNITY-001_WINDOWS_TESTING_GUIDE.md` (21 pages)
- **Quick Checklist:** `UNITY-001_TESTING_CHECKLIST.md`
- **Completion Template:** `UNITY-001_COMPLETION_TEMPLATE.md`
- **Status Tracking:** `UNITY-001_STATUS.md`

### Automation
- **Test Script:** `test-windows.bat`
- **Standalone Test:** `unity/Tests/StandalonePInvokeTest.cs`

### CI/CD
- **Workflow:** `.github/workflows/ci.yml`
- **Actions Page:** https://github.com/yogt1984/SpectraCore/actions
- **Artifacts:** Download from successful workflow runs

### Support
- **API Reference:** `docs/API_Reference.md`
- **Tutorials:** `docs/Tutorials.md`
- **Integration Guide:** `UNITY_INTEGRATION_SUMMARY.md`

---

## Summary

**For most users:**
1. Wait for CI/CD to complete
2. Verify artifacts exist
3. Optionally run `test-windows.bat` (5 min)
4. Mark UNITY-001 complete

**For comprehensive validation:**
1. Download Windows DLL from CI/CD
2. Install Unity Editor on Windows
3. Run full test suite (71 tests)
4. Test editor tools
5. Benchmark performance
6. Fill completion report

**Either approach validates Windows compatibility!**

The choice depends on your requirements, time availability, and confidence in automated testing.
