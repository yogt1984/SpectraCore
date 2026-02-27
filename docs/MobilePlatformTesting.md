# Mobile Platform Testing Guide

Complete guide for validating SpectraCore on iOS and Android devices.

## Overview

This guide provides step-by-step instructions for testing SpectraCore on mobile platforms. All testing infrastructure is prepared and ready - you just need to build, deploy, and run the tests.

## Test Coverage

The mobile validation suite includes:

- **50+ Functional Tests**: Verify all DSP operations work correctly on mobile
- **Performance Benchmarks**: Measure execution speed and real-time capability
- **Automated Reporting**: Results displayed on-device with pass/fail status

### Tested Features

- FFT (forward, inverse, round-trip)
- Window functions (Hann, Hamming, Blackman, etc.)
- IIR filter design (Butterworth, Chebyshev, Elliptic, Bessel)
- IIR filter types (lowpass, highpass, bandpass, bandstop)
- Filter application (LFilter, FiltFilt, StreamingFilter)
- FIR filter design (Fir1, FirPM)
- Frequency analysis (Freqz)
- Correlation (XCorr)
- Power spectral density (Pwelch)
- Hilbert transform
- Resampling
- STFT (analyze, streaming)
- Onset detection

---

## Prerequisites

### For Android Testing

**Required:**
- Unity 2021.3 LTS or later
- Android SDK (included with Unity)
- Android NDK (included with Unity)
- USB debugging enabled on your Android device
- ✅ **Native libraries already built** (`native/build-android/`)

**Optional:**
- Android Studio (for logcat viewing)

### For iOS Testing

**Required:**
- macOS with Xcode 13.0 or later
- Unity 2021.3 LTS or later
- iOS device with iOS 13.0+ (or iOS Simulator)
- Apple Developer account (free or paid)
- USB cable for device connection
- ⚠️ **Native libraries NOT built** (requires macOS build)

---

## Quick Start (5 Minutes)

### Step 1: Import Mobile Validation Sample

1. Open your Unity project with SpectraCore installed
2. In Unity: **Window → Package Manager**
3. Select "SpectraCore" from the list
4. Expand "Samples" section
5. Click "Import" next to "Mobile Validation"

This imports to: `Assets/Samples/SpectraCore/[version]/MobileValidation/`

### Step 2: Open Mobile Test Scene

1. In Project window, navigate to:
   ```
   Assets/Samples/SpectraCore/[version]/MobileValidation/Scenes/
   ```
2. Double-click **MobileTestScene.unity**

The scene contains:
- **Canvas**: UI for test controls
- **TestRunner**: Executes validation tests
- **BenchmarkRunner**: Runs performance tests

### Step 3: Configure Build Settings

#### For Android:

1. **File → Build Settings**
2. Click "Add Open Scenes" (adds MobileTestScene)
3. Select **Android** platform
4. Click "Switch Platform" (if not already Android)
5. Click "Player Settings"
6. Under "Other Settings":
   - Set **Minimum API Level**: Android 5.0 (API 21) or higher
   - Set **Target API Level**: Automatic (highest installed)
   - Enable **ARMv7** and **ARM64** under "Target Architectures"
7. Close Player Settings

#### For iOS:

1. **File → Build Settings**
2. Click "Add Open Scenes"
3. Select **iOS** platform
4. Click "Switch Platform"
5. Click "Player Settings"
6. Under "Other Settings":
   - Set **Target minimum iOS Version**: 13.0 or higher
   - Set **Architecture**: ARM64
   - Set **Camera Usage Description**: "Not used" (required by App Store)
7. Close Player Settings

### Step 4: Build

#### Android Build:

1. In Build Settings, click **"Build"** (not "Build And Run" yet)
2. Choose save location: `Builds/Android/SpectraTest.apk`
3. Wait for build to complete (~2-5 minutes)

#### iOS Build:

