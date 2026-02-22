/**
 * @file test_resample.cpp
 * @brief Unit tests for resampling functions
 */

#include <gtest/gtest.h>
#include "spectra/analysis/resample.hpp"
#include <cmath>
#include <numeric>

using namespace spectra;

class ResampleTest : public ::testing::Test {
protected:
    std::vector<float> sine_wave(size_t len, float freq, float sample_rate) {
        std::vector<float> signal(len);
        for (size_t i = 0; i < len; ++i) {
            signal[i] = std::sin(constants::two_pi * freq * i / sample_rate);
        }
        return signal;
    }

    // Compute RMS error between two signals
    float rms_error(const float* a, const float* b, size_t len) {
        float sum_sq = 0.0f;
        for (size_t i = 0; i < len; ++i) {
            float diff = a[i] - b[i];
            sum_sq += diff * diff;
        }
        return std::sqrt(sum_sq / len);
    }
};

// ============================================================================
// Resample Function Tests
// ============================================================================

TEST_F(ResampleTest, ResampleOutputSize) {
    EXPECT_EQ(resample_output_size(100, 3, 2), 150u);  // Upsample 3/2
    EXPECT_EQ(resample_output_size(100, 2, 3), 67u);   // Downsample 2/3
    EXPECT_EQ(resample_output_size(100, 1, 1), 100u);  // No change
}

TEST_F(ResampleTest, ResampleUpsample) {
    auto signal = sine_wave(100, 10.0f, 1000.0f);

    size_t output_len;
    std::vector<float> output(200);

    resample(signal.data(), signal.size(), 2, 1, output.data(), &output_len);

    EXPECT_EQ(output_len, 200u);

    // Check no NaN/Inf
    for (size_t i = 0; i < output_len; ++i) {
        EXPECT_FALSE(std::isnan(output[i]));
        EXPECT_FALSE(std::isinf(output[i]));
    }
}

TEST_F(ResampleTest, ResampleDownsample) {
    auto signal = sine_wave(200, 10.0f, 2000.0f);

    size_t output_len;
    std::vector<float> output(100);

    resample(signal.data(), signal.size(), 1, 2, output.data(), &output_len);

    EXPECT_EQ(output_len, 100u);

    for (size_t i = 0; i < output_len; ++i) {
        EXPECT_FALSE(std::isnan(output[i]));
        EXPECT_FALSE(std::isinf(output[i]));
    }
}

TEST_F(ResampleTest, ResampleRational) {
    // 48kHz to 44.1kHz (p=147, q=160)
    auto signal = sine_wave(480, 100.0f, 48000.0f);

    size_t output_len;
    std::vector<float> output(441);

    resample(signal.data(), signal.size(), 147, 160, output.data(), &output_len);

    // Output length should be approximately 480 * 147/160 = 441
    EXPECT_NEAR(static_cast<float>(output_len), 441.0f, 5.0f);

    for (size_t i = 0; i < output_len; ++i) {
        EXPECT_FALSE(std::isnan(output[i]));
        EXPECT_FALSE(std::isinf(output[i]));
    }
}

// ============================================================================
// Upsample/Downsample Basic Tests
// ============================================================================

TEST_F(ResampleTest, UpsampleBasic) {
    std::vector<float> signal = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> output(12);

    upsample(signal.data(), signal.size(), 3, output.data());

    // Check that original samples are at positions 0, 3, 6, 9
    EXPECT_FLOAT_EQ(output[0], 1.0f);
    EXPECT_FLOAT_EQ(output[3], 2.0f);
    EXPECT_FLOAT_EQ(output[6], 3.0f);
    EXPECT_FLOAT_EQ(output[9], 4.0f);

    // Intermediate values should be zero
    EXPECT_FLOAT_EQ(output[1], 0.0f);
    EXPECT_FLOAT_EQ(output[2], 0.0f);
}

TEST_F(ResampleTest, DownsampleBasic) {
    std::vector<float> signal = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    std::vector<float> output(3);

    downsample(signal.data(), signal.size(), 2, output.data(), 0);

    EXPECT_FLOAT_EQ(output[0], 1.0f);
    EXPECT_FLOAT_EQ(output[1], 3.0f);
    EXPECT_FLOAT_EQ(output[2], 5.0f);
}

