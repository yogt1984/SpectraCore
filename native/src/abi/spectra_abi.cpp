/**
 * @file spectra_abi.cpp
 * @brief C ABI implementation
 */

#include "spectra_abi.h"
#include "spectra/spectra.hpp"
#include <cstring>

// Thread-local error state
thread_local SpectraError g_last_error = SPECTRA_OK;
thread_local char g_error_message[256] = "";

static void set_error(SpectraError error, const char* message) {
    g_last_error = error;
    strncpy(g_error_message, message, sizeof(g_error_message) - 1);
    g_error_message[sizeof(g_error_message) - 1] = '\0';
}

// Version
SPECTRA_API int spectra_version_major(void) { return 0; }
SPECTRA_API int spectra_version_minor(void) { return 1; }
SPECTRA_API int spectra_version_patch(void) { return 0; }
SPECTRA_API const char* spectra_version_string(void) { return "0.1.0"; }

// Error handling
SPECTRA_API SpectraError spectra_get_last_error(void) { return g_last_error; }
SPECTRA_API const char* spectra_get_error_message(void) { return g_error_message; }
SPECTRA_API void spectra_clear_error(void) {
    g_last_error = SPECTRA_OK;
    g_error_message[0] = '\0';
}

// FFT
SPECTRA_API SpectraFFT spectra_fft_create(int size) {
    try {
        return new spectra::FFT(static_cast<size_t>(size));
    } catch (...) {
        set_error(SPECTRA_ERROR_ALLOCATION_FAILED, "Failed to create FFT");
        return nullptr;
    }
}

SPECTRA_API void spectra_fft_destroy(SpectraFFT fft) {
    delete static_cast<spectra::FFT*>(fft);
}

SPECTRA_API int spectra_fft_forward(SpectraFFT fft,
                                     const float* input,
                                     float* output_real,
                                     float* output_imag) {
    if (!fft) {
        set_error(SPECTRA_ERROR_INVALID_HANDLE, "Invalid FFT handle");
        return -1;
    }
    auto* f = static_cast<spectra::FFT*>(fft);
    f->forward(input, output_real, output_imag);
    return 0;
}

SPECTRA_API int spectra_fft_inverse(SpectraFFT fft,
                                     const float* input_real,
                                     const float* input_imag,
                                     float* output) {
    if (!fft) {
        set_error(SPECTRA_ERROR_INVALID_HANDLE, "Invalid FFT handle");
        return -1;
    }
    auto* f = static_cast<spectra::FFT*>(fft);
    f->inverse(input_real, input_imag, output);
    return 0;
}

SPECTRA_API int spectra_fft_size(SpectraFFT fft) {
    if (!fft) return 0;
    return static_cast<int>(static_cast<spectra::FFT*>(fft)->size());
}

SPECTRA_API int spectra_fft_spectrum_size(SpectraFFT fft) {
    if (!fft) return 0;
    return static_cast<int>(static_cast<spectra::FFT*>(fft)->spectrum_size());
}

// Window functions
SPECTRA_API int spectra_generate_window(float* output, int size, SpectraWindowType type) {
    spectra::generate_window(output, static_cast<size_t>(size),
                             static_cast<spectra::WindowType>(type));
    return 0;
}

SPECTRA_API int spectra_apply_window(float* data, int size, SpectraWindowType type) {
    spectra::apply_window(data, static_cast<size_t>(size),
                          static_cast<spectra::WindowType>(type));
    return 0;
}

// STFT
SPECTRA_API SpectraSTFT spectra_stft_create(const SpectraSTFTConfig* config) {
    try {
        spectra::STFTConfig cfg;
        cfg.fft_size = config->fft_size;
        cfg.hop_size = config->hop_size;
        cfg.window = static_cast<spectra::WindowType>(config->window);
        cfg.center = config->center != 0;
        return new spectra::STFT(cfg);
    } catch (...) {
        set_error(SPECTRA_ERROR_ALLOCATION_FAILED, "Failed to create STFT");
        return nullptr;
    }
}

