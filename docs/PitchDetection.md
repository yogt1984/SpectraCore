# Pitch Detection

Real-time pitch detection using YIN and autocorrelation algorithms with musical note mapping.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Quick Start](#quick-start)
- [Detection Methods](#detection-methods)
- [Usage Examples](#usage-examples)
- [Configuration](#configuration)
- [Musical Note Conversion](#musical-note-conversion)
- [Performance](#performance)
- [Best Practices](#best-practices)
- [API Reference](#api-reference)

## Overview

SpectraCore provides accurate real-time pitch detection for audio signals, perfect for:

- Musical instrument tuners
- Pitch tracking in audio applications
- Voice analysis
- Music transcription
- Real-time pitch correction

The implementation uses the **YIN algorithm** (de Cheveigné & Kawahara, 2002), widely regarded as one of the most accurate pitch detection methods, with autocorrelation and HPS methods as alternatives.

## Features

- **Multiple detection methods:**
  - YIN algorithm (most accurate, default)
  - Autocorrelation (faster, less accurate)
  - Harmonic Product Spectrum (HPS) (good for harmonic signals)
  - Auto mode (automatically selects best method)

- **Sub-sample accuracy** via parabolic interpolation
- **Confidence scoring** for pitch reliability
- **Voiced/unvoiced detection** to reject silence and noise
- **Musical note mapping** with cents deviation
- **Configurable frequency range** and thresholds
- **Alternative tuning support** (e.g., A4 = 432 Hz)
- **Real-time capable** with low latency

## Quick Start

### Unity C#

```csharp
using Spectra;

// Create pitch detector (44.1 kHz, 2048 samples)
using (var detector = new PitchDetector(44100, 2048))
{
    // Get audio buffer from microphone or AudioSource
    float[] audioBuffer = GetAudioBuffer();

    // Detect pitch
    var result = detector.Detect(audioBuffer);

    if (result.Voiced)
    {
        Debug.Log($"Pitch: {result.Frequency:F1} Hz");
        Debug.Log($"Confidence: {result.Confidence:F2}");

        // Get musical note
        var note = PitchDetector.FrequencyToNote(result.Frequency);
        Debug.Log($"Note: {note.Name}{note.Octave} ({note.Cents:+0.0;-0.0} cents)");
    }
}
```

### Native C++

```cpp
#include "spectra/analysis/pitch_detection.hpp"

using namespace spectra;

// Create detector
PitchDetector detector(44100.0f, 2048, 60.0f, 2000.0f);

// Detect pitch
PitchResult result = detector.detect(audioBuffer, bufferSize);

if (result.voiced) {
    std::cout << "Frequency: " << result.frequency << " Hz\n";
    std::cout << "Confidence: " << result.confidence << "\n";

    // Get musical note
    MusicalNote note = frequencyToNote(result.frequency);
    std::cout << "Note: " << note.name << note.octave
              << " (" << note.cents << " cents)\n";
}
```

## Detection Methods

### YIN Algorithm (Recommended)

The YIN algorithm is the most accurate method, using a difference function and cumulative mean normalized difference (CMND) to find the fundamental frequency.

**Pros:**
- Highest accuracy
- Excellent for musical signals
- Robust to harmonics

**Cons:**
- Slightly more computation than autocorrelation

**When to use:** Default choice for most applications

```csharp
var result = detector.Detect(buffer, PitchMethod.YIN);
```

### Autocorrelation

Classic autocorrelation-based pitch detection.

**Pros:**
- Fast computation
- Simple and reliable

**Cons:**
- Less accurate than YIN
- More susceptible to octave errors

**When to use:** Performance-critical applications where slight accuracy loss is acceptable

```csharp
var result = detector.Detect(buffer, PitchMethod.Autocorrelation);
```

### Harmonic Product Spectrum (HPS)

Analyzes frequency domain harmonics (currently falls back to autocorrelation).

**When to use:** Harmonic-rich signals like musical instruments

```csharp
var result = detector.Detect(buffer, PitchMethod.HPS);
```

### Auto Mode

Automatically selects the best method based on signal characteristics.

```csharp
var result = detector.Detect(buffer, PitchMethod.Auto);
```

## Usage Examples

### Guitar Tuner

```csharp
using Spectra;
using UnityEngine;

public class GuitarTuner : MonoBehaviour
{
    private PitchDetector detector;
    private AudioClip micClip;
    private float[] samples;

    void Start()
    {
        detector = new PitchDetector(AudioSettings.outputSampleRate, 2048, 60f, 400f);
        samples = new float[2048];

        // Start microphone
        micClip = Microphone.Start(null, true, 1, AudioSettings.outputSampleRate);
    }

    void Update()
    {
        // Get audio from microphone
        int micPosition = Microphone.GetPosition(null);
        if (micPosition > 0 && micClip != null)
        {
            micClip.GetData(samples, micPosition - 2048);

            // Detect pitch
            var note = detector.DetectNote(samples);

            if (!string.IsNullOrEmpty(note.Name))
            {
                DisplayNote(note.Name, note.Octave, note.Cents);
            }
        }
    }

    void DisplayNote(string name, int octave, float cents)
    {
        Debug.Log($"{name}{octave} - {cents:+0.0;-0.0} cents");

        // Update UI
        // Green if within ±10 cents, yellow if ±10-30 cents, red otherwise
        Color tuningColor = Mathf.Abs(cents) < 10 ? Color.green :
                            Mathf.Abs(cents) < 30 ? Color.yellow : Color.red;
    }

    void OnDestroy()
    {
        detector?.Dispose();
    }
}
```

### Vocal Pitch Tracker

```csharp
using Spectra;
using System.Collections.Generic;

public class VocalPitchTracker : MonoBehaviour
{
    private PitchDetector detector;
    private List<float> pitchHistory = new List<float>();

    void Start()
    {
        // Configure for vocal range (80-800 Hz)
        detector = new PitchDetector(44100, 2048, 80f, 800f);

        // Set higher confidence threshold for vocals
        detector.SetMinConfidence(0.7f);
    }

    public void AnalyzeVocalPitch(float[] audioBuffer)
    {
        var result = detector.Detect(audioBuffer);

        if (result.Voiced && result.Confidence > 0.7f)
        {
            pitchHistory.Add(result.Frequency);

            // Calculate statistics
            float avgPitch = GetAveragePitch();
            float variance = GetPitchVariance();

            Debug.Log($"Current: {result.Frequency:F1} Hz, Avg: {avgPitch:F1} Hz, Variance: {variance:F2}");
        }
    }

    float GetAveragePitch()
    {
        if (pitchHistory.Count == 0) return 0;

        float sum = 0;
        foreach (float pitch in pitchHistory)
            sum += pitch;
        return sum / pitchHistory.Count;
    }

    float GetPitchVariance()
    {
        if (pitchHistory.Count == 0) return 0;

        float avg = GetAveragePitch();
        float sumSquares = 0;
        foreach (float pitch in pitchHistory)
            sumSquares += (pitch - avg) * (pitch - avg);
        return sumSquares / pitchHistory.Count;
    }
}
```

### Real-Time Pitch Visualization

```csharp
using Spectra;
using UnityEngine;

public class PitchVisualizer : MonoBehaviour
{
    [SerializeField] private AudioSource audioSource;
    [SerializeField] private LineRenderer waveform;
    [SerializeField] private TextMesh pitchText;

    private PitchDetector detector;
    private float[] samples;

    void Start()
    {
        int sampleRate = AudioSettings.outputSampleRate;
        detector = new PitchDetector(sampleRate, 2048);
        samples = new float[2048];
    }

    void Update()
    {
        if (audioSource.isPlaying)
        {
            // Get current audio data
            audioSource.GetOutputData(samples, 0);

            // Detect pitch
            var result = detector.Detect(samples);

            if (result.Voiced)
            {
                var note = PitchDetector.FrequencyToNote(result.Frequency);
                pitchText.text = $"{note.Name}{note.Octave}\n{result.Frequency:F1} Hz\n{note.Cents:+0;-0} cents";
                pitchText.color = GetColorFromConfidence(result.Confidence);
            }
            else
            {
                pitchText.text = "---";
                pitchText.color = Color.gray;
            }

            // Draw waveform
            DrawWaveform(samples);
        }
    }

    void DrawWaveform(float[] samples)
    {
        waveform.positionCount = samples.Length;
        for (int i = 0; i < samples.Length; i++)
        {
            waveform.SetPosition(i, new Vector3(i * 0.01f, samples[i], 0));
        }
    }

    Color GetColorFromConfidence(float confidence)
    {
        return Color.Lerp(Color.red, Color.green, confidence);
    }

    void OnDestroy()
    {
        detector?.Dispose();
    }
}
```

## Configuration

### Buffer Size

Buffer size affects accuracy and latency:

- **Larger buffer (4096):** More accurate, higher latency
- **Smaller buffer (1024):** Less accurate, lower latency
- **Recommended:** 2048 samples (good balance)

```csharp
// High accuracy, ~46ms latency at 44.1kHz
var detector = new PitchDetector(44100, 4096);

// Low latency, ~23ms at 44.1kHz
var detector = new PitchDetector(44100, 1024);
```

### Frequency Range

Set the expected frequency range to improve accuracy:

```csharp
// Guitar (82 - 1200 Hz)
var guitarDetector = new PitchDetector(44100, 2048, 82f, 1200f);

// Vocals (80 - 800 Hz)
var vocalDetector = new PitchDetector(44100, 2048, 80f, 800f);

// Bass (40 - 400 Hz)
var bassDetector = new PitchDetector(44100, 2048, 40f, 400f);
```

### Detection Thresholds

Fine-tune detection sensitivity:

```csharp
var detector = new PitchDetector(44100, 2048);

// More sensitive (more detections, more false positives)
detector.SetThreshold(0.05f);

// Less sensitive (fewer detections, fewer false positives)
detector.SetThreshold(0.2f);

// Higher confidence requirement
detector.SetMinConfidence(0.8f);
```

## Musical Note Conversion

### Frequency to Note

```csharp
var note = PitchDetector.FrequencyToNote(440.0f);
// note.Name = "A", note.Octave = 4, note.Cents = 0

var note2 = PitchDetector.FrequencyToNote(445.0f);
// note2.Name = "A", note2.Octave = 4, note2.Cents ≈ +20 (sharp)
```

### Note to Frequency

```csharp
float freqA4 = PitchDetector.NoteToFrequency("A", 4);    // 440.0 Hz
float freqC4 = PitchDetector.NoteToFrequency("C", 4);    // 261.63 Hz
float freqFSharp = PitchDetector.NoteToFrequency("F#", 4); // 369.99 Hz
```

### Alternative Tunings

```csharp
// A4 = 432 Hz (alternative tuning)
var note = PitchDetector.FrequencyToNote(432.0f, 432.0f);
float freq = PitchDetector.NoteToFrequency("A", 4, 432.0f);

// Baroque pitch (A4 = 415 Hz)
var note2 = PitchDetector.FrequencyToNote(415.0f, 415.0f);
```

## Performance

### Benchmarks (44.1 kHz, 2048 samples)

| Method | Processing Time | Accuracy |
|--------|----------------|----------|
| YIN | ~0.5 ms | Excellent |
| Autocorrelation | ~0.3 ms | Good |
| HPS | ~0.4 ms | Good |

All methods are **real-time capable** with latencies under 1ms.

### Memory Usage

- **PitchDetector object:** ~50 KB (preallocated working buffers)
- **Per-frame:** 0 allocations (uses internal buffers)

### Optimization Tips

1. **Reuse detector instances** - avoid creating/destroying per frame
2. **Match buffer size** to your audio callback size
3. **Use appropriate frequency range** - narrower range = better accuracy
4. **Consider Autocorrelation** for performance-critical applications

## Best Practices

### 1. Handle Unvoiced Segments

```csharp
var result = detector.Detect(buffer);

if (result.Voiced)
{
    // Process pitch
    ProcessPitch(result.Frequency);
}
else
{
    // Handle silence/noise
    ShowNoSignal();
}
```

### 2. Smooth Pitch Tracking

```csharp
private float smoothedPitch = 0;
private const float smoothing = 0.8f;

void UpdatePitch(PitchResult result)
{
    if (result.Voiced)
    {
        smoothedPitch = Mathf.Lerp(result.Frequency, smoothedPitch, smoothing);
        DisplayPitch(smoothedPitch);
    }
}
```

### 3. Confidence Gating

```csharp
const float MIN_CONFIDENCE = 0.7f;

var result = detector.Detect(buffer);

if (result.Voiced && result.Confidence >= MIN_CONFIDENCE)
{
    // High-confidence detection
    ProcessReliablePitch(result);
}
```

### 4. Resource Management

```csharp
public class PitchAnalyzer : IDisposable
{
    private PitchDetector detector;

    public PitchAnalyzer()
    {
        detector = new PitchDetector(44100, 2048);
    }

    public void Dispose()
    {
        detector?.Dispose();
    }
}

// Use with 'using' statement
using (var analyzer = new PitchAnalyzer())
{
    // Use analyzer
}
```

## API Reference

### Unity C# API

#### PitchDetector Class

```csharp
public class PitchDetector : IDisposable
{
    // Constructor
    public PitchDetector(
        float sampleRate,
        int bufferSize = 2048,
        float minFreq = 60f,
        float maxFreq = 2000f);

    // Properties
    public float SampleRate { get; }
    public int BufferSize { get; }

    // Methods
    public PitchResult Detect(
        float[] buffer,
        PitchMethod method = PitchMethod.YIN);

    public MusicalNote DetectNote(
        float[] buffer,
        float a4Freq = 440f,
        PitchMethod method = PitchMethod.YIN);

    public void SetThreshold(float threshold);
    public void SetMinConfidence(float minConfidence);

    // Static utilities
    public static MusicalNote FrequencyToNote(float frequency, float a4Freq = 440f);
    public static float NoteToFrequency(string noteName, int octave, float a4Freq = 440f);
}
```

#### PitchResult Struct

```csharp
public struct PitchResult
{
    public float Frequency { get; set; }    // Hz (0 if no pitch)
    public float Confidence { get; set; }   // 0.0 to 1.0
    public bool Voiced { get; set; }        // True if clear pitch
    public float Clarity { get; set; }      // 0.0 to 1.0
}
```

#### MusicalNote Struct

```csharp
public struct MusicalNote
{
    public string Name { get; set; }        // e.g., "A", "C#", "Bb"
    public int Octave { get; set; }         // e.g., 4 for A4
    public float Cents { get; set; }        // -50 to +50 (0 = perfect)
    public float Frequency { get; set; }    // Hz
}
```

#### PitchMethod Enum

```csharp
public enum PitchMethod
{
    YIN,              // YIN algorithm (most accurate)
    Autocorrelation,  // Autocorrelation (faster)
    HPS,              // Harmonic Product Spectrum
    Auto              // Auto-select best method
}
```

### Native C++ API

See `spectra/analysis/pitch_detection.hpp` for the full C++ API.

## References

- de Cheveigné, A., & Kawahara, H. (2002). "YIN, a fundamental frequency estimator for speech and music." *The Journal of the Acoustical Society of America*, 111(4), 1917-1930.
- [YIN Algorithm Paper](http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf)

## See Also

- [Onset Detection](OnsetDetection.md)
- [STFT Analysis](STFT.md)
- [API Reference](API_Reference.md)
