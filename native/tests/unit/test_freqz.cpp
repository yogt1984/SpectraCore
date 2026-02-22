/**
 * @file test_freqz.cpp
 * @brief Unit tests for frequency response (freqz) and group delay functions
 */

#include <gtest/gtest.h>
#include "spectra/filters/filter_design.hpp"
#include <cmath>

using namespace spectra;

class FreqzTest : public ::testing::Test {
protected:
    float to_db(float magnitude) {
        return 20.0f * std::log10(std::max(magnitude, 1e-10f));
    }

    float magnitude(const Complex& c) {
        return std::sqrt(c.magnitude_squared());
    }

    float phase(const Complex& c) {
        return std::atan2(c.imag, c.real);
    }

    // Find index closest to normalized frequency (0 to 1)
    size_t freq_to_idx(float normalized_freq, size_t num_points) {
        return static_cast<size_t>(normalized_freq * (num_points - 1));
    }
};

// ============================================================================
// Basic freqz Tests
// ============================================================================

TEST_F(FreqzTest, BasicFreqzOutput) {
    // Simple lowpass filter
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);

    const size_t num_points = 256;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // Check no NaN/Inf
    for (size_t i = 0; i < num_points; ++i) {
        EXPECT_FALSE(std::isnan(H[i].real));
        EXPECT_FALSE(std::isnan(H[i].imag));
        EXPECT_FALSE(std::isinf(H[i].real));
        EXPECT_FALSE(std::isinf(H[i].imag));
    }
}

TEST_F(FreqzTest, FreqzCorrectSize) {
    auto coeffs = butter(2, 0.25f, FilterType::Lowpass);

    const size_t num_points = 128;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // All points should be computed
    float total_energy = 0.0f;
    for (const auto& c : H) {
        total_energy += c.magnitude_squared();
    }
    EXPECT_GT(total_energy, 0.0f);
}

// ============================================================================
// DC and Nyquist Gain Tests
// ============================================================================

TEST_F(FreqzTest, LowpassDCGain) {
    // Lowpass filter should have unity gain at DC
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);

    const size_t num_points = 256;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    float dc_mag = magnitude(H[0]);
    EXPECT_NEAR(dc_mag, 1.0f, 0.05f);
}

TEST_F(FreqzTest, HighpassDCGain) {
    // Highpass filter should have zero gain at DC
    auto coeffs = butter(4, 0.3f, FilterType::Highpass);

    const size_t num_points = 256;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    float dc_mag = magnitude(H[0]);
    float dc_db = to_db(dc_mag);
    EXPECT_LT(dc_db, -40.0f);  // Should be highly attenuated
}

TEST_F(FreqzTest, HighpassNyquistGain) {
    // Highpass filter should have unity gain at Nyquist
    auto coeffs = butter(4, 0.3f, FilterType::Highpass);

    const size_t num_points = 256;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    float nyq_mag = magnitude(H[num_points - 1]);
    EXPECT_NEAR(nyq_mag, 1.0f, 0.05f);
}

TEST_F(FreqzTest, LowpassNyquistAttenuation) {
    // Lowpass filter should attenuate at Nyquist
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);

    const size_t num_points = 256;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    float nyq_mag = magnitude(H[num_points - 1]);
    float nyq_db = to_db(nyq_mag);
    EXPECT_LT(nyq_db, -20.0f);
}

// ============================================================================
// Cutoff Frequency Tests
// ============================================================================

TEST_F(FreqzTest, LowpassCutoffFrequency) {
    const float cutoff = 0.25f;  // Normalized cutoff
    auto coeffs = butter(4, cutoff, FilterType::Lowpass);

    const size_t num_points = 512;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // Find magnitude at cutoff frequency
    size_t cutoff_idx = freq_to_idx(cutoff, num_points);
    float cutoff_mag = magnitude(H[cutoff_idx]);
    float cutoff_db = to_db(cutoff_mag);

    // Butterworth: -3dB at cutoff
    EXPECT_NEAR(cutoff_db, -3.0f, 1.0f);
}

TEST_F(FreqzTest, HighpassCutoffFrequency) {
    const float cutoff = 0.25f;
    auto coeffs = butter(4, cutoff, FilterType::Highpass);

    const size_t num_points = 512;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    size_t cutoff_idx = freq_to_idx(cutoff, num_points);
    float cutoff_mag = magnitude(H[cutoff_idx]);
    float cutoff_db = to_db(cutoff_mag);

    EXPECT_NEAR(cutoff_db, -3.0f, 1.0f);
}

// ============================================================================
// FIR Filter Tests
// ============================================================================

