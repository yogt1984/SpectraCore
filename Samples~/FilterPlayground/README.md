# Filter Playground Sample

Interactive filter design and Bode plot visualization with real-time audio preview.

## Overview

This sample demonstrates:
- **Interactive filter parameter controls** with real-time updates
- **Bode plot visualization** (magnitude and phase response)
- **Audio preview** of filter effects
- **Coefficient export** for use in other projects

## What You'll Learn

- How to use all 4 filter types (Butterworth, Chebyshev I/II, Elliptic)
- How to compute frequency response using `DSP.Freqz()`
- How to render Bode plots for visualization
- How to apply filters to audio in real-time

## Quick Start

1. Open the sample project in Unity 2021.3 or newer
2. Open the `FilterPlayground` scene
3. Use sliders to adjust:
   - Filter type (Butterworth, Chebyshev I/II, Elliptic)
   - Filter order (1-10)
   - Cutoff frequency (0.01-0.99 normalized)
   - Ripple parameters (for Chebyshev filters)
4. Load an audio clip and click "Play with Filter"
5. Observe the Bode plot updating in real-time

## Files Included

- **FilterPlaygroundUI.cs** - UI controller and parameter management
- **BodePlotRenderer.cs** - Magnitude and phase response visualization
- **FilterAudioProcessor.cs** - Real-time filter application

## Full Documentation

For comprehensive details and implementation guide, see:
[docs/ExampleProjects.md](../../docs/ExampleProjects.md#example-2-filter-playground)

## Features

- **Live Bode Plots**: Magnitude and phase response with grid
- **Parameter Presets**: Save and load filter configurations
- **Coefficient Display**: See the actual b and a coefficients
- **Export Function**: Copy coefficients for use in code
- **Audio Comparison**: Play original vs filtered audio

## Next Steps

- Implement bandpass/bandstop filters (when available in v1.1)
- Add frequency response markers (0dB, -3dB points)
- Create preset system for common filter configurations
- Add impulse response visualization

---

**Happy filtering! 🎛️**
