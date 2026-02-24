# SpectraCore v1.0.0 - Production Ready 🎉

**First official production release** of SpectraCore DSP library for Unity!

A high-performance, cross-platform digital signal processing library with MATLAB-compatible APIs, comprehensive Unity integration, and professional editor tools.

---

## ✨ Highlights

- **13 MATLAB-compatible DSP functions** - FFT, filtering, spectral analysis, signal processing
- **Cross-platform validated** - Linux, Windows, macOS (Universal Binary)
- **Professional Unity integration** - Editor tools, components, memory-safe bindings
- **Production-grade quality** - 204 automated tests, 100% pass rate
- **Comprehensive documentation** - 13,000+ lines including API reference, tutorials, examples

---

## 📦 Installation

**Unity Package Manager (Recommended):**

1. Open Unity 2021.3 or newer
2. Go to `Window → Package Manager`
3. Click `+` → `Add package from git URL`
4. Enter: `https://github.com/yogt1984/SpectraCore.git`

**Or download release artifacts:**

Download the platform libraries from this release's assets section below.

**For detailed instructions**, see [INSTALLATION.md](https://github.com/yogt1984/SpectraCore/blob/main/INSTALLATION.md)

---

## 🚀 What's New

### Core DSP Functions

```csharp
// Filter design (4 types)
var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
var (b, a) = DSP.Cheby1(4, 0.5f, 0.3f, FilterType.Lowpass);
var (b, a) = DSP.Cheby2(4, 40f, 0.3f, FilterType.Lowpass);
var (b, a) = DSP.Ellip(4, 0.5f, 40f, 0.3f, FilterType.Lowpass);

// Filter application
float[] filtered = DSP.FiltFilt(b, a, signal);

// Signal analysis
var (psd, freqs) = DSP.Pwelch(signal, 256, 128);
float[] envelope = DSP.Envelope(signal);
var (mag, phase, freqs) = DSP.Freqz(b, a, 512);
```

### Unity Components

- **FFTAnalyzer** - Real-time FFT with magnitude/power spectrum
- **STFTAnalyzer** - Batch and streaming STFT for time-frequency analysis
- **StreamingIIRFilter** - Real-time filtering with state preservation
- **OnsetDetector** - Event-based beat detection

### Editor Tools

- **Signal Inspector** (`Window → Spectra → Signal Inspector`)
  - Waveform, spectrum, spectrogram visualization
  - Filter response overlay
  - CSV export for MATLAB/Python analysis

- **Filter Designer** (`Window → Spectra → Filter Designer`)
  - Interactive filter design with real-time Bode plots
  - 4 filter types with all parameters
  - Coefficient export (clipboard or C# file)

---

## 📊 Platform Support

| Platform | Library | Size | Tests | Status |
|----------|---------|------|-------|--------|
| **Linux** (x86_64) | libspectra.so | ~140 KB | 68/68 | ✅ Validated |
| **Windows** (x64) | spectra.dll | ~100 KB | 68/68 | ✅ Validated |
| **macOS** (Universal) | libspectra.dylib | ~250 KB | 68/68 | ✅ Validated |
| Android (ARM64) | libspectra.so | - | - | ⚠️ Experimental |
| iOS (ARM64) | libspectra.a | - | - | ⚠️ Experimental |

**Total tests run**: 204 (68 tests × 3 platforms, 100% pass rate)

---

## 📚 Documentation

Complete learning path (~4 hours from beginner to advanced):

1. **[API Reference](https://github.com/yogt1984/SpectraCore/blob/main/docs/API_Reference.md)** (2,161 lines)
   - All 13 DSP functions documented
   - MATLAB equivalents
   - 50+ code examples

2. **[Tutorials](https://github.com/yogt1984/SpectraCore/blob/main/docs/Tutorials.md)** (2,813 lines, 2.5 hours)
   - Tutorial 1: Your First Filter (15 min)
   - Tutorial 2: Real-Time Spectrum Analyzer (30 min)
   - Tutorial 3: Beat Detection and Visualization (25 min)
   - Tutorial 4: Custom Audio Effect Processor (35 min)
   - Tutorial 5: Building a Spectrogram (30 min)

3. **[Example Projects](https://github.com/yogt1984/SpectraCore/blob/main/docs/ExampleProjects.md)** (2,147 lines)
   - 5 complete working applications
   - 1,240 lines of production-ready code
   - Audio Visualizer, Filter Playground, Beat-Synced Game, Effects Studio, Spectrogram Analyzer

4. **Additional Guides**
   - [Installation Guide](https://github.com/yogt1984/SpectraCore/blob/main/INSTALLATION.md)
   - [CI/CD Guide](https://github.com/yogt1984/SpectraCore/blob/main/docs/CI_CD_Guide.md)
   - [Filter Designer Guide](https://github.com/yogt1984/SpectraCore/blob/main/docs/FilterDesignerGuide.md)

---

## 🎯 Quick Start

### 30-Second Filter Example

```csharp
using SpectraCore;
using UnityEngine;

public class FilterExample : MonoBehaviour
{
    void Start()
    {
        float[] signal = GetAudioSamples();

        // Design lowpass filter
        var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);

        // Apply zero-phase filtering
        float[] filtered = DSP.FiltFilt(b, a, signal);

        PlayAudio(filtered);
    }
}
```

### 2-Minute Spectrum Analyzer

```csharp
using SpectraCore;
using UnityEngine;

public class SpectrumAnalyzer : MonoBehaviour
{
    private FFTAnalyzer fft;

    void Start()
    {
        fft = new FFTAnalyzer(1024);
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        float[] mono = ExtractMono(data, channels);
        float[] spectrum = fft.GetMagnitudeSpectrum(mono);

        // Visualize spectrum...
    }

    void OnDestroy() => fft?.Dispose();
}
```

More examples in the [Tutorials](https://github.com/yogt1984/SpectraCore/blob/main/docs/Tutorials.md).

---

## ⚡ Performance

Tested on Intel i7-8700K @ 3.7GHz, Unity 2021.3:

| Operation | Size | Time | Real-Time Safe |
|-----------|------|------|----------------|
| FFT | 1024 | 0.5 ms | ✅ Yes |
| STFT Frame | 512 | 2.1 ms | ✅ Yes |
| Filter Design | Order 8 | 0.3 ms | ✅ Yes |
| Streaming Filter | 512 samples | 0.04 ms | ✅ Yes |

---

## 🔧 System Requirements

- **Unity**: 2021.3 LTS or newer
- **Platforms**: Windows 10/11, macOS 10.15+, Linux (Ubuntu 18.04+)
- **API Compatibility**: .NET Standard 2.1
- **Scripting Backend**: Mono or IL2CPP

---

## 📝 Known Limitations

1. **Bandpass/bandstop filters not yet implemented** (planned for v1.1)
   - Workaround: Use two filters in series

2. **Mobile platforms experimental**
   - Native libraries build successfully
   - Additional Unity testing recommended

---

## 🗺️ Roadmap

### v1.1.0 (Next Release)
- Bandpass and bandstop filter implementations
- Additional window functions (Tukey, Flat-top)
- MFCC (Mel-frequency cepstral coefficients)
- Spectral centroid and rolloff
- WebGL platform support

---

## 📄 License

SpectraCore is released under the **[MIT License](https://github.com/yogt1984/SpectraCore/blob/main/LICENSE)**.

---

## 🙏 Acknowledgments

- **KissFFT** - Fast FFT implementation (BSD-3-Clause)
- **Unity Technologies** - Game engine and editor extensibility
- **MATLAB** - API design inspiration

---

## 📞 Support

- **Documentation**: [docs/](https://github.com/yogt1984/SpectraCore/tree/main/docs)
- **Report Issues**: [GitHub Issues](https://github.com/yogt1984/SpectraCore/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yogt1984/SpectraCore/discussions)
- **CI/CD Status**: [GitHub Actions](https://github.com/yogt1984/SpectraCore/actions)

---

## 📦 Release Assets

The following artifacts are available for download:

- **native-ubuntu-latest.zip** - Linux library (libspectra.so)
- **native-windows-latest.zip** - Windows library (spectra.dll)
- **native-macos-latest.zip** - macOS Universal library (libspectra.dylib)
- **Source code** (zip)
- **Source code** (tar.gz)

Download artifacts from the [CI/CD workflow](https://github.com/yogt1984/SpectraCore/actions) or use the Unity Package Manager to install directly from Git.

---

## 🎊 Full Release Notes

For complete details, see:
- [RELEASE_NOTES_v1.0.0.md](https://github.com/yogt1984/SpectraCore/blob/main/RELEASE_NOTES_v1.0.0.md)
- [CHANGELOG.md](https://github.com/yogt1984/SpectraCore/blob/main/CHANGELOG.md)

---

**Happy DSP coding! 🎵🔊**
