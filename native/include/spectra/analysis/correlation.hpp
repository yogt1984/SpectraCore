/**
 * @file correlation.hpp
 * @brief Cross-correlation and autocorrelation functions
 */

#ifndef SPECTRA_ANALYSIS_CORRELATION_HPP
#define SPECTRA_ANALYSIS_CORRELATION_HPP

#include "../core/types.hpp"
#include <vector>

namespace spectra {

/**
 * @brief Cross-correlation of two signals
 *
 * Equivalent to MATLAB: r = xcorr(x, y)
 * Output length = x_len + y_len - 1
 *
 * @param x First signal
 * @param x_len Length of first signal
 * @param y Second signal
 * @param y_len Length of second signal
 * @param output Output correlation (length = x_len + y_len - 1)
 */
void xcorr(const float* x, size_t x_len,
           const float* y, size_t y_len,
           float* output);

/**
 * @brief Autocorrelation of a signal
 *
 * Equivalent to MATLAB: r = xcorr(x)
 *
 * @param x Input signal
 * @param len Signal length
 * @param output Output autocorrelation (length = 2*len - 1)
 */
void xcorr(const float* x, size_t len, float* output);

/**
 * @brief Cross-correlation with maximum lag limit
 *
 * Equivalent to MATLAB: r = xcorr(x, y, maxlag)
 *
 * @param x First signal
 * @param x_len Length of first signal
 * @param y Second signal
 * @param y_len Length of second signal
 * @param output Output correlation (length = 2*maxlag + 1)
 * @param maxlag Maximum lag
 */
void xcorr(const float* x, size_t x_len,
           const float* y, size_t y_len,
           float* output, size_t maxlag);

/**
 * @brief Normalized cross-correlation
 *
 * Equivalent to MATLAB: r = xcorr(x, y, 'normalized')
 * Output is scaled to [-1, 1]
 */
void xcorr_normalized(const float* x, size_t x_len,
                      const float* y, size_t y_len,
                      float* output);

/**
 * @brief Biased cross-correlation
 *
 * Equivalent to MATLAB: r = xcorr(x, y, 'biased')
 */
void xcorr_biased(const float* x, size_t x_len,
                  const float* y, size_t y_len,
                  float* output);

/**
 * @brief Unbiased cross-correlation
 *
 * Equivalent to MATLAB: r = xcorr(x, y, 'unbiased')
 */
void xcorr_unbiased(const float* x, size_t x_len,
                    const float* y, size_t y_len,
                    float* output);

/**
 * @brief Get output size for xcorr
 */
inline size_t xcorr_output_size(size_t x_len, size_t y_len) {
    return x_len + y_len - 1;
}

/**
 * @brief Find lag of maximum correlation
 *
 * @param correlation Correlation output from xcorr
 * @param corr_len Correlation length
 * @param ref_len Length of reference signal (for lag calculation)
 * @return Lag at maximum correlation
 */
int find_max_correlation_lag(const float* correlation, size_t corr_len, size_t ref_len);

/**
 * @brief Compute correlation coefficient (Pearson)
 *
 * @param x First signal
 * @param y Second signal
 * @param len Signal length (must be same for both)
 * @return Correlation coefficient in [-1, 1]
 */
float corrcoef(const float* x, const float* y, size_t len);

} // namespace spectra

#endif // SPECTRA_ANALYSIS_CORRELATION_HPP
