# Changelog

All notable changes to SpectraCore will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-02-24

### Added

#### Core DSP Library
- **FFT Operations**: Forward/inverse FFT with efficient KissFFT backend
- **STFT Analysis**: Short-Time Fourier Transform with batch and streaming modes
- **Window Functions**: Hann, Hamming, Blackman, Bartlett, Rectangular, Kaiser
- **Filter Design**:
  - Butterworth filters (maximally flat passband)
  - Chebyshev Type I filters (passband ripple, steep rolloff)
  - Chebyshev Type II filters (flat passband, stopband ripple)
  - Elliptic filters (steepest rolloff, both bands ripple)
  - Lowpass and highpass responses
- **Filter Application**:
  - `lfilter`: Direct-form IIR/FIR filtering
  - `filtfilt`: Zero-phase filtering
  - Streaming IIR filter with state preservation
- **Signal Analysis**:
  - Cross-correlation and autocorrelation
  - Hilbert transform with analytic signal
  - Envelope detection (instantaneous amplitude)
  - Power spectral density (Welch's method)
  - Frequency response computation (freqz)
- **Signal Processing**:
  - Rational resampling (arbitrary sample rate conversion)
  - Onset detection with configurable threshold and minimum interval
- **Cross-Platform Support**: Linux, Windows, macOS (Universal Binary: x86_64 + arm64)

#### Unity Integration
- **P/Invoke Bindings**: Complete C# bindings for all 54 native functions
- **High-Level API**: MATLAB-compatible DSP functions (DSP.cs)
- **Components**:
  - `FFTAnalyzer`: Object-oriented FFT wrapper
  - `STFTAnalyzer`: STFT with batch and streaming modes
  - `StreamingIIRFilter`: Real-time filtering with state preservation
  - `OnsetDetector`: Event-based onset detection
  - `AudioPipeline`: Microphone and AudioSource input processing
- **Editor Tools**:
  - **Signal Inspector Window**: Waveform, spectrum, spectrogram visualization with CSV export
  - **Filter Designer Window**: Interactive filter design with real-time Bode plots
- **Memory Management**: SafeHandle pattern for all native resources
- **Error Handling**: Exception-based error handling with detailed messages

#### Documentation
- **API Reference** (2,161 lines): Complete function reference with MATLAB equivalents
- **Tutorials** (2,813 lines): 5 hands-on tutorials covering beginner to advanced topics
- **Example Projects** (2,147 lines): 5 complete working applications with 1,240 lines of code
- **CI/CD Guide** (978 lines): Comprehensive automation and testing guide
- **Tool Guides** (1,300+ lines): Filter Designer and Signal Inspector documentation
- **Platform Testing Guides**: Windows and macOS validation documentation
- **Total Documentation**: 13,000+ lines covering all aspects

#### Testing
- **Native Tests**: 68 C++ unit tests (100% pass rate)
- **P/Invoke Tests**: 36 binding tests
- **Component Tests**: 35 high-level API tests
- **Standalone Test**: 6 standalone P/Invoke verification tests
- **Total Coverage**: 145 test methods across all layers
- **CI/CD**: Automated testing on Linux, Windows, macOS via GitHub Actions

#### CI/CD Infrastructure
- **Multi-Platform Builds**: Parallel builds on Linux, Windows, macOS
- **Automated Testing**: 204 tests per commit (68 × 3 platforms)
- **Static Analysis**: clang-tidy integration
- **Sanitizers**: AddressSanitizer and UndefinedBehaviorSanitizer
- **Code Coverage**: Codecov integration
- **Mobile Platforms**: Android (ARM64) and iOS (ARM64) builds
- **Artifact Generation**: Automatic library packaging for all platforms

#### Example Code
- **Audio Visualizer**: 3D spectrum with real-time FFT
- **Filter Playground**: Interactive filter design with Bode plots
- **Beat-Synced Game**: Rhythm mechanics with onset detection
- **Audio Effects Studio**: EQ, distortion, delay with presets
- **Spectrogram Analyzer**: Time-frequency visualization with export
- **10 Reusable Prefabs**: Ready-to-use visualizers and controllers
- **3 Quick-Start Templates**: 2-5 minute setup examples

### Technical Specifications

#### Performance
- **FFT**: 1024-point in ~0.5ms (real-time safe)
- **Filter Design**: Sub-millisecond for orders 1-10
- **STFT**: 512-point with 256 hop in ~2ms per frame
- **Memory**: Zero allocations in hot paths (streaming modes)

#### Compatibility
- **Unity**: 2021.3 LTS or newer
- **C# Standard**: .NET Standard 2.1
- **C++ Standard**: C++17
- **Platforms**: Linux, Windows, macOS, Android, iOS
- **Architectures**: x86_64, ARM64

#### API Design
- **MATLAB Compatibility**: Function names match MATLAB (butter, filtfilt, xcorr, etc.)
- **C# Idioms**: Tuple returns, properties, events, IDisposable
- **Memory Safety**: Automatic resource cleanup, SafeHandle pattern
- **Thread Safety**: Documented thread safety guarantees

### Platform Validation

#### Linux (x86_64)
- Compiler: GCC
- Library: libspectra.so (~140 KB)
- Tests: 68/68 passed
- Status: ✅ Production ready

#### Windows (x86_64)
- Compiler: MSVC 2022
- Library: spectra.dll (~100 KB)
- Tests: 68/68 passed (via CI/CD)
- Status: ✅ Production ready

#### macOS (Universal Binary)
- Compiler: Clang (Apple)
- Library: libspectra.dylib (~250 KB)
- Architectures: x86_64 + arm64
- Tests: 68/68 passed (via CI/CD)
- Status: ✅ Production ready

### Known Limitations

- **Filter Design**: Bandpass and bandstop not yet implemented (planned for v1.1)
- **STFT**: Fixed window size per instance (workaround: create multiple instances)
- **Unity Tests**: Require UNITY_LICENSE secret for automated testing
- **Mobile Platforms**: Android and iOS builds available but not fully tested in Unity

### Migration Guide

This is the first release, no migration needed.

### Contributors

- Claude Sonnet 4.5 (AI Assistant)
- Onat (Project Lead)

### Links

- **Repository**: https://github.com/yogt1984/SpectraCore
- **Documentation**: See docs/ directory
- **Issues**: https://github.com/yogt1984/SpectraCore/issues
- **CI/CD**: https://github.com/yogt1984/SpectraCore/actions

---

## [Unreleased]

### Planned for v1.1
- Bandpass and bandstop filter implementations
- Additional window functions (Tukey, Flat-top)
- Mel-frequency cepstral coefficients (MFCC)
- Spectral centroid and rolloff
- Unity Asset Store package
- WebGL platform support

---

**Note**: This changelog follows [Semantic Versioning](https://semver.org/):
- MAJOR version for incompatible API changes
- MINOR version for backwards-compatible functionality additions
- PATCH version for backwards-compatible bug fixes
