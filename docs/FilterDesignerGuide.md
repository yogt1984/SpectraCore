# Filter Designer Window - User Guide

The Filter Designer is an interactive Unity Editor tool for designing and visualizing digital filters in real-time. It provides immediate visual feedback through Bode plots and allows exporting filter coefficients for use in your Unity projects.

## Opening the Filter Designer

In Unity Editor, navigate to:
```
Window → Spectra → Filter Designer
```

## Features

### 1. Filter Parameter Panel

#### Filter Type Selection

Choose from four industry-standard IIR filter designs:

- **Butterworth**: Maximally flat passband, smooth rolloff
  - Best for: General-purpose filtering, minimal passband distortion
  - Characteristics: No ripple, moderate rolloff steepness

- **Chebyshev Type I**: Steeper rolloff than Butterworth
  - Best for: When sharp cutoff is needed
  - Characteristics: Passband ripple, monotonic stopband
  - Extra parameter: Passband Ripple (0.1 - 5.0 dB)

- **Chebyshev Type II**: Flat passband, ripple in stopband
  - Best for: When passband flatness is critical
  - Characteristics: Monotonic passband, stopband ripple
  - Extra parameter: Stopband Attenuation (20 - 100 dB)

- **Elliptic (Cauer)**: Steepest possible rolloff
  - Best for: Narrow transition bands, maximum selectivity
  - Characteristics: Ripple in both passband and stopband
  - Extra parameters: Passband Ripple and Stopband Attenuation

#### Response Type

- **Lowpass**: Passes low frequencies, attenuates high frequencies
- **Highpass**: Passes high frequencies, attenuates low frequencies
- **Bandpass**: Passes frequencies in a specific band (not yet supported)
- **Bandstop**: Rejects frequencies in a specific band (not yet supported)

#### Filter Order

Range: 1 to 10

Higher order = steeper rolloff but:
- More computational cost
- Potentially less numerically stable
- More phase distortion

**Recommended orders:**
- Order 2-4: Real-time audio, low latency requirements
- Order 4-6: Most general applications
- Order 6-8: When very sharp cutoff is needed
- Order 8-10: Offline processing only

#### Normalized Cutoff Frequency

Range: 0.01 to 0.99 (normalized)

The cutoff frequency is normalized where:
- 0.0 = DC (0 Hz)
- 0.5 = Nyquist frequency (half the sampling rate)
- 1.0 = Sampling frequency

**Examples at 44100 Hz sampling rate:**
- 0.1 = 4410 Hz
- 0.25 = 11025 Hz
- 0.5 = 22050 Hz (Nyquist)

**To calculate normalized frequency:**
```
normalizedFreq = desiredFreq / (sampleRate / 2)
```

### 2. Bode Plot Visualization

The Bode plot shows the filter's frequency response:

#### Magnitude Response (dB)

- **Y-axis**: Gain in decibels (dB)
  - 0 dB = unity gain (no change)
  - Positive dB = amplification
  - Negative dB = attenuation
  - -3 dB = cutoff point (half power)
  - -40 dB = 1% of original amplitude

- **X-axis**: Normalized frequency (0 to π)
  - 0 = DC
  - 0.5π = Quarter sampling rate
  - π = Nyquist frequency

- **Yellow line**: Your selected cutoff frequency

- **Green curve**: Magnitude response
  - Flat region = passband
  - Sloped region = transition band
  - Attenuated region = stopband

#### Phase Response (degrees)

- **Y-axis**: Phase shift in degrees (-180° to +180°)
  - 0° = no phase shift
  - Negative = phase lag
  - Positive = phase lead

- **Blue curve**: Phase response
  - Smooth phase = good transient response
  - Linear phase = no phase distortion (FIR only)
  - Non-linear phase = IIR characteristic

### 3. Display Options

- **Auto Update**: Automatically recompute when parameters change
- **Update Filter**: Manual update button (when Auto Update is off)
- **Magnitude checkbox**: Show/hide magnitude plot
- **Phase checkbox**: Show/hide phase plot

### 4. Exporting Coefficients