1. In Build Settings, click **"Build"**
2. Choose save location: `Builds/iOS/SpectraTest/`
3. Wait for Unity to generate Xcode project (~2-5 minutes)
4. **Open** the generated `Unity-iPhone.xcodeproj` in Xcode
5. In Xcode:
   - Select your development team under "Signing & Capabilities"
   - Change bundle identifier if needed (e.g., `com.yourcompany.spectratest`)
   - Connect your iOS device via USB
   - Select your device as the build target
   - Click **"Play"** button (▶) to build and deploy

### Step 5: Deploy and Run

#### Android Deployment:

**Option A: Direct Install (Recommended)**
1. Enable "USB Debugging" on your Android device:
   - Settings → About Phone → Tap "Build Number" 7 times
   - Settings → Developer Options → Enable "USB Debugging"
2. Connect device via USB
3. In Unity Build Settings, click **"Build And Run"**
4. Unity will automatically install and launch the app

**Option B: Manual Install**
1. Transfer `SpectraTest.apk` to your device
2. Open file manager on device
3. Tap the APK file
4. Grant "Install from Unknown Sources" permission if prompted
5. Tap "Install"
6. Open the app from your app drawer

#### iOS Deployment:

1. Connect iOS device via USB
2. In Xcode, click **"Play"** (▶) button
3. Xcode will build, deploy, and launch the app
4. If prompted, trust the developer certificate on device:
   - Settings → General → Device Management
   - Tap your developer profile
   - Tap "Trust"

### Step 6: Run Tests

**On Device Screen:**

1. You'll see the Mobile Validation UI
2. Tap **"Run Tests"** button to execute functional tests
3. Wait ~30 seconds for all tests to complete
4. Results will scroll by in real-time
5. Final summary shows: `✅ All 50+ tests passed!` (expected)

**To Run Performance Benchmark:**

1. Tap **"Benchmark"** tab (if available)
2. Tap **"Run Benchmark"** button
3. Wait ~60 seconds for benchmarks to complete
4. Review performance metrics (ops/sec, CPU load)

---

## Expected Results

### Functional Tests

**All tests should PASS** with output similar to:

```
=== SpectraCore Mobile Platform Validation ===
Platform: Android
Device: Samsung Galaxy S21
Processor: Snapdragon 888 (8 cores)
Memory: 8192 MB

--- Version ---
  ✓ Version Major
  ✓ Version Minor
  ✓ Version Patch
  ✓ Version String

--- FFT ---
  ✓ FFT Create/Destroy
  ✓ FFT Forward
  ✓ FFT Inverse

--- Butterworth Filters ---
  ✓ Butterworth Lowpass
  ✓ Butterworth Highpass
  ✓ Butterworth Bandpass
  ✓ Butterworth Bandstop

... (50+ tests) ...

=== Test Summary ===
Total Tests: 52
Passed: 52 (100.0%)
Failed: 0
✅ ALL TESTS PASSED!
```

### Performance Benchmarks

**Example results on mid-range Android device:**

```
--- FFT Operations ---
  FFT Forward:
    Total: 125.34 ms
    Average: 1.253 ms
    Throughput: 798.1 ops/sec
    CPU Load: 2.7% ✓ Real-time capable

  FFT Round-trip:
    Total: 245.78 ms
    Average: 2.458 ms
    Throughput: 406.9 ops/sec
    CPU Load: 5.3% ✓ Real-time capable

--- IIR Filter Design ---
  Butterworth Design:
    Total: 45.23 ms
    Average: 0.452 ms
    Throughput: 2210.9 ops/sec

  Bessel Design:
    Total: 52.67 ms
    Average: 0.527 ms
    Throughput: 1898.5 ops/sec

--- Filter Application ---
  LFilter:
    Total: 89.12 ms
    Average: 0.891 ms
    Throughput: 1122.1 ops/sec
    CPU Load: 1.9% ✓ Real-time capable

  FiltFilt:
    Total: 178.45 ms
    Average: 1.785 ms
    Throughput: 560.4 ops/sec
    CPU Load: 3.8% ✓ Real-time capable
```

**Real-time Capability:**
- CPU Load < 50%: ✓ Real-time capable
- CPU Load 50-100%: ⚠ Marginal (may work but close to limit)
- CPU Load > 100%: ✗ Not real-time (too slow for audio)