SPECTRA_API void spectra_stft_destroy(SpectraSTFT stft) {
    delete static_cast<spectra::STFT*>(stft);
}

SPECTRA_API int spectra_stft_analyze(SpectraSTFT stft,
                                      const float* input, int input_size,
                                      float* output_real, float* output_imag,
                                      int* num_frames) {
    if (!stft) {
        set_error(SPECTRA_ERROR_INVALID_HANDLE, "Invalid STFT handle");
        return -1;
    }
    auto* s = static_cast<spectra::STFT*>(stft);
    size_t frames;

    // Allocate temporary Complex array
    const size_t spectrum_size = s->spectrum_size();
    const size_t max_frames = 100;  // Reasonable upper bound
    std::vector<spectra::Complex> temp_output(max_frames * spectrum_size);

    s->analyze(input, input_size, temp_output.data(), &frames);
    *num_frames = static_cast<int>(frames);

    // Convert Complex to separate real/imag arrays
    for (size_t i = 0; i < frames * spectrum_size; ++i) {
        output_real[i] = temp_output[i].real;
        output_imag[i] = temp_output[i].imag;
    }

    return 0;
}

SPECTRA_API int spectra_stft_num_frames(int input_size, int fft_size, int hop_size) {
    // Same calculation as STFT::calc_num_frames
    if (input_size < fft_size) return 0;
    return 1 + (input_size - fft_size) / hop_size;
}

SPECTRA_API int spectra_stft_push_samples(SpectraSTFT stft, const float* samples, int count) {
    if (!stft) {
        set_error(SPECTRA_ERROR_INVALID_HANDLE, "Invalid STFT handle");
        return -1;
    }
    static_cast<spectra::STFT*>(stft)->push_samples(samples, count);
    return 0;
}

SPECTRA_API int spectra_stft_pop_frame(SpectraSTFT stft, float* real, float* imag) {
    if (!stft) {
        set_error(SPECTRA_ERROR_INVALID_HANDLE, "Invalid STFT handle");
        return -1;
    }

    auto* s = static_cast<spectra::STFT*>(stft);
    const size_t spectrum_size = s->spectrum_size();
    std::vector<spectra::Complex> temp_frame(spectrum_size);

    bool result = s->pop_frame(temp_frame.data());

    if (result) {
        // Convert Complex to separate real/imag arrays
        for (size_t i = 0; i < spectrum_size; ++i) {
            real[i] = temp_frame[i].real;
            imag[i] = temp_frame[i].imag;
        }
    }

    return result ? 0 : -1;
}

SPECTRA_API int spectra_stft_frames_available(SpectraSTFT stft) {
    if (!stft) return 0;
    return static_cast<int>(static_cast<spectra::STFT*>(stft)->frames_available());
}

// Streaming filter
SPECTRA_API SpectraStreamingFilter spectra_streaming_filter_create(
    const float* b, int b_len,
    const float* a, int a_len) {
    try {
        std::vector<float> bv(b, b + b_len);
        std::vector<float> av(a, a + a_len);
        return new spectra::StreamingIIRFilter(bv, av);
    } catch (...) {
        set_error(SPECTRA_ERROR_ALLOCATION_FAILED, "Failed to create filter");
        return nullptr;
    }
}

SPECTRA_API void spectra_streaming_filter_destroy(SpectraStreamingFilter filter) {
    delete static_cast<spectra::StreamingIIRFilter*>(filter);
}

SPECTRA_API int spectra_streaming_filter_process(SpectraStreamingFilter filter,
                                                  const float* input, float* output, int count) {
    if (!filter) {
        set_error(SPECTRA_ERROR_INVALID_HANDLE, "Invalid filter handle");
        return -1;
    }
    static_cast<spectra::StreamingIIRFilter*>(filter)->process(input, output, count);
    return 0;
}

