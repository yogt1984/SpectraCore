/**
 * @file test_correlation.cpp
 * @brief Unit tests for correlation functions
 */

#include <gtest/gtest.h>
#include "spectra/analysis/correlation.hpp"
#include <cmath>

using namespace spectra;

class CorrelationTest : public ::testing::Test {
protected:
};

TEST_F(CorrelationTest, AutocorrelationDelta) {
    // Delta function autocorrelation is also a delta
    float x[] = {0, 0, 1, 0, 0};
    float output[9];

    xcorr(x, 5, output);

    // Peak at center
    size_t center = 4;  // (2*5-1) / 2
    EXPECT_FLOAT_EQ(output[center], 1.0f);
}

TEST_F(CorrelationTest, CrossCorrelationIdentical) {
    float x[] = {1, 2, 3, 4};
    float output[7];  // 4 + 4 - 1

    xcorr(x, 4, x, 4, output);

    // Should equal autocorrelation
    float autocorr[7];
    xcorr(x, 4, autocorr);

    for (size_t i = 0; i < 7; ++i) {
        EXPECT_FLOAT_EQ(output[i], autocorr[i]);
    }
}

TEST_F(CorrelationTest, CrossCorrelationOutputSize) {
    EXPECT_EQ(xcorr_output_size(4, 3), 6u);
    EXPECT_EQ(xcorr_output_size(5, 5), 9u);
    EXPECT_EQ(xcorr_output_size(10, 1), 10u);
}

TEST_F(CorrelationTest, NormalizedCorrelation) {
    float x[] = {1, 2, 3, 4, 5};
    float y[] = {1, 2, 3, 4, 5};
    float output[9];

    xcorr_normalized(x, 5, y, 5, output);

    // Max normalized correlation should be 1
    float max_val = 0;
    for (size_t i = 0; i < 9; ++i) {
        max_val = std::max(max_val, std::abs(output[i]));
    }
    EXPECT_NEAR(max_val, 1.0f, 0.01f);
}

TEST_F(CorrelationTest, CorrCoefIdentical) {
    float x[] = {1, 2, 3, 4, 5};

    float r = corrcoef(x, x, 5);
    EXPECT_NEAR(r, 1.0f, 1e-5f);
}

TEST_F(CorrelationTest, CorrCoefNegative) {
    float x[] = {1, 2, 3, 4, 5};
    float y[] = {5, 4, 3, 2, 1};

    float r = corrcoef(x, y, 5);
    EXPECT_NEAR(r, -1.0f, 1e-5f);
}

TEST_F(CorrelationTest, CorrCoefUncorrelated) {
    float x[] = {1, 0, -1, 0};
    float y[] = {0, 1, 0, -1};

    float r = corrcoef(x, y, 4);
    EXPECT_NEAR(r, 0.0f, 1e-5f);
}

TEST_F(CorrelationTest, FindMaxLag) {
    // Shifted signal
    float x[] = {0, 0, 0, 1, 2, 3, 4, 0, 0};
    float y[] = {1, 2, 3, 4, 0, 0, 0, 0, 0};

    size_t x_len = 9, y_len = 9;
    std::vector<float> output(xcorr_output_size(x_len, y_len));
    xcorr(x, x_len, y, y_len, output.data());

    int lag = find_max_correlation_lag(output.data(), output.size(), y_len);

    // y is shifted left relative to x, so lag should be positive
    EXPECT_GT(lag, 0);
}
