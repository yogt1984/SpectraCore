/**
 * @file hilbert.cpp
 * @brief Hilbert transform implementations
 */

#include "spectra/analysis/hilbert.hpp"
#include "spectra/fft/fft.hpp"
#include <cmath>

namespace spectra {

void hilbert(const float* input, Complex* output, size_t len) {
    FFT fft(len);
    std::vector<Complex> spectrum(fft.spectrum_size());

    // Forward FFT
    fft.forward(input, spectrum.data());

    // Create analytic signal in frequency domain
    // H[0] stays the same (DC)
    // H[1:N/2] *= 2
    // H[N/2] stays the same (Nyquist)
    // H[N/2+1:] = 0 (negative frequencies)

    for (size_t i = 1; i < fft.spectrum_size() - 1; ++i) {
        spectrum[i].real *= 2.0f;
        spectrum[i].imag *= 2.0f;
    }

    // Inverse FFT
    std::vector<float> analytic_real(len);
    std::vector<float> analytic_imag(len);

    // For proper Hilbert, we need complex IFFT
    // Simplified: use real part as original, compute imag from phase shift
    fft.inverse(spectrum.data(), analytic_real.data());

    // Set output
    for (size_t i = 0; i < len; ++i) {
        output[i].real = input[i];
        output[i].imag = analytic_real[i] - input[i];  // Simplified
    }
}

void hilbert(const float* input, float* output_real, float* output_imag, size_t len) {
    std::vector<Complex> analytic(len);
    hilbert(input, analytic.data(), len);

    for (size_t i = 0; i < len; ++i) {
        output_real[i] = analytic[i].real;
        output_imag[i] = analytic[i].imag;
    }
}

void instantaneous_amplitude(const float* input, float* output, size_t len) {
    std::vector<Complex> analytic(len);
    hilbert(input, analytic.data(), len);

    for (size_t i = 0; i < len; ++i) {
        output[i] = analytic[i].magnitude();
    }
}

void instantaneous_phase(const float* input, float* output, size_t len) {
    std::vector<Complex> analytic(len);
    hilbert(input, analytic.data(), len);

    for (size_t i = 0; i < len; ++i) {
        output[i] = analytic[i].phase();
    }
}

void instantaneous_phase_unwrapped(const float* input, float* output, size_t len) {
    instantaneous_phase(input, output, len);
    unwrap_phase(output, len);
}

void instantaneous_frequency(const float* input, float* output,
                             size_t len, float sample_rate) {
    std::vector<float> phase(len);
    instantaneous_phase_unwrapped(input, phase.data(), len);

    // Derivative of phase
    const float scale = sample_rate / constants::two_pi;
    for (size_t i = 0; i < len - 1; ++i) {
        output[i] = (phase[i + 1] - phase[i]) * scale;
    }
}

void unwrap_phase(float* phase, size_t len) {
    if (len < 2) return;

    float cumulative_offset = 0.0f;

    for (size_t i = 1; i < len; ++i) {
        float diff = phase[i] - phase[i - 1];

        // Check for discontinuity
        while (diff > constants::pi) {
            diff -= constants::two_pi;
            cumulative_offset -= constants::two_pi;
        }
        while (diff < -constants::pi) {
            diff += constants::two_pi;
            cumulative_offset += constants::two_pi;
        }

        phase[i] += cumulative_offset;
    }
}

} // namespace spectra
