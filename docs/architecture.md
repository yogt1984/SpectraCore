# SpectraCore Architecture

This document describes the technical architecture of SpectraCore, a deterministic real-time DSP library.

## Design Principles

1. **Deterministic Execution**: No allocations in the hot path, predictable latency
2. **Platform Abstraction**: Single codebase compiles to all target platforms
3. **ABI Stability**: C interface ensures compatibility across language boundaries
4. **MATLAB Familiarity**: API naming mirrors MATLAB Signal Processing Toolbox
5. **Testability**: Every algorithm validated against MATLAB reference output

## System Layers

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Unity C# Layer                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  DSP.Butter()  DSP.Cheby1()  DSP.FiltFilt()  DSP.Stft()       │  │
│  │  (MATLAB-equivalent API)                                       │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                              │                                       │
│                              ▼                                       │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  Managed Wrappers                                              │  │
│  │  IIRFilter, FIRFilter, FFTAnalyzer, StreamingFilter            │  │
│  │  (Handles memory, lifecycle, Unity integration)                │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                              │                                       │
│                              ▼                                       │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  SpectraNative.cs (P/Invoke Bridge)                            │  │
│  │  [DllImport("spectra")] static extern ...                      │  │
│  └───────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
                               │
                               ▼ C ABI Boundary
┌─────────────────────────────────────────────────────────────────────┐
│                       spectra_abi.h                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  SPECTRA_API void* spectra_filter_create(int type, ...)        │  │
│  │  SPECTRA_API void  spectra_filter_process(void*, float*, int)  │  │
│  │  SPECTRA_API void  spectra_filter_destroy(void*)               │  │
│  │  SPECTRA_API int   spectra_fft_forward(float*, float*, int)    │  │
│  └───────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     Native C++ Core                                  │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    Processing Layer                          │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐│    │
│  │  │ FFT Engine  │ │ Filter Bank │ │ Analysis Algorithms     ││    │
│  │  │ (KissFFT)   │ │ FIR/IIR     │ │ Correlation, PSD,       ││    │
│  │  │ STFT        │ │ Butterworth │ │ Hilbert, Resample       ││    │
│  │  │ Windows     │ │ Chebyshev   │ │                         ││    │
│  │  └─────────────┘ └─────────────┘ └─────────────────────────┘│    │
│  └─────────────────────────────────────────────────────────────┘    │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                   Streaming Layer                            │    │
│  │  ┌─────────────────┐ ┌────────────────────────────────────┐ │    │
│  │  │ SPSC RingBuffer │ │ Stateful Filter Objects            │ │    │
│  │  │ (lock-free)     │ │ (maintains delay lines, state)     │ │    │
│  │  └─────────────────┘ └────────────────────────────────────┘ │    │
│  └─────────────────────────────────────────────────────────────┘    │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                      Core Layer                              │    │
│  │  Types, Memory Pool, Platform Abstraction, Assertions        │    │
│  └─────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
```

## Component Details

### 1. Core Layer

The foundation providing platform-independent primitives.

#### Types (`core/types.hpp`)

```cpp
namespace spectra {
    using Sample = float;

    struct Complex {
        float real;
        float imag;

        float magnitude() const;
        float phase() const;
    };

    // Fixed-size vector for filter coefficients
    template<typename T, size_t MaxSize>
    class StaticVector;
}
```

#### Ring Buffer (`core/ring_buffer.hpp`)

Lock-free Single-Producer Single-Consumer queue for real-time audio streaming.

```cpp
template<typename T, size_t Size>
class SpscRingBuffer {
public:
    // Producer thread
    bool push(const T& sample);
    bool push_batch(const T* samples, size_t count);

    // Consumer thread
    bool pop(T& sample);
    size_t pop_batch(T* samples, size_t max_count);

    size_t available() const;

private:
    alignas(64) std::atomic<size_t> head_;  // Cache line aligned
    alignas(64) std::atomic<size_t> tail_;
    T buffer_[Size];
};
```

**Key Properties:**
- Wait-free for single producer, single consumer
- No locks, no allocations
- Cache-line aligned head/tail to prevent false sharing
- Power-of-two size for fast modulo via bitmasking

#### Window Functions (`core/window_functions.hpp`)

```cpp
enum class WindowType {
    Rectangular,
    Hann,
    Hamming,
    Blackman,
    BlackmanHarris,
    FlatTop,
    Kaiser
};

void apply_window(float* data, size_t size, WindowType type);
void generate_window(float* output, size_t size, WindowType type);
```

### 2. FFT Engine

Wraps KissFFT with additional functionality.

#### FFT (`fft/fft.hpp`)

```cpp
class FFT {
public:
    explicit FFT(size_t size);
    ~FFT();

    // Real-to-complex forward transform
    void forward(const float* input, Complex* output);

    // Complex-to-real inverse transform
    void inverse(const Complex* input, float* output);

