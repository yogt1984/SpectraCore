/**
 * @file fft.hpp
 * @brief Fast Fourier Transform wrapper
 */

#ifndef SPECTRA_FFT_FFT_HPP
#define SPECTRA_FFT_FFT_HPP

#include "../core/types.hpp"
#include <memory>
#include <vector>

namespace spectra {

/**
 * @brief FFT processor wrapping KissFFT
 *
 * Provides real-to-complex and complex-to-real FFT operations.
 */
class FFT {
public:
    /**
     * @brief Construct FFT processor
     * @param size FFT size (should be power of 2 for best performance)
     */
    explicit FFT(size_t size);
    ~FFT();

    // Non-copyable
    FFT(const FFT&) = delete;
    FFT& operator=(const FFT&) = delete;

    // Movable
    FFT(FFT&& other) noexcept;
    FFT& operator=(FFT&& other) noexcept;

    /**
     * @brief Compute forward FFT (real to complex)
     * @param input Real input signal (size() elements)
     * @param output Complex output spectrum (spectrum_size() elements)
     */
    void forward(const float* input, Complex* output);

    /**
     * @brief Compute inverse FFT (complex to real)
     * @param input Complex input spectrum (spectrum_size() elements)
     * @param output Real output signal (size() elements)
     */
    void inverse(const Complex* input, float* output);

    /**
     * @brief Compute forward FFT with separate real/imag output
     * @param input Real input signal
     * @param output_real Real part of spectrum
     * @param output_imag Imaginary part of spectrum
     */
    void forward(const float* input, float* output_real, float* output_imag);

    /**
     * @brief Compute inverse FFT from separate real/imag input
     * @param input_real Real part of spectrum
     * @param input_imag Imaginary part of spectrum
     * @param output Real output signal
     */
    void inverse(const float* input_real, const float* input_imag, float* output);

    /**
     * @brief Compute magnitude spectrum
     * @param input Real input signal
     * @param output Magnitude spectrum (spectrum_size() elements)
     */
    void magnitude_spectrum(const float* input, float* output);

    /**
     * @brief Compute power spectrum (magnitude squared)
     * @param input Real input signal
     * @param output Power spectrum (spectrum_size() elements)
     */
    void power_spectrum(const float* input, float* output);

    /**
     * @brief Get FFT size
     */
    [[nodiscard]] size_t size() const { return size_; }

    /**
     * @brief Get spectrum size (N/2 + 1 for real FFT)
     */
    [[nodiscard]] size_t spectrum_size() const { return size_ / 2 + 1; }

private:
    size_t size_;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Compute magnitude from complex spectrum
 * @param spectrum Complex spectrum
 * @param magnitude Output magnitude array
 * @param size Number of elements
 */
void compute_magnitude(const Complex* spectrum, float* magnitude, size_t size);

/**
 * @brief Compute phase from complex spectrum
 * @param spectrum Complex spectrum
 * @param phase Output phase array (radians)
 * @param size Number of elements
 */
void compute_phase(const Complex* spectrum, float* phase, size_t size);

/**
 * @brief Convert magnitude to decibels
 * @param magnitude Input magnitude array
 * @param db Output dB array
 * @param size Number of elements
 * @param ref_value Reference value for 0 dB (default 1.0)
 */
void magnitude_to_db(const float* magnitude, float* db, size_t size, float ref_value = 1.0f);

} // namespace spectra

#endif // SPECTRA_FFT_FFT_HPP
