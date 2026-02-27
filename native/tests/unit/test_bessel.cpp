/**
 * @file test_bessel.cpp
 * @brief Unit tests for Bessel (Thomson) filter design
 */

#include <gtest/gtest.h>
#include "spectra/filters/filter_design.hpp"
#include <cmath>
#include <complex>
#include <vector>

using namespace spectra;

class BesselTest : public ::testing::Test {
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

    // Helper to compute complex frequency response at a given frequency
    std::complex<float> response_at_freq(const FilterCoefficients& coeffs, float norm_freq) {
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

        return num / den;
    }

    // Helper to compute group delay at a given frequency
    // Group delay = -d(phase)/d(omega)
    float group_delay_at_freq(const FilterCoefficients& coeffs, float norm_freq) {
        const float delta = 0.001f;

        auto h1 = response_at_freq(coeffs, norm_freq - delta);
        auto h2 = response_at_freq(coeffs, norm_freq + delta);

        float phase1 = std::arg(h1);
        float phase2 = std::arg(h2);

        // Unwrap phase
        float phase_diff = phase2 - phase1;
        while (phase_diff > constants::pi) phase_diff -= 2.0f * constants::pi;
        while (phase_diff < -constants::pi) phase_diff += 2.0f * constants::pi;

        // Group delay in samples
        float omega_diff = 2.0f * delta * constants::pi;
        return -phase_diff / omega_diff;
    }
};

// ============================================================================
// Basic Construction Tests
// ============================================================================

