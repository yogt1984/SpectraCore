/**
 * @file test_fir_design.cpp
 * @brief Unit tests for FIR filter design using windowed-sinc method
 */

#include <gtest/gtest.h>
#include "spectra/filters/filter_design.hpp"
#include "spectra/core/window_functions.hpp"
#include <cmath>
#include <numeric>

using namespace spectra;

class FIRDesignTest : public ::testing::Test {
protected:
    // Compute frequency response magnitude at normalized frequency w (0 to 1)
    float compute_magnitude(const std::vector<float>& h, float normalized_freq) {
        const float w = constants::pi * normalized_freq;
        float real_sum = 0.0f, imag_sum = 0.0f;
        const int M = static_cast<int>(h.size()) - 1;

        for (size_t n = 0; n < h.size(); ++n) {
            const float phase = w * (static_cast<float>(n) - static_cast<float>(M) / 2.0f);
            real_sum += h[n] * std::cos(phase);
            imag_sum += h[n] * std::sin(phase);
        }

        return std::sqrt(real_sum * real_sum + imag_sum * imag_sum);
    }

    // Convert magnitude to dB
    float to_db(float magnitude) {
        return 20.0f * std::log10(std::max(magnitude, 1e-10f));
    }
};

// ============================================================================
// Basic Construction Tests
// ============================================================================

TEST_F(FIRDesignTest, LowpassBasicConstruction) {
    auto h = fir_windowed_sinc(51, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    EXPECT_EQ(h.size(), 51u);

    // Check no NaN or Inf
    for (float coef : h) {
        EXPECT_FALSE(std::isnan(coef));
        EXPECT_FALSE(std::isinf(coef));
    }
}

TEST_F(FIRDesignTest, HighpassBasicConstruction) {
    auto h = fir_windowed_sinc(51, 0.3f, FilterType::Highpass, WindowType::Hamming);

    // Highpass needs odd taps
    EXPECT_EQ(h.size() % 2, 1u);

    for (float coef : h) {
        EXPECT_FALSE(std::isnan(coef));
        EXPECT_FALSE(std::isinf(coef));
    }
}

TEST_F(FIRDesignTest, BandpassBasicConstruction) {
    auto h = fir_windowed_sinc(65, 0.2f, 0.4f, FilterType::Bandpass, WindowType::Hamming);

    EXPECT_EQ(h.size(), 65u);

    for (float coef : h) {
        EXPECT_FALSE(std::isnan(coef));
        EXPECT_FALSE(std::isinf(coef));
    }
}

TEST_F(FIRDesignTest, BandstopBasicConstruction) {
    auto h = fir_windowed_sinc(65, 0.2f, 0.4f, FilterType::Bandstop, WindowType::Hamming);

    // Bandstop needs odd taps
    EXPECT_EQ(h.size() % 2, 1u);

    for (float coef : h) {
        EXPECT_FALSE(std::isnan(coef));
        EXPECT_FALSE(std::isinf(coef));
    }
}

// ============================================================================
// Symmetry Tests (Linear Phase)
// ============================================================================

TEST_F(FIRDesignTest, LowpassSymmetry) {
    auto h = fir_windowed_sinc(51, 0.25f, FilterType::Lowpass, WindowType::Hamming);

    // FIR coefficients should be symmetric for linear phase
    const size_t n = h.size();
    for (size_t i = 0; i < n / 2; ++i) {
        EXPECT_NEAR(h[i], h[n - 1 - i], 1e-6f) << "Index " << i;
    }
}

TEST_F(FIRDesignTest, HighpassSymmetry) {
    auto h = fir_windowed_sinc(51, 0.25f, FilterType::Highpass, WindowType::Hamming);

    const size_t n = h.size();
    for (size_t i = 0; i < n / 2; ++i) {
        EXPECT_NEAR(h[i], h[n - 1 - i], 1e-6f) << "Index " << i;
    }
}

TEST_F(FIRDesignTest, BandpassSymmetry) {
    auto h = fir_windowed_sinc(65, 0.2f, 0.4f, FilterType::Bandpass, WindowType::Hamming);

    const size_t n = h.size();
    for (size_t i = 0; i < n / 2; ++i) {
        EXPECT_NEAR(h[i], h[n - 1 - i], 1e-6f) << "Index " << i;
    }
}

TEST_F(FIRDesignTest, BandstopSymmetry) {
    auto h = fir_windowed_sinc(65, 0.2f, 0.4f, FilterType::Bandstop, WindowType::Hamming);

    const size_t n = h.size();
    for (size_t i = 0; i < n / 2; ++i) {
        EXPECT_NEAR(h[i], h[n - 1 - i], 1e-6f) << "Index " << i;
    }
}

// ============================================================================
// Frequency Response Tests - Lowpass
// ============================================================================

TEST_F(FIRDesignTest, LowpassDCGain) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    // DC gain should be unity (sum of coefficients)
    float dc_gain = compute_magnitude(h, 0.0f);
    EXPECT_NEAR(dc_gain, 1.0f, 0.01f);
}

