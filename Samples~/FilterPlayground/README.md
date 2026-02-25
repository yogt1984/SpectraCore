# Filter Playground Sample

Interactive filter design and Bode plot visualization with real-time audio preview. Now includes bandpass/bandstop support with practical examples!

## Overview

This sample demonstrates:
- **Interactive filter parameter controls** with real-time updates
- **Bandpass and bandstop filters** with dual-frequency controls ⭐ NEW
- **Bode plot visualization** (magnitude and phase response)
- **Audio preview** of filter effects
- **Coefficient export** for use in other projects
- **Common bandpass use cases** (speech, acoustic, music analysis)

## What You'll Learn

- How to use all 4 filter types (Butterworth, Chebyshev I/II, Elliptic)
- How to design **bandpass and bandstop filters** with dual frequencies ⭐
- How to compute frequency response using `DSP.Freqz()`
- How to render Bode plots for visualization
- How to apply filters to audio in real-time
- **Practical bandpass applications**: Speech enhancement, hum removal, music analysis

## Quick Start

1. Open the sample project in Unity 2021.3 or newer
2. Open the `FilterPlayground` scene
3. Use sliders to adjust:
   - Filter type (Butterworth, Chebyshev I/II, Elliptic)
   - Response type: **Lowpass, Highpass, Bandpass, or Bandstop** ⭐
   - Filter order (1-10)
   - For bandpass/bandstop: Low and high frequency (0.01-0.99 normalized)
   - For single-frequency: Cutoff frequency
   - Ripple parameters (for Chebyshev filters)
4. Load an audio clip and click "Play with Filter"
5. Observe the Bode plot updating in real-time

## Files Included

- **FilterPlaygroundUI.cs** - UI controller with bandpass/bandstop support ⭐
- **BodePlotRenderer.cs** - Magnitude and phase response visualization
- **FilterAudioProcessor.cs** - Real-time filter application with statistics
- **BandpassExamples.cs** - Practical examples and use cases ⭐ NEW

## Full Documentation

For comprehensive details and implementation guide, see:
[docs/ExampleProjects.md](../../docs/ExampleProjects.md#example-2-filter-playground)

## Features

- **Live Bode Plots**: Magnitude and phase response with grid
- **Single & Dual Frequency Modes**: Lowpass/Highpass vs Bandpass/Bandstop ⭐
- **Parameter Presets**: Save and load filter configurations
- **Coefficient Display**: See the actual b and a coefficients
- **Export Function**: Copy coefficients for use in code
- **Audio Comparison**: Play original vs filtered audio
- **Filter Statistics**: Peak gain, bandwidth, frequency response metrics ⭐
- **Common Use Case Examples**: Speech, acoustic, music analysis ⭐

## Bandpass/Bandstop Examples

The `BandpassExamples.cs` class includes ready-to-use filter designs:

### Speech Enhancement (60 Hz - 3.5 kHz)
```csharp
var (b, a) = BandpassExamples.CreateSpeechBandpass();
// Reduces rumble and high-frequency noise
```

### 60 Hz Hum Removal (Narrow Notch)
```csharp
var (b, a) = BandpassExamples.Create60HzNotch();
// Eliminates mains hum while preserving audio
```

### Acoustic Band (80 Hz - 5 kHz)
```csharp
var (b, a) = BandpassExamples.CreateAcousticBandpass();
// Captures acoustic instrument spectrum
```

### Music Frequency Bands
```csharp
var (b_bass, a_bass) = BandpassExamples.MusicBandpass.CreateBassBandpass();
var (b_mid, a_mid) = BandpassExamples.MusicBandpass.CreateMidBandpass();
var (b_treble, a_treble) = BandpassExamples.MusicBandpass.CreateTrebleBandpass();
```

### Custom Bandpass Design
```csharp
var (b, a) = BandpassExamples.DesignCustomBandpass(
    freqLow: 0.2f,
    freqHigh: 0.4f,
    order: 4,
    filterType: "butterworth"
);
```

## Usage Patterns

### Pattern 1: Interactive Design (UI)
Use the Filter Designer Window in the editor to experiment with parameters in real-time.

### Pattern 2: Programmatic Design
Use the example functions in `BandpassExamples.cs` for common applications.

### Pattern 3: Custom Design
Use `BandpassExamples.DesignCustomBandpass()` for dynamic filter generation.

## Frequency Reference (at 44.1 kHz sample rate)

| Frequency | Normalized |
|-----------|-----------|
| 20 Hz (subsonic) | 0.00045 |
| 60 Hz (hum) | 0.00136 |
| 80 Hz (acoustic low) | 0.00362 |
| 250 Hz (bass/mid boundary) | 0.0113 |
| 1 kHz (music center) | 0.0453 |
| 2 kHz (mid/treble boundary) | 0.0906 |
| 3.5 kHz (speech high) | 0.159 |
| 5 kHz (acoustic high) | 0.227 |
| 8 kHz (speech limit) | 0.362 |
| 20 kHz (Nyquist ≈) | 0.906 |

## Filter Type Comparison

| Type | Passband | Rolloff | Use Case |
|------|----------|---------|----------|
| Butterworth | Flat | Smooth | General purpose |
| Chebyshev I | Ripple | Steep | When passband ripple acceptable |
| Chebyshev II | Monotonic | Steep | When stopband ripple acceptable |
| Elliptic | Ripple | Steepest | Sharpest transition needed |

## Next Steps

- ✅ Bandpass/bandstop filters implemented
- ✅ Common use cases documented
- [ ] Add frequency response markers (0dB, -3dB points)
- [ ] Create preset system UI for common configurations
- [ ] Add impulse response visualization
- [ ] Compare filters: side-by-side Bode plots
- [ ] Real-time waveform display

## Tips & Best Practices

1. **Speech Processing**: Use 60-3500 Hz bandpass with Butterworth order 4
2. **Hum Removal**: Use very narrow bandstop (±5 Hz) with high order (6-8)
3. **Music Analysis**: Split into bass/mid/treble bands for separate processing
4. **Audio Quality**: Use FiltFilt (zero-phase) when phase distortion unacceptable
5. **Order Selection**:
   - Order 2-4: Gentle filtering, preserves character
   - Order 4-6: Moderate filtering, noticeable effect
   - Order 6+: Aggressive filtering, may cause artifacts

## Example Audio Results

**Original signal**: 50 Hz rumble + 1 kHz tone + 8 kHz noise
- **Speech bandpass**: 1 kHz remains, rumble and noise removed ✓
- **60 Hz notch**: Rumble remains, but hum component removed ✓
- **Acoustic bandpass**: 1 kHz remains strong, 8 kHz reduced ✓

---

**Happy filtering! 🎛️ Now with bandpass/bandstop support!**
