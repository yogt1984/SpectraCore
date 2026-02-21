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

// ============================================================================
// CORE-003: Additional rigorous FFT tests
// ============================================================================

TEST_F(FFTTest, ParsevalsTheorem) {
    // Parseval's theorem: sum(|x|^2) = (1/N) * sum(|X|^2)
    // Energy in time domain equals energy in frequency domain

    // Create a signal with known energy
    for (size_t i = 0; i < N; ++i) {
        signal[i] = std::sin(2.0f * constants::pi * 4.0f * i / N)
                  + 0.5f * std::cos(2.0f * constants::pi * 16.0f * i / N);
    }

    // Time-domain energy
    float time_energy = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        time_energy += signal[i] * signal[i];
    }

    // Frequency-domain energy
    fft.forward(signal.data(), spectrum.data());

    float freq_energy = 0.0f;
    // DC and Nyquist bins count once, others count twice (positive + negative freq)
    freq_energy += spectrum[0].magnitude_squared();
    for (size_t i = 1; i < spectrum.size() - 1; ++i) {
        freq_energy += 2.0f * spectrum[i].magnitude_squared();
    }
    if (N % 2 == 0) {
        freq_energy += spectrum[spectrum.size() - 1].magnitude_squared();
    }
    freq_energy /= static_cast<float>(N);

    // Energy should be conserved (within floating point tolerance)
    EXPECT_NEAR(time_energy, freq_energy, time_energy * 1e-4f);
}

TEST_F(FFTTest, InverseRecoveryStrictError) {
    // Strict test: roundtrip error < 1e-5

    // Create complex signal
    for (size_t i = 0; i < N; ++i) {
        signal[i] = std::sin(0.1f * i) * std::cos(0.05f * i)
                  + 0.3f * std::sin(0.7f * i);
    }

    fft.forward(signal.data(), spectrum.data());

    std::vector<float> recovered(N);
    fft.inverse(spectrum.data(), recovered.data());

    // Calculate max absolute error
    float max_error = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        float error = std::abs(recovered[i] - signal[i]);
        max_error = std::max(max_error, error);
    }

    EXPECT_LT(max_error, 1e-5f) << "Max roundtrip error: " << max_error;
}

TEST_F(FFTTest, MultipleFrequencies) {
    // Test signal with multiple known frequencies
    const std::vector<size_t> bins = {4, 16, 32, 64};

    std::fill(signal.begin(), signal.end(), 0.0f);
    for (size_t bin : bins) {
        float freq = static_cast<float>(bin) / N;
        for (size_t i = 0; i < N; ++i) {
            signal[i] += std::sin(2.0f * constants::pi * freq * i);
        }
    }

    fft.forward(signal.data(), spectrum.data());

    // Find peaks (bins with magnitude > threshold)
    std::vector<size_t> detected_bins;
    float threshold = static_cast<float>(N) / 4.0f;  // Expected magnitude ~N/2 per sine

    for (size_t i = 0; i < spectrum.size(); ++i) {
        if (spectrum[i].magnitude() > threshold) {
            detected_bins.push_back(i);
        }
    }

    // Should detect all input frequencies
    ASSERT_EQ(detected_bins.size(), bins.size());
    for (size_t j = 0; j < bins.size(); ++j) {
        EXPECT_EQ(detected_bins[j], bins[j]);
    }
}

TEST_F(FFTTest, DifferentSizes) {
    // Test various power-of-2 sizes
    std::vector<size_t> sizes = {64, 128, 512, 1024, 2048};

    for (size_t size : sizes) {
        FFT fft_test(size);
        std::vector<float> sig(size);
        std::vector<Complex> spec(fft_test.spectrum_size());
        std::vector<float> recovered(size);

        // Create test signal
        for (size_t i = 0; i < size; ++i) {
            sig[i] = std::sin(2.0f * constants::pi * 8.0f * i / size);
        }

        fft_test.forward(sig.data(), spec.data());
        fft_test.inverse(spec.data(), recovered.data());

        // Verify recovery
        float max_error = 0.0f;
        for (size_t i = 0; i < size; ++i) {
            max_error = std::max(max_error, std::abs(recovered[i] - sig[i]));
        }

        EXPECT_LT(max_error, 1e-5f) << "Size " << size << " max error: " << max_error;
    }
}