TEST_F(FIRDesignTest, LowpassPassband) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    // Passband should be close to 1 (within 3dB)
    float mag_passband = compute_magnitude(h, 0.15f);
    float db_passband = to_db(mag_passband);
    EXPECT_GT(db_passband, -3.0f);
}

TEST_F(FIRDesignTest, LowpassCutoff) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    // For windowed-sinc filters, the magnitude at the specified cutoff
    // is approximately 0.5 (-6dB), not 0.707 (-3dB). This is a fundamental
    // property of the sinc function: sinc(0) = 1, and the cutoff represents
    // where the ideal response transitions, not the -3dB point.
    float mag_cutoff = compute_magnitude(h, 0.3f);
    EXPECT_NEAR(mag_cutoff, 0.5f, 0.1f);
}

TEST_F(FIRDesignTest, LowpassStopband) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    // Well into stopband should be attenuated
    float mag_stopband = compute_magnitude(h, 0.6f);
    float db_stopband = to_db(mag_stopband);
    EXPECT_LT(db_stopband, -30.0f);  // Hamming window provides ~40dB stopband attenuation
}

TEST_F(FIRDesignTest, LowpassNyquistAttenuation) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    // At Nyquist (normalized freq = 1.0), should be well attenuated
    float mag_nyquist = compute_magnitude(h, 1.0f);
    float db_nyquist = to_db(mag_nyquist);
    EXPECT_LT(db_nyquist, -30.0f);
}

// ============================================================================
// Frequency Response Tests - Highpass
// ============================================================================

TEST_F(FIRDesignTest, HighpassNyquistGain) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Highpass, WindowType::Hamming);

    // Nyquist gain should be unity
    float nyq_gain = compute_magnitude(h, 1.0f);
    EXPECT_NEAR(nyq_gain, 1.0f, 0.01f);
}

TEST_F(FIRDesignTest, HighpassDCAttenuation) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Highpass, WindowType::Hamming);

    // DC should be strongly attenuated
    float dc_gain = compute_magnitude(h, 0.0f);
    float db_dc = to_db(dc_gain);
    EXPECT_LT(db_dc, -30.0f);
}

TEST_F(FIRDesignTest, HighpassPassband) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Highpass, WindowType::Hamming);

    // Passband should be close to 1
    float mag_passband = compute_magnitude(h, 0.7f);
    float db_passband = to_db(mag_passband);
    EXPECT_GT(db_passband, -3.0f);
}

TEST_F(FIRDesignTest, HighpassCutoff) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Highpass, WindowType::Hamming);

    // For windowed-sinc filters, magnitude at cutoff is ~0.5 (-6dB)
    float mag_cutoff = compute_magnitude(h, 0.3f);
    EXPECT_NEAR(mag_cutoff, 0.5f, 0.1f);
}

TEST_F(FIRDesignTest, HighpassStopband) {
    auto h = fir_windowed_sinc(101, 0.3f, FilterType::Highpass, WindowType::Hamming);

    // Well into stopband
    float mag_stopband = compute_magnitude(h, 0.1f);
    float db_stopband = to_db(mag_stopband);
    EXPECT_LT(db_stopband, -30.0f);
}

// ============================================================================
// Frequency Response Tests - Bandpass
// ============================================================================

TEST_F(FIRDesignTest, BandpassCenterGain) {
    auto h = fir_windowed_sinc(101, 0.2f, 0.4f, FilterType::Bandpass, WindowType::Hamming);

    // Center frequency gain should be unity
    float center_gain = compute_magnitude(h, 0.3f);
    EXPECT_NEAR(center_gain, 1.0f, 0.1f);
}

TEST_F(FIRDesignTest, BandpassDCAttenuation) {
    auto h = fir_windowed_sinc(101, 0.2f, 0.4f, FilterType::Bandpass, WindowType::Hamming);

    // DC should be attenuated
    float dc_gain = compute_magnitude(h, 0.0f);
    float db_dc = to_db(dc_gain);
    EXPECT_LT(db_dc, -20.0f);
}

TEST_F(FIRDesignTest, BandpassNyquistAttenuation) {
    auto h = fir_windowed_sinc(101, 0.2f, 0.4f, FilterType::Bandpass, WindowType::Hamming);

    // Nyquist should be attenuated
    float nyq_gain = compute_magnitude(h, 1.0f);
    float db_nyq = to_db(nyq_gain);
    EXPECT_LT(db_nyq, -20.0f);
}

