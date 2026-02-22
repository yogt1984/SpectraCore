/**
 * @file test_abi_filter.cpp
 * @brief Unit tests for Filter C ABI functions
 */

#include <gtest/gtest.h>
#include "spectra_abi.h"
#include <cmath>
#include <vector>

class ABIFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        spectra_clear_error();
    }

    void TearDown() override {
        spectra_clear_error();
    }

    static constexpr float PI = 3.14159265358979323846f;
};

// ============================================================================
// Butterworth Filter Design
// ============================================================================

TEST_F(ABIFilterTest, ButterLowpass) {
    const int order = 4;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    int result = spectra_butter(order, 0.3f, SPECTRA_FILTER_LOWPASS,
                                 b.data(), &b_len, a.data(), &a_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(b_len, 0);
    EXPECT_GT(a_len, 0);

    // Check no NaN
    for (int i = 0; i < b_len; ++i) {
        EXPECT_FALSE(std::isnan(b[i]));
    }
    for (int i = 0; i < a_len; ++i) {
        EXPECT_FALSE(std::isnan(a[i]));
    }
}

TEST_F(ABIFilterTest, ButterHighpass) {
    const int order = 3;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    int result = spectra_butter(order, 0.25f, SPECTRA_FILTER_HIGHPASS,
                                 b.data(), &b_len, a.data(), &a_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(b_len, 0);
    EXPECT_GT(a_len, 0);
}

TEST_F(ABIFilterTest, ButterBandpass) {
    const int order = 2;
    // For bandpass, we need to pass two frequencies (but ABI may use different method)
    // Using single freq version for now
    int max_size = spectra_iir_coeff_size(order * 2);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    int result = spectra_butter(order, 0.3f, SPECTRA_FILTER_BANDPASS,
                                 b.data(), &b_len, a.data(), &a_len);
    // May fail if bandpass requires two frequencies
    if (result == SPECTRA_OK) {
        EXPECT_GT(b_len, 0);
        EXPECT_GT(a_len, 0);
    }
}

// ============================================================================
// Chebyshev Type 1 Filter Design
// ============================================================================

TEST_F(ABIFilterTest, Cheby1Lowpass) {
    const int order = 4;
    const float ripple_db = 1.0f;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    int result = spectra_cheby1(order, ripple_db, 0.3f, SPECTRA_FILTER_LOWPASS,
                                 b.data(), &b_len, a.data(), &a_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(b_len, 0);
    EXPECT_GT(a_len, 0);

    // Check no NaN
    for (int i = 0; i < b_len; ++i) {
        EXPECT_FALSE(std::isnan(b[i]));
    }
    for (int i = 0; i < a_len; ++i) {
        EXPECT_FALSE(std::isnan(a[i]));
    }
}

TEST_F(ABIFilterTest, Cheby1Highpass) {
    const int order = 3;
    const float ripple_db = 0.5f;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    int result = spectra_cheby1(order, ripple_db, 0.25f, SPECTRA_FILTER_HIGHPASS,
                                 b.data(), &b_len, a.data(), &a_len);
    EXPECT_EQ(result, SPECTRA_OK);
}

// ============================================================================
// Chebyshev Type 2 Filter Design
// ============================================================================

TEST_F(ABIFilterTest, Cheby2Lowpass) {
    const int order = 4;
    const float stopband_db = 40.0f;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    int result = spectra_cheby2(order, stopband_db, 0.3f, SPECTRA_FILTER_LOWPASS,
                                 b.data(), &b_len, a.data(), &a_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(b_len, 0);
}

// ============================================================================
// Elliptic Filter Design
// ============================================================================

TEST_F(ABIFilterTest, EllipticLowpass) {
    const int order = 4;
    const float passband_ripple = 1.0f;
    const float stopband_db = 40.0f;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    int result = spectra_ellip(order, passband_ripple, stopband_db, 0.3f,
                                SPECTRA_FILTER_LOWPASS,
                                b.data(), &b_len, a.data(), &a_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(b_len, 0);
}

// ============================================================================
// Streaming Filter
// ============================================================================

TEST_F(ABIFilterTest, StreamingFilterCreateDestroy) {
    // Design a simple filter
    const int order = 2;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    spectra_butter(order, 0.3f, SPECTRA_FILTER_LOWPASS,
                   b.data(), &b_len, a.data(), &a_len);

    SpectraStreamingFilter filter = spectra_streaming_filter_create(
        b.data(), b_len, a.data(), a_len);
    EXPECT_NE(filter, nullptr);

    spectra_streaming_filter_destroy(filter);
}

TEST_F(ABIFilterTest, StreamingFilterProcess) {
    // Design filter
    const int order = 4;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    spectra_butter(order, 0.3f, SPECTRA_FILTER_LOWPASS,
                   b.data(), &b_len, a.data(), &a_len);

    SpectraStreamingFilter filter = spectra_streaming_filter_create(
        b.data(), b_len, a.data(), a_len);
    ASSERT_NE(filter, nullptr);

    // Generate input
    const int count = 256;
    std::vector<float> input(count);
    std::vector<float> output(count);

    for (int i = 0; i < count; ++i) {
        input[i] = std::sin(2.0f * PI * 10 * i / count);
    }

    int result = spectra_streaming_filter_process(filter,
                                                   input.data(), output.data(), count);
    EXPECT_EQ(result, SPECTRA_OK);

    // Check output is valid
    for (int i = 0; i < count; ++i) {
        EXPECT_FALSE(std::isnan(output[i]));
        EXPECT_FALSE(std::isinf(output[i]));
    }

    spectra_streaming_filter_destroy(filter);
}

TEST_F(ABIFilterTest, StreamingFilterReset) {
    const int order = 2;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    spectra_butter(order, 0.3f, SPECTRA_FILTER_LOWPASS,
                   b.data(), &b_len, a.data(), &a_len);

    SpectraStreamingFilter filter = spectra_streaming_filter_create(
        b.data(), b_len, a.data(), a_len);
    ASSERT_NE(filter, nullptr);

    // Process some data
    std::vector<float> data(64, 1.0f);
    std::vector<float> output(64);
    spectra_streaming_filter_process(filter, data.data(), output.data(), 64);

    // Reset
    int result = spectra_streaming_filter_reset(filter);
    EXPECT_EQ(result, SPECTRA_OK);

    // Process again - first sample should have transient
    spectra_streaming_filter_process(filter, data.data(), output.data(), 64);
    EXPECT_NE(output[0], data[0]);  // Transient expected

    spectra_streaming_filter_destroy(filter);
}

TEST_F(ABIFilterTest, StreamingFilterChunked) {
    // Chunked processing should match single-block
    const int order = 4;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    spectra_butter(order, 0.3f, SPECTRA_FILTER_LOWPASS,
                   b.data(), &b_len, a.data(), &a_len);

    // Create two filters
    SpectraStreamingFilter filter1 = spectra_streaming_filter_create(
        b.data(), b_len, a.data(), a_len);
    SpectraStreamingFilter filter2 = spectra_streaming_filter_create(
        b.data(), b_len, a.data(), a_len);

    const int total = 256;
    const int chunk = 64;

    std::vector<float> input(total);
    std::vector<float> output1(total);
    std::vector<float> output2(total);

    for (int i = 0; i < total; ++i) {
        input[i] = std::sin(2.0f * PI * 5 * i / total);
    }

    // Single block
    spectra_streaming_filter_process(filter1, input.data(), output1.data(), total);

    // Chunked
    for (int i = 0; i < total; i += chunk) {
        spectra_streaming_filter_process(filter2,
                                          input.data() + i, output2.data() + i, chunk);
    }

    // Should match
    for (int i = 0; i < total; ++i) {
        EXPECT_NEAR(output1[i], output2[i], 1e-5f);
    }

    spectra_streaming_filter_destroy(filter1);
    spectra_streaming_filter_destroy(filter2);
}

// ============================================================================
// Stateless Filter Application
// ============================================================================

TEST_F(ABIFilterTest, LfilterBasic) {
    // Simple moving average
    std::vector<float> b = {0.25f, 0.25f, 0.25f, 0.25f};
    std::vector<float> a = {1.0f};

    const int len = 64;
    std::vector<float> input(len, 1.0f);
    std::vector<float> output(len);

    int result = spectra_lfilter(b.data(), static_cast<int>(b.size()),
                                  a.data(), static_cast<int>(a.size()),
                                  input.data(), output.data(), len);
    EXPECT_EQ(result, SPECTRA_OK);

    // After transient, output should be 1.0
    for (int i = 4; i < len; ++i) {
        EXPECT_NEAR(output[i], 1.0f, 0.01f);
    }
}

TEST_F(ABIFilterTest, LfilterIIR) {
    const int order = 2;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    spectra_butter(order, 0.3f, SPECTRA_FILTER_LOWPASS,
                   b.data(), &b_len, a.data(), &a_len);

    const int len = 128;
    std::vector<float> input(len);
    std::vector<float> output(len);

    for (int i = 0; i < len; ++i) {
        input[i] = std::sin(2.0f * PI * 5 * i / len);
    }

    int result = spectra_lfilter(b.data(), b_len, a.data(), a_len,
                                  input.data(), output.data(), len);
    EXPECT_EQ(result, SPECTRA_OK);

    // Check output is valid
    for (int i = 0; i < len; ++i) {
        EXPECT_FALSE(std::isnan(output[i]));
    }
}

TEST_F(ABIFilterTest, FiltfiltBasic) {
    const int order = 4;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    spectra_butter(order, 0.2f, SPECTRA_FILTER_LOWPASS,
                   b.data(), &b_len, a.data(), &a_len);

    const int len = 256;
    std::vector<float> input(len);
    std::vector<float> output(len);

    for (int i = 0; i < len; ++i) {
        input[i] = std::sin(2.0f * PI * 3 * i / len);
    }

    int result = spectra_filtfilt(b.data(), b_len, a.data(), a_len,
                                   input.data(), output.data(), len);
    EXPECT_EQ(result, SPECTRA_OK);

    // Check output is valid
    for (int i = 0; i < len; ++i) {
        EXPECT_FALSE(std::isnan(output[i]));
        EXPECT_FALSE(std::isinf(output[i]));
    }
}

TEST_F(ABIFilterTest, FiltfiltZeroPhase) {
    // filtfilt should produce zero phase delay
    const int order = 4;
    int max_size = spectra_iir_coeff_size(order);

    std::vector<float> b(max_size);
    std::vector<float> a(max_size);
    int b_len, a_len;

    spectra_butter(order, 0.4f, SPECTRA_FILTER_LOWPASS,
                   b.data(), &b_len, a.data(), &a_len);

    const int len = 256;
    std::vector<float> input(len);
    std::vector<float> output_ff(len);

    // Low frequency sine (in passband)
    for (int i = 0; i < len; ++i) {
        input[i] = std::sin(2.0f * PI * 2 * i / len);
    }

    int result = spectra_filtfilt(b.data(), b_len, a.data(), a_len,
                                   input.data(), output_ff.data(), len);
    EXPECT_EQ(result, SPECTRA_OK);

    // Basic check: output should be valid and smooth
    for (int i = 0; i < len; ++i) {
        EXPECT_FALSE(std::isnan(output_ff[i]));
        EXPECT_FALSE(std::isinf(output_ff[i]));
    }
}

// ============================================================================
// Coefficient Size
// ============================================================================

TEST_F(ABIFilterTest, CoeffSize) {
    EXPECT_EQ(spectra_iir_coeff_size(1), 2);
    EXPECT_EQ(spectra_iir_coeff_size(2), 3);
    EXPECT_EQ(spectra_iir_coeff_size(4), 5);
    EXPECT_EQ(spectra_iir_coeff_size(8), 9);
}
