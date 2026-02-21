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