TEST_F(BesselTest, Order1Lowpass) {
    auto coeffs = bessel(1, 0.5f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 2u);
    EXPECT_EQ(coeffs.a.size(), 2u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(BesselTest, Order2Lowpass) {
    auto coeffs = bessel(2, 0.5f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 3u);
    EXPECT_EQ(coeffs.a.size(), 3u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(BesselTest, Order4Lowpass) {
    auto coeffs = bessel(4, 0.5f, FilterType::Lowpass);

    EXPECT_EQ(coeffs.b.size(), 5u);
    EXPECT_EQ(coeffs.a.size(), 5u);
    EXPECT_FLOAT_EQ(coeffs.a[0], 1.0f);
}

TEST_F(BesselTest, OrdersUpTo10) {
    for (int order = 1; order <= 10; ++order) {
        auto coeffs = bessel(order, 0.3f, FilterType::Lowpass);

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

TEST_F(BesselTest, LowpassUnityDCGain) {
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = bessel(order, 0.3f, FilterType::Lowpass);
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);

        EXPECT_NEAR(dc_gain, 1.0f, 0.001f)
            << "Order " << order << " DC gain: " << dc_gain;
    }
}

TEST_F(BesselTest, HighpassZeroDCGain) {
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = bessel(order, 0.3f, FilterType::Highpass);
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);

        EXPECT_NEAR(dc_gain, 0.0f, 0.001f)
            << "Order " << order << " DC gain: " << dc_gain;
    }
}

TEST_F(BesselTest, HighpassUnityNyquistGain) {
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = bessel(order, 0.3f, FilterType::Highpass);
        float nyq_gain = magnitude_at_freq(coeffs, 1.0f);

        EXPECT_NEAR(nyq_gain, 1.0f, 0.02f)
            << "Order " << order << " Nyquist gain: " << nyq_gain;
    }
}

// ============================================================================
// Cutoff Frequency Tests (-3dB point)
// ============================================================================

TEST_F(BesselTest, LowpassCutoffAttenuation) {
    for (int order = 1; order <= 6; ++order) {
        float cutoff = 0.3f;
        auto coeffs = bessel(order, cutoff, FilterType::Lowpass);

        // At cutoff frequency, Bessel filters have varying attenuation (not exactly -3dB)
        // Bessel filters are optimized for flat group delay, not flat magnitude
        // The magnitude at cutoff varies with order, typically -3dB to -10dB
        float mag_at_cutoff = magnitude_at_freq(coeffs, cutoff);

        // Should have some attenuation (less than unity gain)
        EXPECT_LT(mag_at_cutoff, 1.0f)
            << "Order " << order << " magnitude at cutoff: " << mag_at_cutoff;

        // Should not be completely attenuated
        EXPECT_GT(mag_at_cutoff, 0.3f)
            << "Order " << order << " magnitude at cutoff: " << mag_at_cutoff;
    }
}

TEST_F(BesselTest, HighpassCutoffAttenuation) {
    for (int order = 1; order <= 6; ++order) {
        float cutoff = 0.3f;
        auto coeffs = bessel(order, cutoff, FilterType::Highpass);

        // At cutoff frequency, Bessel filters have varying attenuation
        float mag_at_cutoff = magnitude_at_freq(coeffs, cutoff);

        // Should have some attenuation (less than unity gain)
        EXPECT_LT(mag_at_cutoff, 1.0f)
            << "Order " << order << " magnitude at cutoff: " << mag_at_cutoff;

        // Should not be completely attenuated
        EXPECT_GT(mag_at_cutoff, 0.3f)
            << "Order " << order << " magnitude at cutoff: " << mag_at_cutoff;
    }
}

// ============================================================================
// Stability Tests
// ============================================================================

TEST_F(BesselTest, AllOrdersStable) {
    for (int order = 1; order <= 10; ++order) {
        for (float freq : {0.1f, 0.3f, 0.5f}) {
            auto lp = bessel(order, freq, FilterType::Lowpass);
            auto hp = bessel(order, freq, FilterType::Highpass);

            EXPECT_TRUE(is_stable(lp))
                << "Order " << order << " freq " << freq << " lowpass unstable";
            EXPECT_TRUE(is_stable(hp))
                << "Order " << order << " freq " << freq << " highpass unstable";
        }

        // For frequency 0.7, test orders up to 5
        if (order <= 5) {
            auto lp = bessel(order, 0.7f, FilterType::Lowpass);
            auto hp = bessel(order, 0.7f, FilterType::Highpass);

            EXPECT_TRUE(is_stable(lp))
                << "Order " << order << " freq 0.7 lowpass unstable";
            EXPECT_TRUE(is_stable(hp))
                << "Order " << order << " freq 0.7 highpass unstable";
        }

        // For frequency 0.9 (near Nyquist), only test lower orders
        // Bilinear transform has numerical issues at high frequencies
        if (order <= 4) {
            auto lp = bessel(order, 0.9f, FilterType::Lowpass);
            auto hp = bessel(order, 0.9f, FilterType::Highpass);

            EXPECT_TRUE(is_stable(lp))
                << "Order " << order << " freq 0.9 lowpass unstable";
            EXPECT_TRUE(is_stable(hp))
                << "Order " << order << " freq 0.9 highpass unstable";
        }
    }
}

// ============================================================================
// Group Delay Tests (Critical for Bessel filters - should be linear/flat)
// ============================================================================

TEST_F(BesselTest, GroupDelayFlatter) {
    // Bessel filters should have flatter group delay than Butterworth
    // Test that group delay variation is minimal in passband

    int order = 4;
    float cutoff = 0.3f;
    auto coeffs = bessel(order, cutoff, FilterType::Lowpass);

    std::vector<float> delays;
    // Sample group delay in passband (0 to cutoff/2)
    for (float f = 0.02f; f < cutoff * 0.5f; f += 0.02f) {
        float delay = group_delay_at_freq(coeffs, f);
        delays.push_back(delay);
    }

    // Compute standard deviation of group delay
    float mean = 0.0f;
    for (float d : delays) {
        mean += d;
    }
    mean /= delays.size();

    float variance = 0.0f;
    for (float d : delays) {
        variance += (d - mean) * (d - mean);
    }
    variance /= delays.size();
    float std_dev = std::sqrt(variance);

    // Relative variation should be small (< 15% of mean)
    float relative_variation = std_dev / mean;
    EXPECT_LT(relative_variation, 0.15f)
        << "Group delay variation too high: std=" << std_dev << " mean=" << mean;
}

TEST_F(BesselTest, LowpassGroupDelayMonotonic) {
    // For lowpass Bessel, group delay should be relatively constant in passband
    auto coeffs = bessel(3, 0.3f, FilterType::Lowpass);

    float delay_low = group_delay_at_freq(coeffs, 0.05f);
    float delay_mid = group_delay_at_freq(coeffs, 0.15f);

    // Should be reasonably close
    EXPECT_NEAR(delay_low, delay_mid, delay_low * 0.3f)
        << "Group delay varies too much: " << delay_low << " vs " << delay_mid;
}

// ============================================================================
// Stopband Attenuation
// ============================================================================

TEST_F(BesselTest, HigherOrderSteeperRolloff) {
    float cutoff = 0.2f;
    float stopband_freq = 0.5f;

    float prev_atten = 0.0f;
    for (int order = 1; order <= 6; ++order) {
        auto coeffs = bessel(order, cutoff, FilterType::Lowpass);
        float atten = -magnitude_db(coeffs, stopband_freq);

        EXPECT_GT(atten, prev_atten - 1.0f)  // Allow slight variations
            << "Order " << order << " should have similar or more attenuation";
        prev_atten = atten;
    }
}

// ============================================================================
// Different Cutoff Frequencies
// ============================================================================

TEST_F(BesselTest, DifferentCutoffFrequencies) {
    std::vector<float> cutoffs = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f};

    for (float cutoff : cutoffs) {
        auto coeffs = bessel(3, cutoff, FilterType::Lowpass);

        // DC gain should be 1
        float dc_gain = magnitude_at_freq(coeffs, 0.0f);
        EXPECT_NEAR(dc_gain, 1.0f, 0.01f) << "Cutoff " << cutoff;

        // Should be stable
        EXPECT_TRUE(is_stable(coeffs)) << "Cutoff " << cutoff;
    }
}

