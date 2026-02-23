# macOS Validation Requirements

**Task:** UNITY-002
**Purpose:** Manual validation of SpectraCore on macOS (optional)
**Status:** CI/CD validates automatically; manual testing is for additional verification

---

## Overview

The CI/CD workflow automatically validates macOS builds by:
- ✅ Compiling universal binary (x86_64 + arm64) with Clang
- ✅ Running 68 native C++ tests
- ✅ Generating macOS library artifact

**Manual validation is optional** but provides additional confidence for:
- Unity Editor integration
- Editor tools (Signal Inspector, Filter Designer)
- Performance benchmarking on both Intel and Apple Silicon
- Real-world usage scenarios

---

## Prerequisites

### Required

- **Operating System:** macOS 10.15 (Catalina) or newer
- **Architecture:** Intel (x86_64) or Apple Silicon (arm64)
- **Runtime:** Mono or .NET SDK (for standalone tests)
- **macOS Library:** Download from CI/CD artifacts

### Optional (for Unity testing)

- **Unity Editor:** 2021.3 LTS or newer
- **Unity Hub:** Latest version
- **Disk Space:** ~5 GB for Unity + project

### Optional (for local building)

- **Xcode Command Line Tools:** `xcode-select --install`
- **CMake:** 3.15 or newer (`brew install cmake`)
- **Git:** For cloning repository

---

## Quick Start (Minimal Validation)

**Time:** 5-10 minutes
**Requirement:** Any Mac with Mono or .NET

### Step 1: Download macOS Library

1. Go to: https://github.com/yogt1984/SpectraCore/actions
2. Click on latest successful workflow run
3. Scroll to "Artifacts" section at bottom
4. Download: `libspectra-macos` (~250 KB .zip file)
5. Extract `libspectra.dylib` from the zip

### Step 2: Run Automated Test Script

```bash
# Clone repository (if not already done)
git clone https://github.com/yogt1984/SpectraCore.git
cd SpectraCore

# Copy downloaded library to repository
cp /path/to/libspectra.dylib .

# Run automated test script
./test-macos.sh libspectra.dylib
```

**Expected Output:**
```
========================================
  SpectraCore macOS Testing Script
  UNITY-002: P/Invoke Testing
========================================

[1/7] Verifying library...           [PASS]
[2/7] Checking architecture...       [PASS]
    ✓ Universal binary (x86_64 + arm64)
[3/7] Checking dependencies...       [PASS]
[4/7] Checking exports...            [PASS]
[5/7] Detecting architecture...      [INFO]
    System: arm64 (or x86_64)
[6/7] Running standalone test...     [PASS]
[7/7] Unity testing...               [SKIP]
[8/8] Performance test...            [PASS]

========================================
  All standalone tests passed!
========================================
```

### Step 3: Verify Results

- ✅ All 6 standalone tests pass
- ✅ Universal binary verified (x86_64 + arm64)
- ✅ FFT performance acceptable
- ✅ No DllNotFoundException
- ✅ No marshaling errors

**If successful:** macOS validation complete! ✅

---

## Full Validation (with Unity Editor)

**Time:** 45-60 minutes
**Requirement:** macOS + Unity Editor

### Step 1: Install Unity

1. **Download Unity Hub:**
   - https://unity.com/download

2. **Install Unity Editor:**
   - Open Unity Hub
   - Go to "Installs" tab
   - Click "Install Editor"
   - Select Unity 2021.3 LTS (or newer)
   - Include macOS Build Support

3. **For Apple Silicon Macs:**
   - Unity 2021.2+ runs natively (no Rosetta needed)
   - Choose "Apple Silicon" version if available

### Step 2: Setup Unity Project

```bash
# Navigate to repository
cd /path/to/SpectraCore

# Run setup script with Unity project path
./test-macos.sh libspectra.dylib "/Users/username/UnityProjects/SpectraTest"

# Or manually:
# 1. Create new Unity project
# 2. Copy unity/* to Assets/SpectraCore/
# 3. Copy libspectra.dylib to Assets/SpectraCore/Runtime/Plugins/macOS/
```

### Step 3: Run Unity Editor Tests

1. **Open Unity project**
   - Unity Hub → Projects → Open
   - Select your project folder

2. **Open Test Runner**
   - Menu: Window → General → Test Runner
   - Or custom shortcut if configured

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
using System.Runtime.InteropServices;

