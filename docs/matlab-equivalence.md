# MATLAB Equivalence Guide

This guide provides side-by-side comparisons between MATLAB Signal Processing Toolbox functions and their SpectraCore equivalents.

## Philosophy

SpectraCore's API is designed to feel familiar to engineers who think in MATLAB. Function names, parameter ordering, and behavior match MATLAB conventions wherever possible.

## Quick Reference

| MATLAB | SpectraCore C# | Description |
|--------|----------------|-------------|
| `[b,a] = butter(n, Wn)` | `DSP.Butter(n, Wn)` | Butterworth filter design |
| `[b,a] = cheby1(n, Rp, Wn)` | `DSP.Cheby1(n, Rp, Wn)` | Chebyshev Type I |
| `[b,a] = cheby2(n, Rs, Wn)` | `DSP.Cheby2(n, Rs, Wn)` | Chebyshev Type II |
| `[b,a] = ellip(n, Rp, Rs, Wn)` | `DSP.Ellip(n, Rp, Rs, Wn)` | Elliptic filter |
| `y = filter(b, a, x)` | `DSP.LFilter(b, a, x)` | Apply filter (causal) |
| `y = filtfilt(b, a, x)` | `DSP.FiltFilt(b, a, x)` | Zero-phase filtering |
| `Y = fft(x)` | `DSP.Fft(x)` | Fast Fourier Transform |
| `[S,F,T] = stft(x)` | `DSP.Stft(x, config)` | Short-Time Fourier Transform |
| `[Pxx,F] = pwelch(x)` | `DSP.Pwelch(x, config)` | Power spectral density |
| `r = xcorr(x, y)` | `DSP.Xcorr(x, y)` | Cross-correlation |
| `y = conv(x, h)` | `DSP.Conv(x, h)` | Convolution |
| `y = hilbert(x)` | `DSP.Hilbert(x)` | Analytic signal |
| `y = resample(x, p, q)` | `DSP.Resample(x, p, q)` | Rational resampling |

---

## Filter Design

### Butterworth Filter

**MATLAB:**
```matlab
% 4th order lowpass Butterworth filter
% Cutoff at 0.2 * Nyquist frequency
[b, a] = butter(4, 0.2, 'low');

% Highpass
[b, a] = butter(4, 0.2, 'high');

% Bandpass (two cutoff frequencies)
[b, a] = butter(4, [0.2, 0.5], 'bandpass');
```

**SpectraCore:**
```csharp
// 4th order lowpass Butterworth filter
var (b, a) = DSP.Butter(4, 0.2f, FilterType.Lowpass);

// Highpass
var (b, a) = DSP.Butter(4, 0.2f, FilterType.Highpass);

// Bandpass
var (b, a) = DSP.Butter(4, new[] { 0.2f, 0.5f }, FilterType.Bandpass);
```

### Chebyshev Type I

**MATLAB:**
```matlab
% 6th order Chebyshev Type I
% 0.5 dB passband ripple, cutoff at 0.3
[b, a] = cheby1(6, 0.5, 0.3, 'low');
```

**SpectraCore:**
```csharp
var (b, a) = DSP.Cheby1(6, 0.5f, 0.3f, FilterType.Lowpass);
```

### Chebyshev Type II

**MATLAB:**
```matlab
% 6th order Chebyshev Type II
% 40 dB stopband attenuation
[b, a] = cheby2(6, 40, 0.3, 'low');
```

**SpectraCore:**
```csharp
var (b, a) = DSP.Cheby2(6, 40f, 0.3f, FilterType.Lowpass);
```

### Elliptic Filter

**MATLAB:**
```matlab
% 4th order elliptic filter
% 1 dB passband ripple, 60 dB stopband attenuation
[b, a] = ellip(4, 1, 60, 0.4, 'low');
```

**SpectraCore:**
```csharp
var (b, a) = DSP.Ellip(4, 1f, 60f, 0.4f, FilterType.Lowpass);
```

---

## Filter Application

### Causal Filtering (filter)

**MATLAB:**
```matlab
% Apply filter
y = filter(b, a, x);
```

**SpectraCore:**
```csharp
float[] y = DSP.LFilter(b, a, x);
```

### Zero-Phase Filtering (filtfilt)

