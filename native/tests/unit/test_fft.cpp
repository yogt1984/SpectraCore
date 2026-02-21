/**
 * @file test_fft.cpp
 * @brief Unit tests for FFT
 */

#include <gtest/gtest.h>
#include "spectra/fft/fft.hpp"
#include <cmath>

using namespace spectra;

class FFTTest : public ::testing::Test {
protected:
    static constexpr size_t N = 256;
    FFT fft{N};
    std::vector<float> signal;
    std::vector<Complex> spectrum;

    void SetUp() override {
        signal.resize(N);
        spectrum.resize(fft.spectrum_size());
    }
};

TEST_F(FFTTest, Size) {
    EXPECT_EQ(fft.size(), N);
    EXPECT_EQ(fft.spectrum_size(), N / 2 + 1);
}

TEST_F(FFTTest, DCSignal) {
    // DC signal should produce energy only at bin 0
    std::fill(signal.begin(), signal.end(), 1.0f);
    fft.forward(signal.data(), spectrum.data());

    EXPECT_NEAR(spectrum[0].magnitude(), static_cast<float>(N), 1.0f);

    for (size_t i = 1; i < spectrum.size(); ++i) {
        EXPECT_NEAR(spectrum[i].magnitude(), 0.0f, 0.1f);
    }
}

TEST_F(FFTTest, SineWave) {
    // Generate sine wave at bin 8
    const size_t target_bin = 8;
    const float freq = static_cast<float>(target_bin) / N;

    for (size_t i = 0; i < N; ++i) {
        signal[i] = std::sin(2.0f * constants::pi * freq * i);
    }

    fft.forward(signal.data(), spectrum.data());

    // Peak should be at target bin
    float max_mag = 0.0f;
    size_t max_bin = 0;
    for (size_t i = 0; i < spectrum.size(); ++i) {
        float mag = spectrum[i].magnitude();
        if (mag > max_mag) {
            max_mag = mag;
            max_bin = i;
        }
    }

    EXPECT_EQ(max_bin, target_bin);
}

TEST_F(FFTTest, InverseRecovery) {
    // Random-ish signal
    for (size_t i = 0; i < N; ++i) {
        signal[i] = std::sin(0.1f * i) + 0.5f * std::cos(0.3f * i);
    }

    // Forward
    fft.forward(signal.data(), spectrum.data());

    // Inverse
    std::vector<float> recovered(N);
    fft.inverse(spectrum.data(), recovered.data());

    // Check recovery
    for (size_t i = 0; i < N; ++i) {
        EXPECT_NEAR(recovered[i], signal[i], 1e-4f);
    }
}

TEST_F(FFTTest, MagnitudeSpectrum) {
    std::fill(signal.begin(), signal.end(), 1.0f);

    std::vector<float> magnitude(fft.spectrum_size());
    fft.magnitude_spectrum(signal.data(), magnitude.data());

    EXPECT_NEAR(magnitude[0], static_cast<float>(N), 1.0f);
}

TEST_F(FFTTest, PowerSpectrum) {
    std::fill(signal.begin(), signal.end(), 1.0f);

    std::vector<float> power(fft.spectrum_size());
    fft.power_spectrum(signal.data(), power.data());

    EXPECT_NEAR(power[0], static_cast<float>(N * N), 10.0f);
}

TEST_F(FFTTest, SeparateRealImag) {
    for (size_t i = 0; i < N; ++i) {
        signal[i] = std::sin(0.1f * i);
    }

    std::vector<float> real(fft.spectrum_size());
    std::vector<float> imag(fft.spectrum_size());

    fft.forward(signal.data(), real.data(), imag.data());

    // Verify against Complex output
    fft.forward(signal.data(), spectrum.data());

    for (size_t i = 0; i < spectrum.size(); ++i) {
        EXPECT_NEAR(real[i], spectrum[i].real, 1e-5f);
        EXPECT_NEAR(imag[i], spectrum[i].imag, 1e-5f);
    }
}
