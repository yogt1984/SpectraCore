# UNITY-001: Windows P/Invoke Testing Guide

**Task ID:** UNITY-001
**Status:** ⏳ IN PROGRESS
**Platform:** Windows (x86_64)
**Prerequisites:** Windows machine or VM, Unity Editor 2021.3+

---

## Objective

Verify that the SpectraCore P/Invoke bindings work correctly on Windows by:
1. Obtaining the Windows native library (spectra.dll)
2. Running Unity Editor tests
3. Running standalone P/Invoke tests
4. Validating all functionality

---

## Part 1: Obtain Windows DLL

### Option A: Download from GitHub Actions (Recommended)

The CI/CD workflow is currently building the Windows library automatically.

**Steps:**

1. **Wait for workflow completion:**
   - Navigate to: https://github.com/yogt1984/SpectraCore/actions
   - Find the latest workflow run: "Complete Unity integration with CI/CD pipeline"
   - Wait for all jobs to complete (green checkmarks)
   - Expected time: 10-15 minutes

2. **Download Windows artifact:**
   - Scroll to bottom of workflow run page
   - Find artifact: `spectra-windows` (~100 KB)
   - Click to download (downloads as .zip file)

3. **Extract the DLL:**
   ```powershell
   # On Windows
   Expand-Archive spectra-windows.zip -DestinationPath .
   # Should contain: spectra.dll
   ```

4. **Verify DLL file:**
   ```powershell
   dir spectra.dll
   # Should be ~100 KB, PE32+ executable
   ```

### Option B: Build Locally on Windows

**Requirements:**
- Visual Studio 2022 (Community Edition or higher)
- CMake 3.15+
- Git

**Build Steps:**

```powershell
# Clone repository (if not already done)
git clone https://github.com/yogt1984/SpectraCore.git
cd SpectraCore

# Configure with Visual Studio generator
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build Release configuration
cmake --build build --config Release

# DLL location
dir build\native\Release\spectra.dll
```

**Expected output:**
```
spectra.dll - ~100 KB PE32+ executable
```

### Option C: Cross-Compile from Linux (Advanced)

Requires MinGW installation:

```bash
sudo apt-get install mingw-w64
cd /home/onat/SpectraCore
./build-windows-from-linux.sh
```

---

## Part 2: Setup Unity Project on Windows

### Install Unity

1. **Download Unity Hub:**
   - https://unity.com/download

2. **Install Unity Editor 2021.3 LTS or newer:**
   - Open Unity Hub
   - Go to Installs tab
   - Add: Unity 2021.3.x LTS (or newer)

3. **Create or Open Project:**

   **Option 1: Import SpectraCore package**
   ```powershell
   # Create new Unity project
   # Unity Hub → New Project → 3D template

   # Copy SpectraCore Unity integration
   cd path\to\SpectraCore
   xcopy /E /I unity\* "C:\UnityProjects\SpectraTest\Assets\SpectraCore\"
   ```

   **Option 2: Use Unity Package (from CI/CD)**
   - Download `SpectraCore-Unity-Package` artifact from GitHub Actions
   - Extract SpectraCore-Unity.tar.gz
   - Import into Unity project

### Install Windows DLL

**Copy DLL to Unity Plugins folder:**

```powershell
# Navigate to Unity project
cd "C:\UnityProjects\SpectraTest"

# Create Plugins directory structure
mkdir -p "Assets\SpectraCore\Runtime\Plugins\Windows\x86_64"

# Copy the DLL
copy spectra.dll "Assets\SpectraCore\Runtime\Plugins\Windows\x86_64\"
```

**Verify in Unity:**
1. Open Unity Editor
2. Navigate to: `Assets/SpectraCore/Runtime/Plugins/Windows/x86_64/`
3. Select `spectra.dll`
4. In Inspector, verify:
   - ✅ Platform: Windows
   - ✅ CPU: x86_64
   - ✅ Load on startup: checked

---

## Part 3: Run Unity Editor Tests

### Open Test Runner

1. **Open Test Runner window:**
   - Menu: Window → General → Test Runner
   - Or: Ctrl+Alt+T

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

**If any tests fail:**
- Check Console for error messages
- Verify DLL is in correct location
- Check DLL is 64-bit (not 32-bit)
- Verify Unity platform settings
- See Troubleshooting section below

---

## Part 4: Run Standalone P/Invoke Test

This test runs outside Unity to verify P/Invoke works in standalone .NET applications.