TEST_F(FFTTest, NonPowerOfTwo) {
    // KissFFT supports non-power-of-2 sizes (must be even for real FFT)
    std::vector<size_t> sizes = {100, 250, 400, 630};

    for (size_t size : sizes) {
        FFT fft_test(size);
        std::vector<float> sig(size);
        std::vector<Complex> spec(fft_test.spectrum_size());
        std::vector<float> recovered(size);

        EXPECT_EQ(fft_test.size(), size);
        EXPECT_EQ(fft_test.spectrum_size(), size / 2 + 1);

        // Create DC signal
        std::fill(sig.begin(), sig.end(), 1.0f);

        fft_test.forward(sig.data(), spec.data());

        // DC should have all energy
        EXPECT_NEAR(spec[0].magnitude(), static_cast<float>(size), 1.0f)
            << "Size: " << size;

        // Test roundtrip
        for (size_t i = 0; i < size; ++i) {
            sig[i] = std::sin(0.1f * i);
        }

        fft_test.forward(sig.data(), spec.data());
        fft_test.inverse(spec.data(), recovered.data());

        float max_error = 0.0f;
        for (size_t i = 0; i < size; ++i) {
            max_error = std::max(max_error, std::abs(recovered[i] - sig[i]));
        }

        // Non-power-of-2 may have slightly larger error, but should still be small
        EXPECT_LT(max_error, 1e-4f) << "Size " << size << " max error: " << max_error;
    }
}

TEST_F(FFTTest, ImpulseResponse) {
    // Delta function should have flat magnitude spectrum
    std::fill(signal.begin(), signal.end(), 0.0f);
    signal[0] = 1.0f;

    fft.forward(signal.data(), spectrum.data());

    // All bins should have magnitude 1
    for (size_t i = 0; i < spectrum.size(); ++i) {
        EXPECT_NEAR(spectrum[i].magnitude(), 1.0f, 1e-5f)
            << "Bin " << i << " magnitude";
    }
}

TEST_F(FFTTest, ZeroSignal) {
    // All-zero input should produce all-zero output
    std::fill(signal.begin(), signal.end(), 0.0f);

    fft.forward(signal.data(), spectrum.data());

    for (size_t i = 0; i < spectrum.size(); ++i) {
        EXPECT_FLOAT_EQ(spectrum[i].real, 0.0f);
        EXPECT_FLOAT_EQ(spectrum[i].imag, 0.0f);
    }
}

TEST_F(FFTTest, NyquistFrequency) {
    // Signal at Nyquist frequency: alternating +1, -1
    for (size_t i = 0; i < N; ++i) {
        signal[i] = (i % 2 == 0) ? 1.0f : -1.0f;
    }

    fft.forward(signal.data(), spectrum.data());

    // Energy should be at Nyquist bin (last bin)
    size_t nyquist_bin = spectrum.size() - 1;
    EXPECT_NEAR(spectrum[nyquist_bin].magnitude(), static_cast<float>(N), 1.0f);

    // Other bins should be near zero
    for (size_t i = 0; i < nyquist_bin; ++i) {
        EXPECT_NEAR(spectrum[i].magnitude(), 0.0f, 0.1f) << "Bin " << i;
    }
}

TEST_F(FFTTest, PhaseCorrectness) {
    // Cosine starts at max, sine starts at zero
    // cos(2*pi*k*n/N) should have real spectrum, sin should have imaginary

    const size_t bin = 8;
    float freq = static_cast<float>(bin) / N;

    // Test cosine (should be mostly real)
    for (size_t i = 0; i < N; ++i) {
        signal[i] = std::cos(2.0f * constants::pi * freq * i);
    }
    fft.forward(signal.data(), spectrum.data());

    // For a pure cosine, the imaginary part at the frequency bin should be ~0
    EXPECT_NEAR(spectrum[bin].imag, 0.0f, 1.0f);
    EXPECT_GT(std::abs(spectrum[bin].real), static_cast<float>(N) / 4);

    // Test sine (should be mostly imaginary)
    for (size_t i = 0; i < N; ++i) {
        signal[i] = std::sin(2.0f * constants::pi * freq * i);
    }
    fft.forward(signal.data(), spectrum.data());

    // For a pure sine, the real part at the frequency bin should be ~0
    EXPECT_NEAR(spectrum[bin].real, 0.0f, 1.0f);
    EXPECT_GT(std::abs(spectrum[bin].imag), static_cast<float>(N) / 4);
}