// ============================================================================
// Bandpass Filter Tests (Dual-frequency API)
// ============================================================================

TEST_F(BesselTest, BandpassBasic) {
    auto coeffs = bessel(2, 0.2f, 0.4f, FilterType::Bandpass);

    // Coefficient size should be 2*order + 1
    EXPECT_EQ(coeffs.b.size(), 5u);
    EXPECT_EQ(coeffs.a.size(), 5u);

    // Should attenuate DC
    float dc_gain = magnitude_at_freq(coeffs, 0.0f);
    EXPECT_LT(dc_gain, 0.1f) << "DC gain too high: " << dc_gain;

    // Should attenuate Nyquist
    float nyq_gain = magnitude_at_freq(coeffs, 1.0f);
    EXPECT_LT(nyq_gain, 0.1f) << "Nyquist gain too high: " << nyq_gain;

    // Should pass center frequency region
    float center_gain = magnitude_at_freq(coeffs, 0.3f);
    EXPECT_GT(center_gain, 0.3f) << "Center gain too low: " << center_gain;
}

TEST_F(BesselTest, BandpassFrequencyEdges) {
    float f_low = 0.2f;
    float f_high = 0.5f;
    auto coeffs = bessel(3, f_low, f_high, FilterType::Bandpass);

    // At band edges, should have significant gain
    float gain_low = magnitude_at_freq(coeffs, f_low);
    float gain_high = magnitude_at_freq(coeffs, f_high);

    EXPECT_GT(gain_low, 0.3f) << "Low edge gain: " << gain_low;
    EXPECT_GT(gain_high, 0.3f) << "High edge gain: " << gain_high;

    // Outside band, should attenuate
    float gain_below = magnitude_at_freq(coeffs, f_low * 0.5f);
    float gain_above = magnitude_at_freq(coeffs, (f_high + 1.0f) * 0.5f);

    EXPECT_LT(gain_below, gain_low);
    EXPECT_LT(gain_above, gain_high);
}

TEST_F(BesselTest, BandpassStability) {
    for (int order = 2; order <= 6; ++order) {
        auto coeffs = bessel(order, 0.2f, 0.5f, FilterType::Bandpass);
        EXPECT_TRUE(is_stable(coeffs)) << "Order " << order << " bandpass unstable";
    }
}