public class MacPerformanceTest : MonoBehaviour
{
    void Start()
    {
        BenchmarkFFT();
        DetectArchitecture();
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

    void DetectArchitecture()
    {
        string arch = RuntimeInformation.ProcessArchitecture.ToString();
        UnityEngine.Debug.Log($"Running on: {arch}");
        UnityEngine.Debug.Log($"macOS: {SystemInfo.operatingSystem}");
    }
}
```

**Expected Performance:**
- **Intel Mac:** < 1.0ms per 1024-point FFT
- **Apple Silicon:** < 0.5ms per 1024-point FFT (potentially faster!)

### Step 6: Build macOS Standalone

1. **Open Build Settings:**
   - Menu: File → Build Settings

2. **Configure build:**
   - Platform: macOS
   - Architecture:
     - Apple Silicon (arm64)
     - Intel (x86_64)
     - Universal (both)

3. **Build and run:**
   - Click "Build and Run"
   - Choose output folder
   - Wait for build completion
   - .app runs automatically

4. **Verify:**
   - ✅ Application launches
   - ✅ No library errors
   - ✅ Audio processing works
   - ✅ No console errors

**Test on both architectures (if available):**
- Build Universal → runs on both Intel and Apple Silicon
- No Rosetta translation needed

---

## Validation Checklist

### Minimal Validation (Recommended)

- [ ] macOS library downloaded from CI/CD
- [ ] `test-macos.sh` executed successfully
- [ ] All 6 standalone tests passed
- [ ] Universal binary verified (x86_64 + arm64)
- [ ] Performance acceptable
- [ ] No errors reported

**If all checked:** macOS validation complete! ✅

### Full Validation (Comprehensive)

**Standalone Tests:**
- [ ] Library verification passed
- [ ] Universal binary verified (lipo -info)
- [ ] Dependency check passed (otool -L)
- [ ] Export verification passed (nm -gU)
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
- [ ] FFT (1024-point): Intel < 1ms, Apple Silicon < 0.5ms
- [ ] STFT acceptable for real-time
- [ ] Filter processing efficient

**Build:**
- [ ] macOS standalone build succeeds
- [ ] .app runs without errors
- [ ] Audio processing functional in build
- [ ] Works on target architecture

**Universal Binary (if both Macs available):**
- [ ] Tested on Intel Mac
- [ ] Tested on Apple Silicon Mac
- [ ] Performance comparison documented

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
- Verify library is in: `Assets/SpectraCore/Runtime/Plugins/macOS/`
- Check filename is: `libspectra.dylib` (with "lib" prefix)
- Verify library platform settings in Unity Inspector:
  - Platform: macOS
  - CPU: Universal (or both x86_64 + ARM64)
- Check permissions: `chmod +x libspectra.dylib`

**2. BadImageFormatException**

**Symptoms:**
```
BadImageFormatException: Format of the executable is invalid
```

**Cause:** Not a universal binary or architecture mismatch

**Solutions:**
- Verify universal binary:
  ```bash
  lipo -info libspectra.dylib
  # Should show: x86_64 arm64
  ```
- Rebuild as universal binary:
  ```bash
  cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
  cmake --build build
  ```

**3. Code Signing Issues (macOS 10.15+)**

**Symptoms:**
```
Code signature invalid
Library cannot be opened
```

**Solutions:**
- Ad-hoc sign the library:
  ```bash
  codesign -s - --force libspectra.dylib
  ```
- For distribution:
  ```bash
  codesign -s "Developer ID Application: Your Name" libspectra.dylib
  ```

**4. Library Not Loading on Apple Silicon**

**Symptoms:**
```
DllNotFoundException on M1/M2/M3 Mac
```

**Solutions:**
- Verify arm64 slice exists:
  ```bash
  lipo -info libspectra.dylib
  # Must show: arm64
  ```
- Ensure Unity runs natively (not Rosetta):
  - Right-click Unity.app → Get Info
  - Uncheck "Open using Rosetta"
- Use native Apple Silicon Unity build (2021.2+)

**5. Slow Performance**

**Expected:**
- Intel Mac: FFT (1024) < 1ms
- Apple Silicon: FFT (1024) < 0.5ms

**Solutions:**
- Verify Release build (not Debug)
- Check Activity Monitor for CPU usage
- Profile in Unity Profiler
- For Apple Silicon, ensure native execution:
  ```bash
  # Check if running via Rosetta
  sysctl sysctl.proc_translated
  # Should return 0 (native), not 1 (Rosetta)
  ```

**For more troubleshooting:** See `UNITY-002_MACOS_TESTING_GUIDE.md`

---

## Success Criteria

### Minimal (Required)
- ✅ CI/CD macOS build passes (automatic)
- ✅ Standalone P/Invoke test passes (6/6)
- ✅ Universal binary verified

### Standard (Recommended)
- Above +
- ✅ Unity Editor tests pass (71/71)
- ✅ Performance acceptable

### Comprehensive (Full Validation)
- Above +
- ✅ Editor tools functional
- ✅ macOS standalone build succeeds
- ✅ Tested on both architectures (if available)

---

## Universal Binary Benefits

**What is a universal binary?**
- Single library file containing code for multiple architectures
- Contains both x86_64 (Intel) and arm64 (Apple Silicon) slices
- macOS automatically loads the correct slice

**Benefits:**
- ✅ Works on Intel Macs natively
- ✅ Works on Apple Silicon natively
- ✅ No Rosetta translation needed
- ✅ Optimal performance on both
- ✅ Single file to distribute

**Verification:**
```bash
# Check architectures
lipo -info libspectra.dylib
# Expected: Architectures in the fat file: libspectra.dylib are: x86_64 arm64

# File size
du -h libspectra.dylib
# Expected: ~250 KB (both architectures combined)
```

---

## Recording Results

### Use Completion Template

1. **Copy template:**
   ```bash
   cp UNITY-002_COMPLETION_TEMPLATE.md UNITY-002_COMPLETION.md
   ```

2. **Fill in results:**
   - Test results (pass/fail counts)
   - Architecture tested (Intel/Apple Silicon/Both)
   - Performance metrics for each architecture
   - Screenshots
   - Issues encountered

3. **Commit results:**
   ```bash
   git add UNITY-002_COMPLETION.md
   git commit -m "docs: complete UNITY-002 macOS validation"
   git push
   ```

---

## Time Estimates

### Minimal Validation
- Download library: 2 min
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

### Testing on Both Architectures
- Above: 75 min × 2 (if testing on two separate Macs)
- Performance comparison: 10 min
- **Total: ~160 minutes**

---

## When to Perform Manual Validation

### You Should Validate If:
- Deploying to macOS users
- Using editor tools on macOS
- Need performance benchmarks for Intel vs Apple Silicon
- Encountered macOS-specific issues
- Want comprehensive verification
- Testing on both architectures

### You Can Skip If:
- CI/CD tests pass (68/68 native tests)
- Only targeting Linux/Windows
- Trust automated CI/CD validation
- Time constraints
- Don't have access to Mac

**Remember:** CI/CD already validates core functionality!

---

## Resources

### Documentation
- **Comprehensive Guide:** `UNITY-002_MACOS_TESTING_GUIDE.md` (detailed)
- **Quick Checklist:** `UNITY-002_TESTING_CHECKLIST.md`
- **Completion Template:** `UNITY-002_COMPLETION_TEMPLATE.md`
- **Navigation:** `UNITY-002_README.md`

### Automation
- **Test Script:** `test-macos.sh`
- **Standalone Test:** `unity/Tests/StandalonePInvokeTest.cs`

### CI/CD
- **Workflow:** `.github/workflows/build-multiplatform.yml`
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
3. Optionally run `test-macos.sh` (5 min)
4. Mark UNITY-002 complete

**For comprehensive validation:**
1. Download macOS library from CI/CD
2. Install Unity Editor on Mac
3. Run full test suite (71 tests)
4. Test editor tools
5. Benchmark performance (note architecture)
6. Fill completion report

**For multi-architecture validation:**
1. Test on Intel Mac
2. Test on Apple Silicon Mac
3. Compare performance
4. Document differences
5. Verify universal binary works on both

**Either approach validates macOS compatibility!**

The choice depends on your requirements, available hardware, and confidence in automated testing.

---

## Platform-Specific Notes

### Intel Macs
- Uses x86_64 slice of universal binary
- Performance similar to Linux
- Well-tested platform
- Being phased out by Apple

### Apple Silicon Macs
- Uses arm64 slice of universal binary
- Potentially better performance
- Energy efficient
- Modern architecture
- Future of Mac platform

### Universal Binary
- Best choice for distribution
- Single file works on all Macs
- Automatic architecture selection
- No configuration needed
- ~2x file size (contains both slices)
