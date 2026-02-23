# Signal Inspector Window - User Guide

The Signal Inspector is a comprehensive Unity Editor tool for visualizing audio signals, analyzing spectra, and comparing filter responses. It provides real-time visualization and data export capabilities.

## Opening the Signal Inspector

In Unity Editor, navigate to:
```
Window → Spectra → Signal Inspector
```

## Features Overview

### 1. AudioClip Analysis

**Select Audio:**
- Drag and drop any AudioClip from your project
- Or use the object picker to select

**Automatic Processing:**
- Mono conversion (if stereo)
- Real-time spectrum computation
- Optional spectrogram generation

**Info Display:**
- Duration in seconds
- Sample rate (Hz)
- Number of channels

### 2. Visualization Modes

#### **Waveform Display**

Shows the time-domain representation of your audio signal.

**Features:**
- **Amplitude range:** -1.0 to +1.0
- **Time axis:** Shows duration in seconds
- **Color:** Green waveform on dark background
- **Zero line:** Gray center line for reference

**What to look for:**
- Clipping (amplitude touching ±1.0)
- DC offset (waveform not centered)
- Relative loudness (visual amplitude)
- Attack/decay characteristics

#### **Spectrum Display**

Shows the frequency-domain representation (magnitude spectrum).

**Features:**
- **Frequency axis:** 0 Hz to Nyquist (half sample rate)
- **Magnitude axis:** Linear or dB scale
- **Scale options:**
  - **dB Scale:** Shows magnitude in decibels (default)
  - **Log Frequency:** Logarithmic frequency axis
- **Filter Overlay:** Orange line shows filter response

**What to look for:**
- **Fundamental frequency:** Lowest peak for tonal content
- **Harmonics:** Integer multiples of fundamental
- **Noise floor:** Low-level components across spectrum
- **Spectral balance:** Energy distribution across frequencies

**Scale Meanings:**
- **0 dB** = Maximum amplitude (reference)
- **-6 dB** = Half amplitude
- **-20 dB** = 10% amplitude
- **-40 dB** = 1% amplitude
- **-60 dB** = 0.1% amplitude (near noise floor)

#### **Spectrogram Display**

Time-frequency visualization showing how spectrum evolves over time.

**Features:**
- **X-axis:** Time (left to right)
- **Y-axis:** Frequency (bottom to top)
- **Color:** Blue (low energy) → Red (high energy)
- **Resolution:** Controlled by FFT size

**What to look for:**
- **Transients:** Vertical bright lines
- **Sustained tones:** Horizontal bright lines
- **Formants:** Consistent frequency bands
- **Time-varying content:** Diagonal or curved patterns

**Use cases:**
- Speech analysis (formant tracking)
- Music transcription (note detection)
- Sound effect design (texture analysis)
- Fault detection (anomaly patterns)

### 3. FFT Settings

#### FFT Size

**Range:** 256 to 8192 samples

**Trade-offs:**

| FFT Size | Frequency Resolution | Time Resolution | CPU Cost |
|----------|---------------------|-----------------|----------|
| 256 | Low (coarse) | High (fast changes) | Low |
| 512 | Medium-low | Medium-high | Low |
| 1024 | Medium | Medium | Medium |
| 2048 | Medium-high (default) | Medium-low | Medium |
| 4096 | High | Low | High |
| 8192 | Very high (fine) | Very low (slow) | Very high |

**Frequency resolution = Sample Rate / FFT Size**

**Examples at 44100 Hz:**
- FFT 256: 172 Hz per bin (good for rhythm, drums)
- FFT 1024: 43 Hz per bin (good for music)
- FFT 4096: 10.8 Hz per bin (good for pitch detection)
- FFT 8192: 5.4 Hz per bin (good for analysis, slow)

**Recommendations:**
- **Drums/percussion:** 512-1024
- **Music/vocals:** 2048-4096
- **Pitch analysis:** 4096-8192
- **Real-time:** 1024-2048