#### Copy Coefficients

Copies filter coefficients to clipboard in this format:
```csharp
// Filter Coefficients
// Type: Butterworth, Order: 4, Cutoff: 0.250

float[] b = new float[] { [ 0.000123, 0.000493, ... ] };
float[] a = new float[] { [ 1.000000, -3.180638, ... ] };
```

**Usage:**
1. Click "Copy Coefficients"
2. Paste into your code
3. Use with `DSP.LFilter()` or `StreamingIIRFilter`

#### Export to C#

Exports a complete C# class file:

```csharp
using Spectra;

/// <summary>
/// Butterworth Lowpass filter
/// Order: 4, Cutoff: 0.250
/// </summary>
public class ButterworthFilter
{
    public static readonly float[] B = new float[]
    {
        0.00012341f, 0.00049364f, ...
    };

    public static readonly float[] A = new float[]
    {
        1.00000000f, -3.18063855f, ...
    };
}
```

**Usage:**
1. Click "Export to C#"
2. Choose save location in your Assets folder
3. Reference the class in your code:
   ```csharp
   var filter = new StreamingIIRFilter(
       ButterworthFilter.B,
       ButterworthFilter.A);
   ```

### 5. Coefficient Panel

Click the "Filter Coefficients" foldout to view:
- **Numerator (b)**: Feed-forward coefficients
- **Denominator (a)**: Feedback coefficients

**Selectable text** - you can select and copy individual coefficients.

## Practical Examples

### Example 1: Audio Lowpass Filter (Remove High-Frequency Noise)

**Scenario**: Remove hiss from microphone input at 44100 Hz

**Settings:**
- Filter Type: Butterworth
- Response Type: Lowpass
- Order: 4
- Cutoff: 0.18 (= 4000 Hz at 44100 Hz SR)

**Why Butterworth?**
- Smooth, musical sound
- No ringing artifacts
- Minimal passband distortion

### Example 2: Subsonic Filter (Remove Rumble)

**Scenario**: Remove unwanted low-frequency rumble below 20 Hz

**Settings:**
- Filter Type: Butterworth
- Response Type: Highpass
- Order: 2
- Cutoff: 0.002 (= 20 Hz at 44100 Hz SR)

**Why order 2?**
- Gentle slope prevents artifacts
- Preserves bass response

### Example 3: Sharp Anti-Aliasing Filter

**Scenario**: Prepare signal for downsampling from 96kHz to 48kHz

**Settings:**
- Filter Type: Elliptic
- Response Type: Lowpass
- Order: 6
- Cutoff: 0.49 (just below new Nyquist)
- Passband Ripple: 0.1 dB
- Stopband Attenuation: 80 dB

**Why Elliptic?**
- Steepest rolloff
- Minimal transition band
- Strong stopband rejection

### Example 4: Telephone Bandpass Simulation

**Note**: Bandpass will be added in a future update. Current workaround:

1. Design lowpass at 0.15 (3400 Hz)
2. Design highpass at 0.007 (300 Hz)
3. Apply both filters in series

## Understanding the Frequency Response

### Reading the Magnitude Plot

**Passband characteristics:**
- Butterworth: Flat (0 dB)
- Cheby I: Ripples up to specified dB
- Cheby II: Flat (0 dB)
- Elliptic: Ripples up to specified dB

**Transition band:**
- Butterworth: Gradual (~20 dB/decade per order)
- Cheby I: Steeper than Butterworth
- Cheby II: Steeper than Butterworth
- Elliptic: Steepest (narrowest)

**Stopband:**
- Butterworth: Gradual attenuation
- Cheby I: Monotonic decay
- Cheby II: Ripples (controlled by Rs parameter)
- Elliptic: Ripples (controlled by Rs parameter)

### The -3 dB Point

The cutoff frequency is defined as the -3 dB point:
- At this frequency, the signal power is reduced by half
- Amplitude is reduced to 70.7% of original
- This is a standard definition across all filter types

**Yellow marker** on the plot shows your selected cutoff frequency.

### Phase Response Interpretation

