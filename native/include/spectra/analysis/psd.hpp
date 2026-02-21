/**
 * @file psd.hpp
 * @brief Power Spectral Density estimation
 */

#ifndef SPECTRA_ANALYSIS_PSD_HPP
#define SPECTRA_ANALYSIS_PSD_HPP

#include "../core/types.hpp"
#include <vector>

namespace spectra {

/**
 * @brief Welch's method configuration
 */
struct WelchConfig {
    size_t segment_size = 256;              ///< Segment length (NFFT)
    size_t overlap = 0;                     ///< Overlap between segments (0 = 50%)
    WindowType window = WindowType::Hann;   ///< Window function
    bool onesided = true;                   ///< Return one-sided spectrum
    bool detrend = true;                    ///< Remove mean from each segment
};

/**
 * @brief Power Spectral Density using Welch's method
 *
 * Equivalent to MATLAB: [Pxx, F] = pwelch(x, window, overlap, nfft, fs)
 *
 * @param input Input signal
 * @param len Signal length
 * @param config Welch configuration
 * @param sample_rate Sample rate in Hz
 * @param psd Output PSD estimate
 * @param frequencies Output frequency vector
 * @param output_len Output: length of psd/frequencies arrays
 */
void pwelch(const float* input, size_t len,
            const WelchConfig& config,
            float sample_rate,
            float* psd, float* frequencies, size_t* output_len);

/**
 * @brief Get output size for pwelch
 */
size_t pwelch_output_size(const WelchConfig& config);

/**
 * @brief Periodogram (single-window PSD estimate)
 *
 * Equivalent to MATLAB: [Pxx, F] = periodogram(x, window, nfft, fs)
 *
 * @param input Input signal
 * @param len Signal length
 * @param window Window function
 * @param sample_rate Sample rate in Hz
 * @param psd Output PSD
 * @param frequencies Output frequencies
 */
void periodogram(const float* input, size_t len,
                 WindowType window,
                 float sample_rate,
                 float* psd, float* frequencies);

/**
 * @brief Cross Power Spectral Density
 *
 * Equivalent to MATLAB: [Pxy, F] = cpsd(x, y, window, overlap, nfft, fs)
 */
void cpsd(const float* x, const float* y, size_t len,
          const WelchConfig& config,
          float sample_rate,
          Complex* psd, float* frequencies, size_t* output_len);

/**
 * @brief Magnitude-Squared Coherence
 *
 * Equivalent to MATLAB: [Cxy, F] = mscohere(x, y, window, overlap, nfft, fs)
 *
 * @param x First signal
 * @param y Second signal
 * @param len Signal length
 * @param config Welch configuration
 * @param sample_rate Sample rate in Hz
 * @param coherence Output coherence (0 to 1)
 * @param frequencies Output frequencies
 * @param output_len Output length
 */
void mscohere(const float* x, const float* y, size_t len,
              const WelchConfig& config,
              float sample_rate,
              float* coherence, float* frequencies, size_t* output_len);

/**
 * @brief Spectrogram
 *
 * Equivalent to MATLAB: [S, F, T] = spectrogram(x, window, overlap, nfft, fs)
 *
 * @param input Input signal
 * @param len Signal length
 * @param config Configuration (same as Welch)
 * @param sample_rate Sample rate
 * @param spectrogram Output spectrogram (power, row-major: num_frames x num_freqs)
 * @param frequencies Output frequency vector
 * @param times Output time vector
 * @param num_frames Output: number of time frames
 * @param num_freqs Output: number of frequency bins
 */
void spectrogram(const float* input, size_t len,
                 const WelchConfig& config,
                 float sample_rate,
                 float* spectrogram,
                 float* frequencies, float* times,
                 size_t* num_frames, size_t* num_freqs);

/**
 * @brief Calculate number of frames for spectrogram
 */
size_t spectrogram_num_frames(size_t signal_len, size_t segment_size, size_t overlap);

} // namespace spectra

#endif // SPECTRA_ANALYSIS_PSD_HPP
