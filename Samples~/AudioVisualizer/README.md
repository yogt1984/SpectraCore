# Audio Visualizer Sample

Real-time 3D spectrum visualization with waveform display and audio analysis.

## Overview

This sample demonstrates:
- **Real-time FFT computation** with streaming audio
- **3D spectrum visualization** using Unity's built-in mesh
- **Waveform display** with time-domain rendering
- **Performance optimization** for smooth 60 FPS animation

## What You'll Learn

- How to use `FFTAnalyzer` for real-time spectrum computation
- How to implement custom mesh rendering for visualization
- How to extract audio from AudioSource and process it
- Best practices for real-time DSP in games

## Quick Start

1. Open the sample project in Unity 2021.3 or newer
2. Create a GameObject with an AudioSource
3. Attach the `AudioVisualizer.cs` script
4. Assign the AudioSource in the inspector
5. Play an audio clip and watch the visualization!

## Files Included

- **AudioVisualizer.cs** - Main visualization component
- **VisualizerRenderer.cs** - Custom mesh rendering
- **SpectrumBand.cs** - Individual band representation

## Full Documentation

For comprehensive details and implementation guide, see:
[docs/ExampleProjects.md](../../docs/ExampleProjects.md#example-1-audio-visualizer)

## Next Steps

- Customize the color scheme (change gradient in inspector)
- Add waveform display alongside spectrum
- Implement beat detection to pulse with music
- Create multiple visualizers for different frequency bands

## Performance Tips

- Use 512-1024 FFT size for good quality/performance balance
- Update mesh only when needed (not every frame if possible)
- Use object pooling for bar meshes if visualizing multiple bands
- Profile on target devices before shipping

---

**Happy coding! 🎵**