**Linear phase** (ideal):
- Straight line on phase plot
- All frequencies delayed equally
- No waveform distortion
- Only achievable with FIR filters

**Non-linear phase** (IIR characteristic):
- Curved line on phase plot
- Different frequencies delayed differently
- Can cause transient distortion
- Trade-off for steep IIR rolloff

**Zero-phase filtering:**
- Use `DSP.FiltFilt()` for offline processing
- Forward-backward filtering
- Achieves zero phase delay
- Doubles the filter order effect

## Performance Considerations

### Real-time Audio (OnAudioFilterRead)

**Recommended:**
- Order 2-4
- Butterworth or Cheby I
- Use StreamingIIRFilter

```csharp
private StreamingIIRFilter filter;

void Start()
{
    var (b, a) = DSP.Butter(4, 0.25f);
    filter = new StreamingIIRFilter(b, a);
}

void OnAudioFilterRead(float[] data, int channels)
{
    // Process mono (left channel only)
    float[] output = new float[data.Length];
    filter.Process(data, output);
    Array.Copy(output, data, data.Length);
}
```

### Offline Processing

**Can use:**
- Order 6-10
- Any filter type
- Use `DSP.FiltFilt()` for zero phase

```csharp
AudioClip clip = ...;
float[] samples = new float[clip.samples * clip.channels];
clip.GetData(samples, 0);

var (b, a) = DSP.Ellip(8, 0.5f, 60f, 0.3f);
float[] filtered = DSP.FiltFilt(b, a, samples);

AudioClip result = AudioClip.Create("Filtered",
    clip.samples, clip.channels, clip.frequency, false);
result.SetData(filtered, 0);
```

## Troubleshooting

### "Filter design failed"

**Possible causes:**
1. Order too high (>10)
2. Cutoff frequency too close to 0 or 1
3. Invalid parameter combination

**Solutions:**
- Reduce filter order
- Use cutoff between 0.01 and 0.99
- For Elliptic: Ensure stopband > passband ripple

### Filter sounds "ringing" or "resonant"

**Causes:**
- Order too high
- Cutoff near Nyquist
- Chebyshev/Elliptic ripple too large

**Solutions:**
- Reduce order to 4 or lower
- Lower cutoff frequency
- Reduce ripple parameter
- Try Butterworth instead

### Numerical instability

**Symptoms:**
- NaN or Infinity in coefficients
- Extreme values in Bode plot
- Crackling/distortion in audio

**Solutions:**
- Reduce filter order
- Use Second-Order Sections (SOS/biquad cascade)
- Avoid extreme cutoff frequencies (< 0.01 or > 0.9)

## Advanced Topics

### Cascading Filters

For sharper rolloff, cascade multiple lower-order filters:

```csharp
var (b1, a1) = DSP.Butter(4, 0.25f);
var (b2, a2) = DSP.Butter(4, 0.25f);

var filter1 = new StreamingIIRFilter(b1, a1);
var filter2 = new StreamingIIRFilter(b2, a2);

// Apply in series
float[] temp = filter1.Process(input);
float[] output = filter2.Process(temp);
```

Result: Effective order 8 with better stability than single order-8 filter.

### Frequency Warping

The bilinear transform causes frequency warping:
```
actualFreq = (2/π) * arctan(π * normalizedFreq)
```

This is pre-compensated in the filter design, but be aware when analyzing results.

### Group Delay

Future feature: Group delay plot will show:
- Constant group delay = linear phase
- Varying group delay = phase distortion
- Peak at cutoff = transient ringing

## Keyboard Shortcuts

- **Ctrl+C** (when coefficients selected): Copy to clipboard
- **Ctrl+E**: Export to C# (future feature)

## Related Documentation

- [DSP API Reference](../UNITY_INTEGRATION_SUMMARY.md#2-high-level-dsp-api-dspcs)
- [Filter Design Theory](../docs/matlab-equivalence.md)
- [ABI Design](../docs/abi-design.md)

## Support

For issues or feature requests, please visit:
https://github.com/yogt1984/SpectraCore/issues
