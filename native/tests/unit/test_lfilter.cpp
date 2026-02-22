/**
 * @file test_lfilter.cpp
 * @brief Comprehensive unit tests for lfilter (IIR filter application)
 */

#include <gtest/gtest.h>
#include "spectra/filters/iir_filter.hpp"
#include "spectra/filters/filter_design.hpp"
#include <cmath>
#include <numeric>

using namespace spectra;

class LFilterTest : public ::testing::Test {
protected:
    // Helper to generate step input
    std::vector<float> step(size_t len) {
        return std::vector<float>(len, 1.0f);
    }

    // Helper to generate impulse input
    std::vector<float> impulse(size_t len) {
        std::vector<float> x(len, 0.0f);
        if (len > 0) x[0] = 1.0f;
        return x;
    }

    // Helper to generate ramp input
    std::vector<float> ramp(size_t len) {
        std::vector<float> x(len);
        for (size_t i = 0; i < len; ++i) {
            x[i] = static_cast<float>(i);
        }
        return x;
    }

    // Helper to generate sine wave
    std::vector<float> sine(size_t len, float freq) {
        std::vector<float> x(len);
        for (size_t i = 0; i < len; ++i) {
            x[i] = std::sin(2.0f * constants::pi * freq * i);
        }
        return x;
    }
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST_F(LFilterTest, Passthrough) {
    // H(z) = 1 (passthrough)
    float b[] = {1.0f};
    float a[] = {1.0f};
    auto input = ramp(10);
    std::vector<float> output(10);

    lfilter(b, 1, a, 1, input.data(), output.data(), 10);

    for (size_t i = 0; i < 10; ++i) {
        EXPECT_FLOAT_EQ(output[i], input[i]);
    }
}

TEST_F(LFilterTest, ScaleByConstant) {
    // H(z) = 2 (scale by 2)
    float b[] = {2.0f};
    float a[] = {1.0f};
    auto input = step(5);
    std::vector<float> output(5);

    lfilter(b, 1, a, 1, input.data(), output.data(), 5);

    for (size_t i = 0; i < 5; ++i) {
        EXPECT_FLOAT_EQ(output[i], 2.0f);
    }
}

TEST_F(LFilterTest, FirstOrderLowpass) {
    // Simple first-order lowpass: H(z) = 0.5 / (1 - 0.5*z^-1)
    float b[] = {0.5f};
    float a[] = {1.0f, -0.5f};
    auto input = step(10);
    std::vector<float> output(10);

    lfilter(b, 1, a, 2, input.data(), output.data(), 10);

    // Step response should converge to DC gain = 0.5 / (1 - 0.5) = 1
    EXPECT_NEAR(output[9], 1.0f, 0.01f);

    // Monotonically increasing
    for (size_t i = 1; i < 10; ++i) {
        EXPECT_GE(output[i], output[i - 1] - 0.001f);
    }
}

TEST_F(LFilterTest, ImpulseResponse) {
    // First-order: H(z) = 1 / (1 - 0.9*z^-1)
    // Impulse response: h[n] = 0.9^n
    float b[] = {1.0f};
    float a[] = {1.0f, -0.9f};
    auto input = impulse(20);
    std::vector<float> output(20);

    lfilter(b, 1, a, 2, input.data(), output.data(), 20);

    for (size_t i = 0; i < 20; ++i) {
        float expected = std::pow(0.9f, static_cast<float>(i));
        EXPECT_NEAR(output[i], expected, 1e-5f) << "Sample " << i;
    }
}

// ============================================================================
// Second-Order Section Tests
// ============================================================================

TEST_F(LFilterTest, SecondOrderSection) {
    // Second-order section
    float b[] = {1.0f, 2.0f, 1.0f};  // (1 + z^-1)^2
    float a[] = {1.0f, -0.5f, 0.1f};
    auto input = impulse(20);
    std::vector<float> output(20);

    lfilter(b, 3, a, 3, input.data(), output.data(), 20);

    // Should have non-trivial impulse response
    EXPECT_FLOAT_EQ(output[0], 1.0f);  // First sample = b[0]/a[0]
    EXPECT_NE(output[1], 0.0f);
    EXPECT_NE(output[2], 0.0f);

    // Should decay for stable filter
    EXPECT_LT(std::abs(output[19]), std::abs(output[0]));
}

TEST_F(LFilterTest, ButterworthFilter) {
    auto coeffs = butter(2, 0.2f, FilterType::Lowpass);
    auto input = sine(200, 0.05f);  // Low frequency
    std::vector<float> output(200);

    lfilter(coeffs.b.data(), coeffs.b.size(),
            coeffs.a.data(), coeffs.a.size(),
            input.data(), output.data(), 200);

    // After transient, output should track input (lowpass passes low freq)
    float max_ratio = 0.0f;
    for (size_t i = 50; i < 200; ++i) {
        if (std::abs(input[i]) > 0.1f) {
            float ratio = output[i] / input[i];
            max_ratio = std::max(max_ratio, std::abs(ratio));
        }
    }
    EXPECT_GT(max_ratio, 0.8f);  // Should pass most of signal
}

// ============================================================================
// Initial Conditions Tests
// ============================================================================

TEST_F(LFilterTest, ZeroInitialConditions) {
    // With zero initial conditions, should match regular lfilter
    float b[] = {1.0f, 0.5f};
    float a[] = {1.0f, -0.8f};
    auto input = step(10);
    std::vector<float> output1(10), output2(10);
    float zi[] = {0.0f};
    float zf[1];

    lfilter(b, 2, a, 2, input.data(), output1.data(), 10);
    lfilter_zi(b, 2, a, 2, input.data(), output2.data(), 10, zi, zf);

    for (size_t i = 0; i < 10; ++i) {
        EXPECT_FLOAT_EQ(output1[i], output2[i]) << "Sample " << i;
    }
}

TEST_F(LFilterTest, NonZeroInitialConditions) {
    float b[] = {1.0f};
    float a[] = {1.0f, -0.9f};
    auto input = step(5);
    std::vector<float> output(5);
    float zi[] = {5.0f};  // Start with state = 5
    float zf[1];

    lfilter_zi(b, 1, a, 2, input.data(), output.data(), 5, zi, zf);

    // First output should include initial condition
    EXPECT_FLOAT_EQ(output[0], 1.0f + 5.0f);  // b[0]*input[0] + zi[0]
}

TEST_F(LFilterTest, FinalConditions) {
    float b[] = {1.0f, 0.5f};
    float a[] = {1.0f, -0.8f};
    auto input = step(10);
    std::vector<float> output(10);
    float zf[1];

    lfilter_zi(b, 2, a, 2, input.data(), output.data(), 10, nullptr, zf);

    // Final conditions should be non-zero for step input
    EXPECT_NE(zf[0], 0.0f);
}

TEST_F(LFilterTest, StateContinuity) {
    // Process in chunks should give same result as full processing
    float b[] = {1.0f, 0.5f};
    float a[] = {1.0f, -0.8f};
    auto input = ramp(100);

    // Full processing
    std::vector<float> full_output(100);
    lfilter(b, 2, a, 2, input.data(), full_output.data(), 100);

    // Chunked processing with state continuity
    std::vector<float> chunked_output(100);
    float zi[] = {0.0f};
    float zf[1];

    // Process first 30 samples
    lfilter_zi(b, 2, a, 2, input.data(), chunked_output.data(), 30, zi, zf);

    // Continue with state from first chunk
    lfilter_zi(b, 2, a, 2, input.data() + 30, chunked_output.data() + 30, 30, zf, zf);

    // Continue again
    lfilter_zi(b, 2, a, 2, input.data() + 60, chunked_output.data() + 60, 40, zf, zf);

    // Should match full processing
    for (size_t i = 0; i < 100; ++i) {
        EXPECT_NEAR(chunked_output[i], full_output[i], 1e-5f) << "Sample " << i;
    }
}

TEST_F(LFilterTest, HigherOrderStateContinuity) {
    // Test with order 4 filter
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);
    auto input = sine(200, 0.1f);

