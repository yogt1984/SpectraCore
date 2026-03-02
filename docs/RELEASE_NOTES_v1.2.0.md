# SpectraCore v1.2.0

**Release Date:** 2026-02-27

## 🎯 Highlights

- ⭐ **Bessel (Thomson) Filters** - Maximally flat group delay for optimal transient response
- 🌐 **WebGL Support** - Run DSP in the browser via WebAssembly
- 📱 **Mobile Platforms** - Full iOS and Android support with on-device testing
- 🧪 **494+ Tests** - Comprehensive test coverage across all platforms

## ✨ New Features

### Bessel Filters
- Complete implementation for all 4 filter types (Lowpass, Highpass, Bandpass, Bandstop)
- Pre-computed Bessel polynomial poles for orders 1-10
- Maximally flat group delay (linear phase response)
- Minimal overshoot and ringing (<1% in step response)
- Ideal for pulse signals, transients, and phase-critical applications

**API Example:**
```csharp
// Design a Bessel lowpass filter
var (b, a) = DSP.Bessel(4, 0.3f, FilterType.Lowpass);

// Design a Bessel bandpass filter
var (b_bp, a_bp) = DSP.Bessel(4, 0.2f, 0.5f, FilterType.Bandpass);
```

### Platform Support
- **WebGL:** Emscripten build system with optimized WASM output
- **iOS:** CMake toolchain with arm64/armv7 support
- **Android:** Multi-ABI builds (armeabi-v7a, arm64-v8a, x86, x86_64)
- **Mobile Testing:** On-device validation with 50+ automated tests

### Documentation
- `docs/EmscriptenSetupGuide.md` - WebGL build instructions
- `docs/CrossPlatformBuildGuide.md` - Platform-specific build guide
- `docs/MobilePlatformTesting.md` - Mobile deployment guide

## 📦 Downloads

- **libspectra-v1.2.0-linux-x64.so** - Native library for Linux (231 KB)
- **SpectraCore-v1.2.0-unity.zip** - Unity package (157 KB)

## 🧪 Testing

- 494+ unit tests passing (100% pass rate)
- All platforms validated
- Mobile on-device testing infrastructure included

## 📊 Statistics

- **Lines Added:** ~4,700
- **Platforms Supported:** 6 (Linux, Windows, macOS, iOS, Android, WebGL)
- **Filter Types:** 4 IIR types × 4 filter modes = 16 configurations
- **Library Size:** 231 KB (optimized release build)

## 🔧 Breaking Changes

None - fully backward compatible with v1.1.0

## 📚 Key Files

### Native C++
- `native/src/filters/filter_design.cpp` - Bessel filter implementation
- `native/tests/unit/test_bessel.cpp` - Comprehensive Bessel tests
- `native/cmake/wasm.toolchain.cmake` - Emscripten toolchain
- `native/cmake/platforms/` - Platform-specific configurations

### Unity
- `unity/Samples~/MobileValidation/` - Mobile testing infrastructure
- `unity/Runtime/Scripts/Core/DSP.cs` - Bessel API additions

## 🎓 Use Cases

- **Audio Processing:** EQ, filtering with minimal phase distortion
- **Pulse Signals:** Drums, percussion, transient preservation
- **Data Transmission:** Minimal intersymbol interference
- **Scientific Instrumentation:** Waveform fidelity
- **Radar/Sonar:** Signal processing with phase linearity

## 🙏 Acknowledgments

Thanks to all contributors and testers who made this release possible!

## 📖 Full Details

See [CHANGELOG.md](https://github.com/yourusername/SpectraCore/blob/main/CHANGELOG.md) for complete technical details.

---

**Full Changelog:** https://github.com/yourusername/SpectraCore/compare/v1.1.0...v1.2.0