    size_t size() const { return size_; }
    size_t spectrum_size() const { return size_ / 2 + 1; }

private:
    size_t size_;
    kiss_fftr_cfg cfg_forward_;
    kiss_fftr_cfg cfg_inverse_;
};
```

#### STFT (`fft/stft.hpp`)

```cpp
struct STFTConfig {
    size_t fft_size = 2048;
    size_t hop_size = 512;
    WindowType window = WindowType::Hann;
    bool center = true;
};

class STFT {
public:
    explicit STFT(const STFTConfig& config);

    // Process entire signal, return spectrogram
    void analyze(const float* input, size_t input_size,
                 Complex* output, size_t* num_frames);

    // Streaming: push samples, get frames when ready
    void push_samples(const float* samples, size_t count);
    bool pop_frame(Complex* frame);

private:
    STFTConfig config_;
    FFT fft_;
    std::vector<float> window_;
    std::vector<float> frame_buffer_;
    size_t buffer_pos_ = 0;
};
```

### 3. Filter Design

Classical analog prototype to digital filter design.

#### Filter Types

```cpp
enum class FilterType {
    Lowpass,
    Highpass,
    Bandpass,
    Bandstop
};

enum class FilterDesign {
    Butterworth,
    ChebyshevI,
    ChebyshevII,
    Elliptic,
    Bessel
};

struct FilterCoefficients {
    std::vector<float> b;  // Numerator (feedforward)
    std::vector<float> a;  // Denominator (feedback)
};
```

#### IIR Design (`filters/iir_design.hpp`)

```cpp
// Butterworth filter design
// Equivalent to MATLAB: [b,a] = butter(order, Wn, 'low')
FilterCoefficients butter(int order, float normalized_freq, FilterType type);

// Chebyshev Type I (ripple in passband)
// Equivalent to MATLAB: [b,a] = cheby1(order, Rp, Wn)
FilterCoefficients cheby1(int order, float ripple_db, float normalized_freq, FilterType type);

// Chebyshev Type II (ripple in stopband)
FilterCoefficients cheby2(int order, float stopband_atten_db, float normalized_freq, FilterType type);

// Elliptic filter (ripple in both bands, steepest rolloff)
FilterCoefficients ellip(int order, float passband_ripple_db,
                         float stopband_atten_db, float normalized_freq, FilterType type);
```

#### FIR Design (`filters/fir_design.hpp`)

```cpp
// Windowed sinc FIR design
std::vector<float> fir_windowed_sinc(int num_taps, float normalized_freq,
                                      FilterType type, WindowType window);

// Parks-McClellan (equiripple) FIR design
// Equivalent to MATLAB: b = firpm(order, f, a)
std::vector<float> firpm(int order,
                         const std::vector<float>& freq_bands,
                         const std::vector<float>& amplitudes,
                         const std::vector<float>& weights = {});
```

### 4. Filter Application

#### Stateless Filtering

```cpp
// Causal filtering (like MATLAB filter())
void lfilter(const float* b, size_t b_len,
             const float* a, size_t a_len,
             const float* input, float* output, size_t len);

// Zero-phase filtering (like MATLAB filtfilt())
// Applies filter forward and backward to eliminate phase distortion
void filtfilt(const float* b, size_t b_len,
              const float* a, size_t a_len,
              const float* input, float* output, size_t len);
```

#### Stateful Streaming Filter

For real-time applications where samples arrive in chunks:

```cpp
class StreamingIIRFilter {
public:
    StreamingIIRFilter(const std::vector<float>& b,
                       const std::vector<float>& a);

    // Process a block of samples
    void process(const float* input, float* output, size_t count);

    // Reset internal state (delay lines)
    void reset();

private:
    std::vector<float> b_, a_;
    std::vector<float> x_state_;  // Input delay line
    std::vector<float> y_state_;  // Output delay line
};
```

**Implementation (Direct Form II Transposed):**

```cpp
void StreamingIIRFilter::process(const float* input, float* output, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        float x = input[i];
        float y = b_[0] * x + state_[0];

        for (size_t j = 1; j < state_.size(); ++j) {
            state_[j-1] = b_[j] * x - a_[j] * y + state_[j];
        }

        output[i] = y;
    }
}
```

### 5. Analysis Algorithms

#### Correlation (`analysis/correlation.hpp`)

```cpp
// Cross-correlation
// Equivalent to MATLAB: xcorr(x, y)
void xcorr(const float* x, size_t x_len,
           const float* y, size_t y_len,
           float* output, size_t* output_len);

// Autocorrelation
// Equivalent to MATLAB: xcorr(x)
void xcorr(const float* x, size_t len, float* output);
```

#### Power Spectral Density (`analysis/psd.hpp`)

```cpp
struct WelchConfig {
    size_t segment_size = 256;
    size_t overlap = 128;
    WindowType window = WindowType::Hann;
    bool onesided = true;
};

// Welch's method for PSD estimation
// Equivalent to MATLAB: [pxx, f] = pwelch(x, window, overlap, nfft, fs)
void pwelch(const float* input, size_t len,
            const WelchConfig& config,
            float* psd, float* frequencies,
            float sample_rate);