**MATLAB:**
```matlab
% Zero-phase filtering (no phase distortion)
y = filtfilt(b, a, x);
```

**SpectraCore:**
```csharp
float[] y = DSP.FiltFilt(b, a, x);
```

**Note:** `FiltFilt` applies the filter forward, then backward, eliminating phase distortion. Use for offline processing only (not real-time).

### Streaming Filter (Real-Time)

For real-time applications where data arrives in chunks:

**SpectraCore (no MATLAB equivalent):**
```csharp
// Create stateful filter
using var filter = new StreamingIIRFilter(b, a);

// Process chunks as they arrive
void OnAudioChunk(float[] chunk)
{
    float[] output = new float[chunk.Length];
    filter.Process(chunk, output);
    // Use output...
}
```

---

## FFT and Spectral Analysis

### FFT

**MATLAB:**
```matlab
% FFT of signal
Y = fft(x);
Y = fft(x, 1024);  % With zero-padding to 1024 points

% Get magnitude and phase
magnitude = abs(Y);
phase = angle(Y);

% Single-sided spectrum
N = length(x);
P1 = abs(Y(1:N/2+1));
```

**SpectraCore:**
```csharp
// FFT
Complex[] Y = DSP.Fft(x);
Complex[] Y = DSP.Fft(x, 1024);  // Zero-padded

// Magnitude and phase
float[] magnitude = DSP.Abs(Y);
float[] phase = DSP.Angle(Y);

// Single-sided spectrum
float[] P1 = DSP.SingleSidedSpectrum(Y);
```

### STFT (Spectrogram)

**MATLAB:**
```matlab
% STFT with 256-sample window, 50% overlap
[S, F, T] = stft(x, fs, 'Window', hann(256), 'OverlapLength', 128);

% Or using spectrogram
[S, F, T] = spectrogram(x, hann(256), 128, 256, fs);
```

**SpectraCore:**
```csharp
var config = new STFTConfig
{
    FftSize = 256,
    HopSize = 128,  // 50% overlap
    Window = WindowType.Hann
};

var (S, F, T) = DSP.Stft(x, config, sampleRate);
```

### Power Spectral Density (Welch)

**MATLAB:**
```matlab
% Welch's method
[Pxx, F] = pwelch(x, hann(256), 128, 256, fs);

% With default parameters
[Pxx, F] = pwelch(x, [], [], [], fs);
```

**SpectraCore:**
```csharp
var config = new WelchConfig
{
    SegmentSize = 256,
    Overlap = 128,
    Window = WindowType.Hann
};

var (Pxx, F) = DSP.Pwelch(x, config, sampleRate);

// Or with defaults
var (Pxx, F) = DSP.Pwelch(x, sampleRate);
```

---

## Correlation

### Cross-Correlation

**MATLAB:**
```matlab
% Cross-correlation
r = xcorr(x, y);

% Normalized
r = xcorr(x, y, 'normalized');

% With max lag
r = xcorr(x, y, maxlag);
```

**SpectraCore:**
```csharp
float[] r = DSP.Xcorr(x, y);
float[] r = DSP.Xcorr(x, y, normalized: true);
float[] r = DSP.Xcorr(x, y, maxLag: 100);
```

### Autocorrelation

**MATLAB:**
```matlab
r = xcorr(x);
```

**SpectraCore:**
```csharp
float[] r = DSP.Xcorr(x);  // Single argument = autocorrelation
```

---

## Convolution

**MATLAB:**
```matlab
% Full convolution
y = conv(x, h);

% Same size as input
y = conv(x, h, 'same');

% Valid (no edge effects)
y = conv(x, h, 'valid');
```

**SpectraCore:**
```csharp
float[] y = DSP.Conv(x, h);
float[] y = DSP.Conv(x, h, ConvMode.Same);
float[] y = DSP.Conv(x, h, ConvMode.Valid);
```

---

## Hilbert Transform

**MATLAB:**
```matlab
% Analytic signal
z = hilbert(x);

% Instantaneous amplitude (envelope)
amp = abs(hilbert(x));

% Instantaneous phase
phase = unwrap(angle(hilbert(x)));

% Instantaneous frequency
inst_freq = diff(phase) * fs / (2*pi);
```