### 4. Filter Overlay

Compare your audio spectrum with a designed filter's frequency response.

#### Loading a Filter

**Current Method:**
1. Click "Load from Designer" button
2. A demo Butterworth lowpass filter is loaded (placeholder)

**Future Enhancement:**
In a future version, this will integrate with Filter Designer window to load your custom filters.

**Manual Loading (for developers):**
```csharp
// In SignalInspectorWindow.cs, modify LoadFilterFromDesigner():
var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
filterB = b;
filterA = a;
filterName = "My Custom Filter";
```

#### Interpreting the Overlay

**Colors:**
- **Blue curve:** Original audio spectrum
- **Orange curve:** Filter frequency response

**Use cases:**
1. **Pre-filtering check:** See what will be affected before applying filter
2. **Filter verification:** Confirm filter behaves as expected
3. **Comparison:** Try different filter designs visually
4. **Troubleshooting:** Identify why filtered audio sounds wrong

**Example scenarios:**

**Scenario 1: Removing high-frequency noise**
- Load your noisy audio
- Load a lowpass filter at 4 kHz
- Orange line shows filter will pass frequencies below 4 kHz
- Blue spectrum shows noise above 4 kHz will be removed

**Scenario 2: Bass boost**
- Load music with weak bass
- Load a lowpass Butterworth filter
- See which bass frequencies will be enhanced
- Adjust filter cutoff until overlay looks right

### 5. Export Functions

#### Export Waveform (CSV)

**Format:**
```csv
Sample,Time(s),Amplitude
0,0.000000,0.123456
1,0.000023,0.234567
2,0.000045,0.345678
...
```

**Use cases:**
- Import into MATLAB/Python for further analysis
- Create reference data for testing
- Document signal characteristics
- Generate plots in external tools

#### Export Spectrum (CSV)

**Format:**
```csv
Bin,Frequency(Hz),Magnitude,Magnitude(dB)
0,0.00,1.234567,-12.34
1,21.53,0.987654,-15.67
2,43.07,0.765432,-18.90
...
```

**Use cases:**
- Frequency response analysis
- Compare multiple recordings
- Scientific documentation
- Custom visualization in Excel/Python

#### Export Spectrogram (CSV)

**Format:**
```csv
Frame,Time(s),Bin0(0Hz),Bin1(21Hz),Bin2(43Hz),...
0,0.000,0.123,0.234,0.345,...
1,0.023,0.234,0.345,0.456,...
2,0.047,0.345,0.456,0.567,...
...
```

**Use cases:**
- 3D surface plots
- Machine learning training data
- Time-frequency analysis in MATLAB/Python
- Research publications

**Note:** Spectrogram files can be large (MBs) for long audio clips.

### 6. Display Options

#### dB Scale (Recommended)

**Enabled (default):**
- Y-axis in decibels (dB)
- Better for viewing wide dynamic range
- Industry standard

**Disabled:**
- Y-axis in linear magnitude (0 to max)
- Better for viewing small signals
- Harder to see low-level components

**When to use linear:**
- Analyzing noise floor
- Comparing relative levels
- Scientific measurements

#### Log Frequency (Future Feature)

**When implemented:**
- X-axis logarithmic (like human hearing)
- Better for musical analysis
- Shows octaves equally spaced

**Currently:** Linear frequency axis (equal Hz spacing)

## Practical Examples

### Example 1: Analyzing a Drum Loop

**Goal:** Check for frequency balance

**Steps:**
1. Load drum loop AudioClip
2. FFT Size: 1024 (good for transients)
3. Enable Spectrum display
4. Enable dB Scale

**What to look for:**
- **Kick drum:** Peak around 60-100 Hz
- **Snare:** Energy around 200 Hz and 4 kHz
- **Hi-hats:** High-frequency content above 8 kHz
- **Balance:** Relatively even across low/mid/high

