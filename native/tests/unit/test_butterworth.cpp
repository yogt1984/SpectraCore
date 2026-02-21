/**
 * @file test_butterworth.cpp
 * @brief Unit tests for Butterworth filter design
 */

#include <gtest/gtest.h>
#include "spectra/filters/filter_design.hpp"
#include <cmath>
#include <complex>

using namespace spectra;

class ButterworthTest : public ::testing::Test {
protected:
    // Helper to compute frequency response magnitude at a given frequency
    float magnitude_at_freq(const FilterCoefficients& coeffs, float norm_freq) {
        std::complex<float> z = std::polar(1.0f, constants::pi * norm_freq);

        // Numerator H(z) = b[0] + b[1]*z^-1 + b[2]*z^-2 + ...
        std::complex<float> num(0.0f, 0.0f);
        std::complex<float> z_inv = 1.0f / z;
        std::complex<float> z_pow = 1.0f;
        for (size_t i = 0; i < coeffs.b.size(); ++i) {
            num += coeffs.b[i] * z_pow;
            z_pow *= z_inv;
        }

        // Denominator
        std::complex<float> den(0.0f, 0.0f);
        z_pow = 1.0f;
        for (size_t i = 0; i < coeffs.a.size(); ++i) {
            den += coeffs.a[i] * z_pow;
            z_pow *= z_inv;
        }

        return std::abs(num / den);
    }

    // Helper to compute magnitude in dB
    float magnitude_db(const FilterCoefficients& coeffs, float norm_freq) {
        float mag = magnitude_at_freq(coeffs, norm_freq);
        return 20.0f * std::log10(mag + 1e-10f);
    }
};

// ============================================================================
// Basic Construction Tests
// ============================================================================

TEST_F(ButterworthTest, Order1Lowpass) {
    auto coeffs = butter(1, 0.5f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 2u);
    EXPECT_EQ(coeffs.a.size(), 2u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(ButterworthTest, Order2Lowpass) {
    auto coeffs = butter(2, 0.5f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 3u);
    EXPECT_EQ(coeffs.a.size(), 3u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(ButterworthTest, Order4Lowpass) {
    auto coeffs = butter(4, 0.5f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 5u);
    EXPECT_EQ(coeffs.a.size(), 5u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(ButterworthTest, OrdersUpTo8) {
    for (int order = 1; order <= 8; ++order) {
        auto coeffs = butter(order, 0.3f, FilterType::Lowpass);

        EXPECT_EQ(coeffs.b.size(), static_cast<size_t>(order + 1))
            << "Order " << order;
        EXPECT_EQ(coeffs.a.size(), static_cast<size_t>(order + 1))
            << "Order " << order;
        EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f) << "Order " << order;
    }
}

// ============================================================================
// DC Gain Tests (Lowpass should have unity gain at DC)
// ============================================================================

TEST_F(ButterworthTest, LowpassUnityDCGain) {
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = butter(order, 0.3f, FilterType::Lowpass);
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);

        EXPECT_NEAR(dc_gain, 1.0f, 0.001f)
            << "Order " << order << " DC gain: " << dc_gain;
    }
}

TEST_F(ButterworthTest, HighpassZeroDCGain) {
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = butter(order, 0.3f, FilterType::Highpass);
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);

        EXPECT_NEAR(dc_gain, 0.0f, 0.001f)
            << "Order " << order << " DC gain: " << dc_gain;
    }
}

TEST_F(ButterworthTest, HighpassUnityNyquistGain) {
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = butter(order, 0.3f, FilterType::Highpass);
        float nyq_gain = magnitude_at_freq(coeffs, 1.0f);

        EXPECT_NEAR(nyq_gain, 1.0f, 0.01f)
            << "Order " << order << " Nyquist gain: " << nyq_gain;
    }
}

// ============================================================================
// Cutoff Frequency Tests (-3dB point)
// ============================================================================