**Test file:** `unity/Tests/StandalonePInvokeTest.cs`

### Compile and Run with .NET Framework

**Requirements:**
- .NET Framework 4.7.2+ (included with Windows 10/11)

**Steps:**

```powershell
# Navigate to project
cd path\to\SpectraCore

# Compile test program
csc /out:PInvokeTest.exe unity\Tests\StandalonePInvokeTest.cs

# Copy DLL to same directory
copy spectra.dll .

# Run test
.\PInvokeTest.exe
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
  Can load spectra.dll from current directory
  ✓ PASSED

=== All Tests Passed! ===
Windows P/Invoke bindings are working correctly.
```

### Compile and Run with .NET Core/5+

**Alternative using modern .NET:**

```powershell
# Create temporary project
mkdir StandaloneTest
cd StandaloneTest

# Initialize .NET project
dotnet new console

# Copy test code
copy ..\unity\Tests\StandalonePInvokeTest.cs Program.cs

# Copy DLL
copy ..\spectra.dll .

# Run
dotnet run
```

---

## Part 5: Validation Checklist

### DLL Validation

- [ ] File exists: `spectra.dll`
- [ ] File size: ~100 KB (acceptable range: 80-150 KB)
- [ ] File type: PE32+ executable (x86-64)
- [ ] Can be loaded by Unity
- [ ] No dependency errors

**Verify with PowerShell:**
```powershell
Get-FileHash spectra.dll
Get-Item spectra.dll | Format-List
```

### Unity Test Validation

- [ ] Unity Editor opens without errors
- [ ] DLL loads on Unity startup
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