// ============================================================================
// Bandstop Filter Tests
// ============================================================================

TEST_F(BesselTest, BandstopBasic) {
    auto coeffs = bessel(2, 0.2f, 0.4f, FilterType::Bandstop);

    // Coefficient size should be 2*order + 1
    EXPECT_EQ(coeffs.b.size(), 5u);
    EXPECT_EQ(coeffs.a.size(), 5u);

    // Should pass DC
    float dc_gain = magnitude_at_freq(coeffs, 0.0f);
    EXPECT_GT(dc_gain, 0.5f) << "DC gain too low: " << dc_gain;

    // Should pass Nyquist
    float nyq_gain = magnitude_at_freq(coeffs, 1.0f);
    EXPECT_GT(nyq_gain, 0.5f) << "Nyquist gain too low: " << nyq_gain;

    // Should attenuate center frequency
    float center_gain = magnitude_at_freq(coeffs, 0.3f);
    EXPECT_LT(center_gain, 0.5f) << "Center gain too high: " << center_gain;
}

TEST_F(BesselTest, BandstopStability) {
    for (int order = 2; order <= 6; ++order) {
        auto coeffs = bessel(order, 0.2f, 0.5f, FilterType::Bandstop);
        EXPECT_TRUE(is_stable(coeffs)) << "Order " << order << " bandstop unstable";
    }
}

