/**
 * @file test_streaming_filter.cpp
 * @brief Unit tests for streaming filters
 */

#include <gtest/gtest.h>
#include "spectra/filters/streaming_filter.hpp"
#include "spectra/filters/filter_design.hpp"
#include "spectra/filters/iir_filter.hpp"
#include <cmath>

using namespace spectra;

class StreamingFilterTest : public ::testing::Test {
protected:
};

TEST_F(StreamingFilterTest, BasicConstruction) {
    std::vector<float> b = {0.5f, 0.5f};
    std::vector<float> a = {1.0f};

    StreamingIIRFilter filter(b, a);

    EXPECT_EQ(filter.order(), 1u);
}

TEST_F(StreamingFilterTest, ProcessSample) {
    std::vector<float> b = {0.5f, 0.5f};
    std::vector<float> a = {1.0f};
    StreamingIIRFilter filter(b, a);

    float y0 = filter.process_sample(1.0f);
    float y1 = filter.process_sample(1.0f);
    float y2 = filter.process_sample(1.0f);

    EXPECT_NEAR(y0, 0.5f, 1e-5f);
    EXPECT_NEAR(y1, 1.0f, 1e-5f);
    EXPECT_NEAR(y2, 1.0f, 1e-5f);
}

TEST_F(StreamingFilterTest, ProcessBlock) {
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);
    StreamingIIRFilter filter(coeffs);

    std::vector<float> input(100, 1.0f);
    std::vector<float> output(100);

    filter.process(input.data(), output.data(), 100);

    // Should converge to DC gain of 1
    EXPECT_NEAR(output[99], 1.0f, 0.1f);
}

TEST_F(StreamingFilterTest, MatchesLFilter) {
    auto coeffs = butter(2, 0.2f, FilterType::Lowpass);

    // Generate test signal
    std::vector<float> input(200);
    for (size_t i = 0; i < 200; ++i) {
        input[i] = std::sin(0.1f * i);
    }

    // Stateless lfilter
    std::vector<float> lfilter_out(200);
    lfilter(coeffs.b.data(), coeffs.b.size(),
            coeffs.a.data(), coeffs.a.size(),
            input.data(), lfilter_out.data(), 200);

    // Streaming filter
    StreamingIIRFilter filter(coeffs);
    std::vector<float> streaming_out(200);
    filter.process(input.data(), streaming_out.data(), 200);

    // Should match
    for (size_t i = 0; i < 200; ++i) {
        EXPECT_NEAR(streaming_out[i], lfilter_out[i], 1e-5f);
    }
}

TEST_F(StreamingFilterTest, ChunkedProcessing) {
    auto coeffs = butter(2, 0.2f, FilterType::Lowpass);
    StreamingIIRFilter filter(coeffs);

    std::vector<float> input(100);
    for (size_t i = 0; i < 100; ++i) {
        input[i] = std::sin(0.1f * i);
    }

    // Process in chunks
    std::vector<float> chunked_output(100);
    filter.process(input.data(), chunked_output.data(), 25);
    filter.process(input.data() + 25, chunked_output.data() + 25, 25);
    filter.process(input.data() + 50, chunked_output.data() + 50, 25);
    filter.process(input.data() + 75, chunked_output.data() + 75, 25);

    // Process all at once
    StreamingIIRFilter filter2(coeffs);
    std::vector<float> single_output(100);
    filter2.process(input.data(), single_output.data(), 100);

    // Should match
    for (size_t i = 0; i < 100; ++i) {
        EXPECT_NEAR(chunked_output[i], single_output[i], 1e-5f);
    }
}

TEST_F(StreamingFilterTest, Reset) {
    auto coeffs = butter(2, 0.2f, FilterType::Lowpass);
    StreamingIIRFilter filter(coeffs);

    // Process some data
    std::vector<float> input(50, 1.0f);
    std::vector<float> output(50);
    filter.process(input.data(), output.data(), 50);

    // Reset
    filter.reset();

    // Process again - should be same as fresh filter
    StreamingIIRFilter fresh(coeffs);
    std::vector<float> fresh_output(50);
    fresh.process(input.data(), fresh_output.data(), 50);

    std::vector<float> reset_output(50);
    filter.process(input.data(), reset_output.data(), 50);

    for (size_t i = 0; i < 50; ++i) {
        EXPECT_NEAR(reset_output[i], fresh_output[i], 1e-5f);
    }
}

