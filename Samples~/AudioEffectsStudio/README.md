# Audio Effects Studio Sample

Multi-effect processor with EQ, distortion, delay, and preset management.

## Overview

This sample demonstrates:
- **Chained audio effects** (EQ → Distortion → Delay)
- **Real-time parameter control** via UI sliders
- **Preset system** for saving/loading effect configurations
- **Frequency response visualization** showing combined effect
- **Real-time audio processing** in OnAudioFilterRead

## What You'll Learn

- How to chain multiple filters and effects
- How to design filters for specific audio shaping (EQ)
- How to implement non-linear effects (distortion, saturation)
- How to create delay/reverb effects
- How to build a professional effects rack UI
- Best practices for real-time audio processing

## Quick Start

1. Open the sample project in Unity 2021.3 or newer
2. Open the `AudioEffectsStudio` scene
3. Attach an AudioSource with audio clip
4. Use sliders to adjust:
   - EQ (3-band: Low, Mid, High)
   - Distortion (amount and tone)
   - Delay (time and feedback)
5. Listen to changes in real-time
6. Save/load presets

## Files Included

- **EffectsProcessor.cs** - Main effects chain manager
- **EQModule.cs** - 3-band equalizer implementation
- **DistortionModule.cs** - Distortion effect
- **DelayModule.cs** - Delay effect with feedback
- **EffectsUI.cs** - Parameter controls and preset system
- **PresetManager.cs** - Save/load effects configurations

## Full Documentation

For comprehensive details and implementation guide, see:
[docs/ExampleProjects.md](../../docs/ExampleProjects.md#example-4-audio-effects-studio)

## Features

- **3-Band EQ**: Low (80 Hz), Mid (1 kHz), High (8 kHz)
- **Distortion**: Soft-clipping saturation with tone control
- **Delay**: Feedback delay with tap-sync capability
- **Preset System**: Save and load unlimited configurations
- **Bypass Toggle**: Disable effects without removing them
- **Real-time Visualization**: See frequency response changes

## Effect Parameters

**EQ:**
- Low/Mid/High gain: ±12 dB

**Distortion:**
- Amount: 0-100 (0=off, 100=maximum)
- Tone: 0-1 (brightens the distortion)

**Delay:**
- Time: 100-2000 ms
- Feedback: 0-0.9 (feedback amount)
- Mix: 0-100% (dry/wet balance)

## Next Steps

- Add reverb effect (using convolution or Freeverb algorithm)
- Implement compressor/limiter
- Add parametric EQ (adjustable frequency bands)
- Create visual feedback showing each effect's impact
- Implement A/B comparison switching

## Performance Tips

- Use OnAudioFilterRead for minimal latency
- Process only when needed (bypass when dry mix is 100%)
- Use lookup tables for sine/cosine in distortion
- Profile audio processing on target device

---

**Let's make some music! 🎼**
