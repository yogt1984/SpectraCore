/**
 * @file hilbert.hpp
 * @brief Hilbert transform and analytic signal
 */

#ifndef SPECTRA_ANALYSIS_HILBERT_HPP
#define SPECTRA_ANALYSIS_HILBERT_HPP

#include "../core/types.hpp"
#include <vector>

namespace spectra {

/**
 * @brief Compute analytic signal using Hilbert transform
 *
 * Equivalent to MATLAB: z = hilbert(x)
 *
 * The analytic signal z = x + j*H(x) where H(x) is the Hilbert transform.
 * The real part is the original signal, imaginary part is the Hilbert transform.
 *
 * @param input Real input signal
 * @param output Complex analytic signal
 * @param len Signal length
 */
void hilbert(const float* input, Complex* output, size_t len);

/**
 * @brief Compute Hilbert transform with separate real/imag output
 *
 * @param input Real input signal
 * @param output_real Real part (copy of input)
 * @param output_imag Imaginary part (Hilbert transform)
 * @param len Signal length
 */
void hilbert(const float* input, float* output_real, float* output_imag, size_t len);

/**
 * @brief Compute instantaneous amplitude (envelope)
 *
 * Equivalent to MATLAB: amp = abs(hilbert(x))
 *
 * @param input Input signal
 * @param output Instantaneous amplitude
 * @param len Signal length
 */
void instantaneous_amplitude(const float* input, float* output, size_t len);

/**
 * @brief Compute instantaneous phase
 *
 * Equivalent to MATLAB: phase = angle(hilbert(x))
 *
 * @param input Input signal
 * @param output Instantaneous phase in radians (-pi to pi)
 * @param len Signal length
 */
void instantaneous_phase(const float* input, float* output, size_t len);

/**
 * @brief Compute unwrapped instantaneous phase
 *
 * Equivalent to MATLAB: phase = unwrap(angle(hilbert(x)))
 *
 * @param input Input signal
 * @param output Unwrapped instantaneous phase in radians
 * @param len Signal length
 */
void instantaneous_phase_unwrapped(const float* input, float* output, size_t len);

/**
 * @brief Compute instantaneous frequency
 *
 * Equivalent to MATLAB: freq = diff(unwrap(angle(hilbert(x)))) * fs / (2*pi)
 *
 * @param input Input signal
 * @param output Instantaneous frequency in Hz (length = len - 1)
 * @param len Signal length
 * @param sample_rate Sample rate in Hz
 */
void instantaneous_frequency(const float* input, float* output,
                             size_t len, float sample_rate);

/**
 * @brief Unwrap phase angles
 *
 * Equivalent to MATLAB: unwrap(phase)
 *
 * @param phase Input phase array (modified in-place)
 * @param len Array length
 */
void unwrap_phase(float* phase, size_t len);

/**
 * @brief Compute envelope using Hilbert transform (alias for instantaneous_amplitude)
 */
inline void envelope(const float* input, float* output, size_t len) {
    instantaneous_amplitude(input, output, len);
}

} // namespace spectra

#endif // SPECTRA_ANALYSIS_HILBERT_HPP
