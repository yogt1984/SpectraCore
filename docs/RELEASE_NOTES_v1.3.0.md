# SpectraCore v1.3.0

**Release Date:** 2026-03-01

## 🎯 Highlights

- ⭐ **Real-Time Pitch Detection** - Industry-standard YIN algorithm with < 0.5ms processing
- 🎵 **Musical Note Mapping** - Automatic frequency-to-note conversion with cents
- 🎸 **Tuner-Grade Accuracy** - ±5 Hz precision for musical applications
- 🎤 **Multiple Methods** - YIN, Autocorrelation, HPS, and Auto selection

## ✨ New Features

### Pitch Detection
- **YIN Algorithm:** Most accurate method (de Cheveigné & Kawahara, 2002)
  - Sub-sample accuracy via parabolic interpolation
  - Confidence scoring for reliability
  - Voiced/unvoiced detection
- **Alternative Methods:**
  - Autocorrelation (faster, less accurate)
  - Harmonic Product Spectrum (HPS)
  - Auto mode (selects best method)
- **Performance:**
  - Processing: < 0.5ms @ 44.1 kHz
  - Accuracy: ±5 Hz for pure tones
  - Memory: ~50 KB per instance
  - Zero per-frame allocations

### Musical Note Features
- Automatic frequency-to-note conversion
- Octave detection (e.g., A4, C4)
- Cents deviation for tuning (±50 cents)
- Alternative tuning support:
  - Standard: A4 = 440 Hz
  - Alternative: A4 = 432 Hz
  - Baroque: A4 = 415 Hz
- MIDI note number conversion

### Unity C# API

**Basic Usage:**
```csharp
using (var detector = new PitchDetector(44100, 2048))
{
    var result = detector.Detect(audioBuffer);

    if (result.Voiced)
    {
        Debug.Log($"Frequency: {result.Frequency:F1} Hz");
        Debug.Log($"Confidence: {result.Confidence:F2}");

        var note = PitchDetector.FrequencyToNote(result.Frequency);
        Debug.Log($"Note: {note.Name}{note.Octave} ({note.Cents:+0;-0} cents)");
    }
}
```

**Guitar Tuner Example:**
```csharp
// Configure for guitar (82-1200 Hz)
var detector = new PitchDetector(44100, 2048, 82f, 1200f);
detector.SetMinConfidence(0.7f);

var note = detector.DetectNote(audioBuffer);
if (!string.IsNullOrEmpty(note.Name))
{
    // Display tuning: Green if ±10 cents, Yellow if ±30, Red otherwise
    Color tuningColor = Mathf.Abs(note.Cents) < 10 ? Color.green :
                        Mathf.Abs(note.Cents) < 30 ? Color.yellow : Color.red;
}
```

### Examples & Documentation
- **SimplePitchTuner:** Complete microphone-based tuner with:
  - Real-time pitch detection from microphone
  - Visual tuning indicator (green/yellow/red)
  - Configurable detection parameters
  - Instrument presets (guitar, vocals, bass)
  - Alternative tuning support
- **Comprehensive docs:** `docs/PitchDetection.md`
  - Complete API reference
  - Performance benchmarks
  - Best practices and optimization tips
  - 10+ usage examples

## 📦 Downloads

- **libspectra-v1.3.0-linux-x64.so** - Native library with pitch detection (231 KB)
- **SpectraCore-v1.3.0-unity.zip** - Unity package with C# wrappers (161 KB)
- **PitchDetectionExample-v1.3.0.zip** - Complete tuner example (4.2 KB)

## 🎮 Use Cases

- **Musical Instrument Tuners:** Guitar, chromatic, vocal tuners
- **Real-Time Pitch Correction:** Auto-tune, pitch shifting
- **Music Transcription:** Melody detection and analysis
- **Voice Analysis:** Singing training and feedback
- **Game Mechanics:** Pitch-based gameplay (singing games)
- **Audio Analysis:** Fundamental frequency tracking

## 🧪 Testing

- **Native Tests:** 32 comprehensive pitch detection tests
  - YIN algorithm validation
  - Autocorrelation method tests
  - Musical note conversion
  - Edge cases (silence, noise, out-of-range)
- **Unity Tests:** 23 C# integration tests
- **Total:** 494+ tests passing (100% pass rate)

## 📊 Statistics

