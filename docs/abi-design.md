# ABI Design

This document describes the C Application Binary Interface (ABI) that bridges the native C++ DSP core with managed language bindings (C#/Unity).

## Design Goals

1. **Stability**: ABI changes are versioned and backward compatible
2. **Safety**: Opaque handles prevent invalid memory access from managed code
3. **Simplicity**: Flat C interface, no C++ types cross the boundary
4. **Debuggability**: Comprehensive error reporting

## Opaque Handle Pattern

All DSP objects are exposed as opaque `void*` handles:

```
C++ Internal                    ABI Boundary                 C# Managed
─────────────────────────────────────────────────────────────────────────
                                     │
class StreamingIIRFilter {    ──►    │    ──►    IntPtr handle
    std::vector<float> b_;           │           (wrapped in SafeHandle)
    std::vector<float> a_;    void*  │
    std::vector<float> state_;       │
};                                   │
                                     │
```

**Why opaque handles?**
- C# cannot safely access C++ objects with vtables, std containers
- Handle validation possible on native side
- Object layout can change without breaking ABI

## ABI Header

```c
// spectra_abi.h

#ifndef SPECTRA_ABI_H
#define SPECTRA_ABI_H

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
    #ifdef SPECTRA_BUILD
        #define SPECTRA_API __declspec(dllexport)
    #else
        #define SPECTRA_API __declspec(dllimport)
    #endif
#else
    #define SPECTRA_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Version
// ============================================================================

SPECTRA_API int spectra_version_major(void);
SPECTRA_API int spectra_version_minor(void);
SPECTRA_API int spectra_version_patch(void);
SPECTRA_API const char* spectra_version_string(void);

// ============================================================================
// Error Handling
// ============================================================================

typedef enum {
    SPECTRA_OK = 0,
    SPECTRA_ERROR_INVALID_ARGUMENT = 1,
    SPECTRA_ERROR_ALLOCATION_FAILED = 2,
    SPECTRA_ERROR_INVALID_HANDLE = 3,
    SPECTRA_ERROR_INVALID_STATE = 4,
    SPECTRA_ERROR_NUMERICAL_INSTABILITY = 5,
    SPECTRA_ERROR_BUFFER_TOO_SMALL = 6
} SpectraError;

SPECTRA_API SpectraError spectra_get_last_error(void);
SPECTRA_API const char* spectra_get_error_message(void);
SPECTRA_API void spectra_clear_error(void);

// ============================================================================
// FFT
// ============================================================================

typedef void* SpectraFFT;

SPECTRA_API SpectraFFT spectra_fft_create(int size);
SPECTRA_API void spectra_fft_destroy(SpectraFFT fft);
SPECTRA_API int spectra_fft_forward(SpectraFFT fft,
                                     const float* input,
                                     float* output_real,
                                     float* output_imag);
SPECTRA_API int spectra_fft_inverse(SpectraFFT fft,
                                     const float* input_real,
                                     const float* input_imag,
                                     float* output);
SPECTRA_API int spectra_fft_size(SpectraFFT fft);
SPECTRA_API int spectra_fft_spectrum_size(SpectraFFT fft);

// ============================================================================
// Window Functions
// ============================================================================

typedef enum {
    SPECTRA_WINDOW_RECTANGULAR = 0,
    SPECTRA_WINDOW_HANN = 1,
    SPECTRA_WINDOW_HAMMING = 2,
    SPECTRA_WINDOW_BLACKMAN = 3,
    SPECTRA_WINDOW_BLACKMAN_HARRIS = 4,
    SPECTRA_WINDOW_FLAT_TOP = 5
} SpectraWindowType;

SPECTRA_API int spectra_generate_window(float* output, int size, SpectraWindowType type);
SPECTRA_API int spectra_apply_window(float* data, int size, SpectraWindowType type);

// ============================================================================
// STFT
// ============================================================================

typedef void* SpectraSTFT;

typedef struct {
    int fft_size;
    int hop_size;
    SpectraWindowType window;
    int center;  // bool as int for C compatibility
} SpectraSTFTConfig;

SPECTRA_API SpectraSTFT spectra_stft_create(const SpectraSTFTConfig* config);
SPECTRA_API void spectra_stft_destroy(SpectraSTFT stft);
SPECTRA_API int spectra_stft_analyze(SpectraSTFT stft,
                                      const float* input, int input_size,
                                      float* output_real, float* output_imag,
                                      int* num_frames);
SPECTRA_API int spectra_stft_num_frames(int input_size, int fft_size, int hop_size);
SPECTRA_API int spectra_stft_push_samples(SpectraSTFT stft, const float* samples, int count);
SPECTRA_API int spectra_stft_pop_frame(SpectraSTFT stft, float* real, float* imag);
SPECTRA_API int spectra_stft_frames_available(SpectraSTFT stft);

// ============================================================================
// Filter Design
// ============================================================================

typedef enum {
    SPECTRA_FILTER_LOWPASS = 0,
    SPECTRA_FILTER_HIGHPASS = 1,
    SPECTRA_FILTER_BANDPASS = 2,
    SPECTRA_FILTER_BANDSTOP = 3
} SpectraFilterType;

// Butterworth
SPECTRA_API int spectra_butter(int order, float normalized_freq, SpectraFilterType type,
                                float* b, int* b_len, float* a, int* a_len);

// Chebyshev Type I
SPECTRA_API int spectra_cheby1(int order, float ripple_db, float normalized_freq,
                                SpectraFilterType type,
                                float* b, int* b_len, float* a, int* a_len);

// Chebyshev Type II
SPECTRA_API int spectra_cheby2(int order, float stopband_db, float normalized_freq,
                                SpectraFilterType type,
                                float* b, int* b_len, float* a, int* a_len);

// Elliptic
SPECTRA_API int spectra_ellip(int order, float passband_ripple_db, float stopband_db,
                               float normalized_freq, SpectraFilterType type,
                               float* b, int* b_len, float* a, int* a_len);

// Get required coefficient array sizes
SPECTRA_API int spectra_iir_coeff_size(int order);

// ============================================================================
// Streaming Filter
// ============================================================================

typedef void* SpectraStreamingFilter;

SPECTRA_API SpectraStreamingFilter spectra_streaming_filter_create(
    const float* b, int b_len,
    const float* a, int a_len);
SPECTRA_API void spectra_streaming_filter_destroy(SpectraStreamingFilter filter);
SPECTRA_API int spectra_streaming_filter_process(SpectraStreamingFilter filter,
                                                  const float* input, float* output, int count);
SPECTRA_API int spectra_streaming_filter_reset(SpectraStreamingFilter filter);

// ============================================================================
// Filter Application (Stateless)
// ============================================================================

SPECTRA_API int spectra_lfilter(const float* b, int b_len,
                                 const float* a, int a_len,
                                 const float* input, float* output, int len);

SPECTRA_API int spectra_filtfilt(const float* b, int b_len,
                                  const float* a, int a_len,
                                  const float* input, float* output, int len);

// ============================================================================
// Correlation
// ============================================================================

SPECTRA_API int spectra_xcorr(const float* x, int x_len,
                               const float* y, int y_len,
                               float* output, int* output_len);

SPECTRA_API int spectra_xcorr_output_size(int x_len, int y_len);

// ============================================================================
// Power Spectral Density
// ============================================================================

typedef struct {
    int segment_size;
    int overlap;
    SpectraWindowType window;
    int onesided;  // bool as int
} SpectraPWelchConfig;

SPECTRA_API int spectra_pwelch(const float* input, int len,
                                const SpectraPWelchConfig* config,
                                float sample_rate,
                                float* psd, float* frequencies, int* output_len);

SPECTRA_API int spectra_pwelch_output_size(const SpectraPWelchConfig* config);

// ============================================================================
// Hilbert Transform
// ============================================================================

SPECTRA_API int spectra_hilbert(const float* input,
                                 float* output_real, float* output_imag, int len);

SPECTRA_API int spectra_instantaneous_amplitude(const float* input, float* output, int len);
SPECTRA_API int spectra_instantaneous_phase(const float* input, float* output, int len);
SPECTRA_API int spectra_instantaneous_frequency(const float* input, float* output,
                                                  int len, float sample_rate);

// ============================================================================
// Resampling
// ============================================================================

SPECTRA_API int spectra_resample(const float* input, int input_len,
                                  int p, int q,
                                  float* output, int* output_len);

SPECTRA_API int spectra_resample_output_size(int input_len, int p, int q);

// ============================================================================
// Onset Detection
// ============================================================================

typedef void* SpectraOnsetDetector;

typedef struct {
    int fft_size;
    int hop_size;
    float threshold;
    float min_interval_ms;
} SpectraOnsetConfig;

typedef void (*SpectraOnsetCallback)(float timestamp, float strength, void* user_data);

SPECTRA_API SpectraOnsetDetector spectra_onset_create(const SpectraOnsetConfig* config,
                                                       float sample_rate);
SPECTRA_API void spectra_onset_destroy(SpectraOnsetDetector detector);
SPECTRA_API int spectra_onset_process(SpectraOnsetDetector detector,
                                        const float* samples, int count,
                                        SpectraOnsetCallback callback, void* user_data);
SPECTRA_API int spectra_onset_reset(SpectraOnsetDetector detector);

#ifdef __cplusplus
}
#endif

#endif // SPECTRA_ABI_H
```

## C# Bindings

### P/Invoke Declarations

```csharp
// SpectraNative.cs

using System;
using System.Runtime.InteropServices;

namespace Spectra
{
    internal static class SpectraNative
    {
        private const string LibraryName = "spectra";

        // Version
        [DllImport(LibraryName)] public static extern int spectra_version_major();
        [DllImport(LibraryName)] public static extern int spectra_version_minor();
        [DllImport(LibraryName)] public static extern int spectra_version_patch();

        // Error handling
        [DllImport(LibraryName)] public static extern int spectra_get_last_error();
        [DllImport(LibraryName)] public static extern IntPtr spectra_get_error_message();

        // FFT
        [DllImport(LibraryName)] public static extern IntPtr spectra_fft_create(int size);
        [DllImport(LibraryName)] public static extern void spectra_fft_destroy(IntPtr fft);
        [DllImport(LibraryName)] public static extern int spectra_fft_forward(
            IntPtr fft, float[] input, float[] outputReal, float[] outputImag);

        // Streaming Filter
        [DllImport(LibraryName)] public static extern IntPtr spectra_streaming_filter_create(
            float[] b, int bLen, float[] a, int aLen);
        [DllImport(LibraryName)] public static extern void spectra_streaming_filter_destroy(IntPtr filter);
        [DllImport(LibraryName)] public static extern int spectra_streaming_filter_process(
            IntPtr filter, float[] input, float[] output, int count);

        // ... additional bindings
    }
}
```

### SafeHandle Wrappers

```csharp
// FFTHandle.cs

using System;
using System.Runtime.InteropServices;

namespace Spectra
{
    internal class FFTHandle : SafeHandle
    {
        public FFTHandle() : base(IntPtr.Zero, true) { }

        public override bool IsInvalid => handle == IntPtr.Zero;

        protected override bool ReleaseHandle()
        {
            SpectraNative.spectra_fft_destroy(handle);
            return true;
        }
    }

    internal class StreamingFilterHandle : SafeHandle
    {
        public StreamingFilterHandle() : base(IntPtr.Zero, true) { }

        public override bool IsInvalid => handle == IntPtr.Zero;

        protected override bool ReleaseHandle()
        {
            SpectraNative.spectra_streaming_filter_destroy(handle);
            return true;
        }
    }
}
```

### High-Level API

```csharp
// DSP.cs - MATLAB-equivalent static API

using System;

namespace Spectra
{
    public static class DSP
    {
        /// <summary>
        /// Design a Butterworth filter.
        /// Equivalent to MATLAB: [b,a] = butter(order, Wn, type)
        /// </summary>
        public static (float[] b, float[] a) Butter(int order, float normalizedFreq,
                                                     FilterType type = FilterType.Lowpass)
        {
            int coeffSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[coeffSize];
            float[] a = new float[coeffSize];
            int bLen = coeffSize, aLen = coeffSize;

            int result = SpectraNative.spectra_butter(
                order, normalizedFreq, (int)type, b, ref bLen, a, ref aLen);

            if (result != 0)
                throw new SpectraException(SpectraNative.spectra_get_last_error());

            Array.Resize(ref b, bLen);
            Array.Resize(ref a, aLen);
            return (b, a);
        }

        /// <summary>
        /// Apply a digital filter to a signal.
        /// Equivalent to MATLAB: y = filter(b, a, x)
        /// </summary>
        public static float[] LFilter(float[] b, float[] a, float[] input)
        {
            float[] output = new float[input.Length];

            int result = SpectraNative.spectra_lfilter(
                b, b.Length, a, a.Length, input, output, input.Length);

            if (result != 0)
                throw new SpectraException(SpectraNative.spectra_get_last_error());

            return output;
        }

        /// <summary>
        /// Zero-phase digital filtering.
        /// Equivalent to MATLAB: y = filtfilt(b, a, x)
        /// </summary>
        public static float[] FiltFilt(float[] b, float[] a, float[] input)
        {
            float[] output = new float[input.Length];

            int result = SpectraNative.spectra_filtfilt(
                b, b.Length, a, a.Length, input, output, input.Length);

            if (result != 0)
                throw new SpectraException(SpectraNative.spectra_get_last_error());

            return output;
        }
    }
}
```

## Callback Pattern for Events

For onset detection and other event-driven functionality:

```c
// C ABI
typedef void (*SpectraOnsetCallback)(float timestamp, float strength, void* user_data);

SPECTRA_API int spectra_onset_process(SpectraOnsetDetector detector,
                                       const float* samples, int count,
                                       SpectraOnsetCallback callback, void* user_data);
```

```csharp
// C# binding
namespace Spectra
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void OnsetCallbackDelegate(float timestamp, float strength, IntPtr userData);

    public class OnsetDetector : IDisposable
    {
        private IntPtr _handle;
        private OnsetCallbackDelegate _nativeCallback;
        private GCHandle _callbackHandle;

        public event Action<float, float> OnOnsetDetected;

        public OnsetDetector(OnsetConfig config, float sampleRate)
        {
            // Pin the callback to prevent GC
            _nativeCallback = NativeCallback;
            _callbackHandle = GCHandle.Alloc(_nativeCallback);

            _handle = SpectraNative.spectra_onset_create(ref config, sampleRate);
        }

        public void Process(float[] samples)
        {
            SpectraNative.spectra_onset_process(
                _handle, samples, samples.Length, _nativeCallback, IntPtr.Zero);
        }

        private void NativeCallback(float timestamp, float strength, IntPtr userData)
        {
            OnOnsetDetected?.Invoke(timestamp, strength);
        }

        public void Dispose()
        {
            if (_handle != IntPtr.Zero)
            {
                SpectraNative.spectra_onset_destroy(_handle);
                _handle = IntPtr.Zero;
            }
            if (_callbackHandle.IsAllocated)
            {
                _callbackHandle.Free();
            }
        }
    }
}
```

## Versioning Strategy

The ABI uses semantic versioning:

- **Major**: Breaking ABI changes (handle layouts, removed functions)
- **Minor**: New functions added (backward compatible)
- **Patch**: Bug fixes only

Version checking at runtime:

```csharp
public static void EnsureCompatibleVersion()
{
    int major = SpectraNative.spectra_version_major();
    if (major != ExpectedMajorVersion)
    {
        throw new SpectraException(
            $"Incompatible native library version. Expected {ExpectedMajorVersion}.x.x, got {major}.x.x");
    }
}
```

## Thread Safety Contract

| Function Category | Thread Safety |
|-------------------|---------------|
| `spectra_*_create` | Not thread-safe (call from main thread) |
| `spectra_*_destroy` | Not thread-safe (call from main thread) |
| `spectra_*_process` | Thread-safe per instance (one thread per object) |
| `spectra_get_last_error` | Thread-safe (thread-local storage) |
| Stateless functions (`lfilter`, `xcorr`) | Thread-safe |

## Memory Ownership

| Pattern | Ownership |
|---------|-----------|
| Input arrays | Caller owns, library reads |
| Output arrays | Caller owns and allocates, library writes |
| Handles (`void*`) | Library owns internal memory, caller manages handle lifetime |
| Strings | Library owns (static or thread-local), caller must copy if needed |
