# SpectraCore Unity API Reference

Complete API documentation for the SpectraCore Unity package.

## Table of Contents

1. [DSP Class](#dsp-class) - Static DSP functions
   - [Filter Design](#filter-design)
   - [Filter Application](#filter-application)
   - [Signal Analysis](#signal-analysis)
   - [Frequency Response](#frequency-response)
   - [Utility Functions](#utility-functions)
2. [FFTAnalyzer Class](#fftanalyzer-class) - FFT computation
3. [STFTAnalyzer Class](#stftanalyzer-class) - Short-Time Fourier Transform
4. [OnsetDetector Class](#onsetdetector-class) - Real-time onset detection
5. [StreamingIIRFilter Class](#streamingiirfilter-class) - Real-time filtering
6. [AudioPipeline Component](#audiopipeline-component) - Audio input
7. [Enumerations](#enumerations)
8. [Exceptions](#exceptions)

---

## DSP Class

**Namespace:** `Spectra`

Static class providing MATLAB-equivalent DSP functions. All functions are thread-safe and throw `SpectraException` on error.

### Filter Design

#### Butter

Design a Butterworth filter.

```csharp
public static (float[] b, float[] a) Butter(
    int order,
    float normalizedFreq,
    FilterType type = FilterType.Lowpass)
```

**Parameters:**
- `order` - Filter order (typical: 2-8)
- `normalizedFreq` - Cutoff frequency normalized to Nyquist (0.0 to 1.0)
  - 0.5 = quarter of sample rate
  - 1.0 = Nyquist frequency (half sample rate)
- `type` - Filter type (Lowpass, Highpass, Bandpass, Bandstop)

**Returns:**
- Tuple of (`b`, `a`) filter coefficients
  - `b` - Numerator coefficients (feedforward)
  - `a` - Denominator coefficients (feedback)

**MATLAB Equivalent:**
```matlab
[b, a] = butter(order, Wn, type)
```

**Example:**
```csharp
// Design 4th-order lowpass at 1000 Hz (fs = 44100 Hz)
float cutoff = 1000f / (44100f / 2f); // Normalize to Nyquist
var (b, a) = DSP.Butter(4, cutoff, FilterType.Lowpass);

// Apply to signal
float[] filtered = DSP.LFilter(b, a, audioData);
```

**Frequency Calculation:**
```csharp
// Given cutoff in Hz and sample rate
float normalizedFreq = cutoffHz / (sampleRate / 2f);

// Examples at 44100 Hz:
float f100Hz = 100f / 22050f;   // 0.0045
float f1kHz = 1000f / 22050f;   // 0.0454
float f10kHz = 10000f / 22050f; // 0.4535
```

**Performance:**
- O(order²) complexity
- Typical execution: < 1ms for order ≤ 8
- Thread-safe

**Notes:**
- Butterworth filters have maximally flat passband
- No ripple in passband or stopband
- Moderate rolloff steepness
- Good phase response
- Recommended for audio applications

---

#### Cheby1

Design a Chebyshev Type I filter (passband ripple).

```csharp
public static (float[] b, float[] a) Cheby1(
    int order,
    float rippleDb,
    float normalizedFreq,
    FilterType type = FilterType.Lowpass)
```

**Parameters:**
- `order` - Filter order (typical: 2-8)
- `rippleDb` - Passband ripple in dB (typical: 0.1 to 3 dB)
  - Lower ripple = closer to Butterworth
  - Higher ripple = steeper rolloff
- `normalizedFreq` - Cutoff frequency (0.0 to 1.0, Nyquist-normalized)
- `type` - Filter type

**Returns:**
- Tuple of (`b`, `a`) filter coefficients

**MATLAB Equivalent:**
```matlab
[b, a] = cheby1(order, Rp, Wn, type)
```

**Example:**
```csharp
// Design 6th-order Chebyshev I lowpass with 0.5 dB ripple
var (b, a) = DSP.Cheby1(6, 0.5f, 0.25f, FilterType.Lowpass);

// Plot frequency response
var (mag, phase, freqs) = DSP.FreqzDb(b, a, 512);
```

**Ripple Guide:**
- **0.1 dB**: Very low ripple, similar to Butterworth
- **0.5 dB**: Balanced (common choice)
- **1.0 dB**: Noticeable ripple, steeper rolloff
- **3.0 dB**: High ripple, very steep rolloff

**Performance:**
- O(order²) complexity
- Similar to Butterworth

**Notes:**
- Steeper rolloff than Butterworth for same order
- Ripple in passband, monotonic stopband
- Worse phase response than Butterworth
- Use when sharp cutoff is needed

---

#### Cheby2

Design a Chebyshev Type II filter (stopband ripple).

```csharp
public static (float[] b, float[] a) Cheby2(
    int order,
    float stopbandDb,
    float normalizedFreq,
    FilterType type = FilterType.Lowpass)
```

**Parameters:**
- `order` - Filter order
- `stopbandDb` - Stopband attenuation in dB (typical: 20 to 80 dB)
  - 20 dB = 10% stopband level
  - 40 dB = 1% stopband level
  - 60 dB = 0.1% stopband level
- `normalizedFreq` - Cutoff frequency (Nyquist-normalized)
- `type` - Filter type

**Returns:**
- Tuple of (`b`, `a`) filter coefficients

**MATLAB Equivalent:**
```matlab
[b, a] = cheby2(order, Rs, Wn, type)
```

**Example:**
```csharp
// Design highpass with 40 dB stopband attenuation
var (b, a) = DSP.Cheby2(5, 40f, 0.1f, FilterType.Highpass);

// Apply with zero-phase filtering
float[] output = DSP.FiltFilt(b, a, input);
```

**Stopband Attenuation Guide:**
- **20 dB**: Light attenuation
- **40 dB**: Good for most audio (1% of original)
- **60 dB**: High quality (0.1% of original)
- **80 dB**: Extreme attenuation (0.01% of original)

**Notes:**
- Monotonic passband, ripple in stopband
- Flatter passband than Chebyshev I
- Better phase response than Chebyshev I
- Use when flat passband is important

---

#### Ellip

Design an Elliptic (Cauer) filter.

```csharp
public static (float[] b, float[] a) Ellip(
    int order,
    float passbandRippleDb,
    float stopbandDb,
    float normalizedFreq,
    FilterType type = FilterType.Lowpass)
```

**Parameters:**
- `order` - Filter order
- `passbandRippleDb` - Passband ripple in dB (typical: 0.1 to 1 dB)
- `stopbandDb` - Stopband attenuation in dB (typical: 40 to 80 dB)
- `normalizedFreq` - Cutoff frequency (Nyquist-normalized)
- `type` - Filter type

**Returns:**
- Tuple of (`b`, `a`) filter coefficients

**MATLAB Equivalent:**
```matlab
[b, a] = ellip(order, Rp, Rs, Wn, type)
```

**Example:**
```csharp
// Design elliptic filter: 0.5 dB passband, 60 dB stopband
var (b, a) = DSP.Ellip(4, 0.5f, 60f, 0.3f, FilterType.Lowpass);

// Compare with Butterworth
var (bButter, aButter) = DSP.Butter(4, 0.3f, FilterType.Lowpass);

// Elliptic has steeper rolloff for same order
```

**Performance:**
- O(order²) complexity
- Slightly more expensive than Butterworth/Chebyshev

**Notes:**
- **Steepest rolloff** for given order
- Ripple in both passband and stopband
- Most complex design, worst phase response
- Use when minimum order/steepest rolloff is critical
- Often achieves desired response with lower order

**Filter Comparison:**

| Filter Type | Passband | Stopband | Rolloff | Phase | Use Case |
|-------------|----------|----------|---------|-------|----------|
| Butterworth | Flat | Monotonic | Moderate | Best | General audio |
| Chebyshev I | Ripple | Monotonic | Steep | Good | Sharp cutoffs |
| Chebyshev II | Flat | Ripple | Steep | Better | Flat passband needed |
| Elliptic | Ripple | Ripple | Steepest | Worst | Minimum order |

---

### Filter Application

#### LFilter

Apply a digital filter (IIR or FIR) to a signal.

```csharp
public static float[] LFilter(float[] b, float[] a, float[] input)
```

**Parameters:**
- `b` - Numerator coefficients (feedforward)
- `a` - Denominator coefficients (feedback)
- `input` - Input signal

**Returns:**
- Filtered output signal (same length as input)

**MATLAB Equivalent:**
```matlab
y = filter(b, a, x)
```

**Example:**
```csharp
// Design and apply lowpass filter
var (b, a) = DSP.Butter(4, 0.25f);
float[] filtered = DSP.LFilter(b, a, audioData);

// FIR filter (a = [1])
float[] firCoeffs = DSP.Window(64, WindowType.Hann);
float[] output = DSP.LFilter(firCoeffs, new float[] { 1f }, input);
```

**Performance:**
- O(N × (B + A)) where N = input length, B = b.Length, A = a.Length
- Real-time capable for typical filter orders
- For 1024 samples, order 8 filter: ~0.1 ms

**Notes:**
- Direct Form II Transposed implementation
- Maintains filter state (not stateless)
- For multiple blocks, use `StreamingIIRFilter` class
- Phase delay = (order / 2) samples for linear-phase FIR

---

#### FiltFilt

Zero-phase digital filtering (forward-backward filtering).

```csharp
public static float[] FiltFilt(float[] b, float[] a, float[] input)
```

**Parameters:**
- `b` - Numerator coefficients
- `a` - Denominator coefficients
- `input` - Input signal

**Returns:**
- Filtered output with zero phase delay

**MATLAB Equivalent:**
```matlab
y = filtfilt(b, a, x)
```

**Example:**
```csharp
// Zero-phase lowpass filtering
var (b, a) = DSP.Butter(6, 0.2f);
float[] output = DSP.FiltFilt(b, a, signal);

// Preserves peak positions (no delay)
int peakIdx = Array.IndexOf(signal, signal.Max());
int outPeakIdx = Array.IndexOf(output, output.Max());
Debug.Assert(peakIdx == outPeakIdx); // Same position
```

**Performance:**
- 2× slower than `LFilter` (filters twice)
- O(2N × (B + A))

**Notes:**
- **Zero phase delay** - preserves timing
- Effectively doubles filter order (applies twice)
- Requires full signal (not real-time)
- Use for offline analysis when timing is critical
- Common for:
  - Feature extraction (onset, pitch)
  - Scientific measurements
  - Audio editing (when delay matters)

---

### Signal Analysis

#### Xcorr (Cross-Correlation)

Compute cross-correlation between two signals.

```csharp
public static float[] Xcorr(float[] x, float[] y)
```

**Parameters:**
- `x` - First signal
- `y` - Second signal

**Returns:**
- Cross-correlation result
- Length: `x.Length + y.Length - 1`

**MATLAB Equivalent:**
```matlab
r = xcorr(x, y)
```

**Example:**
```csharp
// Find time delay between signals
float[] r = DSP.Xcorr(reference, delayed);

// Peak index gives delay in samples
int maxIdx = Array.IndexOf(r, r.Max());
int centerIdx = (r.Length - 1) / 2;
int delay = maxIdx - centerIdx;

Debug.Log($"Delay: {delay} samples");
```

**Autocorrelation:**
```csharp
// Autocorrelation (single argument)
float[] autocorr = DSP.Xcorr(signal);

// Pitch detection using autocorrelation
int minLag = sampleRate / 500;  // 500 Hz max
int maxLag = sampleRate / 50;   // 50 Hz min
float maxCorr = 0f;
int pitchLag = 0;

for (int lag = minLag; lag < maxLag; lag++)
{
    if (autocorr[centerIdx + lag] > maxCorr)
    {
        maxCorr = autocorr[centerIdx + lag];
        pitchLag = lag;
    }
}

float pitchHz = sampleRate / (float)pitchLag;
```

**Performance:**
- O(N × M) for lengths N and M
- For 1024-sample signals: ~1 ms
- FFT-based implementation (efficient)

**Use Cases:**
- Time delay estimation
- Pitch detection
- Pattern matching
- Similarity measurement
- Echo detection

---

#### Hilbert

Compute Hilbert transform (analytic signal).

```csharp
public static (float[] real, float[] imag) Hilbert(float[] input)
```

**Parameters:**
- `input` - Real-valued input signal

**Returns:**
- Tuple of (`real`, `imag`) components of analytic signal
  - `real` - Original signal
  - `imag` - Hilbert transform (quadrature)

**MATLAB Equivalent:**
```matlab
h = hilbert(x)  % Returns complex array
```

**Example:**
```csharp
var (real, imag) = DSP.Hilbert(signal);

// Compute instantaneous amplitude (envelope)
float[] envelope = new float[signal.Length];
for (int i = 0; i < signal.Length; i++)
{
    envelope[i] = Mathf.Sqrt(real[i] * real[i] + imag[i] * imag[i]);
}

// Compute instantaneous phase
float[] phase = new float[signal.Length];
for (int i = 0; i < signal.Length; i++)
{
    phase[i] = Mathf.Atan2(imag[i], real[i]);
}
```

**Use Cases:**
- Envelope extraction (amplitude modulation)
- Instantaneous frequency
- Single-sideband modulation
- Phase analysis

---

#### Envelope

Compute instantaneous amplitude (envelope) using Hilbert transform.

```csharp
public static float[] Envelope(float[] input)
```

**Parameters:**
- `input` - Input signal

**Returns:**
- Envelope (instantaneous amplitude)

**MATLAB Equivalent:**
```matlab
env = abs(hilbert(x))
```

**Example:**
```csharp
// Extract envelope from modulated signal
float[] env = DSP.Envelope(modulatedSignal);

// Detect amplitude variations
float[] deriv = new float[env.Length - 1];
for (int i = 0; i < deriv.Length; i++)
{
    deriv[i] = env[i + 1] - env[i];
}

// Find amplitude peaks
for (int i = 1; i < deriv.Length - 1; i++)
{
    if (deriv[i - 1] > 0 && deriv[i] < 0)
    {
        Debug.Log($"Amplitude peak at sample {i}");
    }
}
```

**Performance:**
- Same as `Hilbert` (wrapper function)
- O(N log N) using FFT

**Use Cases:**
- Tremolo detection
- Beat tracking
- Amplitude modulation analysis
- Transient detection

---

#### Pwelch

Compute Power Spectral Density using Welch's method.

```csharp
public static (float[] psd, float[] frequencies) Pwelch(
    float[] input,
    int segmentSize,
    int overlap,
    WindowType window,
    float sampleRate,
    bool onesided = true)
```

**Parameters:**
- `input` - Input signal
- `segmentSize` - FFT segment size (power of 2)
- `overlap` - Overlapping samples between segments
- `window` - Window function type
- `sampleRate` - Sample rate in Hz
- `onesided` - Return one-sided spectrum (for real signals)

**Returns:**
- Tuple of (`psd`, `frequencies`)
  - `psd` - Power spectral density values
  - `frequencies` - Frequency values in Hz

**MATLAB Equivalent:**
```matlab
[psd, f] = pwelch(x, window, overlap, nfft, fs)
```

**Example:**
```csharp
// Compute PSD with 50% overlap
var (psd, freqs) = DSP.Pwelch(
    audioData,
    segmentSize: 2048,
    overlap: 1024,
    WindowType.Hann,
    sampleRate: 44100f,
    onesided: true
);

// Find dominant frequency
int maxIdx = Array.IndexOf(psd, psd.Max());
float dominantFreq = freqs[maxIdx];
Debug.Log($"Dominant frequency: {dominantFreq:F1} Hz");

// Plot in dB
float[] psdDb = new float[psd.Length];
for (int i = 0; i < psd.Length; i++)
{
    psdDb[i] = 10f * Mathf.Log10(Mathf.Max(psd[i], 1e-10f));
}
```

**Overlap Guide:**
- **0% (overlap = 0)**: No overlap, faster
- **50% (overlap = segmentSize/2)**: Standard (recommended)
- **75% (overlap = 3*segmentSize/4)**: Smoother, slower

**Performance:**
- Depends on signal length and overlap
- More overlap = smoother result, higher cost
- For 10s audio at 44.1kHz, 50% overlap: ~50 ms

**Use Cases:**
- Noise floor estimation
- Stationary signal analysis
- Frequency content measurement
- Audio spectrum analyzer

---

#### Resample

Resample signal by rational factor p/q.

```csharp
public static float[] Resample(float[] input, int p, int q)
```

**Parameters:**
- `input` - Input signal
- `p` - Upsampling factor
- `q` - Downsampling factor
- New sample rate = `originalRate × (p / q)`

**Returns:**
- Resampled signal

**MATLAB Equivalent:**
```matlab
y = resample(x, p, q)
```

**Example:**
```csharp
// Convert 44100 Hz to 48000 Hz
// 48000/44100 = 160/147 (simplified)
float[] resampled = DSP.Resample(audio44k, 160, 147);

// Downsample by 2 (e.g., 44100 → 22050 Hz)
float[] downsampled = DSP.Resample(audioData, 1, 2);

// Upsample by 2 (e.g., 22050 → 44100 Hz)
float[] upsampled = DSP.Resample(audioData, 2, 1);
```

**Common Conversions:**

| From (Hz) | To (Hz) | p | q | Ratio |
|-----------|---------|---|---|-------|
| 44100 | 48000 | 160 | 147 | 1.088 |
| 48000 | 44100 | 147 | 160 | 0.919 |
| 44100 | 22050 | 1 | 2 | 0.5 |
| 22050 | 44100 | 2 | 1 | 2.0 |
| 8000 | 44100 | 441 | 80 | 5.513 |

**Performance:**
- O(N × max(p, q))
- Uses polyphase filtering (high quality)
- Output length: `floor(input.Length × p / q)`

**Notes:**
- High-quality polyphase FIR resampling
- Anti-aliasing filter included
- Use simplified p/q for efficiency
- For arbitrary rates, use GCD to simplify

---

### Frequency Response

#### Freqz

Compute frequency response of a digital filter.

```csharp
public static (float[] magnitude, float[] phase, float[] frequencies) Freqz(
    float[] b,
    float[] a,
    int numPoints = 512)
```

**Parameters:**
- `b` - Numerator coefficients
- `a` - Denominator coefficients
- `numPoints` - Number of frequency points (default: 512)

**Returns:**
- Tuple of (`magnitude`, `phase`, `frequencies`)
  - `magnitude` - Magnitude response (linear scale)
  - `phase` - Phase response in radians
  - `frequencies` - Normalized frequencies (0 to π)

**MATLAB Equivalent:**
```matlab
[H, W] = freqz(b, a, N)
```

**Example:**
```csharp
// Design filter and compute response
var (b, a) = DSP.Butter(4, 0.25f);
var (mag, phase, freqs) = DSP.Freqz(b, a, 1024);

// Convert to Hz (given sample rate)
float sampleRate = 44100f;
float[] freqHz = new float[freqs.Length];
for (int i = 0; i < freqs.Length; i++)
{
    freqHz[i] = freqs[i] / Mathf.PI * sampleRate / 2f;
}

// Find -3 dB point
float[] magDb = new float[mag.Length];
for (int i = 0; i < mag.Length; i++)
{
    magDb[i] = 20f * Mathf.Log10(mag[i]);
}

int cutoffIdx = Array.FindIndex(magDb, m => m < -3f);
float cutoffHz = freqHz[cutoffIdx];
Debug.Log($"Actual -3 dB cutoff: {cutoffHz:F1} Hz");
```

**Normalized Frequency:**
- `0` = DC (0 Hz)
- `π/2` = Quarter of sample rate
- `π` = Nyquist (half sample rate)

**Conversion:**
```csharp
// Normalized (0 to π) → Hz
float Hz = normalizedFreq / Mathf.PI * (sampleRate / 2f);

// Hz → Normalized
float normalized = Hz / (sampleRate / 2f) * Mathf.PI;
```

---

#### FreqzDb

Compute frequency response magnitude in dB.

```csharp
public static (float[] magnitudeDb, float[] phase, float[] frequencies) FreqzDb(
    float[] b,
    float[] a,
    int numPoints = 512)
```

**Parameters:**
- Same as `Freqz`

**Returns:**
- Tuple of (`magnitudeDb`, `phase`, `frequencies`)
  - `magnitudeDb` - Magnitude in dB (20×log₁₀)
  - `phase` - Phase in radians
  - `frequencies` - Normalized frequencies (0 to π)

**Example:**
```csharp
// Bode plot data
var (magDb, phase, freqs) = DSP.FreqzDb(b, a, 512);

// Plot magnitude
for (int i = 0; i < magDb.Length; i++)
{
    Debug.DrawLine(
        new Vector2(freqs[i], magDb[i]),
        new Vector2(freqs[i + 1], magDb[i + 1]),
        Color.blue
    );
}

// Plot phase
for (int i = 0; i < phase.Length; i++)
{
    float phaseDeg = phase[i] * Mathf.Rad2Deg;
    // Plot phase curve
}
```

**dB Scale Reference:**
- **0 dB**: Unity gain (magnitude = 1)
- **-3 dB**: 70.7% amplitude (half power)
- **-6 dB**: 50% amplitude
- **-20 dB**: 10% amplitude
- **-40 dB**: 1% amplitude
- **-60 dB**: 0.1% amplitude

---

### Utility Functions

#### Window

Generate a window function.

```csharp
public static float[] Window(int size, WindowType type)
```

**Parameters:**
- `size` - Window length in samples
- `type` - Window type

**Returns:**
- Window coefficients

**Example:**
```csharp
// Generate Hann window
float[] window = DSP.Window(1024, WindowType.Hann);

// Apply to signal
for (int i = 0; i < signal.Length; i++)
{
    signal[i] *= window[i];
}
```

**Window Types:**

| Type | Mainlobe Width | Sidelobe Level | Use Case |
|------|----------------|----------------|----------|
| Rectangular | Narrowest | Highest (-13 dB) | Max resolution |
| Hann | Medium | Medium (-32 dB) | General purpose |
| Hamming | Medium | Low (-43 dB) | Speech analysis |
| Blackman | Wide | Very low (-58 dB) | Low leakage |
| BlackmanHarris | Widest | Lowest (-92 dB) | Scientific |
| FlatTop | Very wide | Low (-70 dB) | Amplitude accuracy |

**Recommendations:**
- **Hann**: Default choice for most applications
- **Hamming**: Speech and voice analysis
- **Blackman**: When leakage must be minimized
- **FlatTop**: When amplitude accuracy is critical

---

#### ApplyWindow

Apply a window function to data in-place.

```csharp
public static void ApplyWindow(float[] data, WindowType type)
```

**Parameters:**
- `data` - Signal to window (modified in-place)
- `type` - Window type

**Example:**
```csharp
// Apply Hann window before FFT
float[] frame = new float[2048];
Array.Copy(audioData, offset, frame, 0, 2048);
DSP.ApplyWindow(frame, WindowType.Hann);

// Compute FFT
var fft = new FFTAnalyzer(2048);
float[] spectrum = fft.GetMagnitudeSpectrum(frame);
fft.Dispose();
```

**Notes:**
- Modifies input array in-place
- More efficient than `Window()` when window is applied once
- Use `Window()` to reuse the same window multiple times

---

## FFTAnalyzer Class

**Namespace:** `Spectra`

Efficient FFT analyzer for spectral analysis. Implements IDisposable.

### Constructor

```csharp
public FFTAnalyzer(int fftSize)
```

**Parameters:**
- `fftSize` - FFT size (must be power of 2: 64, 128, 256, ..., 32768)

**Example:**
```csharp
using (var fft = new FFTAnalyzer(2048))
{
    float[] spectrum = fft.GetMagnitudeSpectrum(audioFrame);
    // Use spectrum
} // Automatically disposed
```

**Throws:**
- `SpectraException` if FFT creation fails

---

### Properties

#### Size

FFT size.

```csharp
public int Size { get; }
```

**Example:**
```csharp
var fft = new FFTAnalyzer(2048);
Debug.Log($"FFT Size: {fft.Size}"); // 2048
```

---

#### SpectrumSize

Output spectrum size (N/2 + 1 for real signals).

```csharp
public int SpectrumSize { get; }
```

**Example:**
```csharp
var fft = new FFTAnalyzer(2048);
Debug.Log($"Spectrum bins: {fft.SpectrumSize}"); // 1025
```

**Note:** One-sided spectrum for real input (0 Hz to Nyquist).

---

### Methods

#### GetMagnitudeSpectrum

Compute magnitude spectrum.

```csharp
public float[] GetMagnitudeSpectrum(float[] input)
```

**Parameters:**
- `input` - Time-domain signal (length must equal `Size`)

**Returns:**
- Magnitude spectrum (length = `SpectrumSize`)

**Example:**
```csharp
using (var fft = new FFTAnalyzer(1024))
{
    float[] spectrum = fft.GetMagnitudeSpectrum(audioFrame);

    // Convert to dB
    for (int i = 0; i < spectrum.Length; i++)
    {
        spectrum[i] = 20f * Mathf.Log10(Mathf.Max(spectrum[i], 1e-10f));
    }

    // Find peak frequency
    int peakBin = Array.IndexOf(spectrum, spectrum.Max());
    float peakFreq = peakBin * (sampleRate / 2f) / spectrum.Length;
}
```

**Performance:**
- O(N log N) where N = FFT size
- For 2048-point FFT: ~0.1 ms

---

#### GetPowerSpectrum

Compute power spectrum (magnitude squared).

```csharp
public float[] GetPowerSpectrum(float[] input)
```

**Parameters:**
- `input` - Time-domain signal

**Returns:**
- Power spectrum

**Example:**
```csharp
float[] power = fft.GetPowerSpectrum(audioFrame);

// Power is proportional to energy
float totalPower = power.Sum();
Debug.Log($"Total signal power: {totalPower}");
```

**Note:** Power = Magnitude²

---

#### Forward

Compute forward FFT (complex output).

```csharp
public (float[] real, float[] imag) Forward(float[] input)
```

**Parameters:**
- `input` - Time-domain signal

**Returns:**
- Tuple of (`real`, `imag`) complex spectrum

**Example:**
```csharp
var (real, imag) = fft.Forward(signal);

// Compute phase
float[] phase = new float[real.Length];
for (int i = 0; i < real.Length; i++)
{
    phase[i] = Mathf.Atan2(imag[i], real[i]);
}
```

---

#### Inverse

Compute inverse FFT.

```csharp
public void Inverse(float[] inputReal, float[] inputImag, float[] output)
```

**Parameters:**
- `inputReal` - Real part of spectrum
- `inputImag` - Imaginary part of spectrum
- `output` - Time-domain output

**Example:**
```csharp
// Modify spectrum then convert back to time domain
var (real, imag) = fft.Forward(signal);

// Zero out high frequencies
for (int i = 512; i < real.Length; i++)
{
    real[i] = 0f;
    imag[i] = 0f;
}

float[] filtered = new float[fft.Size];
fft.Inverse(real, imag, filtered);
```

---

### Usage Pattern

```csharp
// Typical usage with automatic disposal
using (var fft = new FFTAnalyzer(2048))
{
    // Apply window
    DSP.ApplyWindow(audioFrame, WindowType.Hann);

    // Compute spectrum
    float[] spectrum = fft.GetMagnitudeSpectrum(audioFrame);

    // Analyze
    // ...
} // FFT resources released here
```

**Resource Management:**
- Always use `using` statement or call `Dispose()`
- FFTAnalyzer holds native resources
- Not disposing causes memory leaks

---

## STFTAnalyzer Class

**Namespace:** `Spectra`

Short-Time Fourier Transform for time-frequency analysis. Implements IDisposable.

### Constructor

```csharp
public STFTAnalyzer(
    int fftSize,
    int hopSize,
    WindowType window = WindowType.Hann,
    bool center = true)
```

**Parameters:**
- `fftSize` - FFT size (power of 2)
- `hopSize` - Hop size in samples (distance between frames)
  - Smaller hop = more overlap, smoother result
  - Larger hop = less overlap, faster
- `window` - Window function type
- `center` - Center the windows (adds padding)

**Example:**
```csharp
// 75% overlap (common for spectrograms)
var stft = new STFTAnalyzer(
    fftSize: 2048,
    hopSize: 512,      // 2048/4 = 75% overlap
    window: WindowType.Hann
);
```

**Overlap Guide:**
- **0% (hop = fft)**: No overlap
- **50% (hop = fft/2)**: Standard
- **75% (hop = fft/4)**: Smoother (recommended for visualization)
- **87.5% (hop = fft/8)**: Very smooth, expensive

---

### Properties

```csharp
public int FftSize { get; }
public int HopSize { get; }
public int SpectrumSize { get; }
```

---

### Methods

#### Analyze

Analyze entire signal (batch mode).

```csharp
public (float[] real, float[] imag, int numFrames) Analyze(float[] input)
```

**Parameters:**
- `input` - Audio signal

**Returns:**
- Tuple of (`real`, `imag`, `numFrames`)
  - `real` - Real components (all frames concatenated)
  - `imag` - Imaginary components
  - `numFrames` - Number of frames computed

**Example:**
```csharp
using (var stft = new STFTAnalyzer(2048, 512, WindowType.Hann))
{
    var (real, imag, numFrames) = stft.Analyze(audioData);

    // Create spectrogram
    int binsPerFrame = stft.SpectrumSize;
    float[][] spectrogram = new float[numFrames][];

    for (int frame = 0; frame < numFrames; frame++)
    {
        spectrogram[frame] = new float[binsPerFrame];

        for (int bin = 0; bin < binsPerFrame; bin++)
        {
            int idx = frame * binsPerFrame + bin;
            float r = real[idx];
            float im = imag[idx];
            float magnitude = Mathf.Sqrt(r * r + im * im);
            spectrogram[frame][bin] = 20f * Mathf.Log10(Mathf.Max(magnitude, 1e-10f));
        }
    }

    // spectrogram[frame][bin] is now a 2D time-frequency representation
}
```

**Array Layout:**
- `real` and `imag` are 1D arrays
- Frame `f`, bin `b`: index = `f * SpectrumSize + b`

---

#### PushSamples (Streaming)

Push samples for real-time analysis.

```csharp
public void PushSamples(float[] samples)
```

**Parameters:**
- `samples` - Audio samples

**Example:**
```csharp
var stft = new STFTAnalyzer(2048, 512);

void OnAudioFilterRead(float[] data, int channels)
{
    // Convert to mono
    float[] mono = ConvertToMono(data, channels);

    // Push to STFT
    stft.PushSamples(mono);

    // Pop available frames
    while (stft.FramesAvailable > 0)
    {
        if (stft.PopFrame(out float[] real, out float[] imag))
        {
            ProcessFrame(real, imag);
        }
    }
}
```

---

#### PopFrame (Streaming)

Pop a frame from the streaming buffer.

```csharp
public bool PopFrame(out float[] real, out float[] imag)
```

**Parameters:**
- `real` - Output real components
- `imag` - Output imaginary components

**Returns:**
- `true` if a frame was available

**Example:**
```csharp
if (stft.PopFrame(out float[] real, out float[] imag))
{
    float[] magnitude = STFTAnalyzer.ComputeMagnitude(real, imag);
    // Process magnitude spectrum
}
```

---

#### FramesAvailable

Number of frames ready in streaming buffer.

```csharp
public int FramesAvailable { get; }
```

---

### Static Helper Methods

#### ComputeMagnitude

```csharp
public static float[] ComputeMagnitude(float[] real, float[] imag)
```

Compute magnitude from real/imaginary components.

---

#### ComputePower

```csharp
public static float[] ComputePower(float[] real, float[] imag)
```

Compute power (magnitude squared).

---

#### PowerToDb

```csharp
public static float[] PowerToDb(float[] power, float referenceLevel = 1.0f)
```

Convert power to decibels.

**Example:**
```csharp
float[] power = STFTAnalyzer.ComputePower(real, imag);
float[] dB = STFTAnalyzer.PowerToDb(power);
```

---

## OnsetDetector Class

**Namespace:** `Spectra`

Real-time onset detection using spectral flux. Implements IDisposable.

### Constructor

```csharp
public OnsetDetector(OnsetConfig config, float sampleRate)
```

**Parameters:**
- `config` - Configuration object
- `sampleRate` - Sample rate in Hz

**Example:**
```csharp
var config = new OnsetConfig
{
    FftSize = 2048,
    HopSize = 512,
    Threshold = 0.5f,      // Sensitivity (0.1 = sensitive, 1.0 = conservative)
    MinIntervalMs = 50f    // Minimum time between onsets
};

var detector = new OnsetDetector(config, 44100f);
detector.OnOnsetDetected += (timestamp, strength) =>
{
    Debug.Log($"Onset at {timestamp:F3}s, strength {strength:F2}");
};
```

---

### OnsetConfig Class

```csharp
public class OnsetConfig
{
    public int FftSize { get; set; } = 2048;
    public int HopSize { get; set; } = 512;
    public float Threshold { get; set; } = 0.5f;
    public float MinIntervalMs { get; set; } = 50f;
}
```

**Parameters:**
- `FftSize` - FFT size for analysis
- `HopSize` - Hop size
- `Threshold` - Detection threshold (0.1 to 2.0)
  - Lower = more sensitive, more false positives
  - Higher = less sensitive, may miss weak onsets
- `MinIntervalMs` - Minimum time between detections (prevents duplicates)

**Threshold Guide:**
- **0.1 - 0.3**: Very sensitive (drum rolls, fast passages)
- **0.4 - 0.6**: Balanced (default: 0.5)
- **0.7 - 1.0**: Conservative (strong transients only)
- **> 1.0**: Very conservative

---

### Event

#### OnOnsetDetected

```csharp
public event Action<float, float> OnOnsetDetected;
```

**Parameters:**
- `timestamp` - Time in seconds
- `strength` - Onset strength (0.0 to 1.0+)

**Example:**
```csharp
detector.OnOnsetDetected += OnOnset;

void OnOnset(float time, float strength)
{
    if (strength > 0.8f)
    {
        // Strong onset (e.g., kick drum)
        PlayClick();
    }
}
```

---

### Methods

#### Process

Process audio samples.

```csharp
public void Process(float[] samples)
```

**Example:**
```csharp
void OnAudioFilterRead(float[] data, int channels)
{
    float[] mono = ConvertToMono(data, channels);
    detector.Process(mono);
    // Onsets will trigger OnOnsetDetected event
}
```

---

#### Reset

Reset detector state.

```csharp
public void Reset()
```

**Example:**
```csharp
// Reset when starting new audio file
detector.Reset();
```

---

### Complete Example

```csharp
using UnityEngine;
using Spectra;

public class OnsetDetectionExample : MonoBehaviour
{
    private OnsetDetector detector;
    private AudioSource audioSource;

    void Start()
    {
        var config = new OnsetConfig
        {
            FftSize = 2048,
            HopSize = 512,
            Threshold = 0.5f,
            MinIntervalMs = 50f
        };

        detector = new OnsetDetector(config, AudioSettings.outputSampleRate);
        detector.OnOnsetDetected += OnBeat;

        audioSource = GetComponent<AudioSource>();
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        // Convert to mono
        float[] mono = new float[data.Length / channels];
        for (int i = 0; i < mono.Length; i++)
        {
            if (channels == 2)
                mono[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                mono[i] = data[i];
        }

        detector.Process(mono);
    }

    void OnBeat(float timestamp, float strength)
    {
        Debug.Log($"Beat detected at {timestamp:F3}s, strength: {strength:F2}");

        // Visual feedback
        GameObject cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
        cube.transform.localScale = Vector3.one * strength;
        Destroy(cube, 0.1f);
    }

    void OnDestroy()
    {
        detector?.Dispose();
    }
}
```

---

## StreamingIIRFilter Class

**Namespace:** `Spectra`

Real-time IIR filter for streaming audio. Implements IDisposable.

### Constructor

```csharp
public StreamingIIRFilter(float[] b, float[] a)
public StreamingIIRFilter((float[] b, float[] a) coeffs)
```

**Parameters:**
- `b` - Numerator coefficients
- `a` - Denominator coefficients

**Example:**
```csharp
// Design filter
var (b, a) = DSP.Butter(4, 0.25f, FilterType.Lowpass);

// Create streaming filter
using (var filter = new StreamingIIRFilter(b, a))
{
    // Process multiple blocks
    foreach (var block in audioBlocks)
    {
        float[] filtered = filter.Process(block);
        OutputAudio(filtered);
    }
} // Dispose releases resources
```

**Alternative:**
```csharp
// Direct from tuple
var coeffs = DSP.Butter(6, 0.3f);
var filter = new StreamingIIRFilter(coeffs);
```

---

### Methods

#### Process

Process a block of samples.

```csharp
public void Process(float[] input, float[] output)
public float[] Process(float[] input)
```

**Example:**
```csharp
// In-place version (pre-allocated output)
float[] output = new float[1024];
filter.Process(inputBlock, output);

// Allocating version
float[] filtered = filter.Process(inputBlock);
```

**Notes:**
- Maintains filter state between calls
- State is preserved across blocks
- Use `Reset()` to clear state

---

#### Reset

Reset filter state to zero.

```csharp
public void Reset()
```

**Example:**
```csharp
// Start processing new audio file
filter.Reset();

foreach (var block in newAudioFile)
{
    var output = filter.Process(block);
}
```

---

### Real-Time Example

```csharp
using UnityEngine;
using Spectra;

public class RealtimeFilterExample : MonoBehaviour
{
    private StreamingIIRFilter lowpass;

    void Start()
    {
        // Design 4th-order lowpass at 1 kHz
        float cutoff = 1000f / (AudioSettings.outputSampleRate / 2f);
        var (b, a) = DSP.Butter(4, cutoff, FilterType.Lowpass);

        lowpass = new StreamingIIRFilter(b, a);
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        // Convert to mono
        float[] mono = new float[data.Length / channels];
        for (int i = 0; i < mono.Length; i++)
        {
            if (channels == 2)
                mono[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                mono[i] = data[i];
        }

        // Apply filter
        float[] filtered = lowpass.Process(mono);

        // Convert back to stereo
        for (int i = 0; i < mono.Length; i++)
        {
            data[i * 2] = filtered[i];
            data[i * 2 + 1] = filtered[i];
        }
    }

    void OnDestroy()
    {
        lowpass?.Dispose();
    }
}
```

**Performance:**
- Real-time capable for typical filter orders
- No allocations after first call
- State preserved between blocks

---

## AudioPipeline Component

**Namespace:** `Spectra`

MonoBehaviour for capturing audio input (microphone or AudioSource).

### Inspector Fields

```csharp
[SerializeField] private int bufferSize = 1024;
[SerializeField] private bool useMicrophone = false;
[SerializeField] private string microphoneDevice = "";
```

**Parameters:**
- `bufferSize` - Audio buffer size
- `useMicrophone` - Use microphone input (vs AudioSource)
- `microphoneDevice` - Specific microphone (empty = default)

---

### Event

#### OnAudioSamples

```csharp
public event Action<float[]> OnAudioSamples;
```

Fired when new audio samples are available.

**Example:**
```csharp
audioPipeline.OnAudioSamples += ProcessAudio;

void ProcessAudio(float[] samples)
{
    // Analyze samples
    using (var fft = new FFTAnalyzer(samples.Length))
    {
        float[] spectrum = fft.GetMagnitudeSpectrum(samples);
        // Visualize spectrum
    }
}
```

---

### Properties

```csharp
public int SampleRate { get; }
```

Current sample rate in Hz.

---

### Methods

#### GetAudioBuffer

Get current audio buffer (copy).

```csharp
public float[] GetAudioBuffer()
```

---

### Unity Setup

```csharp
using UnityEngine;
using Spectra;

public class AudioVisualizerExample : MonoBehaviour
{
    private AudioPipeline pipeline;
    private FFTAnalyzer fft;

    void Start()
    {
        // Add AudioPipeline component
        pipeline = gameObject.AddComponent<AudioPipeline>();

        // Subscribe to audio samples
        pipeline.OnAudioSamples += OnAudio;

        // Create FFT analyzer
        fft = new FFTAnalyzer(1024);
    }

    void OnAudio(float[] samples)
    {
        if (samples.Length == fft.Size)
        {
            float[] spectrum = fft.GetMagnitudeSpectrum(samples);
            Visualize(spectrum);
        }
    }

    void Visualize(float[] spectrum)
    {
        // Draw spectrum bars
        for (int i = 0; i < spectrum.Length; i++)
        {
            float height = spectrum[i] * 10f;
            Debug.DrawLine(
                new Vector3(i * 0.1f, 0, 0),
                new Vector3(i * 0.1f, height, 0),
                Color.cyan
            );
        }
    }

    void OnDestroy()
    {
        fft?.Dispose();
    }
}
```

---

## Enumerations

### FilterType

```csharp
public enum FilterType
{
    Lowpass = 0,
    Highpass = 1,
    Bandpass = 2,
    Bandstop = 3
}
```

**Usage:**
```csharp
var lpf = DSP.Butter(4, 0.25f, FilterType.Lowpass);
var hpf = DSP.Butter(4, 0.1f, FilterType.Highpass);
```

---

### WindowType

```csharp
public enum WindowType
{
    Rectangular = 0,
    Hann = 1,
    Hamming = 2,
    Blackman = 3,
    BlackmanHarris = 4,
    FlatTop = 5
}
```

**Usage:**
```csharp
float[] window = DSP.Window(1024, WindowType.Hann);
DSP.ApplyWindow(signal, WindowType.Blackman);
```

---

## Exceptions

### SpectraException

```csharp
public class SpectraException : Exception
{
    public SpectraException(string message)
}
```

Thrown by all SpectraCore operations on error.

**Example:**
```csharp
try
{
    var (b, a) = DSP.Butter(100, 0.5f); // Invalid order
}
catch (SpectraException ex)
{
    Debug.LogError($"Filter design failed: {ex.Message}");
}
```

**Common Causes:**
- Invalid parameters (negative, out of range)
- FFT size not power of 2
- Input/output length mismatch
- Native library not found
- Out of memory

---

## Performance Guidelines

### Memory Management

**Always dispose IDisposable objects:**
```csharp
// Good
using (var fft = new FFTAnalyzer(2048))
{
    // Use fft
}

// Also good
var fft = new FFTAnalyzer(2048);
try
{
    // Use fft
}
finally
{
    fft.Dispose();
}

// Bad - memory leak!
var fft = new FFTAnalyzer(2048);
// ... use fft but never dispose
```

---

### Real-Time Processing

**Minimize allocations in audio thread:**
```csharp
// Bad - allocates every frame
void OnAudioFilterRead(float[] data, int channels)
{
    var fft = new FFTAnalyzer(1024); // Allocation!
    float[] spectrum = fft.GetMagnitudeSpectrum(data); // Allocation!
    fft.Dispose();
}

// Good - reuse objects
private FFTAnalyzer fft;
private float[] spectrum;

void Start()
{
    fft = new FFTAnalyzer(1024);
    spectrum = new float[fft.SpectrumSize];
}

void OnAudioFilterRead(float[] data, int channels)
{
    fft.GetMagnitudeSpectrum(data); // Uses pre-allocated spectrum
}
```

---

### FFT Size Selection

| FFT Size | Frequency Resolution @ 44.1kHz | Time Resolution | CPU Cost |
|----------|-------------------------------|-----------------|----------|
| 256 | 172 Hz | 5.8 ms | Very low |
| 512 | 86 Hz | 11.6 ms | Low |
| 1024 | 43 Hz | 23.2 ms | Medium |
| 2048 | 21.5 Hz | 46.4 ms | Medium |
| 4096 | 10.8 Hz | 92.9 ms | High |
| 8192 | 5.4 Hz | 185.8 ms | Very high |

**Recommendations:**
- **Drums/percussion**: 512-1024
- **Music analysis**: 2048-4096
- **Pitch detection**: 4096-8192
- **Real-time visualization**: 1024-2048

---

### Thread Safety

**Most classes are NOT thread-safe:**
- `FFTAnalyzer`, `STFTAnalyzer`, `OnsetDetector`, `StreamingIIRFilter`
- Use separate instances per thread
- Or use locks for shared access

**DSP static methods ARE thread-safe:**
- All `DSP.XXX()` methods can be called from any thread

---

## Complete Application Examples

### Example 1: Audio Equalizer

```csharp
using UnityEngine;
using Spectra;

public class ThreeBandEqualizer : MonoBehaviour
{
    [Range(-20f, 20f)] public float lowGain = 0f;   // 20-200 Hz
    [Range(-20f, 20f)] public float midGain = 0f;   // 200-2000 Hz
    [Range(-20f, 20f)] public float highGain = 0f;  // 2000-20000 Hz

    private StreamingIIRFilter lowBand, midBand, highBand;
    private float sampleRate;

    void Start()
    {
        sampleRate = AudioSettings.outputSampleRate;
        UpdateFilters();
    }

    void UpdateFilters()
    {
        lowBand?.Dispose();
        midBand?.Dispose();
        highBand?.Dispose();

        // Low: Lowpass at 200 Hz
        float lowCutoff = 200f / (sampleRate / 2f);
        var lpf = DSP.Butter(4, lowCutoff, FilterType.Lowpass);
        lowBand = new StreamingIIRFilter(lpf);

        // Mid: Bandpass 200-2000 Hz (simplified - use two filters)
        float midLow = 200f / (sampleRate / 2f);
        float midHigh = 2000f / (sampleRate / 2f);
        var hpf = DSP.Butter(4, midLow, FilterType.Highpass);
        midBand = new StreamingIIRFilter(hpf);

        // High: Highpass at 2000 Hz
        float highCutoff = 2000f / (sampleRate / 2f);
        var hpf2 = DSP.Butter(4, highCutoff, FilterType.Highpass);
        highBand = new StreamingIIRFilter(hpf2);
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        float[] mono = ConvertToMono(data, channels);

        float[] low = lowBand.Process(mono);
        float[] mid = midBand.Process(mono);
        float[] high = highBand.Process(mono);

        // Mix with gain
        float lowMult = Mathf.Pow(10f, lowGain / 20f);
        float midMult = Mathf.Pow(10f, midGain / 20f);
        float highMult = Mathf.Pow(10f, highGain / 20f);

        for (int i = 0; i < mono.Length; i++)
        {
            float mixed = low[i] * lowMult + mid[i] * midMult + high[i] * highMult;
            data[i * 2] = mixed;
            data[i * 2 + 1] = mixed;
        }
    }

    float[] ConvertToMono(float[] data, int channels)
    {
        float[] mono = new float[data.Length / channels];
        for (int i = 0; i < mono.Length; i++)
        {
            if (channels == 2)
                mono[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                mono[i] = data[i];
        }
        return mono;
    }

    void OnDestroy()
    {
        lowBand?.Dispose();
        midBand?.Dispose();
        highBand?.Dispose();
    }
}
```

---

### Example 2: Beat-Synchronized Visuals

```csharp
using UnityEngine;
using Spectra;

public class BeatVisualizer : MonoBehaviour
{
    public GameObject beatPrefab;
    private OnsetDetector detector;

    void Start()
    {
        var config = new OnsetConfig
        {
            FftSize = 2048,
            HopSize = 512,
            Threshold = 0.6f,
            MinIntervalMs = 100f
        };

        detector = new OnsetDetector(config, AudioSettings.outputSampleRate);
        detector.OnOnsetDetected += OnBeat;
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        float[] mono = new float[data.Length / channels];
        for (int i = 0; i < mono.Length; i++)
        {
            mono[i] = channels == 2
                ? (data[i * 2] + data[i * 2 + 1]) * 0.5f
                : data[i];
        }

        detector.Process(mono);
    }

    void OnBeat(float timestamp, float strength)
    {
        GameObject obj = Instantiate(beatPrefab, Vector3.zero, Quaternion.identity);
        obj.transform.localScale = Vector3.one * strength * 2f;
        Destroy(obj, 0.5f);
    }

    void OnDestroy()
    {
        detector?.Dispose();
    }
}
```

---

### Example 3: Spectrogram Visualization

```csharp
using UnityEngine;
using Spectra;

public class SpectrogramDisplay : MonoBehaviour
{
    public int width = 256;
    public int height = 128;

    private Texture2D spectrogram;
    private STFTAnalyzer stft;
    private Color[] pixels;
    private int currentColumn = 0;

    void Start()
    {
        spectrogram = new Texture2D(width, height);
        pixels = new Color[width * height];
        GetComponent<Renderer>().material.mainTexture = spectrogram;

        stft = new STFTAnalyzer(256, 64, WindowType.Hann);
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        float[] mono = new float[data.Length / channels];
        for (int i = 0; i < mono.Length; i++)
        {
            mono[i] = channels == 2
                ? (data[i * 2] + data[i * 2 + 1]) * 0.5f
                : data[i];
        }

        stft.PushSamples(mono);

        while (stft.FramesAvailable > 0 && stft.PopFrame(out float[] real, out float[] imag))
        {
            float[] magnitude = STFTAnalyzer.ComputeMagnitude(real, imag);
            float[] dB = STFTAnalyzer.PowerToDb(
                STFTAnalyzer.ComputePower(real, imag)
            );

            // Draw column
            for (int y = 0; y < Mathf.Min(height, dB.Length); y++)
            {
                float normalized = Mathf.InverseLerp(-80f, 0f, dB[y]);
                pixels[currentColumn + y * width] = GetHeatmapColor(normalized);
            }

            currentColumn = (currentColumn + 1) % width;
        }

        spectrogram.SetPixels(pixels);
        spectrogram.Apply();
    }

    Color GetHeatmapColor(float value)
    {
        // Blue -> Cyan -> Green -> Yellow -> Red
        if (value < 0.25f)
            return Color.Lerp(Color.blue, Color.cyan, value * 4f);
        else if (value < 0.5f)
            return Color.Lerp(Color.cyan, Color.green, (value - 0.25f) * 4f);
        else if (value < 0.75f)
            return Color.Lerp(Color.green, Color.yellow, (value - 0.5f) * 4f);
        else
            return Color.Lerp(Color.yellow, Color.red, (value - 0.75f) * 4f);
    }

    void OnDestroy()
    {
        stft?.Dispose();
    }
}
```

---

## See Also

- [Filter Designer Guide](FilterDesignerGuide.md)
- [Signal Inspector Guide](SignalInspectorGuide.md)
- [Unity Integration Summary](../UNITY_INTEGRATION_SUMMARY.md)
- [GitHub Repository](https://github.com/yogt1984/SpectraCore)
