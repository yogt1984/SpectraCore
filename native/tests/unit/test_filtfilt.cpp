/**
 * @file test_filtfilt.cpp
 * @brief Comprehensive unit tests for filtfilt (zero-phase filtering)
 */

#include <gtest/gtest.h>
#include "spectra/filters/iir_filter.hpp"
#include "spectra/filters/filter_design.hpp"
#include <cmath>
#include <numeric>
#include <algorithm>

using namespace spectra;

class FiltfiltTest : public ::testing::Test {
protected:
    // Helper to generate sine wave
    std::vector<float> sine(size_t len, float freq, float phase = 0.0f) {
        std::vector<float> x(len);
        for (size_t i = 0; i < len; ++i) {
            x[i] = std::sin(2.0f * constants::pi * freq * i + phase);
        }
        return x;
    }

    // Helper to generate step signal
    std::vector<float> step(size_t len) {
        return std::vector<float>(len, 1.0f);
    }

    // Helper to generate impulse
    std::vector<float> impulse(size_t len, size_t pos = 0) {
        std::vector<float> x(len, 0.0f);
        if (pos < len) x[pos] = 1.0f;
        return x;
    }

    // Helper to find peak index
    size_t find_peak(const std::vector<float>& x) {
        return std::distance(x.begin(), std::max_element(x.begin(), x.end()));
    }

    // Helper to compute group delay
    float estimate_group_delay(const std::vector<float>& input,
                               const std::vector<float>& output) {
        // Find peak positions
        size_t input_peak = find_peak(input);
        size_t output_peak = find_peak(output);
        return static_cast<float>(output_peak) - static_cast<float>(input_peak);
    }
};

// ============================================================================
// Zero Phase Tests
// ============================================================================

TEST_F(FiltfiltTest, ZeroPhaseDelay) {
    // Create a signal with a peak in the middle
    std::vector<float> input(200, 0.0f);
    size_t peak_pos = 100;
    input[peak_pos] = 1.0f;
    // Add some neighbors for smooth peak
    input[peak_pos - 1] = 0.5f;
    input[peak_pos + 1] = 0.5f;
    input[peak_pos - 2] = 0.25f;
    input[peak_pos + 2] = 0.25f;

    auto coeffs = butter(2, 0.1f, FilterType::Lowpass);
    std::vector<float> output(200);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 200);

    // Peak of output should be at same position (zero phase)
    size_t output_peak = find_peak(output);
    EXPECT_EQ(output_peak, peak_pos) << "Output peak at " << output_peak
                                      << ", expected " << peak_pos;
}

TEST_F(FiltfiltTest, SymmetricImpulseResponse) {
    // Impulse in the middle
    auto input = impulse(201, 100);

    auto coeffs = butter(3, 0.2f, FilterType::Lowpass);
    std::vector<float> output(201);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 201);

    // Output should be symmetric around the impulse position
    for (size_t i = 1; i < 50; ++i) {
        float left = output[100 - i];
        float right = output[100 + i];
        EXPECT_NEAR(left, right, 0.001f)
            << "Asymmetric at offset " << i;
    }
}

TEST_F(FiltfiltTest, SineWaveNoPhaseShift) {
    // Low frequency sine through lowpass
    float freq = 0.02f;  // Well below cutoff
    auto input = sine(500, freq);

    auto coeffs = butter(3, 0.2f, FilterType::Lowpass);
    std::vector<float> output(500);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 500);

    // Compare phase by looking at zero crossings
    // In the middle of signal, output should cross zero at same points as input
    for (size_t i = 200; i < 300; ++i) {
        if (input[i] > 0 && input[i + 1] <= 0) {
            // Found a zero crossing in input
            // Output should also have crossing nearby
            bool found_crossing = false;
            for (int j = -2; j <= 2; ++j) {
                size_t idx = i + j;
                if (output[idx] > 0 && output[idx + 1] <= 0) {
                    found_crossing = true;
                    break;
                }
            }
            EXPECT_TRUE(found_crossing)
                << "Zero crossing not preserved at sample " << i;
        }
    }
}

// ============================================================================
// Double Filtering Effect Tests
// ============================================================================