**Target Performance (44.1 kHz, 2048 samples):**
- All filter operations should be < 10% CPU load
- FFT operations should be < 5% CPU load
- Most operations should achieve 100+ ops/sec

---

## Troubleshooting

### Android Issues

**Problem: "App not installed" error**
- **Solution**: Uninstall any previous version of the app first
- Or: Change the package name in Player Settings

**Problem: App crashes on startup**
- **Check**: Logcat output using `adb logcat -s Unity`
- **Common cause**: Missing native library for device architecture
- **Solution**: Verify ARMv7 and ARM64 are both enabled in Player Settings

**Problem: "DllNotFoundException" for libspectra.so**
- **Check**: Native libraries exist in `Assets/Plugins/Android/`
- **Verify**: Libraries for all architectures (armeabi-v7a, arm64-v8a, x86, x86_64)
- **Solution**: Rebuild native libraries with Android NDK

**Problem: Tests fail with "NaN" or "Infinity" values**
- **Likely cause**: Numerical precision issue on specific device
- **Check**: Device specifications (very old devices may have issues)
- **Report**: File an issue with device model and test output

### iOS Issues

**Problem: "Code signing failed" in Xcode**
- **Solution**: Select a valid development team in Xcode project settings
- **Free account**: Change bundle identifier to something unique

**Problem: "Library not loaded: libspectra.framework"**
- **Check**: Native library exists in `Libraries/Plugins/iOS/`
- **Solution**: Ensure iOS build was completed successfully
- **Verify**: Framework is embedded in Xcode project settings

**Problem: App launches but crashes immediately**
- **Check**: Xcode console for error messages
- **Common cause**: Missing iOS native library
- **Solution**: Build iOS native library on macOS first

**Problem: "Untrusted Developer" message on device**
- **Solution**: Settings → General → Device Management → Trust developer profile

### General Issues

**Problem: UI text is tiny / not visible**
- **Solution**: Check Canvas Scaler settings (should use "Scale With Screen Size")
- **Or**: Adjust reference resolution in Canvas Scaler component

**Problem: Tests run but no output appears**
- **Check**: Console logs in Unity Editor (try running in Play Mode first)
- **Verify**: Text component is assigned in MobileTestRunner inspector

**Problem: Some tests fail unexpectedly**
- **Note**: Specific test failures (e.g., 1-2 tests) may indicate:
  - Device-specific numerical precision issues
  - Memory constraints on low-end devices
  - Timing issues (less likely)
- **Action**: Report the specific failing tests with device info

---

## Logging and Diagnostics

### Viewing Android Logs

**Using ADB:**
```bash
# Connect device via USB
adb devices

# View Unity logs in real-time
adb logcat -s Unity

# Filter for SpectraCore messages
adb logcat -s Unity | grep Spectra

# Save full log to file
adb logcat > mobile_test_log.txt
```

### Viewing iOS Logs

**Using Xcode:**
1. Window → Devices and Simulators
2. Select your device
3. Click "Open Console" button
4. Run the app
5. Filter for "Unity" or "Spectra" in the search box

**Using Console.app (macOS):**
1. Open Console.app
2. Select your connected iOS device
3. Run the app
4. Search for "Unity" in the filter box

---

## Validating Specific Features

If you need to test specific DSP operations in detail:

### Custom Test Script

Create a new C# script in the MobileValidation scene:

```csharp
using UnityEngine;
using Spectra;

public class CustomMobileTest : MonoBehaviour
{
    void Start()
    {
        TestSpecificFeature();
    }

    void TestSpecificFeature()
    {
        Debug.Log("=== Custom Test ===");

        try
        {
            // Example: Test Bessel filter in detail
            var (b, a) = DSP.Bessel(4, 0.3f, FilterType.Lowpass);

            Debug.Log($"Bessel filter designed: {b.Length} b coeffs, {a.Length} a coeffs");
            Debug.Log($"b[0] = {b[0]}, a[0] = {a[0]}");

            // Generate test signal
            float[] signal = new float[1024];
            for (int i = 0; i < signal.Length; i++)
            {
                signal[i] = Mathf.Sin(2f * Mathf.PI * 440f * i / 44100f);
            }

            // Apply filter
            float[] filtered = DSP.FiltFilt(b, a, signal);

            Debug.Log($"Filtered {filtered.Length} samples successfully");
            Debug.Log($"First sample: {filtered[0]}, Last sample: {filtered[filtered.Length-1]}");

            Debug.Log("✅ Custom test passed!");
        }
        catch (System.Exception ex)
        {
            Debug.LogError($"❌ Custom test failed: {ex.Message}");
        }
    }
}
```