SPECTRA_API int spectra_streaming_filter_reset(SpectraStreamingFilter filter) {
    if (!filter) {
        set_error(SPECTRA_ERROR_INVALID_HANDLE, "Invalid filter handle");
        return -1;
    }
    static_cast<spectra::StreamingIIRFilter*>(filter)->reset();
    return 0;
}

// Filter design
SPECTRA_API int spectra_butter(int order, float normalized_freq, SpectraFilterType type,
                                float* b, int* b_len, float* a, int* a_len) {
    auto coeffs = spectra::butter(order, normalized_freq,
                                   static_cast<spectra::FilterType>(type));
    *b_len = static_cast<int>(coeffs.b.size());
    *a_len = static_cast<int>(coeffs.a.size());
    std::memcpy(b, coeffs.b.data(), coeffs.b.size() * sizeof(float));
    std::memcpy(a, coeffs.a.data(), coeffs.a.size() * sizeof(float));
    return 0;
}

SPECTRA_API int spectra_cheby1(int order, float ripple_db, float normalized_freq,
                                SpectraFilterType type,
                                float* b, int* b_len, float* a, int* a_len) {
    auto coeffs = spectra::cheby1(order, ripple_db, normalized_freq,
                                   static_cast<spectra::FilterType>(type));
    *b_len = static_cast<int>(coeffs.b.size());
    *a_len = static_cast<int>(coeffs.a.size());
    std::memcpy(b, coeffs.b.data(), coeffs.b.size() * sizeof(float));
    std::memcpy(a, coeffs.a.data(), coeffs.a.size() * sizeof(float));
    return 0;
}

SPECTRA_API int spectra_cheby2(int order, float stopband_db, float normalized_freq,
                                SpectraFilterType type,
                                float* b, int* b_len, float* a, int* a_len) {
    auto coeffs = spectra::cheby2(order, stopband_db, normalized_freq,
                                   static_cast<spectra::FilterType>(type));
    *b_len = static_cast<int>(coeffs.b.size());
    *a_len = static_cast<int>(coeffs.a.size());
    std::memcpy(b, coeffs.b.data(), coeffs.b.size() * sizeof(float));
    std::memcpy(a, coeffs.a.data(), coeffs.a.size() * sizeof(float));
    return 0;
}

SPECTRA_API int spectra_ellip(int order, float passband_ripple_db, float stopband_db,
                               float normalized_freq, SpectraFilterType type,
                               float* b, int* b_len, float* a, int* a_len) {
    auto coeffs = spectra::ellip(order, passband_ripple_db, stopband_db, normalized_freq,
                                  static_cast<spectra::FilterType>(type));
    *b_len = static_cast<int>(coeffs.b.size());
    *a_len = static_cast<int>(coeffs.a.size());
    std::memcpy(b, coeffs.b.data(), coeffs.b.size() * sizeof(float));
    std::memcpy(a, coeffs.a.data(), coeffs.a.size() * sizeof(float));
    return 0;
}

SPECTRA_API int spectra_iir_coeff_size(int order) {
    return spectra::iir_coeff_size(order);
}

// Dual-frequency bandpass/bandstop filter design
SPECTRA_API int spectra_butter_bp(int order, float low_freq, float high_freq,
                                   SpectraFilterType type,
                                   float* b, int* b_len, float* a, int* a_len) {
    auto coeffs = spectra::butter(order, low_freq, high_freq,
                                   static_cast<spectra::FilterType>(type));
    *b_len = static_cast<int>(coeffs.b.size());
    *a_len = static_cast<int>(coeffs.a.size());
    std::memcpy(b, coeffs.b.data(), coeffs.b.size() * sizeof(float));
    std::memcpy(a, coeffs.a.data(), coeffs.a.size() * sizeof(float));
    return 0;
}