TEST_F(FiltfiltTest, SquaredMagnitudeResponse) {
    // filtfilt applies filter twice, so magnitude response is squared
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);

    // Create test signal with known frequency content
    std::vector<float> input(1000);
    for (size_t i = 0; i < 1000; ++i) {
        input[i] = std::sin(0.1f * i) + std::cos(0.5f * i);
    }

    // Single forward filter
    std::vector<float> single_out(1000);
    lfilter(coeffs.b.data(), coeffs.b.size(),
            coeffs.a.data(), coeffs.a.size(),
            input.data(), single_out.data(), 1000);

    // Double filter (forward-backward)
    std::vector<float> double_out(1000);
    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), double_out.data(), 1000);

    // filtfilt should have more attenuation (squared magnitude)
    // Compare RMS in stopband region
    float rms_single = 0.0f, rms_double = 0.0f;
    for (size_t i = 500; i < 1000; ++i) {
        rms_single += single_out[i] * single_out[i];
        rms_double += double_out[i] * double_out[i];
    }
    rms_single = std::sqrt(rms_single / 500);
    rms_double = std::sqrt(rms_double / 500);

    // Double filtering should have more attenuation
    EXPECT_LT(rms_double, rms_single);
}

// ============================================================================
// Edge Handling Tests
// ============================================================================

TEST_F(FiltfiltTest, NoEdgeTransients) {
    // Step signal - edges should be clean
    auto input = step(200);

    auto coeffs = butter(3, 0.2f, FilterType::Lowpass);
    std::vector<float> output(200);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 200);

    // Step through lowpass should remain constant (DC passes)
    // Check that output is close to 1.0 throughout
    for (size_t i = 10; i < 190; ++i) {
        EXPECT_NEAR(output[i], 1.0f, 0.1f) << "Sample " << i;
    }

    // Edge values should also be reasonable (no large transients)
    EXPECT_LT(std::abs(output[0] - 1.0f), 0.5f);
    EXPECT_LT(std::abs(output[199] - 1.0f), 0.5f);
}

TEST_F(FiltfiltTest, ReflectedPadding) {
    // Ramp signal - reflected padding should help at edges
    std::vector<float> input(100);
    for (size_t i = 0; i < 100; ++i) {
        input[i] = static_cast<float>(i) / 100.0f;
    }

    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);
    std::vector<float> output(100);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 100);

    // Output should follow the ramp trend
    // Check monotonicity in the middle
    for (size_t i = 11; i < 89; ++i) {
        EXPECT_GE(output[i], output[i - 1] - 0.01f)
            << "Non-monotonic at " << i;
    }
}

// ============================================================================
// Different Filter Types
// ============================================================================

TEST_F(FiltfiltTest, LowpassFiltering) {
    // Mix of low and high frequency
    std::vector<float> input(500);
    for (size_t i = 0; i < 500; ++i) {
        input[i] = std::sin(0.05f * i) + 0.5f * std::sin(0.5f * i);
    }

    auto coeffs = butter(4, 0.1f, FilterType::Lowpass);
    std::vector<float> output(500);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 500);

    // High frequency should be attenuated
    // Compare variance of high-frequency component
    float input_hf_var = 0.0f, output_hf_var = 0.0f;
    for (size_t i = 200; i < 300; ++i) {
        float hf_in = 0.5f * std::sin(0.5f * i);  // High freq component
        input_hf_var += hf_in * hf_in;
    }
    for (size_t i = 200; i < 300; ++i) {
        // Output should have less high-frequency content
        float diff = output[i] - output[i > 0 ? i - 1 : 0];
        output_hf_var += diff * diff;
    }

    // Output should be smoother
    EXPECT_LT(output_hf_var, input_hf_var);
}

TEST_F(FiltfiltTest, HighpassFiltering) {
    // DC + sine wave
    std::vector<float> input(500);
    for (size_t i = 0; i < 500; ++i) {
        input[i] = 1.0f + std::sin(0.3f * i);  // DC offset + high freq
    }

    auto coeffs = butter(3, 0.2f, FilterType::Highpass);
    std::vector<float> output(500);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 500);

    // DC should be removed
    float mean = 0.0f;
    for (size_t i = 100; i < 400; ++i) {
        mean += output[i];
    }
    mean /= 300;

    EXPECT_NEAR(mean, 0.0f, 0.1f);  // Mean should be close to zero
}

TEST_F(FiltfiltTest, ChebyshevFilter) {
    auto input = sine(300, 0.1f);

    auto coeffs = cheby1(3, 1.0f, 0.2f, FilterType::Lowpass);
    std::vector<float> output(300);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 300);

    // Should work without crashing and produce valid output
    float max_val = *std::max_element(output.begin(), output.end());
    float min_val = *std::min_element(output.begin(), output.end());

    EXPECT_GT(max_val, 0.5f);  // Should preserve some signal
    EXPECT_LT(min_val, -0.5f);
    EXPECT_FALSE(std::isnan(max_val));
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(FiltfiltTest, ShortSignal) {
    // Signal just longer than filter order
    std::vector<float> input(20, 1.0f);

    auto coeffs = butter(3, 0.3f, FilterType::Lowpass);
    std::vector<float> output(20);

    // Should not crash
    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 20);

    // Verify no NaN
    for (float y : output) {
        EXPECT_FALSE(std::isnan(y));
    }
}

