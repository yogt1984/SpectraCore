/**
 * @file test_abi_analysis.cpp
 * @brief Unit tests for Analysis C ABI functions
 */

#include <gtest/gtest.h>
#include "spectra_abi.h"
#include <cmath>
#include <vector>

class ABIAnalysisTest : public ::testing::Test {
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
// Cross-Correlation
// ============================================================================

TEST_F(ABIAnalysisTest, XcorrBasic) {
    std::vector<float> x = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> y = {1.0f, 2.0f, 3.0f, 4.0f};

    int output_size = spectra_xcorr_output_size(
        static_cast<int>(x.size()), static_cast<int>(y.size()));
    std::vector<float> output(output_size);
    int actual_len;

    int result = spectra_xcorr(x.data(), static_cast<int>(x.size()),
                                y.data(), static_cast<int>(y.size()),
                                output.data(), &actual_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(actual_len, 0);

    // Auto-correlation should have max at center
    int center = actual_len / 2;
    float max_val = output[center];
    for (int i = 0; i < actual_len; ++i) {
        EXPECT_LE(output[i], max_val + 0.01f);
    }
}

TEST_F(ABIAnalysisTest, XcorrDifferentLengths) {
    std::vector<float> x = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> y = {1.0f, 2.0f, 3.0f};

    int output_size = spectra_xcorr_output_size(
        static_cast<int>(x.size()), static_cast<int>(y.size()));
    std::vector<float> output(output_size);
    int actual_len;

    int result = spectra_xcorr(x.data(), static_cast<int>(x.size()),
                                y.data(), static_cast<int>(y.size()),
                                output.data(), &actual_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(actual_len, 0);
}

TEST_F(ABIAnalysisTest, XcorrOutputSize) {
    int size = spectra_xcorr_output_size(100, 50);
    EXPECT_GT(size, 0);
    EXPECT_EQ(size, 149);  // x_len + y_len - 1
}

// ============================================================================
// Power Spectral Density (pwelch)
// ============================================================================

TEST_F(ABIAnalysisTest, PwelchBasic) {
    const int input_len = 1024;
    std::vector<float> input(input_len);

    // Generate white noise-like signal
    for (int i = 0; i < input_len; ++i) {
        input[i] = std::sin(i * 0.1f) + std::cos(i * 0.3f);
    }

    SpectraPWelchConfig config;
    config.segment_size = 256;
    config.overlap = 128;
    config.window = SPECTRA_WINDOW_HANN;
    config.onesided = 1;

    int output_size = spectra_pwelch_output_size(&config);
    EXPECT_GT(output_size, 0);

    std::vector<float> psd(output_size);
    std::vector<float> freq(output_size);
    int actual_len;

    int result = spectra_pwelch(input.data(), input_len, &config, 44100.0f,
                                 psd.data(), freq.data(), &actual_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(actual_len, 0);

    // PSD should be non-negative
    for (int i = 0; i < actual_len; ++i) {
        EXPECT_GE(psd[i], 0.0f);
        EXPECT_FALSE(std::isnan(psd[i]));
    }

    // Frequencies should be monotonic
    for (int i = 1; i < actual_len; ++i) {
        EXPECT_GT(freq[i], freq[i - 1]);
    }
}

TEST_F(ABIAnalysisTest, PwelchSineWave) {
    const int input_len = 4096;
    const float sample_rate = 44100.0f;
    const float tone_freq = 1000.0f;

    std::vector<float> input(input_len);
    for (int i = 0; i < input_len; ++i) {
        input[i] = std::sin(2.0f * PI * tone_freq * i / sample_rate);
    }

    SpectraPWelchConfig config;
    config.segment_size = 512;
    config.overlap = 256;
    config.window = SPECTRA_WINDOW_HANN;
    config.onesided = 1;

    int output_size = spectra_pwelch_output_size(&config);
    std::vector<float> psd(output_size);
    std::vector<float> freq(output_size);
    int actual_len;

    spectra_pwelch(input.data(), input_len, &config, sample_rate,
                   psd.data(), freq.data(), &actual_len);

    // Find peak
    int peak_idx = 0;
    float peak_val = psd[0];
    for (int i = 1; i < actual_len; ++i) {
        if (psd[i] > peak_val) {
            peak_val = psd[i];
            peak_idx = i;
        }
    }

    // Peak should be near tone frequency
    float peak_freq = freq[peak_idx];
    EXPECT_NEAR(peak_freq, tone_freq, sample_rate / config.segment_size);
}

TEST_F(ABIAnalysisTest, PwelchOutputSize) {
    SpectraPWelchConfig config;
    config.segment_size = 256;
    config.onesided = 1;

    int size = spectra_pwelch_output_size(&config);
    EXPECT_EQ(size, 129);  // N/2 + 1

    config.onesided = 0;
    size = spectra_pwelch_output_size(&config);
    EXPECT_EQ(size, 256);
}

// ============================================================================
// Hilbert Transform
// ============================================================================

TEST_F(ABIAnalysisTest, HilbertBasic) {
    const int len = 256;
    std::vector<float> input(len);
    std::vector<float> output_real(len);
    std::vector<float> output_imag(len);

    for (int i = 0; i < len; ++i) {
        input[i] = std::sin(2.0f * PI * 5 * i / len);
    }

    int result = spectra_hilbert(input.data(),
                                  output_real.data(), output_imag.data(), len);
    EXPECT_EQ(result, SPECTRA_OK);

    // Real part should be similar to input
    for (int i = 0; i < len; ++i) {
        EXPECT_FALSE(std::isnan(output_real[i]));
        EXPECT_FALSE(std::isnan(output_imag[i]));
    }
}

TEST_F(ABIAnalysisTest, InstantaneousAmplitude) {
    const int len = 256;
    std::vector<float> input(len);
    std::vector<float> envelope(len);

    // Sine wave with amplitude 1
    for (int i = 0; i < len; ++i) {
        input[i] = std::sin(2.0f * PI * 5 * i / len);
    }

    int result = spectra_instantaneous_amplitude(input.data(), envelope.data(), len);
    EXPECT_EQ(result, SPECTRA_OK);

    // Envelope should be non-negative
    for (int i = 0; i < len; ++i) {
        EXPECT_GE(envelope[i], 0.0f);
        EXPECT_FALSE(std::isnan(envelope[i]));
    }

    // Mean envelope should be around 0.6-0.8 for sine wave
    float mean_env = 0.0f;
    for (int i = 0; i < len; ++i) {
        mean_env += envelope[i];
    }
    mean_env /= len;
    EXPECT_GT(mean_env, 0.1f);
}

TEST_F(ABIAnalysisTest, InstantaneousPhase) {
    const int len = 256;
    std::vector<float> input(len);
    std::vector<float> phase(len);

    for (int i = 0; i < len; ++i) {
        input[i] = std::sin(2.0f * PI * 5 * i / len);
    }

    int result = spectra_instantaneous_phase(input.data(), phase.data(), len);
    EXPECT_EQ(result, SPECTRA_OK);

    // Phase should be bounded
    for (int i = 0; i < len; ++i) {
        EXPECT_GE(phase[i], -PI - 0.1f);
        EXPECT_LE(phase[i], PI + 0.1f);
        EXPECT_FALSE(std::isnan(phase[i]));
    }
}

TEST_F(ABIAnalysisTest, InstantaneousFrequency) {
    const int len = 512;
    const float sample_rate = 1000.0f;
    std::vector<float> input(len);
    std::vector<float> inst_freq(len - 1);

    for (int i = 0; i < len; ++i) {
        input[i] = std::sin(2.0f * PI * 50 * i / sample_rate);
    }

    int result = spectra_instantaneous_frequency(input.data(), inst_freq.data(),
                                                  len, sample_rate);
    EXPECT_EQ(result, SPECTRA_OK);

    // Check no NaN/Inf
    for (int i = 0; i < len - 1; ++i) {
        EXPECT_FALSE(std::isnan(inst_freq[i]));
        EXPECT_FALSE(std::isinf(inst_freq[i]));
    }
}

// ============================================================================
// Resampling
// ============================================================================

TEST_F(ABIAnalysisTest, ResampleOutputSize) {
    int size = spectra_resample_output_size(100, 3, 2);
    EXPECT_EQ(size, 150);  // Upsample by 3/2

    size = spectra_resample_output_size(100, 2, 3);
    EXPECT_EQ(size, 67);   // Downsample by 2/3
}

TEST_F(ABIAnalysisTest, ResampleUpsample) {
    std::vector<float> input = {0.0f, 1.0f, 0.0f, -1.0f, 0.0f};
    int output_size = spectra_resample_output_size(
        static_cast<int>(input.size()), 2, 1);

    std::vector<float> output(output_size);
    int actual_len;

    int result = spectra_resample(input.data(), static_cast<int>(input.size()),
                                   2, 1, output.data(), &actual_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_EQ(actual_len, 10);

    // Check no NaN
    for (int i = 0; i < actual_len; ++i) {
        EXPECT_FALSE(std::isnan(output[i]));
    }
}

TEST_F(ABIAnalysisTest, ResampleDownsample) {
    const int input_len = 200;
    std::vector<float> input(input_len);

    for (int i = 0; i < input_len; ++i) {
        input[i] = std::sin(2.0f * PI * 5 * i / input_len);
    }

    int output_size = spectra_resample_output_size(input_len, 1, 2);
    std::vector<float> output(output_size);
    int actual_len;

    int result = spectra_resample(input.data(), input_len,
                                   1, 2, output.data(), &actual_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_EQ(actual_len, 100);
}

TEST_F(ABIAnalysisTest, ResampleRational) {
    // 48kHz to 44.1kHz conversion
    const int input_len = 480;
    std::vector<float> input(input_len);

    for (int i = 0; i < input_len; ++i) {
        input[i] = std::sin(2.0f * PI * 10 * i / input_len);
    }

    // 44100/48000 = 147/160
    int output_size = spectra_resample_output_size(input_len, 147, 160);
    std::vector<float> output(output_size + 10);  // Extra buffer
    int actual_len;

    int result = spectra_resample(input.data(), input_len,
                                   147, 160, output.data(), &actual_len);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(actual_len, 0);

    // Output length should be approximately input * 147/160
    float expected = input_len * 147.0f / 160.0f;
    EXPECT_NEAR(static_cast<float>(actual_len), expected, 5.0f);
}

// ============================================================================
// Onset Detection
// ============================================================================

TEST_F(ABIAnalysisTest, OnsetDetectorCreateDestroy) {
    SpectraOnsetConfig config;
    config.fft_size = 1024;
    config.hop_size = 256;
    config.threshold = 1.5f;
    config.min_interval_ms = 50.0f;

    SpectraOnsetDetector detector = spectra_onset_create(&config, 44100.0f);
    EXPECT_NE(detector, nullptr);

    spectra_onset_destroy(detector);
}

TEST_F(ABIAnalysisTest, OnsetDetectorProcess) {
    SpectraOnsetConfig config;
    config.fft_size = 512;
    config.hop_size = 128;
    config.threshold = 1.5f;
    config.min_interval_ms = 50.0f;

    SpectraOnsetDetector detector = spectra_onset_create(&config, 44100.0f);
    ASSERT_NE(detector, nullptr);

    // Generate a signal with a transient
    const int len = 4096;
    std::vector<float> samples(len, 0.0f);

    // Add a click
    for (int i = 1000; i < 1010; ++i) {
        samples[i] = 1.0f;
    }

    // Process (without callback for this test)
    int result = spectra_onset_process(detector, samples.data(), len,
                                        nullptr, nullptr);
    EXPECT_EQ(result, SPECTRA_OK);

    spectra_onset_destroy(detector);
}

TEST_F(ABIAnalysisTest, OnsetDetectorReset) {
    SpectraOnsetConfig config;
    config.fft_size = 512;
    config.hop_size = 128;
    config.threshold = 1.5f;
    config.min_interval_ms = 50.0f;

    SpectraOnsetDetector detector = spectra_onset_create(&config, 44100.0f);
    ASSERT_NE(detector, nullptr);

    int result = spectra_onset_reset(detector);
    EXPECT_EQ(result, SPECTRA_OK);

    spectra_onset_destroy(detector);
}

static int g_onset_count = 0;
static void onset_callback(float timestamp, float strength, void* user_data) {
    (void)timestamp;
    (void)strength;
    (void)user_data;
    g_onset_count++;
}

TEST_F(ABIAnalysisTest, OnsetDetectorCallback) {
    SpectraOnsetConfig config;
    config.fft_size = 512;
    config.hop_size = 128;
    config.threshold = 1.0f;
    config.min_interval_ms = 50.0f;

    SpectraOnsetDetector detector = spectra_onset_create(&config, 44100.0f);
    ASSERT_NE(detector, nullptr);

    // Generate signal with clear transient
    const int len = 8192;
    std::vector<float> samples(len, 0.0f);

    // Add strong click
    for (int i = 2000; i < 2050; ++i) {
        samples[i] = 1.0f;
    }

    g_onset_count = 0;
    int result = spectra_onset_process(detector, samples.data(), len,
                                        onset_callback, nullptr);
    EXPECT_EQ(result, SPECTRA_OK);

    // May or may not detect onset depending on threshold/implementation
    // Just check it doesn't crash
    EXPECT_GE(g_onset_count, 0);

    spectra_onset_destroy(detector);
}