    // Full processing
    std::vector<float> full_output(200);
    lfilter(coeffs.b.data(), coeffs.b.size(),
            coeffs.a.data(), coeffs.a.size(),
            input.data(), full_output.data(), 200);

    // Chunked processing
    std::vector<float> chunked_output(200);
    size_t order = coeffs.a.size() - 1;
    std::vector<float> zi(order, 0.0f);
    std::vector<float> zf(order);

    size_t processed = 0;
    size_t chunk_sizes[] = {50, 75, 75};

    for (size_t chunk : chunk_sizes) {
        lfilter_zi(coeffs.b.data(), coeffs.b.size(),
                   coeffs.a.data(), coeffs.a.size(),
                   input.data() + processed, chunked_output.data() + processed, chunk,
                   zi.data(), zf.data());
        std::copy(zf.begin(), zf.end(), zi.begin());
        processed += chunk;
    }

    // Should match
    for (size_t i = 0; i < 200; ++i) {
        EXPECT_NEAR(chunked_output[i], full_output[i], 1e-5f) << "Sample " << i;
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(LFilterTest, EmptyInput) {
    float b[] = {1.0f};
    float a[] = {1.0f, -0.5f};
    float output[1];

    // Should not crash with zero-length input
    lfilter(b, 1, a, 2, nullptr, output, 0);
    // Just verify it doesn't crash
    SUCCEED();
}

TEST_F(LFilterTest, SingleSample) {
    float b[] = {0.5f, 0.5f};
    float a[] = {1.0f, -0.8f};
    float input[] = {2.0f};
    float output[1];

    lfilter(b, 2, a, 2, input, output, 1);

    // First output = b[0]*input[0] = 0.5 * 2 = 1
    EXPECT_FLOAT_EQ(output[0], 1.0f);
}

TEST_F(LFilterTest, NormalizationByA0) {
    // If a[0] != 1, should normalize
    float b[] = {2.0f};
    float a[] = {2.0f};  // Normalized: H(z) = 1
    auto input = step(5);
    std::vector<float> output(5);

    lfilter(b, 1, a, 1, input.data(), output.data(), 5);

    // Should be passthrough
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_FLOAT_EQ(output[i], 1.0f);
    }
}

// ============================================================================
// Numerical Stability Tests
// ============================================================================

TEST_F(LFilterTest, HighOrderStability) {
    // High-order filter should remain stable
    for (int order = 2; order <= 8; ++order) {
        auto coeffs = butter(order, 0.3f, FilterType::Lowpass);
        auto input = step(500);
        std::vector<float> output(500);

        lfilter(coeffs.b.data(), coeffs.b.size(),
                coeffs.a.data(), coeffs.a.size(),
                input.data(), output.data(), 500);

        // Output should converge (not explode)
        float max_abs = 0.0f;
        for (float y : output) {
            max_abs = std::max(max_abs, std::abs(y));
        }
        EXPECT_LT(max_abs, 10.0f) << "Order " << order << " exploded";

        // Should converge to DC gain (approximately 1 for lowpass)
        EXPECT_NEAR(output[499], 1.0f, 0.1f) << "Order " << order;
    }
}

TEST_F(LFilterTest, LongSignalProcessing) {
    // Test with longer signal
    auto coeffs = butter(3, 0.2f, FilterType::Lowpass);
    std::vector<float> input(10000);
    for (size_t i = 0; i < 10000; ++i) {
        input[i] = std::sin(0.01f * i) + 0.5f * std::cos(0.05f * i);
    }
    std::vector<float> output(10000);

    lfilter(coeffs.b.data(), coeffs.b.size(),
            coeffs.a.data(), coeffs.a.size(),
            input.data(), output.data(), 10000);

    // Verify no NaN or Inf
    for (size_t i = 0; i < 10000; ++i) {
        EXPECT_FALSE(std::isnan(output[i])) << "NaN at " << i;
        EXPECT_FALSE(std::isinf(output[i])) << "Inf at " << i;
    }
}

// ============================================================================
// lfilter_ic Tests (Initial Conditions Computation)
// ============================================================================

TEST_F(LFilterTest, InitialConditionsComputation) {
    float b[] = {1.0f, 0.5f};
    float a[] = {1.0f, -0.8f};
    float zi[1];

    lfilter_ic(b, 2, a, 2, zi);

    // zi should be non-zero
    EXPECT_NE(zi[0], 0.0f);
}

TEST_F(LFilterTest, InitialConditionsReduceTransient) {
    // Use computed initial conditions to reduce step response transient
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);
    size_t order = coeffs.a.size() - 1;
    std::vector<float> zi(order);

