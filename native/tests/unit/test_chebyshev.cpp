/**
 * @file test_chebyshev.cpp
 * @brief Unit tests for Chebyshev filter design
 */

#include <gtest/gtest.h>
#include "spectra/filters/filter_design.hpp"
#include <cmath>
#include <complex>

using namespace spectra;

class ChebyshevTest : public ::testing::Test {
protected:
    // Helper to compute frequency response magnitude at a given frequency
    float magnitude_at_freq(const FilterCoefficients& coeffs, float norm_freq) {
        std::complex<float> z = std::polar(1.0f, constants::pi * norm_freq);

        std::complex<float> num(0.0f, 0.0f);
        std::complex<float> z_inv = 1.0f / z;
        std::complex<float> z_pow = 1.0f;
        for (size_t i = 0; i < coeffs.b.size(); ++i) {
            num += coeffs.b[i] * z_pow;
            z_pow *= z_inv;
        }

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
// Chebyshev Type I Basic Construction Tests
// ============================================================================

TEST_F(ChebyshevTest, Order1Lowpass) {
    auto coeffs = cheby1(1, 1.0f, 0.5f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 2u);
    EXPECT_EQ(coeffs.a.size(), 2u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(ChebyshevTest, Order2Lowpass) {
    auto coeffs = cheby1(2, 1.0f, 0.5f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 3u);
    EXPECT_EQ(coeffs.a.size(), 3u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(ChebyshevTest, Order4Lowpass) {
    auto coeffs = cheby1(4, 1.0f, 0.5f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 5u);
    EXPECT_EQ(coeffs.a.size(), 5u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(ChebyshevTest, OrdersUpTo8) {
    for (int order = 1; order <= 8; ++order) {
        auto coeffs = cheby1(order, 0.5f, 0.3f, FilterType::Lowpass);

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

TEST_F(ChebyshevTest, LowpassDCGain) {
    // For Chebyshev Type I:
    // - Odd order: DC is at passband maximum (gain = 1)
    // - Even order: DC is at passband minimum (gain = 1/sqrt(1+eps^2))
    float ripple_db = 1.0f;
    float epsilon = std::sqrt(std::pow(10.0f, ripple_db / 10.0f) - 1.0f);
    float min_gain = 1.0f / std::sqrt(1.0f + epsilon * epsilon);

    for (int order = 1; order <= 6; ++order) {
        auto coeffs = cheby1(order, ripple_db, 0.3f, FilterType::Lowpass);
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);

        if (order % 2 == 1) {
            // Odd order: DC at maximum
            EXPECT_NEAR(dc_gain, 1.0f, 0.02f)
                << "Order " << order << " (odd) DC gain: " << dc_gain;
        } else {
            // Even order: DC at minimum
            EXPECT_NEAR(dc_gain, min_gain, 0.02f)
                << "Order " << order << " (even) DC gain: " << dc_gain
                << " expected: " << min_gain;
        }
    }
}

TEST_F(ChebyshevTest, HighpassZeroDCGain) {
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = cheby1(order, 1.0f, 0.3f, FilterType::Highpass);
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);

        EXPECT_NEAR(dc_gain, 0.0f, 0.01f)
            << "Order " << order << " DC gain: " << dc_gain;
    }
}

TEST_F(ChebyshevTest, HighpassNyquistGain) {
    // For Chebyshev Type I highpass:
    // - Odd order: Nyquist is at passband maximum (gain = 1)
    // - Even order: Nyquist is at passband minimum (gain = 1/sqrt(1+eps^2))
    float ripple_db = 1.0f;
    float epsilon = std::sqrt(std::pow(10.0f, ripple_db / 10.0f) - 1.0f);
    float min_gain = 1.0f / std::sqrt(1.0f + epsilon * epsilon);

    for (int order = 1; order <= 6; ++order) {
        auto coeffs = cheby1(order, ripple_db, 0.3f, FilterType::Highpass);
        float nyq_gain = magnitude_at_freq(coeffs, 1.0f);

        if (order % 2 == 1) {
            EXPECT_NEAR(nyq_gain, 1.0f, 0.1f)
                << "Order " << order << " (odd) Nyquist gain: " << nyq_gain;
        } else {
            EXPECT_NEAR(nyq_gain, min_gain, 0.1f)
                << "Order " << order << " (even) Nyquist gain: " << nyq_gain;
        }
    }
}

// ============================================================================
// Passband Ripple Tests (Key Chebyshev Property)
// ============================================================================

TEST_F(ChebyshevTest, PassbandRippleMatches1dB) {
    float ripple_db = 1.0f;
    // Use odd order so DC is at maximum (gain = 1)
    auto coeffs = cheby1(3, ripple_db, 0.3f, FilterType::Lowpass);

    // DC gain should be 1 for odd order
    float dc_gain = magnitude_at_freq(coeffs, 0.0f);
    EXPECT_NEAR(dc_gain, 1.0f, 0.02f);

    // At passband edge (cutoff), magnitude should be down by Rp dB
    float cutoff_gain = magnitude_at_freq(coeffs, 0.3f);
    float cutoff_db = 20.0f * std::log10(cutoff_gain);

    // For Chebyshev I, gain at cutoff is 1/sqrt(1+eps^2) = 10^(-Rp/20)
    float expected_gain = std::pow(10.0f, -ripple_db / 20.0f);
    EXPECT_NEAR(cutoff_gain, expected_gain, 0.1f)
        << "Cutoff gain: " << cutoff_gain << " dB: " << cutoff_db;
}

TEST_F(ChebyshevTest, PassbandRippleMatches3dB) {
    float ripple_db = 3.0f;
    // Use odd order so DC is at maximum (gain = 1)
    auto coeffs = cheby1(3, ripple_db, 0.3f, FilterType::Lowpass);

    float dc_gain = magnitude_at_freq(coeffs, 0.0f);
    EXPECT_NEAR(dc_gain, 1.0f, 0.02f);

    float cutoff_gain = magnitude_at_freq(coeffs, 0.3f);
    float expected_gain = std::pow(10.0f, -ripple_db / 20.0f);
    EXPECT_NEAR(cutoff_gain, expected_gain, 0.1f);
}

TEST_F(ChebyshevTest, PassbandHasRipple) {
    auto coeffs = cheby1(4, 1.0f, 0.5f, FilterType::Lowpass);

    // Sample passband at multiple points
    float min_gain = 2.0f, max_gain = 0.0f;
    for (float f = 0.0f; f < 0.5f; f += 0.05f) {
        float gain = magnitude_at_freq(coeffs, f);
        min_gain = std::min(min_gain, gain);
        max_gain = std::max(max_gain, gain);
    }

    // There should be some ripple variation in the passband
    float ripple_range = max_gain - min_gain;

    // Ripple should be present but bounded
    EXPECT_GT(ripple_range, 0.01f) << "No ripple detected";
    EXPECT_LT(ripple_range, 0.3f) << "Ripple too large";
}

// ============================================================================
// Comparison with Butterworth (Chebyshev should have steeper rolloff)
// ============================================================================

TEST_F(ChebyshevTest, SteeperThanButterworth) {
    float cutoff = 0.3f;
    float stopband_freq = 0.6f;
    int order = 4;

    auto butter_coeffs = butter(order, cutoff, FilterType::Lowpass);
    auto cheby_coeffs = cheby1(order, 1.0f, cutoff, FilterType::Lowpass);

    float butter_atten = -magnitude_db(butter_coeffs, stopband_freq);
    float cheby_atten = -magnitude_db(cheby_coeffs, stopband_freq);

    // Chebyshev should have more attenuation in stopband for same order
    EXPECT_GT(cheby_atten, butter_atten)
        << "Butterworth: " << butter_atten << "dB, Chebyshev: " << cheby_atten << "dB";
}

TEST_F(ChebyshevTest, SteeperRolloffAllOrders) {
    // Compare at a deeper stopband frequency where Chebyshev advantage is clearer
    float cutoff = 0.2f;
    float stopband_freq = 0.6f;  // Further into stopband

    for (int order = 3; order <= 6; ++order) {  // Start from order 3
        auto butter_coeffs = butter(order, cutoff, FilterType::Lowpass);
        // Use higher ripple for more aggressive transition band
        auto cheby_coeffs = cheby1(order, 1.0f, cutoff, FilterType::Lowpass);

        float butter_atten = -magnitude_db(butter_coeffs, stopband_freq);
        float cheby_atten = -magnitude_db(cheby_coeffs, stopband_freq);

        EXPECT_GT(cheby_atten, butter_atten)
            << "Order " << order << ": Butterworth " << butter_atten
            << "dB, Chebyshev " << cheby_atten << "dB";
    }
}

// ============================================================================
// Stability Tests
// ============================================================================

TEST_F(ChebyshevTest, AllOrdersStable) {
    for (int order = 1; order <= 8; ++order) {
        for (float freq : {0.1f, 0.3f, 0.5f, 0.7f, 0.9f}) {
            auto lp = cheby1(order, 1.0f, freq, FilterType::Lowpass);
            auto hp = cheby1(order, 1.0f, freq, FilterType::Highpass);

            EXPECT_TRUE(is_stable(lp))
                << "Order " << order << " freq " << freq << " lowpass unstable";
            EXPECT_TRUE(is_stable(hp))
                << "Order " << order << " freq " << freq << " highpass unstable";
        }
    }
}

// ============================================================================
// Monotonic Stopband (Chebyshev Type I property)
// ============================================================================

TEST_F(ChebyshevTest, MonotonicStopband) {
    auto coeffs = cheby1(4, 1.0f, 0.3f, FilterType::Lowpass);

    // In the stopband, magnitude should monotonically decrease
    float prev_mag = magnitude_at_freq(coeffs, 0.4f);
    for (float f = 0.5f; f <= 1.0f; f += 0.1f) {
        float mag = magnitude_at_freq(coeffs, f);
        EXPECT_LE(mag, prev_mag + 0.01f)  // Allow small numerical error
            << "Non-monotonic at freq " << f;
        prev_mag = mag;
    }
}

// ============================================================================
// Higher Order = More Attenuation
// ============================================================================

TEST_F(ChebyshevTest, HigherOrderMoreAttenuation) {
    float cutoff = 0.2f;
    float stopband_freq = 0.5f;

    float prev_atten = 0.0f;
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = cheby1(order, 1.0f, cutoff, FilterType::Lowpass);
        float atten = -magnitude_db(coeffs, stopband_freq);

        EXPECT_GT(atten, prev_atten)
            << "Order " << order << " should have more attenuation";
        prev_atten = atten;
    }
}

// ============================================================================
// Different Ripple Values
// ============================================================================

TEST_F(ChebyshevTest, DifferentRippleValues) {
    std::vector<float> ripples = {0.1f, 0.5f, 1.0f, 2.0f, 3.0f};

    for (float ripple : ripples) {
        // Use odd order so DC is at passband maximum
        auto coeffs = cheby1(3, ripple, 0.3f, FilterType::Lowpass);

        // DC gain should be 1 for odd order
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);
        EXPECT_NEAR(dc_gain, 1.0f, 0.02f) << "Ripple " << ripple;

        // Should be stable
        EXPECT_TRUE(is_stable(coeffs)) << "Ripple " << ripple;

        // Cutoff gain should match ripple spec
        float cutoff_gain = magnitude_at_freq(coeffs, 0.3f);
        float expected_gain = std::pow(10.0f, -ripple / 20.0f);
        EXPECT_NEAR(cutoff_gain, expected_gain, 0.15f)
            << "Ripple " << ripple << " dB";
    }
}

// ============================================================================
// Different Cutoff Frequencies
// ============================================================================

TEST_F(ChebyshevTest, DifferentCutoffFrequencies) {
    std::vector<float> cutoffs = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f};

    for (float cutoff : cutoffs) {
        auto coeffs = cheby1(3, 1.0f, cutoff, FilterType::Lowpass);

        // DC gain should be 1
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);
        EXPECT_NEAR(dc_gain, 1.0f, 0.01f) << "Cutoff " << cutoff;

        // Should be stable
        EXPECT_TRUE(is_stable(coeffs)) << "Cutoff " << cutoff;
    }
}

// ============================================================================
// Highpass Tests
// ============================================================================

TEST_F(ChebyshevTest, HighpassMonotonicIncrease) {
    auto coeffs = cheby1(4, 1.0f, 0.3f, FilterType::Highpass);

    // Below cutoff, magnitude should increase toward cutoff
    float prev_mag = magnitude_at_freq(coeffs, 0.0f);
    for (float f = 0.05f; f <= 0.25f; f += 0.05f) {
        float mag = magnitude_at_freq(coeffs, f);
        EXPECT_GE(mag, prev_mag - 0.01f)
            << "Non-monotonic at freq " << f;
        prev_mag = mag;
    }
}

TEST_F(ChebyshevTest, HighpassBasicBehavior) {
    float ripple_db = 1.0f;
    auto coeffs = cheby1(3, ripple_db, 0.3f, FilterType::Highpass);

    // DC should be attenuated (stopband)
    float dc_gain = magnitude_at_freq(coeffs, 0.0f);
    EXPECT_LT(dc_gain, 0.01f) << "DC should be in stopband";

    // Deep stopband should be attenuated
    float stopband_gain = magnitude_at_freq(coeffs, 0.1f);
    EXPECT_LT(stopband_gain, 0.5f) << "Stopband not attenuated enough";

    // High frequencies (passband) should have significant gain
    float passband_gain = magnitude_at_freq(coeffs, 0.8f);
    EXPECT_GT(passband_gain, 0.5f) << "Passband should have gain";

    // Nyquist should have reasonable gain (within ripple tolerance)
    float nyq_gain = magnitude_at_freq(coeffs, 1.0f);
    EXPECT_GT(nyq_gain, 0.7f) << "Nyquist should be in passband";
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ChebyshevTest, InvalidOrder) {
    auto coeffs = cheby1(0, 1.0f, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs.b.size(), 1u);
    EXPECT_EQ(coeffs.a.size(), 1u);
}

TEST_F(ChebyshevTest, InvalidFrequency) {
    auto coeffs1 = cheby1(2, 1.0f, 0.0f, FilterType::Lowpass);
    EXPECT_EQ(coeffs1.b.size(), 1u);

    auto coeffs2 = cheby1(2, 1.0f, 1.0f, FilterType::Lowpass);
    EXPECT_EQ(coeffs2.b.size(), 1u);

    auto coeffs3 = cheby1(2, 1.0f, -0.1f, FilterType::Lowpass);
    EXPECT_EQ(coeffs3.b.size(), 1u);

    auto coeffs4 = cheby1(2, 1.0f, 1.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs4.b.size(), 1u);
}

TEST_F(ChebyshevTest, InvalidRipple) {
    auto coeffs1 = cheby1(2, 0.0f, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs1.b.size(), 1u);

    auto coeffs2 = cheby1(2, -1.0f, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs2.b.size(), 1u);
}

// ============================================================================
// Frequency Response Tests
// ============================================================================

TEST_F(ChebyshevTest, FreqzComputation) {
    auto coeffs = cheby1(3, 1.0f, 0.3f, FilterType::Lowpass);

    const size_t num_points = 128;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // DC should have magnitude ~1
    EXPECT_NEAR(H[0].magnitude(), 1.0f, 0.01f);

    // Magnitude should generally decrease (with ripple in passband)
    float dc_mag = H[0].magnitude();
    float nyq_mag = H[num_points - 1].magnitude();
    EXPECT_LT(nyq_mag, dc_mag);
}