TEST_F(ButterworthTest, LowpassCutoffAt3dB) {
    for (int order = 1; order <= 6; ++order) {
        float cutoff = 0.3f;
        auto coeffs = butter(order, cutoff, FilterType::Lowpass);

        // At cutoff frequency, magnitude should be -3dB (1/sqrt(2))
        float mag_at_cutoff = magnitude_at_freq(coeffs, cutoff);
        float expected = 1.0f / std::sqrt(2.0f);

        EXPECT_NEAR(mag_at_cutoff, expected, 0.05f)
            << "Order " << order << " magnitude at cutoff: " << mag_at_cutoff;
    }
}

TEST_F(ButterworthTest, HighpassCutoffAt3dB) {
    for (int order = 1; order <= 6; ++order) {
        float cutoff = 0.3f;
        auto coeffs = butter(order, cutoff, FilterType::Highpass);

        // At cutoff frequency, magnitude should be -3dB (1/sqrt(2))
        float mag_at_cutoff = magnitude_at_freq(coeffs, cutoff);
        float expected = 1.0f / std::sqrt(2.0f);

        EXPECT_NEAR(mag_at_cutoff, expected, 0.05f)
            << "Order " << order << " magnitude at cutoff: " << mag_at_cutoff;
    }
}

// ============================================================================
// Stability Tests
// ============================================================================

TEST_F(ButterworthTest, AllOrdersStable) {
    for (int order = 1; order <= 8; ++order) {
        for (float freq : {0.1f, 0.3f, 0.5f, 0.7f, 0.9f}) {
            auto lp = butter(order, freq, FilterType::Lowpass);
            auto hp = butter(order, freq, FilterType::Highpass);

            EXPECT_TRUE(is_stable(lp))
                << "Order " << order << " freq " << freq << " lowpass unstable";
            EXPECT_TRUE(is_stable(hp))
                << "Order " << order << " freq " << freq << " highpass unstable";
        }
    }
}

// ============================================================================
// Monotonic Response (Butterworth is maximally flat)
// ============================================================================

TEST_F(ButterworthTest, LowpassMonotonicDecrease) {
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);

    float prev_mag = magnitude_at_freq(coeffs, 0.0f);
    for (float f = 0.1f; f <= 1.0f; f += 0.1f) {
        float mag = magnitude_at_freq(coeffs, f);
        EXPECT_LE(mag, prev_mag + 0.01f)  // Allow small numerical error
            << "Non-monotonic at freq " << f;
        prev_mag = mag;
    }
}

TEST_F(ButterworthTest, HighpassMonotonicIncrease) {
    auto coeffs = butter(4, 0.3f, FilterType::Highpass);

    float prev_mag = magnitude_at_freq(coeffs, 0.0f);
    for (float f = 0.1f; f <= 1.0f; f += 0.1f) {
        float mag = magnitude_at_freq(coeffs, f);
        EXPECT_GE(mag, prev_mag - 0.01f)  // Allow small numerical error
            << "Non-monotonic at freq " << f;
        prev_mag = mag;
    }
}

// ============================================================================
// Stopband Attenuation (Higher order = steeper rolloff)
// ============================================================================

TEST_F(ButterworthTest, HigherOrderSteperRolloff) {
    float cutoff = 0.2f;
    float stopband_freq = 0.5f;

    float prev_atten = 0.0f;
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = butter(order, cutoff, FilterType::Lowpass);
        float atten = -magnitude_db(coeffs, stopband_freq);

        EXPECT_GT(atten, prev_atten)
            << "Order " << order << " should have more attenuation";
        prev_atten = atten;
    }
}

// ============================================================================
// Different Cutoff Frequencies
// ============================================================================