SPECTRA_API int spectra_cheby1_bp(int order, float ripple_db, float low_freq, float high_freq,
                                   SpectraFilterType type,
                                   float* b, int* b_len, float* a, int* a_len) {
    auto coeffs = spectra::cheby1(order, ripple_db, low_freq, high_freq,
                                   static_cast<spectra::FilterType>(type));
    *b_len = static_cast<int>(coeffs.b.size());
    *a_len = static_cast<int>(coeffs.a.size());
    std::memcpy(b, coeffs.b.data(), coeffs.b.size() * sizeof(float));
    std::memcpy(a, coeffs.a.data(), coeffs.a.size() * sizeof(float));
    return 0;
}

SPECTRA_API int spectra_cheby2_bp(int order, float stopband_db, float low_freq, float high_freq,
                                   SpectraFilterType type,
                                   float* b, int* b_len, float* a, int* a_len) {
    auto coeffs = spectra::cheby2(order, stopband_db, low_freq, high_freq,
                                   static_cast<spectra::FilterType>(type));
    *b_len = static_cast<int>(coeffs.b.size());
    *a_len = static_cast<int>(coeffs.a.size());
    std::memcpy(b, coeffs.b.data(), coeffs.b.size() * sizeof(float));
    std::memcpy(a, coeffs.a.data(), coeffs.a.size() * sizeof(float));
    return 0;
}

SPECTRA_API int spectra_ellip_bp(int order, float passband_ripple_db, float stopband_db,
                                  float low_freq, float high_freq, SpectraFilterType type,
                                  float* b, int* b_len, float* a, int* a_len) {
    auto coeffs = spectra::ellip(order, passband_ripple_db, stopband_db, low_freq, high_freq,
                                  static_cast<spectra::FilterType>(type));
    *b_len = static_cast<int>(coeffs.b.size());
    *a_len = static_cast<int>(coeffs.a.size());
    std::memcpy(b, coeffs.b.data(), coeffs.b.size() * sizeof(float));
    std::memcpy(a, coeffs.a.data(), coeffs.a.size() * sizeof(float));
    return 0;
}

// Filter application
SPECTRA_API int spectra_lfilter(const float* b, int b_len,
                                 const float* a, int a_len,
                                 const float* input, float* output, int len) {
    spectra::lfilter(b, b_len, a, a_len, input, output, len);
    return 0;
}

SPECTRA_API int spectra_filtfilt(const float* b, int b_len,
                                  const float* a, int a_len,
                                  const float* input, float* output, int len) {
    spectra::filtfilt(b, b_len, a, a_len, input, output, len);
    return 0;
}

// Filter Analysis
SPECTRA_API int spectra_freqz(const float* b, int b_len,
                               const float* a, int a_len,
                               float* magnitude, float* phase, int num_points) {
    // Create filter coefficients structure
    spectra::FilterCoefficients coeffs;
    coeffs.b.assign(b, b + b_len);
    coeffs.a.assign(a, a + a_len);

    // Allocate temporary complex array for frequency response
    std::vector<spectra::Complex> H(num_points);

    // Compute frequency response
    spectra::freqz(coeffs, H.data(), num_points);

    // Convert to magnitude and phase
    for (int i = 0; i < num_points; ++i) {
        magnitude[i] = std::sqrt(H[i].real * H[i].real + H[i].imag * H[i].imag);
        phase[i] = std::atan2(H[i].imag, H[i].real);
    }

    return 0;
}

// Correlation
SPECTRA_API int spectra_xcorr(const float* x, int x_len,
                               const float* y, int y_len,
                               float* output, int* output_len) {
    *output_len = static_cast<int>(spectra::xcorr_output_size(x_len, y_len));
    spectra::xcorr(x, x_len, y, y_len, output);
    return 0;
}

SPECTRA_API int spectra_xcorr_output_size(int x_len, int y_len) {
    return static_cast<int>(spectra::xcorr_output_size(x_len, y_len));
}