TEST_F(FIRDesignTest, BandpassPassband) {
    auto h = fir_windowed_sinc(101, 0.2f, 0.4f, FilterType::Bandpass, WindowType::Hamming);

    // Frequencies in passband should be close to unity
    float mag1 = compute_magnitude(h, 0.25f);
    float mag2 = compute_magnitude(h, 0.35f);

    EXPECT_GT(mag1, 0.5f);
    EXPECT_GT(mag2, 0.5f);
}

// ============================================================================
// Frequency Response Tests - Bandstop
// ============================================================================

TEST_F(FIRDesignTest, BandstopDCGain) {
    auto h = fir_windowed_sinc(101, 0.2f, 0.4f, FilterType::Bandstop, WindowType::Hamming);

    // DC gain should be unity
    float dc_gain = compute_magnitude(h, 0.0f);
    EXPECT_NEAR(dc_gain, 1.0f, 0.05f);
}

TEST_F(FIRDesignTest, BandstopNyquistGain) {
    auto h = fir_windowed_sinc(101, 0.2f, 0.4f, FilterType::Bandstop, WindowType::Hamming);

    // Nyquist gain should be near unity
    float nyq_gain = compute_magnitude(h, 1.0f);
    EXPECT_NEAR(nyq_gain, 1.0f, 0.1f);
}

TEST_F(FIRDesignTest, BandstopCenterAttenuation) {
    auto h = fir_windowed_sinc(101, 0.2f, 0.4f, FilterType::Bandstop, WindowType::Hamming);

    // Center of notch should be attenuated
    float center_gain = compute_magnitude(h, 0.3f);
    float db_center = to_db(center_gain);
    EXPECT_LT(db_center, -20.0f);
}

TEST_F(FIRDesignTest, BandstopPassbands) {
    auto h = fir_windowed_sinc(101, 0.2f, 0.4f, FilterType::Bandstop, WindowType::Hamming);

    // Below and above the notch should be near unity
    float mag_low = compute_magnitude(h, 0.1f);
    float mag_high = compute_magnitude(h, 0.7f);

    EXPECT_GT(mag_low, 0.7f);
    EXPECT_GT(mag_high, 0.7f);
}

// ============================================================================
// Different Window Types
// ============================================================================

TEST_F(FIRDesignTest, DifferentWindows) {
    std::vector<WindowType> windows = {
        WindowType::Rectangular,
        WindowType::Hann,
        WindowType::Hamming,
        WindowType::Blackman,
        WindowType::BlackmanHarris
    };

    for (WindowType win : windows) {
        auto h = fir_windowed_sinc(51, 0.25f, FilterType::Lowpass, win);

        EXPECT_EQ(h.size(), 51u) << "Window type failed";

        // DC gain should be near unity
        float dc_gain = compute_magnitude(h, 0.0f);
        EXPECT_NEAR(dc_gain, 1.0f, 0.05f) << "Window type DC gain failed";

        // No NaN/Inf
        for (float coef : h) {
            EXPECT_FALSE(std::isnan(coef));
            EXPECT_FALSE(std::isinf(coef));
        }
    }
}

TEST_F(FIRDesignTest, WindowAffectsStopbandAttenuation) {
    // Rectangular window has less stopband attenuation than Blackman
    auto h_rect = fir_windowed_sinc(51, 0.25f, FilterType::Lowpass, WindowType::Rectangular);
    auto h_blackman = fir_windowed_sinc(51, 0.25f, FilterType::Lowpass, WindowType::Blackman);

    float mag_rect = compute_magnitude(h_rect, 0.8f);
    float mag_blackman = compute_magnitude(h_blackman, 0.8f);

    // Blackman should have better stopband attenuation
    EXPECT_LT(mag_blackman, mag_rect);
}

// ============================================================================
// Filter Length Effects
// ============================================================================

TEST_F(FIRDesignTest, LongerFilterSharperTransition) {
    auto h_short = fir_windowed_sinc(31, 0.25f, FilterType::Lowpass, WindowType::Hamming);
    auto h_long = fir_windowed_sinc(101, 0.25f, FilterType::Lowpass, WindowType::Hamming);

    // At transition band, longer filter should have steeper rolloff
    float mag_short = compute_magnitude(h_short, 0.35f);
    float mag_long = compute_magnitude(h_long, 0.35f);

    // Longer filter has more attenuation in transition
    EXPECT_LT(mag_long, mag_short);
}