TEST_F(FiltfiltTest, LongSignal) {
    std::vector<float> input(10000);
    for (size_t i = 0; i < 10000; ++i) {
        input[i] = std::sin(0.01f * i) + 0.5f * std::cos(0.1f * i);
    }

    auto coeffs = butter(4, 0.2f, FilterType::Lowpass);
    std::vector<float> output(10000);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 10000);

    // Verify no NaN/Inf
    for (size_t i = 0; i < 10000; ++i) {
        EXPECT_FALSE(std::isnan(output[i])) << "NaN at " << i;
        EXPECT_FALSE(std::isinf(output[i])) << "Inf at " << i;
    }
}

TEST_F(FiltfiltTest, HighOrderFilter) {
    auto input = sine(500, 0.05f);

    // High-order filter
    auto coeffs = butter(8, 0.3f, FilterType::Lowpass);
    std::vector<float> output(500);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 500);

    // Should be stable
    float max_abs = 0.0f;
    for (float y : output) {
        max_abs = std::max(max_abs, std::abs(y));
    }
    EXPECT_LT(max_abs, 10.0f);  // Should not explode
}

// ============================================================================
// Comparison with MATLAB-like Behavior
// ============================================================================

TEST_F(FiltfiltTest, DCPreservation) {
    // Lowpass should preserve DC perfectly
    auto input = step(100);

    auto coeffs = butter(3, 0.3f, FilterType::Lowpass);
    std::vector<float> output(100);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 100);

    // DC should be exactly preserved (gain = 1)
    float mean = std::accumulate(output.begin() + 20, output.end() - 20, 0.0f) / 60;
    EXPECT_NEAR(mean, 1.0f, 0.01f);
}

TEST_F(FiltfiltTest, NyquistAttenuation) {
    // Alternating signal (Nyquist frequency)
    std::vector<float> input(100);
    for (size_t i = 0; i < 100; ++i) {
        input[i] = (i % 2 == 0) ? 1.0f : -1.0f;
    }

    auto coeffs = butter(3, 0.3f, FilterType::Lowpass);
    std::vector<float> output(100);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 100);

    // Nyquist should be heavily attenuated
    float max_abs = 0.0f;
    for (size_t i = 30; i < 70; ++i) {
        max_abs = std::max(max_abs, std::abs(output[i]));
    }
    EXPECT_LT(max_abs, 0.1f);
}

TEST_F(FiltfiltTest, PassbandPreservation) {
    // Low frequency should pass through
    float freq = 0.05f;  // Well below cutoff
    auto input = sine(500, freq);

    auto coeffs = butter(3, 0.3f, FilterType::Lowpass);
    std::vector<float> output(500);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 500);

    // RMS should be preserved (within passband)
    float rms_in = 0.0f, rms_out = 0.0f;
    for (size_t i = 100; i < 400; ++i) {
        rms_in += input[i] * input[i];
        rms_out += output[i] * output[i];
    }
    rms_in = std::sqrt(rms_in / 300);
    rms_out = std::sqrt(rms_out / 300);

    EXPECT_NEAR(rms_out, rms_in, 0.1f);
}

// ============================================================================
// Real-World Signal Tests
// ============================================================================

TEST_F(FiltfiltTest, NoisySignal) {
    // Signal with noise
    std::vector<float> input(500);
    std::srand(12345);
    for (size_t i = 0; i < 500; ++i) {
        float clean = std::sin(0.05f * i);
        float noise = 0.3f * (static_cast<float>(std::rand()) / RAND_MAX - 0.5f);
        input[i] = clean + noise;
    }

    auto coeffs = butter(3, 0.1f, FilterType::Lowpass);
    std::vector<float> output(500);

    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 500);

    // Output should be smoother (lower variance of derivative)
    float input_var = 0.0f, output_var = 0.0f;
    for (size_t i = 101; i < 400; ++i) {
        float d_in = input[i] - input[i - 1];
        float d_out = output[i] - output[i - 1];
        input_var += d_in * d_in;
        output_var += d_out * d_out;
    }

    EXPECT_LT(output_var, input_var);
}

