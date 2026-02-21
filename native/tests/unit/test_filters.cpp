/**
 * @file test_filters.cpp
 * @brief Unit tests for filter design and application
 */

#include <gtest/gtest.h>
#include "spectra/filters/filter_design.hpp"
#include "spectra/filters/iir_filter.hpp"
#include <cmath>

using namespace spectra;

class FilterDesignTest : public ::testing::Test {
protected:
};

TEST_F(FilterDesignTest, ButterCreatesCoefficients) {
    auto coeffs = butter(4, 0.2f, FilterType::Lowpass);

    EXPECT_FALSE(coeffs.b.empty());
    EXPECT_FALSE(coeffs.a.empty());
    EXPECT_EQ(coeffs.a[0], 1.0f);  // Normalized
}

TEST_F(FilterDesignTest, ButterLowpassPassesDC) {
    auto coeffs = butter(2, 0.3f, FilterType::Lowpass);

    // DC input
    std::vector<float> input(100, 1.0f);
    std::vector<float> output(100);

    lfilter(coeffs.b.data(), coeffs.b.size(),
            coeffs.a.data(), coeffs.a.size(),
            input.data(), output.data(), 100);

    // Output should converge to 1 (DC gain = 1)
    EXPECT_NEAR(output[99], 1.0f, 0.1f);
}

TEST_F(FilterDesignTest, LFilterBasic) {
    // Simple averaging filter: b = [0.5, 0.5], a = [1]
    float b[] = {0.5f, 0.5f};
    float a[] = {1.0f};
    float input[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float output[4];

    lfilter(b, 2, a, 1, input, output, 4);

    EXPECT_NEAR(output[0], 0.5f, 1e-5f);  // 0.5*1 + 0.5*0
    EXPECT_NEAR(output[1], 1.0f, 1e-5f);  // 0.5*1 + 0.5*1
    EXPECT_NEAR(output[2], 1.0f, 1e-5f);
    EXPECT_NEAR(output[3], 1.0f, 1e-5f);
}

TEST_F(FilterDesignTest, FiltFiltZeroPhase) {
    auto coeffs = butter(2, 0.1f, FilterType::Lowpass);

    // Create test signal with known phase
    std::vector<float> input(200);
    for (size_t i = 0; i < 200; ++i) {
        input[i] = std::sin(0.1f * i);
    }

    std::vector<float> output(200);
    filtfilt(coeffs.b.data(), coeffs.b.size(),
             coeffs.a.data(), coeffs.a.size(),
             input.data(), output.data(), 200);

    // Peak of filtfilt output should align with peak of input
    // (zero phase means no delay)
    size_t input_peak = 0, output_peak = 0;
    float input_max = input[0], output_max = output[0];

    for (size_t i = 50; i < 150; ++i) {
        if (input[i] > input_max) {
            input_max = input[i];
            input_peak = i;
        }
        if (output[i] > output_max) {
            output_max = output[i];
            output_peak = i;
        }
    }

    EXPECT_NEAR(static_cast<float>(output_peak), static_cast<float>(input_peak), 2.0f);
}

TEST_F(FilterDesignTest, FIRWindowedSinc) {
    auto h = fir_windowed_sinc(31, 0.2f, FilterType::Lowpass, WindowType::Hamming);

    EXPECT_EQ(h.size(), 31u);

    // Should be symmetric
    for (size_t i = 0; i < h.size() / 2; ++i) {
        EXPECT_NEAR(h[i], h[h.size() - 1 - i], 1e-5f);
    }
}

TEST_F(FilterDesignTest, FrequencyResponse) {
    auto coeffs = butter(4, 0.2f, FilterType::Lowpass);

    std::vector<Complex> H(128);
    freqz(coeffs, H.data(), 128);

    // DC gain should be ~1
    EXPECT_NEAR(H[0].magnitude(), 1.0f, 0.1f);

    // High frequency gain should be attenuated
    EXPECT_LT(H[127].magnitude(), 0.5f);
}

TEST_F(FilterDesignTest, IIRCoeffSize) {
    EXPECT_EQ(iir_coeff_size(1), 2);
    EXPECT_EQ(iir_coeff_size(2), 3);
    EXPECT_EQ(iir_coeff_size(4), 5);
}