**SpectraCore:**
```csharp
// Analytic signal
Complex[] z = DSP.Hilbert(x);

// Instantaneous amplitude
float[] amp = DSP.InstantaneousAmplitude(x);

// Instantaneous phase
float[] phase = DSP.InstantaneousPhase(x);

// Instantaneous frequency
float[] freq = DSP.InstantaneousFrequency(x, sampleRate);
```

---

## Resampling

**MATLAB:**
```matlab
% Resample by rational factor p/q
y = resample(x, p, q);

% Example: 48kHz to 44.1kHz
y = resample(x, 441, 480);
```

**SpectraCore:**
```csharp
float[] y = DSP.Resample(x, p, q);

// Example: 48kHz to 44.1kHz
float[] y = DSP.Resample(x, 441, 480);
```

**MATLAB:**
```matlab
% Simple decimation (downsample by integer factor)
y = downsample(x, n);

% Simple interpolation (upsample by integer factor)
y = upsample(x, n);
```

**SpectraCore:**
```csharp
float[] y = DSP.Downsample(x, n);
float[] y = DSP.Upsample(x, n);
```

---

## Window Functions

**MATLAB:**
```matlab
w = hann(N);
w = hamming(N);
w = blackman(N);
w = flattopwin(N);
```

**SpectraCore:**
```csharp
float[] w = DSP.Window(N, WindowType.Hann);
float[] w = DSP.Window(N, WindowType.Hamming);
float[] w = DSP.Window(N, WindowType.Blackman);
float[] w = DSP.Window(N, WindowType.FlatTop);

// Or apply directly to signal
DSP.ApplyWindow(signal, WindowType.Hann);
```

---

## Frequency Response

**MATLAB:**
```matlab
% Frequency response of filter
[H, W] = freqz(b, a, 512);

% Magnitude in dB
mag_db = 20*log10(abs(H));

% Phase in degrees
phase_deg = rad2deg(angle(H));
```

**SpectraCore:**
```csharp
var (H, W) = DSP.Freqz(b, a, 512);

float[] magDb = DSP.MagnitudeDB(H);
float[] phaseDeg = DSP.PhaseDegrees(H);
```

---

## Complete Example: Filtering ECG Signal

**MATLAB:**
```matlab
% Load ECG signal (1000 Hz sample rate)
load ecg_signal.mat
fs = 1000;

% Design bandpass filter (0.5 - 40 Hz)
[b, a] = butter(4, [0.5 40]/(fs/2), 'bandpass');

% Apply zero-phase filter
ecg_filtered = filtfilt(b, a, ecg);

% Compute envelope using Hilbert transform
envelope = abs(hilbert(ecg_filtered));

% Find R-peaks using peak detection
% (Custom implementation needed)
```

**SpectraCore:**
```csharp
float fs = 1000f;
float[] ecg = LoadEcgSignal();

// Design bandpass filter (0.5 - 40 Hz)
float[] normalizedFreqs = { 0.5f / (fs/2), 40f / (fs/2) };
var (b, a) = DSP.Butter(4, normalizedFreqs, FilterType.Bandpass);

// Apply zero-phase filter
float[] ecgFiltered = DSP.FiltFilt(b, a, ecg);

// Compute envelope
float[] envelope = DSP.InstantaneousAmplitude(ecgFiltered);

// R-peak detection would follow...
```

---

## Numerical Precision

SpectraCore aims for numerical equivalence with MATLAB within floating-point tolerance:

| Operation | Typical Error vs MATLAB |
|-----------|------------------------|
| FFT | < 1e-6 (relative) |
| Filter coefficients | < 1e-7 (relative) |
| Filter application | < 1e-6 (relative) |
| Correlation | < 1e-6 (relative) |

**Validation:** Every release includes automated tests comparing SpectraCore output against MATLAB-generated reference data.

---

## What's Different

### Indexing
- MATLAB uses 1-based indexing
- SpectraCore uses 0-based indexing (C# convention)

### Complex Numbers
- MATLAB: Native complex support
- SpectraCore: `Spectra.Complex` struct or separate real/imag arrays

### Memory
- MATLAB: Automatic memory management
- SpectraCore: Explicit disposal for streaming objects (`using` statement)

### Real-Time
- MATLAB: Not designed for real-time
- SpectraCore: `StreamingIIRFilter` and `STFT.PushSamples()` for real-time processing
