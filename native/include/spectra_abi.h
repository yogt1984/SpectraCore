/**
 * @file spectra_abi.h
 * @brief C ABI for SpectraCore DSP library
 *
 * This header provides the C interface for cross-language interoperability.
 * All functions use opaque handles and C-compatible types.
 */

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

/* ============================================================================
 * Version
 * ============================================================================ */

SPECTRA_API int spectra_version_major(void);
SPECTRA_API int spectra_version_minor(void);
SPECTRA_API int spectra_version_patch(void);
SPECTRA_API const char* spectra_version_string(void);

/* ============================================================================
 * Error Handling
 * ============================================================================ */

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

/* ============================================================================
 * FFT
 * ============================================================================ */

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

/* ============================================================================
 * Window Functions
 * ============================================================================ */

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

/* ============================================================================
 * STFT
 * ============================================================================ */

typedef void* SpectraSTFT;

typedef struct {
    int fft_size;
    int hop_size;
    SpectraWindowType window;
    int center;
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

/* ============================================================================
 * Filter Design
 * ============================================================================ */

typedef enum {
    SPECTRA_FILTER_LOWPASS = 0,
    SPECTRA_FILTER_HIGHPASS = 1,
    SPECTRA_FILTER_BANDPASS = 2,
    SPECTRA_FILTER_BANDSTOP = 3
} SpectraFilterType;

SPECTRA_API int spectra_butter(int order, float normalized_freq, SpectraFilterType type,
                                float* b, int* b_len, float* a, int* a_len);

SPECTRA_API int spectra_cheby1(int order, float ripple_db, float normalized_freq,
                                SpectraFilterType type,
                                float* b, int* b_len, float* a, int* a_len);

SPECTRA_API int spectra_cheby2(int order, float stopband_db, float normalized_freq,
                                SpectraFilterType type,
                                float* b, int* b_len, float* a, int* a_len);

SPECTRA_API int spectra_ellip(int order, float passband_ripple_db, float stopband_db,
                               float normalized_freq, SpectraFilterType type,
                               float* b, int* b_len, float* a, int* a_len);

/* Dual-frequency overloads for bandpass/bandstop filters */

SPECTRA_API int spectra_butter_bp(int order, float low_freq, float high_freq,
                                   SpectraFilterType type,
                                   float* b, int* b_len, float* a, int* a_len);

SPECTRA_API int spectra_cheby1_bp(int order, float ripple_db, float low_freq, float high_freq,
                                   SpectraFilterType type,
                                   float* b, int* b_len, float* a, int* a_len);

SPECTRA_API int spectra_cheby2_bp(int order, float stopband_db, float low_freq, float high_freq,
                                   SpectraFilterType type,
                                   float* b, int* b_len, float* a, int* a_len);

SPECTRA_API int spectra_ellip_bp(int order, float passband_ripple_db, float stopband_db,
                                  float low_freq, float high_freq, SpectraFilterType type,
                                  float* b, int* b_len, float* a, int* a_len);

SPECTRA_API int spectra_iir_coeff_size(int order);

/* ============================================================================
 * Streaming Filter
 * ============================================================================ */

typedef void* SpectraStreamingFilter;

SPECTRA_API SpectraStreamingFilter spectra_streaming_filter_create(
    const float* b, int b_len,
    const float* a, int a_len);
SPECTRA_API void spectra_streaming_filter_destroy(SpectraStreamingFilter filter);
SPECTRA_API int spectra_streaming_filter_process(SpectraStreamingFilter filter,
                                                  const float* input, float* output, int count);
SPECTRA_API int spectra_streaming_filter_reset(SpectraStreamingFilter filter);

/* ============================================================================
 * Filter Application (Stateless)
 * ============================================================================ */

SPECTRA_API int spectra_lfilter(const float* b, int b_len,
                                 const float* a, int a_len,
                                 const float* input, float* output, int len);

SPECTRA_API int spectra_filtfilt(const float* b, int b_len,
                                  const float* a, int a_len,
                                  const float* input, float* output, int len);

/* ============================================================================
 * Filter Analysis
 * ============================================================================ */

/**
 * @brief Compute frequency response of a filter
 * Equivalent to MATLAB: [H, W] = freqz(b, a, nfft)
 *
 * @param b Numerator coefficients
 * @param b_len Length of b array
 * @param a Denominator coefficients
 * @param a_len Length of a array
 * @param magnitude Output magnitude response
 * @param phase Output phase response (radians)
 * @param num_points Number of frequency points
 * @return 0 on success, negative on error
 */
SPECTRA_API int spectra_freqz(const float* b, int b_len,
                               const float* a, int a_len,
                               float* magnitude, float* phase, int num_points);

/* ============================================================================
 * Correlation
 * ============================================================================ */

SPECTRA_API int spectra_xcorr(const float* x, int x_len,
                               const float* y, int y_len,
                               float* output, int* output_len);

SPECTRA_API int spectra_xcorr_output_size(int x_len, int y_len);

/* ============================================================================
 * Power Spectral Density
 * ============================================================================ */

typedef struct {
    int segment_size;
    int overlap;
    SpectraWindowType window;
    int onesided;
} SpectraPWelchConfig;

SPECTRA_API int spectra_pwelch(const float* input, int len,
                                const SpectraPWelchConfig* config,
                                float sample_rate,
                                float* psd, float* frequencies, int* output_len);

SPECTRA_API int spectra_pwelch_output_size(const SpectraPWelchConfig* config);

/* ============================================================================
 * Hilbert Transform
 * ============================================================================ */

SPECTRA_API int spectra_hilbert(const float* input,
                                 float* output_real, float* output_imag, int len);

SPECTRA_API int spectra_instantaneous_amplitude(const float* input, float* output, int len);
SPECTRA_API int spectra_instantaneous_phase(const float* input, float* output, int len);
SPECTRA_API int spectra_instantaneous_frequency(const float* input, float* output,
                                                  int len, float sample_rate);

/* ============================================================================
 * Resampling
 * ============================================================================ */

SPECTRA_API int spectra_resample(const float* input, int input_len,
                                  int p, int q,
                                  float* output, int* output_len);

SPECTRA_API int spectra_resample_output_size(int input_len, int p, int q);

/* ============================================================================
 * Onset Detection
 * ============================================================================ */

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

#endif /* SPECTRA_ABI_H */