- **Lines Added:** ~1,880 (C++, C#, docs, tests, examples)
- **Performance:** < 0.5ms per frame @ 44.1 kHz
- **Accuracy:** ±5 Hz (pure tones), ±10 Hz (complex signals)
- **Platforms:** All 6 platforms from v1.2.0 supported
- **Library Size:** 231 KB (includes all features)

## 🔧 Breaking Changes

None - fully backward compatible with v1.2.0

## 🆕 API Additions

### C++ (`spectra/analysis/pitch_detection.hpp`)
```cpp
class PitchDetector {
    PitchDetector(float sample_rate, size_t buffer_size,
                  float min_freq, float max_freq);
    PitchResult detect(const float* buffer, size_t size,
                      PitchMethod method);
    MusicalNote detectNote(const float* buffer, size_t size,
                          float a4_freq, PitchMethod method);
    void setThreshold(float threshold);
    void setMinConfidence(float min_confidence);
};

MusicalNote frequencyToNote(float frequency, float a4_freq = 440.0f);
float noteToFrequency(const std::string& note_name, int octave,
                     float a4_freq = 440.0f);
```

### C# (`PitchDetector.cs`)
```csharp
public class PitchDetector : IDisposable {
    public PitchDetector(float sampleRate, int bufferSize,
                        float minFreq, float maxFreq);
    public PitchResult Detect(float[] buffer, PitchMethod method);
    public MusicalNote DetectNote(float[] buffer, float a4Freq,
                                 PitchMethod method);
    public void SetThreshold(float threshold);
    public void SetMinConfidence(float minConfidence);

    public static MusicalNote FrequencyToNote(float frequency,
                                              float a4Freq = 440f);
    public static float NoteToFrequency(string noteName, int octave,
                                       float a4Freq = 440f);
}
```

## 📚 Key Files

### Native C++
- `native/include/spectra/analysis/pitch_detection.hpp` - Core API (~180 lines)
- `native/src/analysis/pitch_detection.cpp` - Implementation (~550 lines)
- `native/tests/unit/test_pitch_detection.cpp` - Tests (~500 lines)
- `native/include/spectra_abi.h` - C ABI additions
- `native/src/abi/spectra_abi.cpp` - C ABI implementation

### Unity C#
- `unity/Runtime/Scripts/Analysis/PitchDetector.cs` - High-level API (~280 lines)
- `unity/Runtime/Scripts/Core/SpectraNative.cs` - P/Invoke bindings
- `unity/Tests/Editor/SpectraComponentTests.cs` - Integration tests
- `unity/Samples~/PitchDetectionExample/Scripts/SimplePitchTuner.cs` - Example
- `unity/Samples~/PitchDetectionExample/README.md` - Usage guide

### Documentation
- `docs/PitchDetection.md` - Complete guide (~600 lines)
- `RELEASE_GUIDE.md` - Release process documentation

## 🎯 Detection Methods Comparison

| Method | Accuracy | Speed | Best For |
|--------|----------|-------|----------|
| YIN (default) | Excellent | Fast | Most applications |
| Autocorrelation | Good | Very fast | Performance-critical |
| HPS | Good | Fast | Harmonic signals |
| Auto | Varies | Fast | Adaptive scenarios |

## ⚙️ Configuration Examples

### Guitar Tuner
```csharp
var detector = new PitchDetector(44100, 2048, 82f, 1200f);
detector.SetMinConfidence(0.7f);
```

### Vocal Pitch Tracker
```csharp
var detector = new PitchDetector(44100, 2048, 80f, 800f);
detector.SetMinConfidence(0.8f);
```

### Chromatic Tuner
```csharp
var detector = new PitchDetector(44100, 2048, 40f, 2000f);
detector.SetMinConfidence(0.7f);
```

## 🙏 References

- de Cheveigné, A., & Kawahara, H. (2002). "YIN, a fundamental frequency estimator for speech and music." *The Journal of the Acoustical Society of America*, 111(4), 1917-1930.
- [YIN Algorithm Paper](http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf)

## 📖 Full Details

See [CHANGELOG.md](https://github.com/yourusername/SpectraCore/blob/main/CHANGELOG.md) for complete technical details.

---

**Full Changelog:** https://github.com/yourusername/SpectraCore/compare/v1.2.0...v1.3.0