**Troubleshooting:**
- If spectrum drops off above 10 kHz → Low-quality recording
- If huge peak at 50/60 Hz → Electrical hum
- If very uneven → Poor mix, may need EQ

### Example 2: Detecting Clipping

**Goal:** Check if audio is clipped

**Steps:**
1. Load suspected clip
2. Enable Waveform display
3. Look at amplitude range

**Clipping indicators:**
- Flat tops/bottoms at ±1.0
- Waveform looks "squared off"
- Spectrum shows excessive harmonics

**Solution:**
- Reduce gain in source
- Apply soft clipping/limiting
- Re-record at lower level

### Example 3: Filter Design Verification

**Goal:** Verify a highpass filter design before applying

**Steps:**
1. Load audio with unwanted low-frequency rumble
2. Enable Spectrum (dB scale)
3. Note where rumble appears (e.g., 0-50 Hz)
4. Design highpass filter in Filter Designer (cutoff: 50 Hz)
5. Click "Load from Designer" in Signal Inspector
6. Orange overlay shows filter response
7. Verify filter attenuates 0-50 Hz range

**Result:** Visually confirm filter will solve the problem

### Example 4: Spectrogram for Speech Analysis

**Goal:** Analyze formants in speech

**Steps:**
1. Load speech recording
2. FFT Size: 2048 (good frequency resolution)
3. Enable Spectrogram only
4. Disable Waveform/Spectrum

**What to look for:**
- **Formants:** Horizontal bright bands (F1, F2, F3)
- **Vowels:** Clear, stable formant patterns
- **Consonants:** Vertical bursts or gaps
- **Pitch:** Vertical striations (harmonics)

**Formant frequencies (approximate):**
- **F1:** 250-800 Hz (tongue height)
- **F2:** 800-2500 Hz (tongue position)
- **F3:** 2000-3500 Hz (lip rounding)

### Example 5: Exporting for Machine Learning

**Goal:** Create training data for audio classification

**Steps:**
1. Load training audio samples one by one
2. FFT Size: 2048
3. Enable Spectrogram
4. Click "Export Spectrogram (CSV)"
5. Save with descriptive name (e.g., `dog_bark_01.csv`)
6. Repeat for all samples

**Use exported data:**
- Import into Python/TensorFlow
- Use as features for classifier
- Create mel-spectrograms
- Train neural networks

## Performance Considerations

### Window Responsiveness

**Fast:**
- FFT Size 256-1024
- Waveform only
- Spectrum only
- Small AudioClips (< 10 seconds)

**Moderate:**
- FFT Size 2048-4096
- Spectrogram enabled
- Medium AudioClips (10-60 seconds)

**Slow:**
- FFT Size 8192
- Spectrogram on long clips (> 60 seconds)
- Very large AudioClips (> 5 minutes)

**Optimization tips:**
- Use smaller FFT for preview, larger for final analysis
- Disable spectrogram if not needed (most CPU-intensive)
- Work with shorter clips when possible
- Close other heavy Editor windows

### Memory Usage

**Waveform:** Low (single array)
**Spectrum:** Low (single FFT)
**Spectrogram:** High (frame_count × spectrum_size floats)

**Spectrogram memory estimate:**
- 10s audio at 44.1kHz, FFT 2048, 75% overlap
  - ~1700 frames × 1025 bins × 4 bytes ≈ 7 MB
- 60s audio: ~42 MB
- 5min audio: ~210 MB

**Limits:**
- Spectrogram capped at 500 frames for performance
- For longer audio, frames are skipped (lower time resolution)

## Troubleshooting

### "Select an AudioClip to visualize"

**Cause:** No AudioClip selected
**Solution:** Drag an AudioClip to the "Audio Clip" field

### Spectrum appears flat or empty

**Causes:**
1. Audio is silent (all zeros)
2. FFT size larger than audio length
3. dB scale with very low signal

**Solutions:**
1. Check waveform shows signal
2. Reduce FFT size
3. Try linear scale
4. Increase audio gain

