/**
 * @file resample.hpp
 * @brief Signal resampling functions
 */

#ifndef SPECTRA_ANALYSIS_RESAMPLE_HPP
#define SPECTRA_ANALYSIS_RESAMPLE_HPP

#include "../core/types.hpp"
#include <vector>

namespace spectra {

/**
 * @brief Resample signal by rational factor p/q
 *
 * Equivalent to MATLAB: y = resample(x, p, q)
 *
 * Uses polyphase FIR filter for anti-aliasing.
 * Output length = ceil(input_len * p / q)
 *
 * @param input Input signal
 * @param input_len Input length
 * @param p Upsampling factor
 * @param q Downsampling factor
 * @param output Output signal
 * @param output_len Output: actual output length
 */
void resample(const float* input, size_t input_len,
              int p, int q,
              float* output, size_t* output_len);

/**
 * @brief Get output length for resample operation
 */
inline size_t resample_output_size(size_t input_len, int p, int q) {
    return (input_len * p + q - 1) / q;
}

/**
 * @brief Upsample by integer factor (insert zeros)
 *
 * Equivalent to MATLAB: y = upsample(x, n)
 *
 * @param input Input signal
 * @param input_len Input length
 * @param factor Upsampling factor
 * @param output Output signal (length = input_len * factor)
 */
void upsample(const float* input, size_t input_len,
              int factor, float* output);

/**
 * @brief Downsample by integer factor (keep every nth sample)
 *
 * Equivalent to MATLAB: y = downsample(x, n)
 *
 * @note Does not include anti-aliasing filter. Use decimate() for filtered downsampling.
 *
 * @param input Input signal
 * @param input_len Input length
 * @param factor Downsampling factor
 * @param output Output signal (length = ceil(input_len / factor))
 * @param phase Phase offset (0 to factor-1)
 */
void downsample(const float* input, size_t input_len,
                int factor, float* output, int phase = 0);

/**
 * @brief Decimate signal (lowpass filter + downsample)
 *
 * Equivalent to MATLAB: y = decimate(x, r)
 *
 * Applies anti-aliasing filter before downsampling.
 *
 * @param input Input signal
 * @param input_len Input length
 * @param factor Decimation factor
 * @param output Output signal
 * @param output_len Output: actual output length
 */
void decimate(const float* input, size_t input_len,
              int factor,
              float* output, size_t* output_len);

/**
 * @brief Interpolate signal (upsample + lowpass filter)
 *
 * Equivalent to MATLAB: y = interp(x, r)
 *
 * @param input Input signal
 * @param input_len Input length
 * @param factor Interpolation factor
 * @param output Output signal
 */
void interp(const float* input, size_t input_len,
            int factor, float* output);

/**
 * @brief Linear interpolation resampling
 *
 * Simple linear interpolation, faster but lower quality than polyphase.
 *
 * @param input Input signal
 * @param input_len Input length
 * @param output Output signal
 * @param output_len Desired output length
 */
void resample_linear(const float* input, size_t input_len,
                     float* output, size_t output_len);

/**
 * @brief Cubic interpolation resampling
 *
 * Higher quality than linear, still faster than polyphase.
 */
void resample_cubic(const float* input, size_t input_len,
                    float* output, size_t output_len);

} // namespace spectra

#endif // SPECTRA_ANALYSIS_RESAMPLE_HPP