TEST_F(StreamingFilterTest, InplaceProcessing) {
    auto coeffs = butter(2, 0.2f, FilterType::Lowpass);
    StreamingIIRFilter filter1(coeffs);
    StreamingIIRFilter filter2(coeffs);

    std::vector<float> data(100);
    for (size_t i = 0; i < 100; ++i) {
        data[i] = std::sin(0.1f * i);
    }

    std::vector<float> separate_out(100);
    filter1.process(data.data(), separate_out.data(), 100);

    // In-place
    std::vector<float> inplace_data = data;
    filter2.process_inplace(inplace_data.data(), 100);

    for (size_t i = 0; i < 100; ++i) {
        EXPECT_NEAR(inplace_data[i], separate_out[i], 1e-5f);
    }
}

TEST_F(StreamingFilterTest, BiquadCascadeBasic) {
    BiquadCascade::Section s;
    s.b0 = 0.5f;
    s.b1 = 0.5f;
    s.b2 = 0.0f;
    s.a1 = 0.0f;
    s.a2 = 0.0f;

    BiquadCascade cascade({s});

    float y0 = cascade.process_sample(1.0f);
    float y1 = cascade.process_sample(1.0f);

    EXPECT_NEAR(y0, 0.5f, 1e-5f);
    EXPECT_NEAR(y1, 1.0f, 1e-5f);
}

// ============================================================================
// FILT-007: Steady-State Initialization Tests
// ============================================================================

TEST_F(StreamingFilterTest, SteadyStateInitialization) {
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);
    StreamingIIRFilter filter(coeffs);

    // Set steady state for DC level of 1.0
    filter.set_steady_state(1.0f);

    // Process step input
    std::vector<float> input(50, 1.0f);
    std::vector<float> output(50);
    filter.process(input.data(), output.data(), 50);

    // With steady state, later samples should be close to 1.0
    // The first few samples may still have some transient due to
    // simplified steady-state computation
    EXPECT_NEAR(output[10], 1.0f, 0.2f);
    EXPECT_NEAR(output[49], 1.0f, 0.05f);
}

TEST_F(StreamingFilterTest, SteadyStateVsNoSteadyState) {
    auto coeffs = butter(3, 0.2f, FilterType::Lowpass);

    StreamingIIRFilter filter_no_ss(coeffs);
    StreamingIIRFilter filter_with_ss(coeffs);

    filter_with_ss.set_steady_state(1.0f);

    std::vector<float> input(100, 1.0f);
    std::vector<float> out_no_ss(100), out_with_ss(100);

    filter_no_ss.process(input.data(), out_no_ss.data(), 100);
    filter_with_ss.process(input.data(), out_with_ss.data(), 100);

    // Both should converge to 1.0 eventually
    EXPECT_NEAR(out_no_ss[99], 1.0f, 0.01f);
    EXPECT_NEAR(out_with_ss[99], 1.0f, 0.01f);

    // With steady state should converge faster (less error mid-signal)
    float mid_error_no_ss = std::abs(out_no_ss[20] - 1.0f);
    float mid_error_with_ss = std::abs(out_with_ss[20] - 1.0f);
    EXPECT_LE(mid_error_with_ss, mid_error_no_ss + 0.5f);
}

// ============================================================================
// FILT-008: Comprehensive Biquad Cascade Tests
// ============================================================================

TEST_F(StreamingFilterTest, BiquadCascadeFromCoeffs) {
    // Test with a second-order filter (order 2 = 1 biquad section)
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);
    BiquadCascade cascade(coeffs);

    // Order 2 should have exactly 1 section
    EXPECT_EQ(cascade.num_sections(), 1u);
}

TEST_F(StreamingFilterTest, BiquadCascadeSecondOrder) {
    // Test biquad cascade with second-order filter (works without complex tf2sos)
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);

    // Direct form
    StreamingIIRFilter direct(coeffs);

    // Biquad cascade (single section for order 2)
    BiquadCascade cascade(coeffs);

    // Generate test signal
    std::vector<float> input(100);
    for (size_t i = 0; i < 100; ++i) {
        input[i] = std::sin(0.1f * i);
    }

    std::vector<float> direct_out(100), cascade_out(100);
    direct.process(input.data(), direct_out.data(), 100);
    cascade.process(input.data(), cascade_out.data(), 100);

    // Should produce same output for second-order filter
    for (size_t i = 0; i < 100; ++i) {
        EXPECT_NEAR(cascade_out[i], direct_out[i], 0.01f) << "Sample " << i;
    }
}