TEST_F(FIRDesignTest, MinimumLength) {
    auto h = fir_windowed_sinc(3, 0.25f, FilterType::Lowpass, WindowType::Hamming);

    EXPECT_EQ(h.size(), 3u);

    // Should still work, just poor performance
    for (float coef : h) {
        EXPECT_FALSE(std::isnan(coef));
        EXPECT_FALSE(std::isinf(coef));
    }
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(FIRDesignTest, InvalidCutoffLow) {
    auto h = fir_windowed_sinc(51, 0.0f, FilterType::Lowpass, WindowType::Hamming);

    // Should return valid (non-empty) filter even with invalid input
    EXPECT_FALSE(h.empty());
}

TEST_F(FIRDesignTest, InvalidCutoffHigh) {
    auto h = fir_windowed_sinc(51, 1.0f, FilterType::Lowpass, WindowType::Hamming);

    EXPECT_FALSE(h.empty());
}

TEST_F(FIRDesignTest, InvalidBandFrequencies) {
    // Low freq >= High freq
    auto h = fir_windowed_sinc(51, 0.5f, 0.3f, FilterType::Bandpass, WindowType::Hamming);

    EXPECT_FALSE(h.empty());
}

TEST_F(FIRDesignTest, EvenTapsForHighpass) {
    // Even number of taps is problematic for Type I highpass (zero at Nyquist)
    // Implementation should adjust to odd
    auto h = fir_windowed_sinc(50, 0.3f, FilterType::Highpass, WindowType::Hamming);

    // Should have been adjusted to 51
    EXPECT_EQ(h.size() % 2, 1u);
}

// ============================================================================
// Cutoff Frequency Accuracy
// ============================================================================

TEST_F(FIRDesignTest, LowpassCutoffFrequencyAccuracy) {
    std::vector<float> cutoffs = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};

    for (float fc : cutoffs) {
        auto h = fir_windowed_sinc(201, fc, FilterType::Lowpass, WindowType::Hamming);

        // Windowed-sinc: magnitude at specified cutoff is ~0.5 (-6dB)
        float mag_cutoff = compute_magnitude(h, fc);
        EXPECT_NEAR(mag_cutoff, 0.5f, 0.1f) << "Cutoff: " << fc;
    }
}

TEST_F(FIRDesignTest, HighpassCutoffFrequencyAccuracy) {
    std::vector<float> cutoffs = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};

    for (float fc : cutoffs) {
        auto h = fir_windowed_sinc(201, fc, FilterType::Highpass, WindowType::Hamming);

        // Windowed-sinc: magnitude at specified cutoff is ~0.5 (-6dB)
        float mag_cutoff = compute_magnitude(h, fc);
        EXPECT_NEAR(mag_cutoff, 0.5f, 0.1f) << "Cutoff: " << fc;
    }
}

// ============================================================================
// Frequency Response via freqz
// ============================================================================

TEST_F(FIRDesignTest, FreqzMatches) {
    auto h = fir_windowed_sinc(51, 0.25f, FilterType::Lowpass, WindowType::Hamming);

    const size_t num_points = 256;
    std::vector<Complex> H(num_points);

    freqz(h, H.data(), num_points);

    // Check DC gain
    float dc_mag = std::sqrt(H[0].magnitude_squared());
    EXPECT_NEAR(dc_mag, 1.0f, 0.05f);

    // Check Nyquist attenuation
    float nyq_mag = std::sqrt(H[num_points - 1].magnitude_squared());
    float nyq_db = to_db(nyq_mag);
    EXPECT_LT(nyq_db, -20.0f);
}

// ============================================================================
// Real-Time Application Test
// ============================================================================

TEST_F(FIRDesignTest, ConvolutionTest) {
    // Design lowpass filter
    auto h = fir_windowed_sinc(31, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    // Create test signal: DC + high frequency component
    std::vector<float> input(200);
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = 1.0f + std::sin(0.8f * constants::pi * i);  // DC + near-Nyquist
    }

    // Convolve
    std::vector<float> output(input.size());
    const int M = static_cast<int>(h.size());

    for (size_t n = M - 1; n < input.size(); ++n) {
        float sum = 0.0f;
        for (int k = 0; k < M; ++k) {
            sum += h[k] * input[n - k];
        }
        output[n] = sum;
    }

    // After settling, output should be close to DC (1.0), high freq removed
    float mean_output = 0.0f;
    for (size_t i = M + 20; i < output.size(); ++i) {
        mean_output += output[i];
    }
    mean_output /= (output.size() - M - 20);

    // Mean should be close to 1.0 (DC component preserved)
    EXPECT_NEAR(mean_output, 1.0f, 0.15f);

    // Variance should be low (high freq removed)
    float variance = 0.0f;
    for (size_t i = M + 20; i < output.size(); ++i) {
        float diff = output[i] - mean_output;
        variance += diff * diff;
    }
    variance /= (output.size() - M - 20);

    // Variance should be much lower than input (which has high freq component)
    EXPECT_LT(variance, 0.1f);
}
