/**
 * @file filter_design.hpp
 * @brief IIR and FIR filter design functions
 */

#ifndef SPECTRA_FILTERS_FILTER_DESIGN_HPP
#define SPECTRA_FILTERS_FILTER_DESIGN_HPP

#include "../core/types.hpp"
#include <vector>

namespace spectra {

// ============================================================================
// IIR Filter Design
// ============================================================================

/**
 * @brief Design Butterworth filter
 *
 * Equivalent to MATLAB: [b,a] = butter(order, Wn, type)
 *
 * @param order Filter order
 * @param normalized_freq Normalized cutoff frequency (0 to 1, where 1 = Nyquist)
 * @param type Filter type (lowpass, highpass, bandpass, bandstop)
 * @return Filter coefficients
 */
FilterCoefficients butter(int order, float normalized_freq, FilterType type = FilterType::Lowpass);

/**
 * @brief Design Butterworth bandpass/bandstop filter
 * @param order Filter order
 * @param low_freq Lower normalized cutoff frequency
 * @param high_freq Upper normalized cutoff frequency
 * @param type Filter type (must be Bandpass or Bandstop)
 * @return Filter coefficients
 */
FilterCoefficients butter(int order, float low_freq, float high_freq, FilterType type);

/**
 * @brief Design Chebyshev Type I filter
 *
 * Chebyshev Type I filters have ripple in the passband and monotonic stopband.
 * Equivalent to MATLAB: [b,a] = cheby1(order, Rp, Wn, type)
 *
 * @param order Filter order
 * @param ripple_db Maximum passband ripple in dB
 * @param normalized_freq Normalized cutoff frequency
 * @param type Filter type
 * @return Filter coefficients
 */
FilterCoefficients cheby1(int order, float ripple_db, float normalized_freq,
                          FilterType type = FilterType::Lowpass);

/**
 * @brief Design Chebyshev Type II filter
 *
 * Chebyshev Type II filters have monotonic passband and ripple in stopband.
 * Equivalent to MATLAB: [b,a] = cheby2(order, Rs, Wn, type)
 *
 * @param order Filter order
 * @param stopband_db Minimum stopband attenuation in dB
 * @param normalized_freq Normalized cutoff frequency
 * @param type Filter type
 * @return Filter coefficients
 */
FilterCoefficients cheby2(int order, float stopband_db, float normalized_freq,
                          FilterType type = FilterType::Lowpass);

/**
 * @brief Design Elliptic (Cauer) filter
 *
 * Elliptic filters have the steepest rolloff but ripple in both bands.
 * Equivalent to MATLAB: [b,a] = ellip(order, Rp, Rs, Wn, type)
 *
 * @param order Filter order
 * @param passband_ripple_db Maximum passband ripple in dB
 * @param stopband_db Minimum stopband attenuation in dB
 * @param normalized_freq Normalized cutoff frequency
 * @param type Filter type
 * @return Filter coefficients
 */
FilterCoefficients ellip(int order, float passband_ripple_db, float stopband_db,
                         float normalized_freq, FilterType type = FilterType::Lowpass);

/**
 * @brief Design Bessel filter
 *
 * Bessel filters have maximally flat group delay (linear phase approximation).
 * Equivalent to MATLAB: [b,a] = besself(order, Wn) with bilinear transform
 *
 * @param order Filter order
 * @param normalized_freq Normalized cutoff frequency
 * @param type Filter type
 * @return Filter coefficients
 */
FilterCoefficients bessel(int order, float normalized_freq,
                          FilterType type = FilterType::Lowpass);

// ============================================================================
// FIR Filter Design
// ============================================================================

/**
 * @brief Design FIR filter using windowed sinc method
 *
 * @param num_taps Number of filter taps (filter length)
 * @param normalized_freq Normalized cutoff frequency (0 to 1)
 * @param type Filter type
 * @param window Window function for design
 * @return FIR filter coefficients
 */
std::vector<float> fir_windowed_sinc(int num_taps, float normalized_freq,
                                      FilterType type = FilterType::Lowpass,
                                      WindowType window = WindowType::Hamming);

/**
 * @brief Design FIR bandpass filter using windowed sinc
 */
std::vector<float> fir_windowed_sinc(int num_taps, float low_freq, float high_freq,
                                      FilterType type, WindowType window = WindowType::Hamming);

/**
 * @brief Design FIR filter using Parks-McClellan (Remez) algorithm
 *
 * Equivalent to MATLAB: b = firpm(order, f, a, w)
 *
 * @param order Filter order (number of taps = order + 1)
 * @param freq_bands Frequency band edges (normalized, 0 to 1)
 * @param amplitudes Desired amplitude at each band edge
 * @param weights Weight for each band (optional)
 * @return FIR filter coefficients
 */
std::vector<float> firpm(int order,
                         const std::vector<float>& freq_bands,
                         const std::vector<float>& amplitudes,
                         const std::vector<float>& weights = {});

// ============================================================================
// Filter Analysis
// ============================================================================

/**
 * @brief Compute frequency response of a filter
 *
 * Equivalent to MATLAB: [H, W] = freqz(b, a, nfft)
 *
 * @param coeffs Filter coefficients
 * @param H Output complex frequency response
 * @param num_points Number of frequency points
 */
void freqz(const FilterCoefficients& coeffs, Complex* H, size_t num_points);

/**
 * @brief Compute frequency response of FIR filter
 */
void freqz(const std::vector<float>& b, Complex* H, size_t num_points);

/**
 * @brief Compute group delay of a filter
 *
 * Equivalent to MATLAB: [gd, W] = grpdelay(b, a, nfft)
 *
 * @param coeffs Filter coefficients
 * @param gd Output group delay (in samples)
 * @param num_points Number of frequency points
 */
void grpdelay(const FilterCoefficients& coeffs, float* gd, size_t num_points);

/**
 * @brief Check if filter is stable (all poles inside unit circle)
 */
bool is_stable(const FilterCoefficients& coeffs);

/**
 * @brief Get required coefficient array size for IIR filter of given order
 */
inline int iir_coeff_size(int order) {
    return order + 1;
}

} // namespace spectra

#endif // SPECTRA_FILTERS_FILTER_DESIGN_HPP