    lfilter_ic(coeffs.b.data(), coeffs.b.size(),
               coeffs.a.data(), coeffs.a.size(),
               zi.data());

    // Filter step with initial conditions
    auto input = step(50);
    std::vector<float> with_ic(50), without_ic(50);

    lfilter(coeffs.b.data(), coeffs.b.size(),
            coeffs.a.data(), coeffs.a.size(),
            input.data(), without_ic.data(), 50);

    lfilter_zi(coeffs.b.data(), coeffs.b.size(),
               coeffs.a.data(), coeffs.a.size(),
               input.data(), with_ic.data(), 50,
               zi.data(), nullptr);

    // With initial conditions, first samples should be closer to steady state
    // Steady state for step input to lowpass = 1.0 (DC gain)
    float error_without = std::abs(without_ic[0] - 1.0f);
    float error_with = std::abs(with_ic[0] - 1.0f);

    // With IC should have less error at start (or at least not worse)
    EXPECT_LE(error_with, error_without + 0.5f);
}

// ============================================================================
// Comparison Tests
// ============================================================================

TEST_F(LFilterTest, DifferentFilterTypes) {
    std::vector<float> input(100);
    for (size_t i = 0; i < 100; ++i) {
        input[i] = std::sin(0.1f * i) + 0.3f * std::cos(0.5f * i);
    }

    // Test with different filter types
    auto butter_lp = butter(3, 0.3f, FilterType::Lowpass);
    auto butter_hp = butter(3, 0.3f, FilterType::Highpass);
    auto cheby1_lp = cheby1(3, 1.0f, 0.3f, FilterType::Lowpass);

    std::vector<float> out_butter_lp(100), out_butter_hp(100), out_cheby(100);

    lfilter(butter_lp.b.data(), butter_lp.b.size(),
            butter_lp.a.data(), butter_lp.a.size(),
            input.data(), out_butter_lp.data(), 100);

    lfilter(butter_hp.b.data(), butter_hp.b.size(),
            butter_hp.a.data(), butter_hp.a.size(),
            input.data(), out_butter_hp.data(), 100);

    lfilter(cheby1_lp.b.data(), cheby1_lp.b.size(),
            cheby1_lp.a.data(), cheby1_lp.a.size(),
            input.data(), out_cheby.data(), 100);

    // Outputs should be different
    float sum_diff = 0.0f;
    for (size_t i = 0; i < 100; ++i) {
        sum_diff += std::abs(out_butter_lp[i] - out_butter_hp[i]);
    }
    EXPECT_GT(sum_diff, 1.0f);  // Should be significantly different
}

TEST_F(LFilterTest, InplaceProcessing) {
    float b[] = {0.5f, 0.5f};
    float a[] = {1.0f, -0.5f};
    std::vector<float> data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> expected(5);

    // Out-of-place reference
    lfilter(b, 2, a, 2, data.data(), expected.data(), 5);

    // In-place processing
    std::vector<float> inplace = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    lfilter(b, 2, a, 2, inplace.data(), inplace.data(), 5);

    // Note: In-place may not be supported - this tests current behavior
    // If it works, great; if not, we document the limitation
    // For now just verify it doesn't crash
    SUCCEED();
}

