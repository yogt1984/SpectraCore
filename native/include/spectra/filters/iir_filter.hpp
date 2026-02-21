/**
 * @file iir_filter.hpp
 * @brief IIR filter implementation
 */

#ifndef SPECTRA_FILTERS_IIR_FILTER_HPP
#define SPECTRA_FILTERS_IIR_FILTER_HPP

#include "../core/types.hpp"
#include <vector>
#include <memory>

namespace spectra {

/**
 * @brief Apply IIR filter to signal (stateless, causal)
 *
 * Implements Direct Form II Transposed structure.
 * Equivalent to MATLAB: y = filter(b, a, x)
 *
 * @param b Numerator (feedforward) coefficients
 * @param b_len Number of b coefficients
 * @param a Denominator (feedback) coefficients (a[0] should be 1.0)
 * @param a_len Number of a coefficients
 * @param input Input signal
 * @param output Output signal
 * @param len Signal length
 */
void lfilter(const float* b, size_t b_len,
             const float* a, size_t a_len,
             const float* input, float* output, size_t len);

/**
 * @brief Apply IIR filter with initial conditions
 *
 * @param b Numerator coefficients
 * @param b_len Number of b coefficients
 * @param a Denominator coefficients
 * @param a_len Number of a coefficients
 * @param input Input signal
 * @param output Output signal
 * @param len Signal length
 * @param zi Initial conditions (length = max(b_len, a_len) - 1)
 * @param zf Final conditions output (same length as zi, can be nullptr)
 */
void lfilter_zi(const float* b, size_t b_len,
                const float* a, size_t a_len,
                const float* input, float* output, size_t len,
                const float* zi, float* zf);

/**
 * @brief Zero-phase IIR filtering (forward-backward)
 *
 * Applies filter forward then backward to eliminate phase distortion.
 * Equivalent to MATLAB: y = filtfilt(b, a, x)
 *
 * @note This function is for offline processing only, not real-time.
 *
 * @param b Numerator coefficients
 * @param b_len Number of b coefficients
 * @param a Denominator coefficients
 * @param a_len Number of a coefficients
 * @param input Input signal
 * @param output Output signal
 * @param len Signal length
 */
void filtfilt(const float* b, size_t b_len,
              const float* a, size_t a_len,
              const float* input, float* output, size_t len);

/**
 * @brief Compute initial conditions for filter to start in steady state
 *
 * Equivalent to MATLAB: zi = filtic(b, a, y, x) or scipy.signal.lfilter_zi
 *
 * @param b Numerator coefficients
 * @param b_len Number of b coefficients
 * @param a Denominator coefficients
 * @param a_len Number of a coefficients
 * @param zi Output initial conditions
 */
void lfilter_ic(const float* b, size_t b_len,
                const float* a, size_t a_len,
                float* zi);

} // namespace spectra

#endif // SPECTRA_FILTERS_IIR_FILTER_HPP