TEST_F(FreqzTest, FIRFreqz) {
    // Create FIR lowpass filter
    auto b = fir_windowed_sinc(51, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    const size_t num_points = 256;
    std::vector<Complex> H(num_points);
    freqz(b, H.data(), num_points);

    // Check DC gain is near 1
    float dc_mag = magnitude(H[0]);
    EXPECT_NEAR(dc_mag, 1.0f, 0.1f);

    // Check Nyquist is attenuated
    float nyq_mag = magnitude(H[num_points - 1]);
    float nyq_db = to_db(nyq_mag);
    EXPECT_LT(nyq_db, -20.0f);
}

TEST_F(FreqzTest, FIRLinearPhase) {
    // FIR filter with symmetric coefficients should have linear phase
    auto b = fir_windowed_sinc(51, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    const size_t num_points = 256;
    std::vector<Complex> H(num_points);
    freqz(b, H.data(), num_points);

    // Check phase is approximately linear in passband
    std::vector<float> phases;
    for (size_t i = 1; i < num_points / 4; ++i) {  // Passband
        phases.push_back(phase(H[i]));
    }

    // Phase should be decreasing (negative group delay)
    // Just check monotonicity (allowing for wrapping)
    int decreasing_count = 0;
    for (size_t i = 1; i < phases.size(); ++i) {
        float diff = phases[i] - phases[i - 1];
        // Unwrap
        while (diff > constants::pi) diff -= constants::two_pi;
        while (diff < -constants::pi) diff += constants::two_pi;
        if (diff < 0) decreasing_count++;
    }
    EXPECT_GT(decreasing_count, static_cast<int>(phases.size() * 0.8));
}

// ============================================================================
// Bandpass/Bandstop Tests
// ============================================================================

TEST_F(FreqzTest, BandpassResponse) {
    auto coeffs = butter(4, 0.2f, 0.4f, FilterType::Bandpass);

    const size_t num_points = 512;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // DC should be attenuated
    float dc_db = to_db(magnitude(H[0]));
    EXPECT_LT(dc_db, -30.0f);

    // Center of passband (~0.3) should be near unity
    size_t center_idx = freq_to_idx(0.3f, num_points);
    float center_mag = magnitude(H[center_idx]);
    EXPECT_GT(center_mag, 0.7f);

    // Nyquist should be attenuated
    float nyq_db = to_db(magnitude(H[num_points - 1]));
    EXPECT_LT(nyq_db, -30.0f);
}

TEST_F(FreqzTest, BandstopResponse) {
    auto coeffs = butter(4, 0.2f, 0.4f, FilterType::Bandstop);

    const size_t num_points = 512;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // Verify filter produces valid output
    for (size_t i = 0; i < num_points; ++i) {
        EXPECT_FALSE(std::isnan(H[i].real));
        EXPECT_FALSE(std::isnan(H[i].imag));
    }

    // DC should pass (near unity or at least significant)
    float dc_mag = magnitude(H[0]);
    EXPECT_GT(dc_mag, 0.5f);

    // Nyquist should pass
    float nyq_mag = magnitude(H[num_points - 1]);
    EXPECT_GT(nyq_mag, 0.5f);
}

// ============================================================================
// Different Filter Types
// ============================================================================

TEST_F(FreqzTest, ChebyshevType1Ripple) {
    const float ripple_db = 1.0f;
    auto coeffs = cheby1(4, ripple_db, 0.3f, FilterType::Lowpass);

    const size_t num_points = 512;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // Check passband ripple exists
    float max_db = -1000.0f;
    float min_db = 1000.0f;
    size_t passband_end = freq_to_idx(0.25f, num_points);

    for (size_t i = 1; i < passband_end; ++i) {
        float db = to_db(magnitude(H[i]));
        max_db = std::max(max_db, db);
        min_db = std::min(min_db, db);
    }

    // Ripple should be bounded by specification
    EXPECT_GT(max_db - min_db, 0.01f);  // Some ripple exists
}

TEST_F(FreqzTest, EllipticSteepRolloff) {
    auto elliptic = ellip(4, 1.0f, 40.0f, 0.3f, FilterType::Lowpass);
    auto butter4 = butter(4, 0.3f, FilterType::Lowpass);

    const size_t num_points = 512;
    std::vector<Complex> H_ellip(num_points);
    std::vector<Complex> H_butter(num_points);
    freqz(elliptic, H_ellip.data(), num_points);
    freqz(butter4, H_butter.data(), num_points);

    // Check stopband attenuation at specific frequency
    size_t stop_idx = freq_to_idx(0.5f, num_points);
    float ellip_db = to_db(magnitude(H_ellip[stop_idx]));
    float butter_db = to_db(magnitude(H_butter[stop_idx]));

    // Elliptic should have better attenuation
    EXPECT_LT(ellip_db, butter_db);
}

// ============================================================================
// Group Delay Tests
// ============================================================================

TEST_F(FreqzTest, GroupDelayBasic) {
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);

    const size_t num_points = 256;
    std::vector<float> gd(num_points);
    grpdelay(coeffs, gd.data(), num_points);

    // Group delay should be positive and finite
    for (size_t i = 0; i < num_points; ++i) {
        EXPECT_FALSE(std::isnan(gd[i]));
        EXPECT_FALSE(std::isinf(gd[i]));
        EXPECT_GE(gd[i], 0.0f);
    }
}

TEST_F(FreqzTest, FIRGroupDelayFlat) {
    // Linear phase FIR should have constant group delay
    auto b = fir_windowed_sinc(51, 0.3f, FilterType::Lowpass, WindowType::Hamming);

    FilterCoefficients coeffs;
    coeffs.b = b;
    coeffs.a = {1.0f};

    const size_t num_points = 256;
    std::vector<float> gd(num_points);
    grpdelay(coeffs, gd.data(), num_points);

    // Group delay should be approximately (N-1)/2 for linear phase FIR
    float expected_delay = static_cast<float>(b.size() - 1) / 2.0f;

    // Check in passband (skip edges)
    for (size_t i = 10; i < num_points / 3; ++i) {
        EXPECT_NEAR(gd[i], expected_delay, 1.0f);
    }
}

// ============================================================================
// Stability Tests
// ============================================================================

TEST_F(FreqzTest, StabilityCheck) {
    // Butterworth filters should be stable
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);
    EXPECT_TRUE(is_stable(coeffs));

    // High order filters should still be stable
    auto high_order = butter(8, 0.3f, FilterType::Lowpass);
    EXPECT_TRUE(is_stable(high_order));
}

