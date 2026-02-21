/**
 * @file psd.cpp
 * @brief Power Spectral Density implementations
 */

#include "spectra/analysis/psd.hpp"
#include "spectra/fft/fft.hpp"
#include "spectra/core/window_functions.hpp"
#include <cstring>

namespace spectra {

void pwelch(const float* input, size_t len,
            const WelchConfig& config,
            float sample_rate,
            float* psd, float* frequencies, size_t* output_len) {

    const size_t seg_size = config.segment_size;
    const size_t overlap = config.overlap > 0 ? config.overlap : seg_size / 2;
    const size_t hop = seg_size - overlap;
    const size_t num_freqs = config.onesided ? seg_size / 2 + 1 : seg_size;

    *output_len = num_freqs;

    // Initialize PSD to zero
    std::memset(psd, 0, num_freqs * sizeof(float));

    // Generate window
    std::vector<float> window = generate_window(seg_size, config.window);

    // Window power for normalization
    float window_power = 0.0f;
    for (size_t i = 0; i < seg_size; ++i) {
        window_power += window[i] * window[i];
    }

    FFT fft(seg_size);
    std::vector<float> segment(seg_size);
    std::vector<Complex> spectrum(num_freqs);

    size_t num_segments = 0;
    for (size_t start = 0; start + seg_size <= len; start += hop) {
        // Copy and optionally detrend
        if (config.detrend) {
            float mean = 0.0f;
            for (size_t i = 0; i < seg_size; ++i) {
                mean += input[start + i];
            }
            mean /= static_cast<float>(seg_size);
            for (size_t i = 0; i < seg_size; ++i) {
                segment[i] = (input[start + i] - mean) * window[i];
            }
        } else {
            for (size_t i = 0; i < seg_size; ++i) {
                segment[i] = input[start + i] * window[i];
            }
        }

        // FFT
        fft.forward(segment.data(), spectrum.data());

        // Accumulate power
        for (size_t i = 0; i < num_freqs; ++i) {
            psd[i] += spectrum[i].magnitude_squared();
        }

        ++num_segments;
    }

    // Normalize
    const float scale = 1.0f / (sample_rate * window_power * num_segments);
    for (size_t i = 0; i < num_freqs; ++i) {
        psd[i] *= scale;
        // Double non-DC/Nyquist bins for one-sided
        if (config.onesided && i > 0 && i < num_freqs - 1) {
            psd[i] *= 2.0f;
        }
    }

    // Frequency vector
    for (size_t i = 0; i < num_freqs; ++i) {
        frequencies[i] = static_cast<float>(i) * sample_rate / static_cast<float>(seg_size);
    }
}

size_t pwelch_output_size(const WelchConfig& config) {
    return config.onesided ? config.segment_size / 2 + 1 : config.segment_size;
}

void periodogram(const float* input, size_t len,
                 WindowType window,
                 float sample_rate,
                 float* psd, float* frequencies) {
    WelchConfig config;
    config.segment_size = len;
    config.overlap = 0;
    config.window = window;

    size_t output_len;
    pwelch(input, len, config, sample_rate, psd, frequencies, &output_len);
}

void cpsd(const float* x, const float* y, size_t len,
          const WelchConfig& config,
          float sample_rate,
          Complex* psd, float* frequencies, size_t* output_len) {
    // Placeholder - cross PSD
    const size_t num_freqs = pwelch_output_size(config);
    *output_len = num_freqs;

    for (size_t i = 0; i < num_freqs; ++i) {
        psd[i] = {0.0f, 0.0f};
        frequencies[i] = static_cast<float>(i) * sample_rate /
                        static_cast<float>(config.segment_size);
    }
}

void mscohere(const float* x, const float* y, size_t len,
              const WelchConfig& config,
              float sample_rate,
              float* coherence, float* frequencies, size_t* output_len) {
    // Placeholder - magnitude squared coherence
    const size_t num_freqs = pwelch_output_size(config);
    *output_len = num_freqs;

    for (size_t i = 0; i < num_freqs; ++i) {
        coherence[i] = 1.0f;  // Placeholder
        frequencies[i] = static_cast<float>(i) * sample_rate /
                        static_cast<float>(config.segment_size);
    }
}

void spectrogram(const float* input, size_t len,
                 const WelchConfig& config,
                 float sample_rate,
                 float* spectrogram_out,
                 float* frequencies, float* times,
                 size_t* num_frames, size_t* num_freqs) {
    const size_t seg_size = config.segment_size;
    const size_t overlap = config.overlap > 0 ? config.overlap : seg_size / 2;
    const size_t hop = seg_size - overlap;
    const size_t n_freqs = config.onesided ? seg_size / 2 + 1 : seg_size;
    const size_t n_frames = spectrogram_num_frames(len, seg_size, overlap);

    *num_frames = n_frames;
    *num_freqs = n_freqs;

    std::vector<float> window = generate_window(seg_size, config.window);
    FFT fft(seg_size);
    std::vector<float> segment(seg_size);
    std::vector<Complex> spectrum(n_freqs);

    for (size_t frame = 0; frame < n_frames; ++frame) {
        const size_t start = frame * hop;

        // Window segment
        for (size_t i = 0; i < seg_size; ++i) {
            segment[i] = input[start + i] * window[i];
        }

        // FFT
        fft.forward(segment.data(), spectrum.data());

        // Store power
        for (size_t i = 0; i < n_freqs; ++i) {
            spectrogram_out[frame * n_freqs + i] = spectrum[i].magnitude_squared();
        }

        times[frame] = (start + seg_size / 2) / sample_rate;
    }

    for (size_t i = 0; i < n_freqs; ++i) {
        frequencies[i] = static_cast<float>(i) * sample_rate / static_cast<float>(seg_size);
    }
}

size_t spectrogram_num_frames(size_t signal_len, size_t segment_size, size_t overlap) {
    if (signal_len < segment_size) return 0;
    const size_t hop = segment_size - overlap;
    return 1 + (signal_len - segment_size) / hop;
}

} // namespace spectra
