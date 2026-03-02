# SpectraCore

**Professional-grade DSP library for Unity with comprehensive C# bindings and editor tools**

[![Release](https://img.shields.io/badge/release-v1.3.0-blue.svg)](https://github.com/yogt1984/SpectraCore/releases)
[![CI/CD](https://github.com/yogt1984/SpectraCore/workflows/CI/badge.svg)](https://github.com/yogt1984/SpectraCore/actions)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Unity](https://img.shields.io/badge/unity-2021.3+-orange.svg)](https://unity.com)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS%20%7C%20iOS%20%7C%20Android%20%7C%20WebGL-lightgrey.svg)](#platform-support)

---

## 🎯 Overview

SpectraCore is a high-performance, cross-platform digital signal processing (DSP) library designed for Unity. It provides MATLAB-equivalent functions with native C++ performance, comprehensive Unity integration, and professional editor tools.

**Perfect for:**
- 🎮 Audio-reactive games and rhythm mechanics
- 🎵 Real-time audio effects and visualization
- 📊 Signal analysis and scientific computing
- 🔧 Audio tool development in Unity
- 🎓 Education and research

---

## ✨ Features

### 🚀 Core Capabilities

- **Real-time FFT & STFT** - Fast Fourier Transform with configurable sizes (256-8192)
- **Professional Filter Design** - Butterworth, Chebyshev I/II, Elliptic, **Bessel** ⭐ NEW in v1.2.0
  - All 4 filter types: Lowpass, Highpass, Bandpass, Bandstop
  - Bessel filters: Maximally flat group delay for best transient response
- **Signal Analysis** - Correlation, PSD, Hilbert transform, onset detection, **Pitch Detection** ⭐ NEW in v1.3.0
  - YIN algorithm for accurate pitch tracking
  - Musical note mapping with cents deviation
  - Real-time tuner-grade performance
- **Streaming Processing** - Lock-free, zero-allocation audio pipeline
- **Cross-Platform** - Desktop (Win/Mac/Linux), Mobile (iOS/Android), WebGL
- **Unity Integration** - Native plugins with managed C# wrappers

### 🛠️ Editor Tools

| Tool | Description |
|------|-------------|
| **Signal Inspector** | Visualize waveforms, spectra, and spectrograms with CSV export |
| **Filter Designer** | Interactive filter design with real-time Bode plots |

### 📦 5 Complete Example Projects

1. **Audio Visualizer** - 3D spectrum bars and waveform display
2. **Filter Playground** - Interactive filter design with frequency response
3. **Beat-Synced Game** - Rhythm game mechanics with onset detection
4. **Audio Effects Studio** - EQ, distortion, delay with presets
5. **Spectrogram Analyzer** - Time-frequency analysis with export

---

## 🎬 Quick Start

### Installation

**Unity Package Manager (Recommended):**
```
Window → Package Manager → + → Add package from git URL
https://github.com/yogt1984/SpectraCore.git
```

**Or download the latest release:**
- [Releases Page](https://github.com/yogt1984/SpectraCore/releases/tag/v1.0.0)

**For detailed installation instructions**, see [INSTALLATION.md](INSTALLATION.md)

### First Example

```csharp
using Spectra;
using UnityEngine;

public class SimpleFFT : MonoBehaviour
{
    void Start()
    {
        // Create FFT analyzer
        using (var fft = new FFTAnalyzer(1024))
        {
            // Get audio data
            float[] signal = new float[1024];

            // Compute spectrum
            float[] spectrum = fft.GetMagnitudeSpectrum(signal);

            Debug.Log($"Spectrum size: {spectrum.Length}"); // 513 bins
        }
    }
}
```

### Filter Design Example

```csharp
using Spectra;

// Design a 4th-order Butterworth lowpass filter
var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);

// Apply zero-phase filtering
float[] filtered = DSP.FiltFilt(b, a, inputSignal);

// NEW: Design a bandpass filter (e.g., 0.2 - 0.5 normalized frequency)
var (b_bp, a_bp) = DSP.Butter(4, 0.2f, 0.5f, FilterType.Bandpass);
float[] bandpassed = DSP.FiltFilt(b_bp, a_bp, inputSignal);

// Or use streaming for real-time
using var filter = new StreamingIIRFilter(b, a);
filter.Process(audioChunk);
```

### Pitch Detection Example ⭐ NEW

```csharp
using Spectra;

// Create pitch detector (44.1 kHz, 2048 samples)
using (var detector = new PitchDetector(44100, 2048))
{
    // Get audio from microphone or AudioSource
    float[] audioBuffer = GetAudioBuffer();

    // Detect pitch
    var result = detector.Detect(audioBuffer);

    if (result.Voiced)
    {
        Debug.Log($"Frequency: {result.Frequency:F1} Hz");
        Debug.Log($"Confidence: {result.Confidence:F2}");

        // Get musical note
        var note = PitchDetector.FrequencyToNote(result.Frequency);
        Debug.Log($"Note: {note.Name}{note.Octave} ({note.Cents:+0.0;-0.0} cents)");
    }
}
```

---

## 📚 API Reference

### DSP Functions (MATLAB-Compatible)

| Function | Description | MATLAB Equivalent |
|----------|-------------|-------------------|
| `DSP.Butter()` | Butterworth filter design | `butter` |
| `DSP.Cheby1()` | Chebyshev Type I filter | `cheby1` |
| `DSP.Cheby2()` | Chebyshev Type II filter | `cheby2` |
| `DSP.Ellip()` | Elliptic filter | `ellip` |
| `DSP.LFilter()` | Apply IIR/FIR filter | `filter` |
| `DSP.FiltFilt()` | Zero-phase filtering | `filtfilt` |
| `DSP.Xcorr()` | Cross-correlation | `xcorr` |
| `DSP.Hilbert()` | Hilbert transform | `hilbert` |
| `DSP.Envelope()` | Instantaneous amplitude | `abs(hilbert())` |
| `DSP.Pwelch()` | Power spectral density | `pwelch` |
| `DSP.Resample()` | Rational resampling | `resample` |
| `DSP.Window()` | Generate window | `hann`, `hamming`, etc. |
| `DSP.Freqz()` | Frequency response | `freqz` |

### Analysis Components

| Class | Purpose |
|-------|---------|
| `FFTAnalyzer` | Real-time FFT with managed memory |
| `STFTAnalyzer` | Short-Time Fourier Transform (batch + streaming) |
| `StreamingIIRFilter` | Real-time filtering with state preservation |
| `OnsetDetector` | Beat/transient detection with callbacks |
| `PitchDetector` | Real-time pitch detection with YIN algorithm ⭐ NEW |
| `AudioPipeline` | Microphone input and AudioSource processing |

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      Unity C# Layer                              │
│                                                                   │
│  DSP Functions    FFTAnalyzer    StreamingIIRFilter              │
│  (Static API)     STFTAnalyzer   OnsetDetector                   │
│                                                                   │
│                   SpectraNative.cs (P/Invoke)                     │
└──────────────────────────────┬──────────────────────────────────┘
                               │ C ABI
┌──────────────────────────────┴──────────────────────────────────┐
│                     Native C++ Core                              │
│                                                                   │
│  ┌────────────────────────────────────────────────────────┐     │
│  │ DSP: FFT, STFT, Filters, Correlation, Hilbert          │     │
│  ├────────────────────────────────────────────────────────┤     │
│  │ Analysis: PSD, Onset Detection, Resampling             │     │
│  ├────────────────────────────────────────────────────────┤     │
│  │ Core: Memory Management, Thread Safety, Error Handling │     │
│  └────────────────────────────────────────────────────────┘     │
└─────────────────────────────────────────────────────────────────┘
```

---

## 💻 Platform Support

| Platform | Status | Architecture | Compiler | Notes |
|----------|--------|--------------|----------|-------|
| **Linux** | ✅ Complete | x86_64 | GCC 11+ | Desktop |
| **Windows** | ✅ Complete | x64 | MSVC 2022 | Desktop |
| **macOS** | ✅ Complete | Universal (x86_64 + arm64) | Clang | Desktop |
| **iOS** | ✅ Complete ⭐ NEW | arm64, armv7 | Xcode | Mobile |
| **Android** | ✅ Complete ⭐ NEW | arm64-v8a, armeabi-v7a, x86, x86_64 | NDK r21+ | Mobile |
| **WebGL** | ✅ Complete ⭐ NEW | WebAssembly | Emscripten 3.1+ | Browser |

### Build Status

All platforms automatically built and tested via CI/CD:
- **204 tests** run on every commit (68 per platform × 3)
- **Automated artifacts** available for download
- **Universal binaries** for maximum compatibility

---

## 🔨 Building from Source

### Prerequisites

- **CMake** 3.15 or newer
- **C++17 Compiler:**
  - Windows: Visual Studio 2022
  - macOS: Xcode Command Line Tools
  - Linux: GCC 11+ or Clang 15+
- **Unity** 2021.3 LTS or newer (for Unity integration)

### Linux

```bash
cd native
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build  # Run 68 tests
```

### Windows

```powershell
# From Developer Command Prompt for VS 2022
cd native
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release
```

### macOS (Universal Binary)

```bash
cd native
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build
ctest --test-dir build
```

### Output Files

| Platform | File | Size | Location |
|----------|------|------|----------|
| Linux | `libspectra.so` | ~140 KB | `build/native/` |
| Windows | `spectra.dll` | ~100 KB | `build/native/Release/` |
| macOS | `libspectra.dylib` | ~250 KB | `build/native/` |

---

## 📖 Documentation

### Comprehensive Guides

| Document | Description | Lines |
|----------|-------------|-------|
| [API Reference](docs/API_Reference.md) | Complete API documentation with examples | 2,161 |
| [Tutorials](docs/Tutorials.md) | 5 hands-on tutorials (2.5 hours) | 2,813 |
| [Example Projects](docs/ExampleProjects.md) | 5 complete working projects | 2,147 |
| [Filter Designer Guide](docs/FilterDesignerGuide.md) | Editor tool usage | 580 |
| [Signal Inspector Guide](docs/SignalInspectorGuide.md) | Editor tool usage | 720 |
| [CI/CD Guide](docs/CI_CD_Guide.md) | Build automation | 978 |

**Total:** 13,000+ lines of professional documentation

### Quick Links

- **Getting Started:** See [Tutorials.md](docs/Tutorials.md) - Tutorial 1 (15 min)
- **API Functions:** See [API_Reference.md](docs/API_Reference.md)
- **Build Guide:** See [CI_CD_Guide.md](docs/CI_CD_Guide.md)
- **Examples:** See [ExampleProjects.md](docs/ExampleProjects.md)

### Platform-Specific Testing

- [Windows Validation](docs/Windows_Validation_Requirements.md)
- [macOS Validation](docs/macOS_Validation_Requirements.md)
- [Cross-Platform Build Guide](docs/CrossPlatformBuildGuide.md)

---

## 🎓 Tutorials

### 5 Complete Tutorials (2.5 hours total)

1. **Your First Filter** (15 min) - Design and apply filters
2. **Real-Time Spectrum Analyzer** (30 min) - FFT visualization
3. **Beat Detection and Visualization** (25 min) - Onset detection for gameplay
4. **Custom Audio Effect Processor** (35 min) - EQ + distortion + delay
5. **Building a Spectrogram** (30 min) - STFT time-frequency analysis

Each tutorial includes:
- Complete working code
- Step-by-step instructions
- Expected outputs
- Troubleshooting tips

**Start here:** [docs/Tutorials.md](docs/Tutorials.md)

---

## 🎨 Example Projects

### Complete Working Examples

All examples include full source code, setup instructions, and ready-to-use scripts.

#### 1. Audio Visualizer
- 3D spectrum bars (64 bars with color gradient)
- Waveform display (oscilloscope-style)
- Real-time FFT visualization
- Microphone input support
- **Setup time:** 10 minutes

#### 2. Filter Playground ⭐ NEW
- Interactive filter designer with bandpass/bandstop support
- Real-time frequency response plots (Bode diagrams)
- 4 filter types (Butterworth, Chebyshev I/II, Elliptic) × 4 response types (LP/HP/BP/BS)
- Dual-frequency controls for bandpass/bandstop design
- Practical examples (speech enhancement, hum removal, music analysis)
- Audio preview and filter statistics
- **Setup time:** 15 minutes

#### 3. Beat-Synced Game
- Real-time beat detection
- Rhythm game mechanics
- Score system with combo multiplier
- Visual and particle effects
- **Setup time:** 12 minutes

#### 4. Audio Effects Studio
- 3-band EQ processor
- Distortion and delay effects
- Effect presets
- Real-time parameter control
- **Setup time:** 15 minutes

#### 5. Spectrogram Analyzer
- Scrolling spectrogram display
- Time-frequency visualization
- Interactive analysis
- PNG export
- **Setup time:** 10 minutes

**Total code:** 1,240 lines of production-ready scripts

**See:** [docs/ExampleProjects.md](docs/ExampleProjects.md)

---

## 🧪 Testing

### Comprehensive Test Coverage

- **Native Tests:** 68 C++ unit tests (all platforms)
- **P/Invoke Tests:** 36 binding tests
- **Component Tests:** 35 high-level wrapper tests
- **Total:** 71 Unity tests + 68 native tests per platform

### CI/CD Testing

Every commit automatically:
- ✅ Builds on Linux, Windows, macOS
- ✅ Runs 204 tests (68 × 3 platforms)
- ✅ Generates artifacts for all platforms
- ✅ Creates Unity package

**Current Status:** All tests passing (100% success rate)

---

## 🎯 Use Cases

### Audio-Reactive Games
- Rhythm games (beat detection)
- Music visualizers
- Audio-driven gameplay
- Beat-synchronized effects

### Audio Tools
- Real-time audio analysis
- Filter design and testing
- Spectrogram analysis
- Audio effect prototyping

### Research & Education
- Signal processing education
- Audio feature extraction
- DSP algorithm prototyping
- Scientific data analysis

### Professional Audio
- Audio plugin development
- Effect processors
- Analysis tools
- Quality assurance

---

## 📦 Project Structure

```
SpectraCore/
├── native/                          # C++ DSP Core
│   ├── include/spectra/             # Public C++ headers
│   ├── src/                         # Implementation
│   │   ├── core/                    # Core utilities
│   │   ├── fft/                     # FFT, STFT
│   │   ├── filters/                 # Filter design & application
│   │   ├── analysis/                # Correlation, PSD, Hilbert, etc.
│   │   └── abi/                     # C ABI layer
│   ├── tests/                       # 68 unit tests
│   └── CMakeLists.txt
│
├── unity/                           # Unity Integration
│   ├── Runtime/
│   │   ├── Plugins/                 # Platform-specific libraries
│   │   │   ├── Linux/x86_64/
│   │   │   ├── Windows/x86_64/
│   │   │   └── macOS/
│   │   └── Scripts/                 # C# API
│   │       ├── Core/                # DSP, P/Invoke
│   │       ├── Analysis/            # FFTAnalyzer, STFTAnalyzer
│   │       └── Filters/             # StreamingIIRFilter
│   ├── Editor/
│   │   └── Scripts/                 # Editor tools
│   │       ├── SignalInspectorWindow.cs
│   │       └── FilterDesignerWindow.cs
│   └── Tests/                       # 71 Unity tests
│
├── docs/                            # 13,000+ lines of documentation
│   ├── API_Reference.md             # Complete API docs (2,161 lines)
│   ├── Tutorials.md                 # 5 tutorials (2,813 lines)
│   ├── ExampleProjects.md           # 5 examples (2,147 lines)
│   ├── FilterDesignerGuide.md       # Tool guide (580 lines)
│   ├── SignalInspectorGuide.md      # Tool guide (720 lines)
│   └── CI_CD_Guide.md               # Build guide (978 lines)
│
├── .github/workflows/               # CI/CD
│   └── build-multiplatform.yml      # Multi-platform builds
│
└── README.md                        # This file
```

---

## 🚀 Performance

### Benchmarks (1024-point FFT)

| Platform | Time per FFT | Throughput |
|----------|--------------|------------|
| Linux (x86_64) | ~0.3 ms | 3,300 FFT/sec |
| Windows (x64) | ~0.5 ms | 2,000 FFT/sec |
| macOS (Intel) | ~0.5 ms | 2,000 FFT/sec |
| macOS (Apple Silicon) | ~0.3 ms | 3,300 FFT/sec |

**All platforms:** Suitable for real-time audio at 44.1kHz

### Optimization

- Zero-allocation hot path
- SIMD-optimized (SSE/AVX/NEON where available)
- Lock-free ring buffers
- Efficient memory reuse
- Direct Form II Transposed for filters

---

## 🔄 CI/CD

### Automated Multi-Platform Builds

Every commit triggers:

1. **build-linux** - Ubuntu latest, GCC
2. **build-windows** - Windows latest, MSVC 2022
3. **build-macos** - macOS latest, Clang (universal binary)
4. **package-unity** - Combines all into Unity package

**Total time:** 10-15 minutes (parallel execution)

### Artifacts

Each successful build generates:
- Platform-specific libraries (3 files)
- Complete Unity package (1 file)
- Available for 90 days

**Download:** Visit [Actions](https://github.com/yogt1984/SpectraCore/actions)

---

## 🤝 Contributing

Contributions are welcome! Please see:
- Code style guidelines
- Test requirements
- Pull request process

**Before contributing:**
1. Check existing issues
2. Discuss major changes first
3. Follow coding standards
4. Add tests for new features
5. Update documentation

---

## 📄 License

**MIT License** - See [LICENSE](LICENSE) file

Free for commercial and personal use.

---

## 🙏 Acknowledgments

- **KissFFT** - Fast FFT implementation
- **Unity Technologies** - Unity Engine
- **MATLAB** - Signal Processing Toolbox inspiration
- **Contributors** - Everyone who helped build this project

---

## 📞 Support

- **Documentation:** [docs/](docs/)
- **Issues:** [GitHub Issues](https://github.com/yogt1984/SpectraCore/issues)
- **Discussions:** [GitHub Discussions](https://github.com/yogt1984/SpectraCore/discussions)

---

## 🗺️ Roadmap

### Current (v1.1.0)
- ✅ Core DSP functions
- ✅ Multi-platform support (Linux, Windows, macOS)
- ✅ Unity integration with editor tools
- ✅ Comprehensive documentation (13,000+ lines)
- ✅ CI/CD automation with 204+ automated tests
- ✅ **Bandpass and Bandstop filters** for all IIR types (NEW)
- ✅ **Enhanced Filter Designer** with dual-frequency controls (NEW)
- ✅ **FilterPlayground sample** with practical examples (NEW)

### Future (v1.2+)
- 🔄 Mobile platforms (iOS, Android)
- 🔄 WebGL/WASM support
- 🔄 Additional filter types (Bessel, Parks-McClellan)
- 🔄 Advanced analysis functions (MFCC, spectral centroid)
- 🔄 Performance optimizations (SIMD, Burst)
- 🔄 Real-time waveform display in Filter Designer
- 🔄 Preset system for common filter configurations

---

## ⭐ Star this Repository

If you find SpectraCore useful, please consider giving it a star! ⭐

---

<p align="center">
  <strong>Built with ❤️ for the Unity audio community</strong>
</p>

<p align="center">
  <a href="#-overview">Overview</a> •
  <a href="#-quick-start">Quick Start</a> •
  <a href="#-documentation">Documentation</a> •
  <a href="#-tutorials">Tutorials</a> •
  <a href="#-example-projects">Examples</a> •
  <a href="#-platform-support">Platforms</a>
</p>
