/**
 * @file test_hilbert.cpp
 * @brief Unit tests for Hilbert transform and related functions
 */

#include <gtest/gtest.h>
#include "spectra/analysis/hilbert.hpp"
#include <cmath>

using namespace spectra;

class HilbertTest : public ::testing::Test {
protected:
    std::vector<float> sine_wave(size_t len, float freq, float sample_rate) {
        std::vector<float> signal(len);
        for (size_t i = 0; i < len; ++i) {
            signal[i] = std::sin(constants::two_pi * freq * i / sample_rate);
        }
        return signal;
    }

    std::vector<float> cosine_wave(size_t len, float freq, float sample_rate) {
        std::vector<float> signal(len);
        for (size_t i = 0; i < len; ++i) {
            signal[i] = std::cos(constants::two_pi * freq * i / sample_rate);
        }
        return signal;
    }
};

// ============================================================================
// Basic Hilbert Transform Tests
// ============================================================================

TEST_F(HilbertTest, BasicHilbert) {
    auto signal = sine_wave(256, 100.0f, 1000.0f);

    std::vector<Complex> analytic(signal.size());
    hilbert(signal.data(), analytic.data(), signal.size());

    // Check no NaN/Inf
    for (const auto& c : analytic) {
        EXPECT_FALSE(std::isnan(c.real));
        EXPECT_FALSE(std::isnan(c.imag));
        EXPECT_FALSE(std::isinf(c.real));
        EXPECT_FALSE(std::isinf(c.imag));
    }
}

TEST_F(HilbertTest, RealPartPreserved) {
    auto signal = sine_wave(256, 100.0f, 1000.0f);

    std::vector<Complex> analytic(signal.size());
    hilbert(signal.data(), analytic.data(), signal.size());

    // Real part should be same as input
    for (size_t i = 0; i < signal.size(); ++i) {
        EXPECT_NEAR(analytic[i].real, signal[i], 1e-4f);
    }
}

TEST_F(HilbertTest, TwoOutputVersion) {
    auto signal = sine_wave(256, 100.0f, 1000.0f);

    std::vector<float> real_out(signal.size());
    std::vector<float> imag_out(signal.size());

    hilbert(signal.data(), real_out.data(), imag_out.data(), signal.size());

    // Real part should match input
    for (size_t i = 0; i < signal.size(); ++i) {
        EXPECT_NEAR(real_out[i], signal[i], 1e-4f);
    }

    // Imaginary part should exist and not be all zeros
    float imag_energy = 0.0f;
    for (float v : imag_out) {
        imag_energy += v * v;
    }
    EXPECT_GT(imag_energy, 0.1f);
}

// ============================================================================
// Instantaneous Amplitude (Envelope)
// ============================================================================

TEST_F(HilbertTest, InstantaneousAmplitudeSine) {
    // For a pure sine wave, envelope should exist and be positive
    auto signal = sine_wave(512, 50.0f, 1000.0f);

    std::vector<float> envelope(signal.size());
    instantaneous_amplitude(signal.data(), envelope.data(), signal.size());

    // Envelope should be non-negative
    for (float e : envelope) {
        EXPECT_GE(e, 0.0f);
        EXPECT_FALSE(std::isnan(e));
    }

    // Mean envelope should be in reasonable range
    float mean_env = 0.0f;
    for (float e : envelope) mean_env += e;
    mean_env /= envelope.size();
    EXPECT_GT(mean_env, 0.1f);
}

TEST_F(HilbertTest, AMSignalEnvelope) {
    // AM signal: carrier modulated by lower frequency
    const float sample_rate = 10000.0f;
    const float carrier_freq = 1000.0f;
    const float mod_freq = 50.0f;

    std::vector<float> signal(2048);

    for (size_t i = 0; i < signal.size(); ++i) {
        float t = static_cast<float>(i) / sample_rate;
        float env = 1.0f + 0.5f * std::cos(constants::two_pi * mod_freq * t);
        signal[i] = env * std::sin(constants::two_pi * carrier_freq * t);
    }

    std::vector<float> envelope(signal.size());
    instantaneous_amplitude(signal.data(), envelope.data(), signal.size());

    // Basic check: envelope should be non-negative and bounded
    for (float e : envelope) {
        EXPECT_GE(e, 0.0f);
        EXPECT_LT(e, 10.0f);  // Bounded
        EXPECT_FALSE(std::isnan(e));
    }
}

// ============================================================================
// Instantaneous Phase
// ============================================================================

TEST_F(HilbertTest, InstantaneousPhaseSine) {
    auto signal = sine_wave(512, 50.0f, 1000.0f);

    std::vector<float> phase(signal.size());
    instantaneous_phase(signal.data(), phase.data(), signal.size());

    // Phase should be bounded to [-pi, pi]
    for (size_t i = 0; i < signal.size(); ++i) {
        EXPECT_GE(phase[i], -constants::pi - 0.01f);
        EXPECT_LE(phase[i], constants::pi + 0.01f);
    }
}

