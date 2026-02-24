# Spectrogram Analyzer Sample

Time-frequency visualization with color-coded magnitude, real-time streaming, and CSV export.

## Overview

This sample demonstrates:
- **STFT computation** in streaming mode with `STFTAnalyzer`
- **Time-frequency visualization** with color-coded magnitude
- **Real-time spectrogram** that scrolls as audio plays
- **CSV export** for analysis in MATLAB/Python
- **Frequency/time axis labels** for professional visualization

## What You'll Learn

- How to use `STFTAnalyzer` for time-frequency analysis
- How to render spectrograms efficiently
- How to handle streaming audio in chunks
- How to export data for external analysis
- Best practices for time-frequency visualization
- Applications in speech analysis and music information retrieval

## Quick Start

1. Open the sample project in Unity 2021.3 or newer
2. Open the `SpectrogramAnalyzer` scene
3. Attach an AudioSource with audio clip
4. The spectrogram updates in real-time as audio plays
5. Click "Export to CSV" to save for analysis in MATLAB/Python
6. Adjust FFT/Hop size for different time/frequency resolution tradeoffs

## Files Included

- **SpectrogramAnalyzer.cs** - STFT computation and frame management
- **SpectrogramRenderer.cs** - Real-time visualization and color mapping
- **SpectrogramExporter.cs** - CSV export functionality
- **AxisLabels.cs** - Time and frequency axis rendering

## Full Documentation

For comprehensive details and implementation guide, see:
[docs/ExampleProjects.md](../../docs/ExampleProjects.md#example-5-spectrogram-analyzer)

## Features

- **Real-Time Streaming**: Processes audio as it plays
- **Adjustable Resolution**:
  - FFT size: 512-4096 (frequency resolution)
  - Hop size: varies (time resolution)
  - Window: Hann, Hamming, Blackman
- **Color Mapping**: Jet colormap (blue → red) for magnitude
- **Axis Labels**: Time (seconds) and frequency (Hz)
- **Zoom/Pan**: Navigate large spectrograms
- **CSV Export**: For MATLAB/Python analysis

## STFT Parameters

**Time Resolution vs Frequency Resolution Tradeoff:**

| FFT Size | Hop Size | Frequency Res | Time Res | Use Case |
|----------|----------|---------------|----------|----------|
| 512 | 128 | Good | Excellent | Speech, transients |
| 1024 | 256 | Better | Good | General analysis |
| 2048 | 512 | Best | Fair | Harmonic analysis |
| 4096 | 1024 | Excellent | Poor | Pitch detection |

## Applications

- **Speech Analysis**: Formant tracking, phoneme visualization
- **Music Analysis**: Onset detection, harmonic content
- **Acoustic Analysis**: Noise characterization, signal quality
- **Machine Learning**: Feature extraction for classification
- **Audio Quality Assurance**: Artifact detection

## Next Steps

- Implement log-frequency scale (mel-scale or MIDI note scale)
- Add click-to-listen feature (play selected time/frequency region)
- Compute and display spectral features (centroid, rolloff, flux)
- Implement automatic peak detection
- Add harmonic/percussive source separation visualization

## Tips for Best Results

- Use 75% overlap (hop = FFT/4) for smooth spectrograms
- Hann window provides good time/frequency tradeoff
- Log-scale display (dB) often reveals details better than linear
- 44.1 kHz sample rate: Use 1024 FFT for ~21 Hz frequency resolution

## Export Format

CSV contains:
- Header: Frequency labels (Hz)
- Rows: Time frames (ms)
- Values: Magnitude (linear or dB)

```csv
,0,43,86,129,...  # Frequency (Hz)
0,0.1,0.2,0.15,...   # Time 0 ms
46,0.12,0.22,...     # Time 46 ms
92,0.08,0.18,...     # Time 92 ms
```

---

**Let's analyze some audio! 📊🔊**