// Power Spectral Density
SPECTRA_API int spectra_pwelch(const float* input, int len,
                                const SpectraPWelchConfig* config,
                                float sample_rate,
                                float* psd, float* frequencies, int* output_len) {
    spectra::WelchConfig cfg;
    cfg.segment_size = config->segment_size;
    cfg.overlap = config->overlap;
    cfg.window = static_cast<spectra::WindowType>(config->window);
    cfg.onesided = config->onesided != 0;
    cfg.detrend = false;

    size_t out_len;
    spectra::pwelch(input, len, cfg, sample_rate, psd, frequencies, &out_len);
    *output_len = static_cast<int>(out_len);
    return 0;
}

SPECTRA_API int spectra_pwelch_output_size(const SpectraPWelchConfig* config) {
    spectra::WelchConfig cfg;
    cfg.segment_size = config->segment_size;
    cfg.onesided = config->onesided != 0;
    return static_cast<int>(spectra::pwelch_output_size(cfg));
}

// Hilbert
SPECTRA_API int spectra_hilbert(const float* input,
                                 float* output_real, float* output_imag, int len) {
    spectra::hilbert(input, output_real, output_imag, len);
    return 0;
}

SPECTRA_API int spectra_instantaneous_amplitude(const float* input, float* output, int len) {
    spectra::instantaneous_amplitude(input, output, len);
    return 0;
}

SPECTRA_API int spectra_instantaneous_phase(const float* input, float* output, int len) {
    spectra::instantaneous_phase(input, output, len);
    return 0;
}

SPECTRA_API int spectra_instantaneous_frequency(const float* input, float* output,
                                                  int len, float sample_rate) {
    spectra::instantaneous_frequency(input, output, len, sample_rate);
    return 0;
}

// Resampling
SPECTRA_API int spectra_resample(const float* input, int input_len,
                                  int p, int q,
                                  float* output, int* output_len) {
    size_t out_len;
    spectra::resample(input, input_len, p, q, output, &out_len);
    *output_len = static_cast<int>(out_len);
    return 0;
}

SPECTRA_API int spectra_resample_output_size(int input_len, int p, int q) {
    return static_cast<int>(spectra::resample_output_size(input_len, p, q));
}

// Onset detection
SPECTRA_API SpectraOnsetDetector spectra_onset_create(const SpectraOnsetConfig* config,
                                                       float sample_rate) {
    try {
        spectra::OnsetConfig cfg;
        cfg.fft_size = config->fft_size;
        cfg.hop_size = config->hop_size;
        cfg.threshold = config->threshold;
        cfg.min_interval_ms = config->min_interval_ms;
        return new spectra::OnsetDetector(cfg, sample_rate);
    } catch (...) {
        set_error(SPECTRA_ERROR_ALLOCATION_FAILED, "Failed to create onset detector");
        return nullptr;
    }
}

SPECTRA_API void spectra_onset_destroy(SpectraOnsetDetector detector) {
    delete static_cast<spectra::OnsetDetector*>(detector);
}

SPECTRA_API int spectra_onset_process(SpectraOnsetDetector detector,
                                       const float* samples, int count,
                                       SpectraOnsetCallback callback, void* user_data) {
    if (!detector) {
        set_error(SPECTRA_ERROR_INVALID_HANDLE, "Invalid onset detector handle");
        return -1;
    }

    auto* d = static_cast<spectra::OnsetDetector*>(detector);
    d->process(samples, count, [callback, user_data](float time, float strength) {
        if (callback) {
            callback(time, strength, user_data);
        }
    });
    return 0;
}

SPECTRA_API int spectra_onset_reset(SpectraOnsetDetector detector) {
    if (!detector) {
        set_error(SPECTRA_ERROR_INVALID_HANDLE, "Invalid onset detector handle");
        return -1;
    }
    static_cast<spectra::OnsetDetector*>(detector)->reset();
    return 0;
}