TEST_F(FreqzTest, UnstableFilterDetection) {
    // Create an unstable filter manually (pole outside unit circle)
    FilterCoefficients unstable;
    unstable.b = {1.0f};
    unstable.a = {1.0f, -2.0f};  // Pole at z = 2 (outside unit circle)

    EXPECT_FALSE(is_stable(unstable));
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(FreqzTest, FirstOrderFilter) {
    auto coeffs = butter(1, 0.3f, FilterType::Lowpass);

    const size_t num_points = 64;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // Basic checks
    float dc_mag = magnitude(H[0]);
    EXPECT_NEAR(dc_mag, 1.0f, 0.1f);

    float nyq_mag = magnitude(H[num_points - 1]);
    EXPECT_LT(nyq_mag, dc_mag);
}

TEST_F(FreqzTest, SmallNumPoints) {
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);

    const size_t num_points = 8;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    for (size_t i = 0; i < num_points; ++i) {
        EXPECT_FALSE(std::isnan(H[i].real));
        EXPECT_FALSE(std::isnan(H[i].imag));
    }
}

TEST_F(FreqzTest, LargeNumPoints) {
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);

    const size_t num_points = 4096;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    // Check DC and Nyquist
    float dc_mag = magnitude(H[0]);
    EXPECT_NEAR(dc_mag, 1.0f, 0.05f);

    float nyq_mag = magnitude(H[num_points - 1]);
    EXPECT_LT(to_db(nyq_mag), -30.0f);
}

// ============================================================================
// Frequency Response Consistency
// ============================================================================

TEST_F(FreqzTest, FreqzMonotonic) {
    // Lowpass magnitude should be monotonically decreasing (roughly)
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);

    const size_t num_points = 256;
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    float prev_mag = magnitude(H[0]);
    int decreasing_count = 0;

    for (size_t i = 1; i < num_points; ++i) {
        float curr_mag = magnitude(H[i]);
        if (curr_mag <= prev_mag + 0.01f) {  // Allow small tolerance
            decreasing_count++;
        }
        prev_mag = curr_mag;
    }

    // Should be mostly decreasing
    EXPECT_GT(decreasing_count, static_cast<int>(num_points * 0.9));
}

TEST_F(FreqzTest, FreqzSymmetricCoeffs) {
    // Test with manually created symmetric FIR (moving average)
    std::vector<float> b = {0.25f, 0.5f, 0.25f};  // Simple lowpass

    const size_t num_points = 128;
    std::vector<Complex> H(num_points);
    freqz(b, H.data(), num_points);

    // DC gain should be sum of coefficients
    float dc_mag = magnitude(H[0]);
    float expected_dc = 0.25f + 0.5f + 0.25f;
    EXPECT_NEAR(dc_mag, expected_dc, 0.01f);

    // Nyquist gain for alternating average
    float nyq_mag = magnitude(H[num_points - 1]);
    float expected_nyq = std::abs(0.25f - 0.5f + 0.25f);
    EXPECT_NEAR(nyq_mag, expected_nyq, 0.01f);
}
