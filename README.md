# SpectraCore

A deterministic, real-time C++ DSP library with Unity integration. Designed as a MATLAB Signal Processing Toolbox equivalent for simulation engineers, researchers, and audio developers.

## Overview

SpectraCore is a native C++ signal processing runtime that provides:

- **Real-time DSP primitives**: FFT, STFT, FIR/IIR filtering, correlation, spectral analysis
- **MATLAB-equivalent API**: Familiar function names (`DSP.Butter()`, `DSP.FiltFilt()`, `DSP.Stft()`)
- **Lock-free streaming**: Zero-allocation hot path with SPSC ring buffers
- **Cross-platform**: Windows, macOS, iOS, Android
- **Unity integration**: Native plugin with C# bindings and editor tools

## Target Audiences

| Audience | Use Case |
|----------|----------|
| **Simulation Engineers** | Digital twins, automotive simulators, defense systems |
| **Academic Researchers** | Robotics, neuroscience, HCI, biosignal analysis |
| **Game/Audio Developers** | Rhythm games, audio-reactive experiences, karaoke |
| **Tool Builders** | Unity editor tools, profilers, diagnostic systems |

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      Unity C# Layer                              │
│   DSP.Butter()  DSP.Cheby1()  DSP.FiltFilt()  DSP.Stft()        │
│                              │                                   │
│                    Managed Wrappers                              │
│          IIRFilter, FIRFilter, FFTAnalyzer, StreamingFilter      │
│                              │                                   │
│              SpectraNative.cs (P/Invoke Bridge)                  │
└──────────────────────────────┬──────────────────────────────────┘
                               │ C ABI Boundary
┌──────────────────────────────┴──────────────────────────────────┐
│                     Native C++ Core                              │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ Processing: FFT, STFT, FIR/IIR, Correlation, PSD, Hilbert  │ │
│  ├────────────────────────────────────────────────────────────┤ │
│  │ Streaming: SPSC RingBuffer, Stateful Filters               │ │
│  ├────────────────────────────────────────────────────────────┤ │
│  │ Core: Types, Memory Pool, Platform Abstraction             │ │
│  └────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

## Features

### Phase 0 - Open Source Core (MIT License)

- Native C++ shared library wrapping KissFFT
- Real-time FFT (256-8192 samples)
- Window functions: Hann, Hamming, Blackman, Flat Top
- Spectral flux calculation
- Adaptive onset detection with C# callbacks
- AudioSource and microphone input pipelines
- 100+ unit tests

### Phase 1 - Engineering DSP Toolbox

**Filter Design & Application**
| Function | Description | MATLAB Equivalent |
|----------|-------------|-------------------|
| `DSP.Butter()` | Butterworth filter design | `butter` |
| `DSP.Cheby1()` | Chebyshev Type I | `cheby1` |
| `DSP.Cheby2()` | Chebyshev Type II | `cheby2` |
| `DSP.Ellip()` | Elliptic filter | `ellip` |
| `DSP.Bessel()` | Bessel filter | `besself` |
| `DSP.FirPM()` | Parks-McClellan FIR | `firpm` |
| `DSP.LFilter()` | Apply filter (causal) | `filter` |
| `DSP.FiltFilt()` | Zero-phase filtering | `filtfilt` |

**Signal Analysis**
| Function | Description | MATLAB Equivalent |
|----------|-------------|-------------------|
| `DSP.Fft()` | Fast Fourier Transform | `fft` |
| `DSP.Stft()` | Short-Time Fourier Transform | `stft` |
| `DSP.Pwelch()` | Power Spectral Density | `pwelch` |
| `DSP.Xcorr()` | Cross-correlation | `xcorr` |
| `DSP.Conv()` | Convolution | `conv` |
| `DSP.Hilbert()` | Hilbert transform | `hilbert` |
| `DSP.Resample()` | Signal resampling | `resample` |

**Editor Tools**
- Signal Inspector: Visualize waveforms, spectra, filter responses
- Filter Designer: Interactive Bode plots with real-time parameter adjustment
- Data export to CSV/JSON

## Project Structure

```
SpectraCore/
├── native/                     # C++ DSP Core
│   ├── include/spectra/        # Public headers
│   ├── src/                    # Implementation
│   ├── third_party/kissfft/    # FFT library
│   ├── tests/                  # GoogleTest/Catch2
│   └── CMakeLists.txt
├── unity/                      # Unity Package
│   ├── Runtime/
│   │   ├── Plugins/            # Native binaries per platform
│   │   └── Scripts/            # C# API
│   ├── Editor/                 # Editor tools
│   ├── Tests/
│   └── package.json
├── tools/                      # Build scripts, MATLAB reference generators
├── docs/                       # Documentation
└── .github/workflows/          # CI/CD pipelines
```

## Building

### Prerequisites

- CMake 3.20+
- C++17 compiler (MSVC 2022, Clang 15+, GCC 11+)
- Unity 2021.3 LTS or newer (for Unity integration)

### Native Library

```bash
cd native
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

### Cross-Platform Builds

```bash
# Windows (from Developer Command Prompt)
cmake -B build -G "Visual Studio 17 2022" -A x64

# macOS
cmake -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

# iOS
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/ios.toolchain.cmake

# Android
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake
```

## Usage Example

```csharp
using Spectra;

// Design a 4th order Butterworth lowpass filter at 0.2 normalized frequency
var (b, a) = DSP.Butter(4, 0.2f, FilterType.Lowpass);

// Apply filter to signal
float[] filtered = DSP.LFilter(b, a, inputSignal);

// Zero-phase filtering (offline)
float[] zeroPhase = DSP.FiltFilt(b, a, inputSignal);

// Real-time streaming filter
using var filter = new StreamingIIRFilter(b, a);
filter.Process(inputChunk, outputChunk);
```

## Documentation

- [Architecture Overview](docs/architecture.md)
- [ABI Design](docs/abi-design.md)
- [MATLAB Equivalence Guide](docs/matlab-equivalence.md)

## Testing

The project maintains >80% code coverage with:

- Unit tests for all DSP primitives
- Numerical stability tests
- Frequency response validation
- MATLAB reference comparison tests
- Sanitizer runs (ASan, TSan, UBSan)

## Roadmap

- [ ] Phase 0: Open source core with FFT, onset detection
- [ ] Phase 1: Engineering DSP Toolbox ($179)
- [ ] Phase 2: Audio & Rhythm Analysis Suite ($55)
- [ ] Phase 3: WebGL/WASM, DOTS/Burst, ONNX integration

## License

Phase 0 (Core): MIT License
Phase 1+ (Toolbox): Commercial License

## Contributing

Contributions welcome for Phase 0 components. See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.
