/**
 * @file window_functions.hpp
 * @brief Window function generation and application
 */

#ifndef SPECTRA_CORE_WINDOW_FUNCTIONS_HPP
#define SPECTRA_CORE_WINDOW_FUNCTIONS_HPP

#include "types.hpp"
#include <vector>

namespace spectra {

/**
 * @brief Generate a window function
 * @param size Window size in samples
 * @param type Type of window function
 * @return Vector containing window coefficients
 */
std::vector<float> generate_window(size_t size, WindowType type);

/**
 * @brief Generate window function into pre-allocated buffer
 * @param output Output buffer (must have at least 'size' elements)
 * @param size Window size in samples
 * @param type Type of window function
 */
void generate_window(float* output, size_t size, WindowType type);

/**
 * @brief Apply window function to signal in-place
 * @param data Signal data (modified in-place)
 * @param size Signal size in samples
 * @param type Type of window function
 */
void apply_window(float* data, size_t size, WindowType type);

/**
 * @brief Apply pre-computed window to signal in-place
 * @param data Signal data (modified in-place)
 * @param window Pre-computed window coefficients
 * @param size Signal size in samples
 */
void apply_window(float* data, const float* window, size_t size);

/**
 * @brief Get coherent gain of window function
 *
 * Coherent gain is the sum of window coefficients normalized by window length.
 * Used for amplitude correction in spectral analysis.
 *
 * @param type Type of window function
 * @return Coherent gain factor
 */
float window_coherent_gain(WindowType type);

/**
 * @brief Get equivalent noise bandwidth of window function
 *
 * ENBW is the bandwidth of a rectangular window with the same peak power
 * and same noise power as the specified window.
 *
 * @param type Type of window function
 * @return ENBW in bins
 */
float window_enbw(WindowType type);

} // namespace spectra

#endif // SPECTRA_CORE_WINDOW_FUNCTIONS_HPP