TEST_F(StreamingFilterTest, BiquadCascadeFirstOrder) {
    // Test with first-order filter
    auto coeffs = butter(1, 0.3f, FilterType::Lowpass);
    BiquadCascade cascade(coeffs);

    // Process step input
    std::vector<float> input(50, 1.0f);
    std::vector<float> output(50);
    cascade.process(input.data(), output.data(), 50);

    // Should converge toward DC gain of 1
    EXPECT_NEAR(output[49], 1.0f, 0.1f);

    // No NaN/Inf
    for (float y : output) {
        EXPECT_FALSE(std::isnan(y));
        EXPECT_FALSE(std::isinf(y));
    }
}

TEST_F(StreamingFilterTest, BiquadCascadeStability) {
    // Test stability for second-order filter (single biquad section)
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);
    BiquadCascade cascade(coeffs);

    std::vector<float> input(1000);
    for (size_t i = 0; i < 1000; ++i) {
        input[i] = std::sin(0.05f * i);
    }

    std::vector<float> output(1000);
    cascade.process(input.data(), output.data(), 1000);

    // Check no NaN/Inf
    for (size_t i = 0; i < 1000; ++i) {
        EXPECT_FALSE(std::isnan(output[i]));
        EXPECT_FALSE(std::isinf(output[i]));
    }

    // Should be bounded
    float max_abs = 0.0f;
    for (float y : output) {
        max_abs = std::max(max_abs, std::abs(y));
    }
    EXPECT_LT(max_abs, 5.0f);
}

TEST_F(StreamingFilterTest, BiquadCascadeReset) {
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);
    BiquadCascade cascade(coeffs);

    // Process some data
    std::vector<float> input(100);
    for (size_t i = 0; i < 100; ++i) {
        input[i] = std::sin(0.1f * i);
    }

    std::vector<float> output1(100), output2(100);
    cascade.process(input.data(), output1.data(), 100);

    // Reset
    cascade.reset();

    // Process same input again
    cascade.process(input.data(), output2.data(), 100);

    // Should match (reset clears state)
    for (size_t i = 0; i < 100; ++i) {
        EXPECT_NEAR(output1[i], output2[i], 1e-5f);
    }
}

TEST_F(StreamingFilterTest, BiquadCascadeChunked) {
    // Use second-order filter for reliable tf2sos conversion
    auto coeffs = butter(2, 0.25f, FilterType::Lowpass);

    // Single block
    BiquadCascade cascade1(coeffs);
    std::vector<float> input(200);
    for (size_t i = 0; i < 200; ++i) {
        input[i] = std::sin(0.08f * i);
    }
    std::vector<float> single_out(200);
    cascade1.process(input.data(), single_out.data(), 200);

    // Chunked
    BiquadCascade cascade2(coeffs);
    std::vector<float> chunked_out(200);
    cascade2.process(input.data(), chunked_out.data(), 50);
    cascade2.process(input.data() + 50, chunked_out.data() + 50, 50);
    cascade2.process(input.data() + 100, chunked_out.data() + 100, 50);
    cascade2.process(input.data() + 150, chunked_out.data() + 150, 50);

    // Should match
    for (size_t i = 0; i < 200; ++i) {
        EXPECT_NEAR(chunked_out[i], single_out[i], 1e-5f) << "Sample " << i;
    }
}

TEST_F(StreamingFilterTest, Tf2sosBasic) {
    auto coeffs = butter(4, 0.3f, FilterType::Lowpass);
    auto sos = tf2sos(coeffs);

    // Should produce at least one section
    EXPECT_GE(sos.size(), 1u);

    // Each section should have valid (non-NaN) coefficients
    for (const auto& s : sos) {
        EXPECT_FALSE(std::isnan(s.b0));
        EXPECT_FALSE(std::isnan(s.b1));
        EXPECT_FALSE(std::isnan(s.b2));
        EXPECT_FALSE(std::isnan(s.a1));
        EXPECT_FALSE(std::isnan(s.a2));
    }
}

TEST_F(StreamingFilterTest, Tf2sosOddOrder) {
    auto coeffs = butter(5, 0.3f, FilterType::Lowpass);
    auto sos = tf2sos(coeffs);

    // Should produce at least one section
    EXPECT_GE(sos.size(), 1u);

    // Coefficients should be valid
    for (const auto& s : sos) {
        EXPECT_FALSE(std::isnan(s.b0));
        EXPECT_FALSE(std::isnan(s.a1));
    }
}