public class PerformanceTest : MonoBehaviour
{
    void Start()
    {
        // Test FFT performance
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
}
```

**Expected performance:**
- 1024-point FFT: < 0.5ms per iteration
- 1000 iterations: < 500ms total
- Should be suitable for real-time audio (44.1kHz)

### Memory Leak Test

```csharp
using UnityEngine;
using Spectra;

public class MemoryLeakTest : MonoBehaviour
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

1. **Verify DLL location:**
   ```powershell
   dir "Assets\SpectraCore\Runtime\Plugins\Windows\x86_64\spectra.dll"
   ```

2. **Check DLL naming:**
   - Should be: `spectra.dll` (not `libspectra.dll`)
   - Case-sensitive even on Windows in Unity

3. **Verify platform settings:**
   - Select DLL in Unity
   - Inspector → Platform: Windows
   - CPU: x86_64

4. **Try placing in Assets root:**
   ```powershell
   copy spectra.dll Assets\
   ```

5. **Check dependencies:**
   ```powershell
   # Use Dependency Walker or dumpbin
   dumpbin /dependents spectra.dll
   ```

### Issue: BadImageFormatException

**Error:**
```
BadImageFormatException: An attempt was made to load a program with an incorrect format.
```

**Cause:** Architecture mismatch (32-bit vs 64-bit)

**Solutions:**

1. **Verify Unity editor is 64-bit:**
   - Unity 2021+ is always 64-bit

2. **Verify DLL is 64-bit:**
   ```powershell
   dumpbin /headers spectra.dll | findstr machine
   # Should show: x64
   ```

3. **Rebuild DLL with correct architecture:**
   ```powershell
   cmake -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release
   ```

### Issue: EntryPointNotFoundException

**Error:**
```
EntryPointNotFoundException: Unable to find an entry point named 'spectra_fft_create' in DLL 'spectra'.
```

**Solutions:**

1. **Verify DLL exports:**
   ```powershell
   dumpbin /exports spectra.dll
   # Should list all spectra_* functions
   ```

2. **Check function name mangling:**
   - C++ functions must be `extern "C"` to avoid name mangling
   - Verify in `spectra_abi.h`:
   ```cpp
   #ifdef __cplusplus
   extern "C" {
   #endif
   ```

3. **Rebuild with correct export settings**

### Issue: Tests Fail with AccessViolationException

**Error:**
```
AccessViolationException: Attempted to read or write protected memory.
```

**Causes:**
- Incorrect marshaling
- Buffer overruns
- Disposed handles being used

**Solutions:**

1. **Check array sizes:**
   - Input arrays must match expected sizes
   - Output arrays must be pre-allocated

2. **Verify handle lifetime:**
   - Don't use disposed objects
   - Use `using` statements

3. **Check marshaling attributes:**
   - Arrays should have `[In]` or `[Out]` attributes
   - See `SpectraNative.cs` for examples

### Issue: Slow Performance

**Expected:**
- FFT (1024 points): < 1ms
- Filter (1000 samples): < 1ms
- STFT frame: < 2ms

**Solutions:**

1. **Verify Release build:**
   ```powershell
   # DLL should be from Release, not Debug
   dir build\native\Release\spectra.dll
   ```

2. **Check CPU usage:**
   - Open Task Manager during tests
   - CPU should spike briefly then drop

3. **Profile in Unity:**
   - Window → Analysis → Profiler
   - Check "Deep Profile"
   - Identify bottlenecks

---

## Part 8: Success Criteria

**UNITY-001 is considered COMPLETE when:**

- [x] Windows DLL obtained (from CI/CD or local build)
- [ ] DLL verified as 64-bit PE32+ executable
- [ ] Unity Editor opens with DLL loaded
- [ ] All 36 P/Invoke tests pass
- [ ] All 35 Component tests pass
- [ ] Standalone test compiles and runs successfully
- [ ] All 6 standalone test sections pass
- [ ] FFT performance acceptable (< 1ms for 1024 points)
- [ ] No memory leaks detected
- [ ] Filter Designer tool works on Windows
- [ ] Signal Inspector tool works on Windows
- [ ] No platform-specific errors

**Additional validation:**
- [ ] Example projects run on Windows
- [ ] OnAudioFilterRead integration works
- [ ] Microphone input works
- [ ] Build for Windows standalone succeeds

---

## Part 9: Documentation Updates

After successful testing, update these files:

1. **UNITY_INTEGRATION_SUMMARY.md:**
   ```markdown
   ### Platform Compatibility

   - ✅ Linux (x86_64) - Native library built and tested
   - ✅ Windows (x86_64) - P/Invoke bindings verified  ← UPDATE
   - ⏳ macOS - Requires testing
   ```

2. **Create UNITY-001_COMPLETION.md:**
   - Test results summary
   - Performance metrics
   - Any Windows-specific notes
   - Screenshots of passing tests

3. **README.md updates:**
   - Add Windows to supported platforms
   - Note any Windows-specific setup steps

---

## Part 10: Next Steps

After completing UNITY-001:

1. **Commit test results:**
   ```bash
   git add UNITY-001_COMPLETION.md
   git commit -m "Complete UNITY-001: Windows P/Invoke testing

   - All 71 Unity tests passing on Windows
   - Standalone P/Invoke test verified
   - Performance benchmarks recorded
   - No platform-specific issues found"
   git push
   ```

2. **Update task tracking:**
   - Mark UNITY-001 as complete
   - Move to UNITY-002 (macOS testing)

3. **Optional: Create Windows-specific examples:**
   - WASAPI integration
   - Windows Audio Session API
   - DirectSound integration

---

## Quick Reference Commands

**Download from CI/CD:**
```powershell
# Using GitHub CLI (if installed)
gh run download --name spectra-windows

# Or download manually from:
# https://github.com/yogt1984/SpectraCore/actions
```

**Build locally:**
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

**Copy to Unity:**
```powershell
copy build\native\Release\spectra.dll "Assets\SpectraCore\Runtime\Plugins\Windows\x86_64\"
```

**Run tests:**
- Unity: Window → General → Test Runner → Run All
- Standalone: `csc /out:PInvokeTest.exe StandalonePInvokeTest.cs && PInvokeTest.exe`

**Verify DLL:**
```powershell
Get-Item spectra.dll | Format-List
dumpbin /headers spectra.dll | findstr machine
dumpbin /exports spectra.dll | findstr spectra_
```

---

## Support

**Need help?**

1. Check error message in Unity Console
2. Verify DLL is in correct location
3. Check this troubleshooting guide
4. Review SpectraNative.cs for correct marshaling
5. Test with standalone P/Invoke test first

**Common resources:**
- Unity P/Invoke docs: https://docs.unity3d.com/Manual/PlatformDependentCompilation.html
- .NET Marshaling: https://docs.microsoft.com/en-us/dotnet/standard/native-interop/
- SpectraCore API docs: `docs/API_Reference.md`

---

## Estimated Time

- **DLL download/build:** 5-15 minutes
- **Unity setup:** 10-20 minutes (if installing Unity)
- **Running tests:** 2-5 minutes
- **Validation:** 10-15 minutes
- **Total:** 30-60 minutes

**If you already have Unity installed and DLL from CI/CD:** ~15 minutes

---

**Status:** Ready for Windows testing once CI/CD artifacts are available (currently building)