```

#### Hilbert Transform (`analysis/hilbert.hpp`)

```cpp
// Analytic signal via Hilbert transform
// Equivalent to MATLAB: hilbert(x)
void hilbert(const float* input, Complex* output, size_t len);

// Extract instantaneous amplitude, phase, frequency
void instantaneous_amplitude(const float* input, float* output, size_t len);
void instantaneous_phase(const float* input, float* output, size_t len);
void instantaneous_frequency(const float* input, float* output, size_t len, float fs);
```

#### Resampling (`analysis/resample.hpp`)

```cpp
// Polyphase resampling with anti-aliasing
// Equivalent to MATLAB: resample(x, p, q)
std::vector<float> resample(const float* input, size_t len, int p, int q);

// Simple integer factor operations
std::vector<float> upsample(const float* input, size_t len, int factor);
std::vector<float> downsample(const float* input, size_t len, int factor);
```

## Real-Time Considerations

### Memory Management

```cpp
// Pre-allocate all buffers at initialization
class DSPProcessor {
public:
    DSPProcessor(size_t max_block_size)
        : fft_(max_block_size)
        , scratch_buffer_(max_block_size)
        , spectrum_buffer_(max_block_size / 2 + 1)
    {}

    // Hot path - no allocations
    void process(const float* input, float* output, size_t len) {
        assert(len <= scratch_buffer_.size());
        // Use pre-allocated buffers only
    }

private:
    FFT fft_;
    std::vector<float> scratch_buffer_;
    std::vector<Complex> spectrum_buffer_;
};
```

### Thread Safety Model

```
┌─────────────────────────────────────────────────────────────┐
│                     Unity Main Thread                        │
│   - Create/destroy DSP objects                              │
│   - Configure parameters                                     │
│   - Read analysis results                                    │
└─────────────────────────────────────────────────────────────┘
                              ▲
                              │ Thread-safe queue
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Unity Audio Thread                        │
│   - Push samples to ring buffer                             │
│   - Must not block                                          │
│   - Must not allocate                                       │
└─────────────────────────────────────────────────────────────┘
                              ▲
                              │ SPSC Ring Buffer
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    DSP Worker Thread                         │
│   - Pop samples from ring buffer                            │
│   - Run analysis pipeline                                   │
│   - Push results to output queue                            │
└─────────────────────────────────────────────────────────────┘
```

### Latency Budget

For real-time audio at 48kHz with 10ms latency budget:

| Component | Typical Latency |
|-----------|-----------------|
| Audio input buffer | 2-5ms |
| Ring buffer transit | <0.1ms |
| DSP processing (2048-pt FFT) | 0.1-0.5ms |
| Result delivery | <0.1ms |
| **Total** | **3-6ms** |

## Error Handling

Errors are handled via return codes at the ABI boundary:

```cpp
enum class SpectraError {
    Success = 0,
    InvalidArgument = 1,
    AllocationFailed = 2,
    InvalidState = 3,
    NumericalInstability = 4
};

// Thread-local error storage
thread_local SpectraError last_error = SpectraError::Success;
thread_local char error_message[256] = "";

extern "C" {
    SPECTRA_API int spectra_get_last_error();
    SPECTRA_API const char* spectra_get_error_message();
}
```

## Platform Abstraction

```cpp
// Platform detection
#if defined(_WIN32)
    #define SPECTRA_WINDOWS
    #define SPECTRA_EXPORT __declspec(dllexport)
#elif defined(__APPLE__)
    #define SPECTRA_APPLE
    #define SPECTRA_EXPORT __attribute__((visibility("default")))
#elif defined(__ANDROID__)
    #define SPECTRA_ANDROID
    #define SPECTRA_EXPORT __attribute__((visibility("default")))
#else
    #define SPECTRA_LINUX
    #define SPECTRA_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
    #define SPECTRA_API extern "C" SPECTRA_EXPORT
#else
    #define SPECTRA_API SPECTRA_EXPORT
#endif
```

## Directory Structure

```
native/
├── include/spectra/
│   ├── core/
│   │   ├── types.hpp
│   │   ├── ring_buffer.hpp
│   │   └── window_functions.hpp
│   ├── fft/
│   │   ├── fft.hpp
│   │   └── stft.hpp
│   ├── filters/
│   │   ├── iir_design.hpp
│   │   ├── fir_design.hpp
│   │   └── streaming_filter.hpp
│   ├── analysis/
│   │   ├── correlation.hpp
│   │   ├── psd.hpp
│   │   ├── hilbert.hpp
│   │   └── resample.hpp
│   └── spectra.hpp              # Main include
├── src/
│   ├── core/
│   ├── fft/
│   ├── filters/
│   ├── analysis/
│   └── abi/
│       └── spectra_abi.cpp      # C ABI implementation
├── third_party/
│   └── kissfft/
├── tests/
│   ├── unit/
│   ├── benchmarks/
│   └── fixtures/
└── CMakeLists.txt
```
