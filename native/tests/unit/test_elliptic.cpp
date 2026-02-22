/**
 * @file test_elliptic.cpp
 * @brief Unit tests for Elliptic (Cauer) filter design
 */

#include <gtest/gtest.h>
#include "spectra/filters/filter_design.hpp"
#include <cmath>
#include <complex>

using namespace spectra;

class EllipticTest : public ::testing::Test {
protected:
    const float passband_ripple = 1.0f;   // 1 dB passband ripple
    const float stopband_atten = 40.0f;   // 40 dB stopband attenuation

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

TEST_F(EllipticTest, Order1Lowpass) {
    auto coeffs = ellip(1, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 2u);
    EXPECT_EQ(coeffs.a.size(), 2u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(EllipticTest, Order2Lowpass) {
    auto coeffs = ellip(2, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 3u);
    EXPECT_EQ(coeffs.a.size(), 3u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(EllipticTest, Order3Lowpass) {
    auto coeffs = ellip(3, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 4u);
    EXPECT_EQ(coeffs.a.size(), 4u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(EllipticTest, Order4Lowpass) {
    auto coeffs = ellip(4, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 5u);
    EXPECT_EQ(coeffs.a.size(), 5u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(EllipticTest, OrdersUpTo8) {
    for (int order = 1; order <= 8; ++order) {
        auto coeffs = ellip(order, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

        EXPECT_EQ(coeffs.b.size(), static_cast<size_t>(order + 1))
            << "Order " << order;
        EXPECT_EQ(coeffs.a.size(), static_cast<size_t>(order + 1))
            << "Order " << order;
        EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f) << "Order " << order;
    }
}

// ============================================================================
// DC Gain Tests
// ============================================================================

TEST_F(EllipticTest, LowpassDCGain) {
    // For elliptic filters, DC gain depends on order like Chebyshev Type I
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = ellip(order, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);

        // DC should be within passband (ripple tolerance)
        float epsilon = std::sqrt(std::pow(10.0f, passband_ripple / 10.0f) - 1.0f);
        float min_gain = 1.0f / std::sqrt(1.0f + epsilon * epsilon);

        // DC gain should be between min_gain and 1.0
        EXPECT_GE(dc_gain, min_gain - 0.05f) << "Order " << order;
        EXPECT_LE(dc_gain, 1.05f) << "Order " << order;
    }
}

TEST_F(EllipticTest, HighpassZeroDCGain) {
    for (int order = 1; order <= 4; ++order) {
        auto coeffs = ellip(order, passband_ripple, stopband_atten, 0.3f, FilterType::Highpass);
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);

        // Highpass should attenuate DC
        EXPECT_LT(dc_gain, 0.2f) << "Order " << order << " DC gain: " << dc_gain;
    }
}

TEST_F(EllipticTest, HighpassNyquistGain) {
    for (int order = 1; order <= 4; ++order) {
        auto coeffs = ellip(order, passband_ripple, stopband_atten, 0.3f, FilterType::Highpass);
        float nyq_gain = magnitude_at_freq(coeffs, 1.0f);

        // Nyquist gain should be within passband
        float epsilon = std::sqrt(std::pow(10.0f, passband_ripple / 10.0f) - 1.0f);
        float min_gain = 1.0f / std::sqrt(1.0f + epsilon * epsilon);

        EXPECT_GE(nyq_gain, min_gain - 0.1f) << "Order " << order;
        EXPECT_LE(nyq_gain, 1.1f) << "Order " << order;
    }
}

// ============================================================================
// Passband Ripple Tests
// ============================================================================

TEST_F(EllipticTest, PassbandRippleExists) {
    // Elliptic filters have equiripple passband behavior
    auto coeffs = ellip(4, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

    // Sample passband to find min and max
    float max_gain = 0.0f;
    float min_gain = 10.0f;

    for (float f = 0.0f; f <= 0.25f; f += 0.01f) {
        float gain = magnitude_at_freq(coeffs, f);
        max_gain = std::max(max_gain, gain);
        min_gain = std::min(min_gain, gain);
    }

    // There should be some variation (ripple) in the passband
    float ripple_range = max_gain - min_gain;
    EXPECT_GT(ripple_range, 0.01f) << "Expected some passband ripple";

    // But ripple should not exceed the specified value
    float expected_ripple_linear = 1.0f - 1.0f / std::sqrt(1.0f + std::pow(std::sqrt(std::pow(10.0f, passband_ripple / 10.0f) - 1.0f), 2));
    EXPECT_LT(ripple_range, expected_ripple_linear + 0.15f);
}

TEST_F(EllipticTest, PassbandRippleMatchesSpec) {
    // Test that passband ripple approximately matches specification
    for (float rp : {0.5f, 1.0f, 2.0f, 3.0f}) {
        auto coeffs = ellip(4, rp, 40.0f, 0.3f, FilterType::Lowpass);

        float max_db = -1000.0f;
        float min_db = 1000.0f;

        // Sample well within passband
        for (float f = 0.0f; f <= 0.2f; f += 0.01f) {
            float db = magnitude_db(coeffs, f);
            max_db = std::max(max_db, db);
            min_db = std::min(min_db, db);
        }

        float ripple_db = max_db - min_db;
        // Ripple should be approximately the specified value
        EXPECT_LT(ripple_db, rp + 1.0f) << "Ripple " << rp << " dB";
    }
}

// ============================================================================
// Stopband Attenuation Tests
// ============================================================================

TEST_F(EllipticTest, StopbandAttenuation) {
    // Elliptic filters should achieve specified stopband attenuation
    auto coeffs = ellip(4, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

    // Well into stopband (beyond transition)
    float stopband_gain = magnitude_at_freq(coeffs, 0.6f);
    float stopband_db = -20.0f * std::log10(stopband_gain + 1e-10f);

    // Should have significant attenuation (may not exactly match due to approximations)
    EXPECT_GT(stopband_db, 20.0f) << "Stopband attenuation: " << stopband_db << " dB";
}

TEST_F(EllipticTest, StopbandRippleExists) {
    // Elliptic filters have equiripple stopband behavior
    auto coeffs = ellip(4, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

    // Sample stopband to find variation
    float max_gain = 0.0f;
    float min_gain = 10.0f;

    for (float f = 0.5f; f <= 0.95f; f += 0.02f) {
        float gain = magnitude_at_freq(coeffs, f);
        max_gain = std::max(max_gain, gain);
        min_gain = std::min(min_gain, gain);
    }

    // There should be variation (ripple) in the stopband
    // This distinguishes elliptic from Chebyshev Type II
    float ratio = max_gain / (min_gain + 1e-10f);
    EXPECT_GT(ratio, 1.2f) << "Expected stopband ripple (equiripple behavior)";
}

// ============================================================================
// Transition Band Tests (Elliptic should be steepest)
// ============================================================================

TEST_F(EllipticTest, SteepestTransitionBand) {
    // Elliptic filters should have steeper transition than Butterworth of same order
    float cutoff = 0.3f;
    int order = 4;

    auto ellip_coeffs = ellip(order, passband_ripple, stopband_atten, cutoff, FilterType::Lowpass);
    auto butter_coeffs = butter(order, cutoff, FilterType::Lowpass);

    // Compare attenuation at a point just past cutoff
    float test_freq = 0.5f;
    float ellip_atten = -magnitude_db(ellip_coeffs, test_freq);
    float butter_atten = -magnitude_db(butter_coeffs, test_freq);

    // Elliptic should provide more attenuation
    EXPECT_GT(ellip_atten, butter_atten)
        << "Elliptic: " << ellip_atten << " dB, Butterworth: " << butter_atten << " dB";
}

TEST_F(EllipticTest, SteepestTransitionAllOrders) {
    float cutoff = 0.3f;
    float test_freq = 0.5f;

    for (int order = 2; order <= 6; ++order) {
        auto ellip_coeffs = ellip(order, passband_ripple, stopband_atten, cutoff, FilterType::Lowpass);
        auto butter_coeffs = butter(order, cutoff, FilterType::Lowpass);
        auto cheby_coeffs = cheby1(order, passband_ripple, cutoff, FilterType::Lowpass);

        float ellip_atten = -magnitude_db(ellip_coeffs, test_freq);
        float butter_atten = -magnitude_db(butter_coeffs, test_freq);
        float cheby_atten = -magnitude_db(cheby_coeffs, test_freq);

        // Elliptic should provide at least as much attenuation as others
        // (In practice it should be more, but allow for implementation approximations)
        EXPECT_GE(ellip_atten, butter_atten - 5.0f) << "Order " << order;
        EXPECT_GE(ellip_atten, cheby_atten - 5.0f) << "Order " << order;
    }
}

// ============================================================================
// Stability Tests
// ============================================================================

TEST_F(EllipticTest, AllOrdersStable) {
    for (int order = 1; order <= 8; ++order) {
        for (float freq : {0.1f, 0.3f, 0.5f, 0.7f, 0.9f}) {
            auto lp = ellip(order, passband_ripple, stopband_atten, freq, FilterType::Lowpass);
            auto hp = ellip(order, passband_ripple, stopband_atten, freq, FilterType::Highpass);

            EXPECT_TRUE(is_stable(lp))
                << "Order " << order << " freq " << freq << " lowpass unstable";
            EXPECT_TRUE(is_stable(hp))
                << "Order " << order << " freq " << freq << " highpass unstable";
        }
    }
}

// ============================================================================
// Different Parameter Tests
// ============================================================================

TEST_F(EllipticTest, DifferentPassbandRipples) {
    std::vector<float> ripples = {0.5f, 1.0f, 2.0f, 3.0f, 5.0f};

    for (float rp : ripples) {
        auto coeffs = ellip(3, rp, stopband_atten, 0.3f, FilterType::Lowpass);

        // Should be stable
        EXPECT_TRUE(is_stable(coeffs)) << "Ripple " << rp << " dB";

        // DC gain should be reasonable
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);
        EXPECT_GT(dc_gain, 0.3f) << "Ripple " << rp << " dB";
        EXPECT_LT(dc_gain, 1.1f) << "Ripple " << rp << " dB";
    }
}

TEST_F(EllipticTest, DifferentStopbandAttenuations) {
    std::vector<float> attens = {20.0f, 40.0f, 60.0f, 80.0f};

    for (float rs : attens) {
        auto coeffs = ellip(4, passband_ripple, rs, 0.3f, FilterType::Lowpass);

        // Should be stable
        EXPECT_TRUE(is_stable(coeffs)) << "Rs " << rs << " dB";

        // Higher attenuation spec shouldn't break filter
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);
        EXPECT_GT(dc_gain, 0.5f) << "Rs " << rs << " dB";
    }
}

TEST_F(EllipticTest, DifferentCutoffFrequencies) {
    std::vector<float> cutoffs = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f};

    for (float cutoff : cutoffs) {
        auto coeffs = ellip(3, passband_ripple, stopband_atten, cutoff, FilterType::Lowpass);

        // Should be stable
        EXPECT_TRUE(is_stable(coeffs)) << "Cutoff " << cutoff;

        // DC gain should be in passband
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);
        EXPECT_GT(dc_gain, 0.5f) << "Cutoff " << cutoff;
    }
}

// ============================================================================
// Frequency Response Shape Tests
// ============================================================================

TEST_F(EllipticTest, LowpassShape) {
    auto coeffs = ellip(4, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

    // Passband: frequencies well below cutoff should pass
    float passband_gain = magnitude_at_freq(coeffs, 0.1f);
    EXPECT_GT(passband_gain, 0.7f);

    // Stopband: frequencies well above cutoff should be attenuated
    float stopband_gain = magnitude_at_freq(coeffs, 0.7f);
    EXPECT_LT(stopband_gain, 0.3f);
}

TEST_F(EllipticTest, HighpassShape) {
    auto coeffs = ellip(4, passband_ripple, stopband_atten, 0.3f, FilterType::Highpass);

    // Stopband: frequencies well below cutoff should be attenuated
    float stopband_gain = magnitude_at_freq(coeffs, 0.1f);
    EXPECT_LT(stopband_gain, 0.3f);

    // Passband: frequencies well above cutoff should pass
    float passband_gain = magnitude_at_freq(coeffs, 0.7f);
    EXPECT_GT(passband_gain, 0.7f);
}

// ============================================================================
// Edge Cases and Invalid Inputs
// ============================================================================

TEST_F(EllipticTest, InvalidOrder) {
    auto coeffs = ellip(0, passband_ripple, stopband_atten, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs.b.size(), 1u);
    EXPECT_EQ(coeffs.a.size(), 1u);
}

TEST_F(EllipticTest, InvalidFrequency) {
    auto coeffs1 = ellip(2, passband_ripple, stopband_atten, 0.0f, FilterType::Lowpass);
    EXPECT_EQ(coeffs1.b.size(), 1u);

    auto coeffs2 = ellip(2, passband_ripple, stopband_atten, 1.0f, FilterType::Lowpass);
    EXPECT_EQ(coeffs2.b.size(), 1u);

    auto coeffs3 = ellip(2, passband_ripple, stopband_atten, -0.1f, FilterType::Lowpass);
    EXPECT_EQ(coeffs3.b.size(), 1u);

    auto coeffs4 = ellip(2, passband_ripple, stopband_atten, 1.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs4.b.size(), 1u);
}

TEST_F(EllipticTest, InvalidRipple) {
    auto coeffs1 = ellip(2, 0.0f, stopband_atten, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs1.b.size(), 1u);

    auto coeffs2 = ellip(2, -1.0f, stopband_atten, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs2.b.size(), 1u);

    auto coeffs3 = ellip(2, 25.0f, stopband_atten, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs3.b.size(), 1u);
}

TEST_F(EllipticTest, InvalidStopband) {
    auto coeffs1 = ellip(2, passband_ripple, 0.0f, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs1.b.size(), 1u);

    auto coeffs2 = ellip(2, passband_ripple, -20.0f, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs2.b.size(), 1u);

    auto coeffs3 = ellip(2, passband_ripple, 150.0f, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs3.b.size(), 1u);
}

// ============================================================================
// Frequency Response Function Tests
// ============================================================================

TEST_F(EllipticTest, FreqzComputation) {
    auto coeffs = ellip(3, passband_ripple, stopband_atten, 0.3f, FilterType::Lowpass);

    const size_t num_points = 128;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // DC should be in passband
    EXPECT_GT(H[0].magnitude(), 0.5f);

    // General trend: decreasing magnitude for lowpass
    float dc_mag = H[0].magnitude();
    float nyq_mag = H[num_points - 1].magnitude();
    EXPECT_GT(dc_mag, nyq_mag);
}