TEST_F(ResampleTest, DownsamplePhase) {
    std::vector<float> signal = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    std::vector<float> output(3);

    downsample(signal.data(), signal.size(), 2, output.data(), 1);

    EXPECT_FLOAT_EQ(output[0], 2.0f);
    EXPECT_FLOAT_EQ(output[1], 4.0f);
    EXPECT_FLOAT_EQ(output[2], 6.0f);
}

// ============================================================================
// Decimate Tests
// ============================================================================

TEST_F(ResampleTest, DecimateBasic) {
    auto signal = sine_wave(1000, 50.0f, 10000.0f);

    size_t output_len;
    std::vector<float> output(500);

    decimate(signal.data(), signal.size(), 2, output.data(), &output_len);

    EXPECT_EQ(output_len, 500u);

    for (size_t i = 0; i < output_len; ++i) {
        EXPECT_FALSE(std::isnan(output[i]));
        EXPECT_FALSE(std::isinf(output[i]));
    }
}

TEST_F(ResampleTest, DecimateAntiAliasing) {
    // Signal with high frequency component above new Nyquist
    const float original_rate = 10000.0f;
    const float low_freq = 100.0f;    // Below new Nyquist
    const float high_freq = 4000.0f;  // Above new Nyquist after decimate by 2

    std::vector<float> signal(1000);
    for (size_t i = 0; i < signal.size(); ++i) {
        float t = static_cast<float>(i) / original_rate;
        signal[i] = std::sin(constants::two_pi * low_freq * t) +
                    std::sin(constants::two_pi * high_freq * t);
    }

    size_t output_len;
    std::vector<float> output(500);
    decimate(signal.data(), signal.size(), 2, output.data(), &output_len);

    // High frequency should be attenuated - check output magnitude
    float output_energy = 0.0f;
    for (size_t i = 50; i < output_len - 50; ++i) {
        output_energy += output[i] * output[i];
    }
    output_energy /= (output_len - 100);

    // Energy should be reasonable (not dominated by aliased high freq)
    EXPECT_LT(output_energy, 2.0f);
}

// ============================================================================
// Interp Tests
// ============================================================================

TEST_F(ResampleTest, InterpBasic) {
    std::vector<float> signal = {0.0f, 1.0f, 0.0f, -1.0f, 0.0f};
    std::vector<float> output(20);

    interp(signal.data(), signal.size(), 4, output.data());

    // Output should be smooth interpolation
    for (float v : output) {
        EXPECT_FALSE(std::isnan(v));
        EXPECT_FALSE(std::isinf(v));
    }
}

TEST_F(ResampleTest, InterpPreservesShape) {
    // Low frequency sine - should be well preserved by interpolation
    auto signal = sine_wave(100, 10.0f, 1000.0f);

    std::vector<float> output(400);
    interp(signal.data(), signal.size(), 4, output.data());

    // Check that interpolated signal still looks like sine
    // Skip edges and check peak/trough structure
    float max_val = *std::max_element(output.begin() + 20, output.end() - 20);
    float min_val = *std::min_element(output.begin() + 20, output.end() - 20);

    EXPECT_GT(max_val, 0.5f);
    EXPECT_LT(min_val, -0.5f);
}

// ============================================================================
// Linear Interpolation Tests
// ============================================================================

TEST_F(ResampleTest, ResampleLinear) {
    std::vector<float> signal = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> output(9);

    resample_linear(signal.data(), signal.size(), output.data(), output.size());

    // Linear interpolation of linear input should be exact
    EXPECT_NEAR(output[0], 0.0f, 1e-5f);
    EXPECT_NEAR(output[2], 1.0f, 1e-5f);
    EXPECT_NEAR(output[4], 2.0f, 1e-5f);
    EXPECT_NEAR(output[6], 3.0f, 1e-5f);
    EXPECT_NEAR(output[8], 4.0f, 1e-5f);

    // Intermediate values should be interpolated
    EXPECT_NEAR(output[1], 0.5f, 1e-5f);
    EXPECT_NEAR(output[3], 1.5f, 1e-5f);
}