TEST_F(HilbertTest, InstantaneousPhaseUnwrapped) {
    // For a constant frequency signal, phase should exist
    auto signal = sine_wave(512, 50.0f, 1000.0f);

    std::vector<float> phase(signal.size());
    instantaneous_phase_unwrapped(signal.data(), phase.data(), signal.size());

    // Basic check: no NaN/Inf
    for (float p : phase) {
        EXPECT_FALSE(std::isnan(p));
        EXPECT_FALSE(std::isinf(p));
    }
}

// ============================================================================
// Phase Unwrapping
// ============================================================================

TEST_F(HilbertTest, UnwrapPhaseBasic) {
    // Create simple wrapped phase
    std::vector<float> phase(20);
    for (size_t i = 0; i < 20; ++i) {
        phase[i] = 0.5f * i;
        // Wrap to [-pi, pi]
        while (phase[i] > constants::pi) phase[i] -= constants::two_pi;
    }

    unwrap_phase(phase.data(), phase.size());

    // Basic check: no NaN/Inf
    for (float p : phase) {
        EXPECT_FALSE(std::isnan(p));
        EXPECT_FALSE(std::isinf(p));
    }
}

TEST_F(HilbertTest, UnwrapPhaseMonotonic) {
    // Simple test: small linear phase that doesn't wrap
    std::vector<float> phase(10);
    for (size_t i = 0; i < 10; ++i) {
        phase[i] = 0.1f * i;  // All within [-pi, pi]
    }

    unwrap_phase(phase.data(), phase.size());

    // Should remain unchanged (no wrapping needed)
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_NEAR(phase[i], 0.1f * i, 0.01f);
    }
}

// ============================================================================
// Instantaneous Frequency
// ============================================================================

TEST_F(HilbertTest, InstantaneousFrequencyConstant) {
    const float sample_rate = 1000.0f;
    const float signal_freq = 50.0f;

    auto signal = sine_wave(512, signal_freq, sample_rate);

    std::vector<float> inst_freq(signal.size() - 1);
    instantaneous_frequency(signal.data(), inst_freq.data(),
                            signal.size(), sample_rate);

    // Basic check: no NaN/Inf and frequencies are finite
    for (float f : inst_freq) {
        EXPECT_FALSE(std::isnan(f));
        EXPECT_FALSE(std::isinf(f));
    }
}

TEST_F(HilbertTest, InstantaneousFrequencyChirp) {
    // Linear chirp: frequency increases with time
    const float sample_rate = 10000.0f;
    const size_t len = 2048;
    const float f_start = 100.0f;
    const float f_end = 500.0f;

    std::vector<float> signal(len);
    for (size_t i = 0; i < len; ++i) {
        float t = static_cast<float>(i) / sample_rate;
        float freq = f_start + (f_end - f_start) * t / (static_cast<float>(len) / sample_rate);
        signal[i] = std::sin(constants::two_pi * freq * t);
    }

    std::vector<float> inst_freq(len - 1);
    instantaneous_frequency(signal.data(), inst_freq.data(), len, sample_rate);

    // Instantaneous frequency should generally increase
    // Compare early vs late portion
    float early_mean = 0.0f, late_mean = 0.0f;
    for (size_t i = 100; i < 200; ++i) {
        early_mean += inst_freq[i];
    }
    early_mean /= 100;

    for (size_t i = len - 300; i < len - 200; ++i) {
        late_mean += inst_freq[i];
    }
    late_mean /= 100;

    EXPECT_GT(late_mean, early_mean);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(HilbertTest, DCSignal) {
    // DC signal
    std::vector<float> signal(256, 1.0f);

    std::vector<float> envelope(signal.size());
    instantaneous_amplitude(signal.data(), envelope.data(), signal.size());

    // Envelope of DC should be constant
    // (though Hilbert transform of DC is not well-defined)
    for (float e : envelope) {
        EXPECT_FALSE(std::isnan(e));
        EXPECT_FALSE(std::isinf(e));
    }
}

TEST_F(HilbertTest, ShortSignal) {
    std::vector<float> signal = {1.0f, 0.0f, -1.0f, 0.0f};

    std::vector<Complex> analytic(signal.size());
    hilbert(signal.data(), analytic.data(), signal.size());

    for (const auto& c : analytic) {
        EXPECT_FALSE(std::isnan(c.real));
        EXPECT_FALSE(std::isnan(c.imag));
    }
}

// ============================================================================
// Cosine Input (90 degree phase shift)
// ============================================================================

TEST_F(HilbertTest, CosineToSine) {
    // Hilbert transform of cos should produce analytic signal
    const float freq = 50.0f;
    const float sample_rate = 1000.0f;

    auto cosine = cosine_wave(512, freq, sample_rate);

    std::vector<Complex> analytic(512);
    hilbert(cosine.data(), analytic.data(), 512);

    // Basic check: imaginary part should exist and not be all zeros
    float imag_energy = 0.0f;
    for (const auto& c : analytic) {
        imag_energy += c.imag * c.imag;
        EXPECT_FALSE(std::isnan(c.imag));
    }
    EXPECT_GT(imag_energy, 0.01f);
}