TEST_F(ButterworthTest, DifferentCutoffFrequencies) {
    std::vector<float> cutoffs = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f};

    for (float cutoff : cutoffs) {
        auto coeffs = butter(3, cutoff, FilterType::Lowpass);

        // DC gain should be 1
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);
        EXPECT_NEAR(dc_gain, 1.0f, 0.01f) << "Cutoff " << cutoff;

        // Should be stable
        EXPECT_TRUE(is_stable(coeffs)) << "Cutoff " << cutoff;

        // Magnitude at cutoff should be -3dB
        float mag_cutoff = magnitude_at_freq(coeffs, cutoff);
        EXPECT_NEAR(mag_cutoff, 1.0f / std::sqrt(2.0f), 0.1f)
            << "Cutoff " << cutoff;
    }
}

// ============================================================================
// Bandpass Filter Tests
// ============================================================================

TEST_F(ButterworthTest, BandpassBasic) {
    auto coeffs = butter(2, 0.2f, 0.4f, FilterType::Bandpass);

    // Should attenuate DC
    float dc_gain = magnitude_at_freq(coeffs, 0.0f);
    EXPECT_LT(dc_gain, 0.1f);

    // Should attenuate Nyquist
    float nyq_gain = magnitude_at_freq(coeffs, 1.0f);
    EXPECT_LT(nyq_gain, 0.1f);

    // Should pass center frequency
    float center_gain = magnitude_at_freq(coeffs, 0.3f);
    EXPECT_GT(center_gain, 0.5f);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ButterworthTest, InvalidOrder) {
    auto coeffs = butter(0, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs.b.size(), 1u);
    EXPECT_EQ(coeffs.a.size(), 1u);
}

TEST_F(ButterworthTest, InvalidFrequency) {
    auto coeffs1 = butter(2, 0.0f, FilterType::Lowpass);
    EXPECT_EQ(coeffs1.b.size(), 1u);

    auto coeffs2 = butter(2, 1.0f, FilterType::Lowpass);
    EXPECT_EQ(coeffs2.b.size(), 1u);

    auto coeffs3 = butter(2, -0.1f, FilterType::Lowpass);
    EXPECT_EQ(coeffs3.b.size(), 1u);

    auto coeffs4 = butter(2, 1.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs4.b.size(), 1u);
}

// ============================================================================
// Frequency Response Function Tests
// ============================================================================

TEST_F(ButterworthTest, FreqzComputation) {
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);

    const size_t num_points = 128;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // DC should have magnitude 1
    EXPECT_NEAR(H[0].magnitude(), 1.0f, 0.01f);

    // Magnitude should decrease monotonically
    float prev_mag = H[0].magnitude();
    for (size_t i = 1; i < num_points; ++i) {
        float mag = H[i].magnitude();
        EXPECT_LE(mag, prev_mag + 0.01f);
        prev_mag = mag;
    }
}

// ============================================================================
// Coefficient Size Test
// ============================================================================

TEST_F(ButterworthTest, IIRCoeffSize) {
    EXPECT_EQ(iir_coeff_size(1), 2);
    EXPECT_EQ(iir_coeff_size(2), 3);
    EXPECT_EQ(iir_coeff_size(4), 5);
    EXPECT_EQ(iir_coeff_size(8), 9);
}

// ============================================================================
// Symmetry Tests
// ============================================================================

TEST_F(ButterworthTest, LowpassHighpassComplementary) {
    float cutoff = 0.3f;
    auto lp = butter(3, cutoff, FilterType::Lowpass);
    auto hp = butter(3, cutoff, FilterType::Highpass);

    // At various frequencies, |H_lp|^2 + |H_hp|^2 should be approximately 1
    // (This is true for Butterworth filters at all frequencies)
    for (float f = 0.0f; f <= 1.0f; f += 0.1f) {
        float mag_lp = magnitude_at_freq(lp, f);
        float mag_hp = magnitude_at_freq(hp, f);

        // Due to design approximations, allow some tolerance
        float sum_sq = mag_lp * mag_lp + mag_hp * mag_hp;
        EXPECT_NEAR(sum_sq, 1.0f, 0.15f)
            << "Freq " << f << ": LP=" << mag_lp << " HP=" << mag_hp;
    }
}