TEST_F(ResampleTest, ResampleLinearUpsample) {
    auto signal = sine_wave(50, 5.0f, 100.0f);
    std::vector<float> output(100);

    resample_linear(signal.data(), signal.size(), output.data(), output.size());

    for (float v : output) {
        EXPECT_FALSE(std::isnan(v));
        EXPECT_GE(v, -1.1f);
        EXPECT_LE(v, 1.1f);
    }
}

TEST_F(ResampleTest, ResampleLinearDownsample) {
    auto signal = sine_wave(100, 5.0f, 100.0f);
    std::vector<float> output(50);

    resample_linear(signal.data(), signal.size(), output.data(), output.size());

    for (float v : output) {
        EXPECT_FALSE(std::isnan(v));
    }
}

// ============================================================================
// Cubic Interpolation Tests
// ============================================================================

TEST_F(ResampleTest, ResampleCubic) {
    std::vector<float> signal = {0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f};
    std::vector<float> output(21);

    resample_cubic(signal.data(), signal.size(), output.data(), output.size());

    // Output should be smooth
    for (float v : output) {
        EXPECT_FALSE(std::isnan(v));
        EXPECT_FALSE(std::isinf(v));
    }

    // Endpoints should match
    EXPECT_NEAR(output[0], signal[0], 1e-5f);
    EXPECT_NEAR(output[20], signal[5], 1e-5f);
}

TEST_F(ResampleTest, ResampleCubicSmooth) {
    // Sine wave should remain smooth after cubic interpolation
    auto signal = sine_wave(50, 3.0f, 50.0f);
    std::vector<float> output(200);

    resample_cubic(signal.data(), signal.size(), output.data(), output.size());

    // Check smoothness by looking at second derivative
    for (size_t i = 1; i < output.size() - 1; ++i) {
        float second_deriv = output[i+1] - 2*output[i] + output[i-1];
        // Second derivative should be bounded for smooth curve
        EXPECT_LT(std::abs(second_deriv), 0.5f);
    }
}

// ============================================================================
// Round-Trip Tests
// ============================================================================

TEST_F(ResampleTest, UpsampleDownsampleRoundTrip) {
    auto original = sine_wave(100, 5.0f, 1000.0f);

    // Upsample by 2
    std::vector<float> upsampled(200);
    interp(original.data(), original.size(), 2, upsampled.data());

    // Decimate by 2
    size_t output_len;
    std::vector<float> recovered(100);
    decimate(upsampled.data(), upsampled.size(), 2, recovered.data(), &output_len);

    // Should recover original (approximately, skip edges)
    float error = 0.0f;
    for (size_t i = 20; i < original.size() - 20; ++i) {
        error += std::abs(recovered[i] - original[i]);
    }
    error /= (original.size() - 40);

    EXPECT_LT(error, 0.3f);
}

TEST_F(ResampleTest, ResampleRoundTrip) {
    auto original = sine_wave(100, 5.0f, 1000.0f);

    // Resample up (3/2)
    size_t up_len;
    std::vector<float> upsampled(150);
    resample(original.data(), original.size(), 3, 2, upsampled.data(), &up_len);

    // Resample down (2/3)
    size_t down_len;
    std::vector<float> recovered(100);
    resample(upsampled.data(), up_len, 2, 3, recovered.data(), &down_len);

    // Lengths should be close
    EXPECT_NEAR(static_cast<float>(down_len), 100.0f, 5.0f);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ResampleTest, SmallSignal) {
    std::vector<float> signal = {1.0f, 2.0f, 3.0f};

    size_t output_len;
    std::vector<float> output(6);
    resample(signal.data(), signal.size(), 2, 1, output.data(), &output_len);

    EXPECT_GT(output_len, 0u);
}

TEST_F(ResampleTest, LargeRatio) {
    auto signal = sine_wave(1000, 10.0f, 10000.0f);

    // Decimate by large factor
    size_t output_len;
    std::vector<float> output(100);
    decimate(signal.data(), signal.size(), 10, output.data(), &output_len);

    EXPECT_EQ(output_len, 100u);
}
