# Mobile Validation Sample

Comprehensive testing suite for validating SpectraCore on mobile platforms (iOS and Android).

## What's Included

- **MobileTestRunner.cs**: Executes 50+ functional tests covering all DSP operations
- **MobilePerformanceBenchmark.cs**: Performance benchmarking for real-time capability assessment
- **MobileTestScene.unity**: Pre-configured test scene with UI (user must create)
- **Complete documentation**: Step-by-step deployment guide

## Quick Start

1. **Import this sample** using Unity Package Manager
2. **Open** `Scenes/MobileTestScene.unity`
3. **Build Settings** → Add scene → Select platform (Android/iOS)
4. **Build and Run** to deploy to device
5. **Tap "Run Tests"** button on device

Tests will execute automatically and display results on-screen.

## Features Tested

### DSP Operations (50+ tests)
- FFT (forward, inverse, round-trip)
- Window functions (all types)
- IIR filters (Butterworth, Chebyshev I/II, Elliptic, Bessel)
- Filter types (lowpass, highpass, bandpass, bandstop)
- Filter application (LFilter, FiltFilt, StreamingFilter)
- FIR filter design (Fir1, FirPM)
- Frequency analysis (Freqz)
- Correlation (XCorr)
- Power spectral density (Pwelch)
- Hilbert transform
- Resampling
- STFT (batch and streaming)
- Onset detection

### Performance Metrics
- Operations per second
- CPU load estimation
- Real-time capability assessment
- Memory usage tracking

## Expected Results

**All 50+ tests should pass** with output showing:
```
✅ ALL TESTS PASSED!
Total Tests: 52
Passed: 52 (100.0%)
Failed: 0
```

**Performance benchmarks** should show:
- FFT: < 5% CPU load for 2048 samples
- Filters: < 10% CPU load
- All operations: Real-time capable on modern devices (2018+)

## Documentation

Complete deployment instructions: `docs/MobilePlatformTesting.md`

Includes:
- Step-by-step build instructions
- Android and iOS deployment procedures
- Troubleshooting guide
- Expected performance baselines
- Logging and diagnostics

## Platform Support

- **Android**: API Level 21+ (Android 5.0+)
- **iOS**: iOS 13.0+
- **Architectures**: ARM64, ARMv7 (Android), ARM64 (iOS)

## Requirements

### Android
- Unity 2021.3 LTS+
- Android SDK & NDK (included with Unity)
- USB debugging enabled on device
- Native libraries already built (`native/build-android/`)

### iOS
- macOS with Xcode 13.0+
- Apple Developer account (free or paid)
- iOS device or Simulator
- Native libraries must be built on macOS first

## Scene Setup (Manual)

If you need to create the scene from scratch:

1. Create new scene: `File → New Scene`
2. Add **Canvas** (UI → Canvas)
3. Add **EventSystem** (if not auto-created)
4. Add child to Canvas: **Panel** (UI → Panel)
5. Add to Panel:
   - **Text**: Status text
   - **Text**: Results text (in ScrollView)
   - **Button**: "Run Tests" button
   - **Button**: "Run Benchmark" button
6. Create empty GameObject: "TestRunner"
7. Attach **MobileTestRunner** component
8. Create empty GameObject: "BenchmarkRunner"
9. Attach **MobilePerformanceBenchmark** component
10. Assign UI references in inspector
11. Save scene as `MobileTestScene.unity`

Or use the pre-configured scene provided.

## Usage Tips

### Running Tests
- Tests run automatically if `runOnStart = true`
- Or tap "Run Tests" button manually
- Enable "Verbose" toggle for detailed output

### Performance Benchmarking
- Run tests first to verify correctness
- Then run benchmarks to measure speed
- Compare results across different devices
- Note: Benchmarks take ~60 seconds

### Custom Testing
Add custom test code to `MobileTestRunner.cs`:
```csharp
private void TestCustomFeature()
{
    Test("My Custom Test", () =>
    {
        // Your test logic here
        var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
        return b.Length == 5;
    });
}
```

Then call in `RunAllTestsCoroutine()`:
```csharp
yield return RunTestCategory("Custom", TestCustomFeature);
```

## Troubleshooting

### App Crashes on Startup
- Check device architecture matches built libraries
- Verify native libraries are in `Assets/Plugins/Android/` or `iOS/`
- Review logcat (Android) or Xcode console (iOS) for errors

### Tests Fail
- 1-2 failures on very old devices may be acceptable
- Check specific test names that failed
- Report issues with device model and complete output

### Performance Poor
- Older devices (pre-2018) may struggle
- Background apps can affect results
- Try closing other apps and running again

## File Structure

```
MobileValidation/
├── README.md                           (this file)
├── Scripts/
│   ├── MobileTestRunner.cs            (functional tests)
│   └── MobilePerformanceBenchmark.cs  (performance tests)
└── Scenes/
    └── MobileTestScene.unity          (test scene)
```

## Support

For issues or questions:
- Check `docs/MobilePlatformTesting.md` for detailed guide
- Review troubleshooting section
- File issue at: https://github.com/anthropics/SpectraCore/issues

---

**Ready to test!** Import sample, build for mobile, and run tests on your device.