Attach this script to a GameObject in the scene and rebuild.

---

## Performance Optimization Tips

### For Best Performance on Mobile:

1. **Use appropriate filter orders**:
   - Order 4-6 is ideal for most audio applications
   - Avoid order > 8 unless absolutely necessary

2. **Prefer StreamingFilter for real-time**:
   - Processes small chunks efficiently
   - Lower latency than LFilter/FiltFilt on full buffers

3. **Choose filter type wisely**:
   - Butterworth: Good balance of performance and quality
   - Bessel: Slightly slower but best transient response
   - Elliptic: Fastest convergence but most complex design

4. **Optimize FFT sizes**:
   - Power-of-2 sizes only (128, 256, 512, 1024, 2048, 4096)
   - Larger FFTs are slower but higher frequency resolution

5. **Reuse filter objects**:
   - Create filters once, reuse for multiple signals
   - Avoid recreating StreamingFilter or STFT in audio callback

6. **Use FiltFilt sparingly**:
   - FiltFilt is 2x slower than LFilter (processes twice)
   - Use only when zero-phase is critical

---

## Reporting Results

When reporting test results, please include:

1. **Device Information**:
   - Device model (e.g., "iPhone 13 Pro", "Samsung Galaxy S22")
   - OS version (e.g., "iOS 16.5", "Android 13")
   - Processor type and cores

2. **Test Results**:
   - Number of tests passed/failed
   - Specific test names that failed (if any)
   - Complete log output (if tests failed)

3. **Performance Metrics**:
   - Operations per second for key operations
   - CPU load percentages
   - Any operations marked as "Not real-time"

4. **Build Configuration**:
   - Unity version
   - SpectraCore version
   - Build configuration (Debug/Release)

---

## Advanced: Running Tests in Unity Editor

For rapid iteration, you can run mobile-specific tests in Unity Editor:

1. Enter Play Mode in Unity Editor
2. The tests will run automatically (if `runOnStart = true`)
3. View results in Console window
4. Faster iteration than deploying to device

**Note**: Performance will be different from actual mobile devices!

---

## Next Steps

After validating on mobile:

1. ✅ **Mark mobile validation as complete**
2. 📝 **Document any device-specific quirks**
3. 🚀 **Ready for production mobile deployment!**

---

## FAQ

**Q: Do I need to rebuild native libraries?**
A: No! Android native libraries are already built in `native/build-android/`. For iOS, you'll need to build on macOS (see `docs/CrossPlatformBuildGuide.md`).

**Q: Can I test on emulator/simulator?**
A: Yes! Android Emulator and iOS Simulator both work, but performance will be different from real devices.

**Q: How long do tests take?**
A: Functional tests: ~30 seconds. Performance benchmarks: ~60 seconds.

**Q: What if a few tests fail?**
A: 1-2 test failures on very old/low-end devices may be acceptable. Report specific failures for investigation.

**Q: Can I test in Unity Editor Play Mode?**
A: Yes, tests run in Editor too, but performance and behavior may differ slightly from actual mobile.

**Q: How do I test on multiple devices?**
A: Build once, deploy to multiple devices. Each device will run tests independently.

---

## Support

If you encounter issues:

1. Check this troubleshooting guide
2. Review device logs (adb logcat or Xcode console)
3. File an issue at: https://github.com/anthropics/SpectraCore/issues
4. Include: device info, test output, and logs

---

**You're all set!** The testing infrastructure is ready. Just follow Steps 1-6 above to validate SpectraCore on your mobile device.