### Spectrogram shows nothing

**Causes:**
1. Audio too short for FFT size
2. Signal too weak
3. Spectrogram not enabled

**Solutions:**
1. Reduce FFT size
2. Check "Show Spectrogram" is enabled
3. Verify waveform has signal

### Export button does nothing

**Cause:** No data computed yet

**Solution:**
- Load an AudioClip first
- Wait for spectrum/spectrogram to compute
- Check Unity Console for errors

### Filter overlay not showing

**Causes:**
1. No filter loaded
2. Filter overlay disabled
3. Spectrum not displayed

**Solutions:**
1. Click "Load from Designer"
2. Expand "Filter Overlay" foldout
3. Enable "Show Spectrum"

### Window freezes on large files

**Causes:**
1. FFT size too large (8192)
2. Spectrogram on very long clip
3. Insufficient RAM

**Solutions:**
1. Reduce FFT size to 2048 or lower
2. Disable spectrogram
3. Work with shorter clips
4. Close other applications

## Keyboard Shortcuts

*Future enhancement:*
- **Ctrl+E:** Export spectrum
- **Ctrl+W:** Export waveform
- **Ctrl+G:** Export spectrogram
- **Space:** Play/pause audio preview

## Integration with Other Tools

### Filter Designer

**Workflow:**
1. Design filter in Filter Designer window
2. Copy coefficients
3. Load in Signal Inspector for overlay
4. Verify filter response matches expectations

**Future:** Direct integration via shared preferences

### Unity Audio Mixer

**Workflow:**
1. Analyze raw audio in Signal Inspector
2. Design filter based on analysis
3. Apply filter using StreamingIIRFilter
4. Re-analyze to verify result
5. Fine-tune parameters

### External Tools

**MATLAB:**
```matlab
% Import exported spectrum
data = readtable('spectrum.csv');
plot(data.Frequency_Hz_, data.Magnitude_dB_);
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('Spectrum Analysis');
```

**Python:**
```python
import pandas as pd
import matplotlib.pyplot as plt

# Import spectrogram
data = pd.read_csv('spectrogram.csv')
# Plot using pcolormesh or imshow
```

## Advanced Tips

### Multi-Resolution Analysis

For comprehensive analysis:

1. **Wideband (FFT 512):** See transients, rhythm
2. **Medium (FFT 2048):** General purpose, musical content
3. **Narrowband (FFT 8192):** Fine pitch, harmonics

**Run all three**, compare results

### Comparative Analysis

1. Load reference audio (professional mix)
2. Export spectrum
3. Load your audio
4. Compare spectra in Excel/Python
5. Identify differences
6. Apply corrective filtering

### Batch Processing (via scripting)

```csharp
// Future: Automate analysis of multiple clips
foreach (var clip in audioClips)
{
    // Load clip
    // Export spectrum
    // Log results
}
```

## Known Limitations

1. **No real-time audio input:** Only AudioClips, not microphone
   - **Workaround:** Use AudioPipeline component for real-time

2. **Filter overlay is manual:** No direct link to Filter Designer
   - **Future:** Planned integration

3. **No zoom/pan:** Fixed view of entire signal
   - **Future:** Planned enhancement

4. **No playback controls:** Can't listen while viewing
   - **Workaround:** Use Unity's AudioSource preview

5. **Spectrogram limited to 500 frames:** Long audio is downsampled
   - **Reason:** Performance and memory constraints

## Related Documentation

- [Filter Designer Guide](FilterDesignerGuide.md)
- [Unity Integration Summary](../UNITY_INTEGRATION_SUMMARY.md)
- [DSP API Reference](../UNITY_INTEGRATION_SUMMARY.md#2-high-level-dsp-api-dspcs)

## Support

For issues or feature requests:
- GitHub: https://github.com/yogt1984/SpectraCore/issues
- Documentation: Check TASKS.md for planned features