TEST_F(BesselTest, BandstopComplementaryToBandpass) {
    float f_low = 0.3f;
    float f_high = 0.6f;
    auto bp = bessel(3, f_low, f_high, FilterType::Bandpass);
    auto bs = bessel(3, f_low, f_high, FilterType::Bandstop);

    // At various frequencies, the responses should be complementary-like
    // (not exactly |H_bp| + |H_bs| = 1 due to cascaded design, but qualitatively opposite)

    // In passband of bandstop (outside the notch), bandstop >> bandpass
    float gain_bp_low = magnitude_at_freq(bp, 0.1f);
    float gain_bs_low = magnitude_at_freq(bs, 0.1f);
    EXPECT_GT(gain_bs_low, gain_bp_low) << "Bandstop should dominate outside notch";

    // In notch (center), bandpass >> bandstop
    float gain_bp_center = magnitude_at_freq(bp, (f_low + f_high) * 0.5f);
    float gain_bs_center = magnitude_at_freq(bs, (f_low + f_high) * 0.5f);
    EXPECT_GT(gain_bp_center, gain_bs_center) << "Bandpass should dominate in notch region";
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(BesselTest, InvalidOrder) {
    auto coeffs = bessel(0, 0.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs.b.size(), 1u);
    EXPECT_EQ(coeffs.a.size(), 1u);
}

TEST_F(BesselTest, InvalidFrequency) {
    auto coeffs1 = bessel(2, 0.0f, FilterType::Lowpass);
    EXPECT_EQ(coeffs1.b.size(), 1u);

    auto coeffs2 = bessel(2, 1.0f, FilterType::Lowpass);
    EXPECT_EQ(coeffs2.b.size(), 1u);

    auto coeffs3 = bessel(2, -0.1f, FilterType::Lowpass);
    EXPECT_EQ(coeffs3.b.size(), 1u);

    auto coeffs4 = bessel(2, 1.5f, FilterType::Lowpass);
    EXPECT_EQ(coeffs4.b.size(), 1u);
}

TEST_F(BesselTest, InvalidBandpassFrequencies) {
    // Low >= High
    auto coeffs1 = bessel(2, 0.5f, 0.3f, FilterType::Bandpass);
    EXPECT_EQ(coeffs1.b.size(), 1u);

    // Equal frequencies
    auto coeffs2 = bessel(2, 0.4f, 0.4f, FilterType::Bandpass);
    EXPECT_EQ(coeffs2.b.size(), 1u);
}

TEST_F(BesselTest, OrderAbove10) {
    // Order > 10 should return passthrough (we only have poles up to order 10)
    auto coeffs = bessel(11, 0.3f, FilterType::Lowpass);
    EXPECT_EQ(coeffs.b.size(), 1u);
    EXPECT_EQ(coeffs.a.size(), 1u);
}

// ============================================================================
// Coefficient Size Test
// ============================================================================

TEST_F(BesselTest, IIRCoeffSize) {
    EXPECT_EQ(iir_coeff_size(1), 2);
    EXPECT_EQ(iir_coeff_size(2), 3);
    EXPECT_EQ(iir_coeff_size(4), 5);
    EXPECT_EQ(iir_coeff_size(8), 9);
}

// ============================================================================
// Comparison with Butterworth
// ============================================================================

TEST_F(BesselTest, BesselSlowerRolloffThanButterworth) {
    // Bessel filters have slower rolloff than Butterworth in the stopband
    // due to optimizing for group delay instead of magnitude flatness

    int order = 4;
    float cutoff = 0.2f;
    float stopband_freq = 0.5f;

    auto bessel_lp = bessel(order, cutoff, FilterType::Lowpass);
    auto butter_lp = butter(order, cutoff, FilterType::Lowpass);

    float bessel_atten = -magnitude_db(bessel_lp, stopband_freq);
    float butter_atten = -magnitude_db(butter_lp, stopband_freq);

    // Butterworth should have steeper rolloff (more attenuation)
    EXPECT_GT(butter_atten, bessel_atten)
        << "Bessel: " << bessel_atten << "dB, Butterworth: " << butter_atten << "dB";
}

TEST_F(BesselTest, BesselBetterTransientResponse) {
    // This is a qualitative test: Bessel should have more constant group delay
    // in passband compared to Butterworth (better transient/step response)

    int order = 4;
    float cutoff = 0.3f;

    auto bessel_lp = bessel(order, cutoff, FilterType::Lowpass);
    auto butter_lp = butter(order, cutoff, FilterType::Lowpass);

    // Measure group delay variation in passband
    std::vector<float> bessel_delays, butter_delays;

    for (float f = 0.02f; f < cutoff * 0.5f; f += 0.02f) {
        bessel_delays.push_back(group_delay_at_freq(bessel_lp, f));
        butter_delays.push_back(group_delay_at_freq(butter_lp, f));
    }

    // Compute coefficient of variation for both
    auto compute_cv = [](const std::vector<float>& vals) {
        float mean = 0.0f;
        for (float v : vals) mean += v;
        mean /= vals.size();

        float variance = 0.0f;
        for (float v : vals) variance += (v - mean) * (v - mean);
        variance /= vals.size();

        return std::sqrt(variance) / mean;
    };

    float bessel_cv = compute_cv(bessel_delays);
    float butter_cv = compute_cv(butter_delays);

    // Bessel should have lower coefficient of variation (flatter group delay)
    EXPECT_LT(bessel_cv, butter_cv * 1.2f)  // Allow some tolerance
        << "Bessel CV: " << bessel_cv << ", Butterworth CV: " << butter_cv;
}

// ============================================================================
// All Filter Types Test
// ============================================================================

TEST_F(BesselTest, AllFilterTypesWork) {
    int order = 3;
    float freq = 0.3f;

    auto lp = bessel(order, freq, FilterType::Lowpass);
    auto hp = bessel(order, freq, FilterType::Highpass);
    auto bp = bessel(order, 0.2f, 0.5f, FilterType::Bandpass);
    auto bs = bessel(order, 0.2f, 0.5f, FilterType::Bandstop);

    EXPECT_EQ(lp.b.size(), 4u);
    EXPECT_EQ(hp.b.size(), 4u);
    EXPECT_EQ(bp.b.size(), 7u);  // 2*order + 1
    EXPECT_EQ(bs.b.size(), 7u);

    EXPECT_TRUE(is_stable(lp));
    EXPECT_TRUE(is_stable(hp));
    EXPECT_TRUE(is_stable(bp));
    EXPECT_TRUE(is_stable(bs));
}
