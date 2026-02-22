/**
 * @file test_psd.cpp
 * @brief Unit tests for Power Spectral Density (pwelch) functions
 */

#include <gtest/gtest.h>
#include "spectra/analysis/psd.hpp"
#include <cmath>
#include <numeric>

using namespace spectra;

class PSDTest : public ::testing::Test {
protected:
    // Generate white noise
    std::vector<float> white_noise(size_t len, float amplitude = 1.0f) {
        std::vector<float> signal(len);
        uint32_t seed = 12345;
        for (size_t i = 0; i < len; ++i) {
            seed = seed * 1103515245 + 12345;
            signal[i] = amplitude * (static_cast<float>(seed % 10000) / 5000.0f - 1.0f);
        }
        return signal;
    }

    // Generate sine wave
    std::vector<float> sine_wave(size_t len, float freq, float sample_rate, float amplitude = 1.0f) {
        std::vector<float> signal(len);
        for (size_t i = 0; i < len; ++i) {
            signal[i] = amplitude * std::sin(constants::two_pi * freq * i / sample_rate);
        }
        return signal;
    }
};

// ============================================================================
// Basic Tests
// ============================================================================

TEST_F(PSDTest, BasicPwelch) {
    auto signal = white_noise(1024);

    WelchConfig config;
    config.segment_size = 256;
    config.overlap = 128;
    config.window = WindowType::Hann;

    size_t output_len;
    std::vector<float> psd(config.segment_size / 2 + 1);
    std::vector<float> freq(config.segment_size / 2 + 1);

    pwelch(signal.data(), signal.size(), config, 44100.0f,
           psd.data(), freq.data(), &output_len);

    EXPECT_EQ(output_len, config.segment_size / 2 + 1);

    // Check no NaN/Inf
    for (size_t i = 0; i < output_len; ++i) {
        EXPECT_FALSE(std::isnan(psd[i]));
        EXPECT_FALSE(std::isinf(psd[i]));
        EXPECT_GE(psd[i], 0.0f);  // PSD must be non-negative
    }
}

TEST_F(PSDTest, FrequencyVector) {
    WelchConfig config;
    config.segment_size = 256;
    config.overlap = 128;

    size_t output_len;
    std::vector<float> psd(129);
    std::vector<float> freq(129);

    auto signal = white_noise(1024);
    pwelch(signal.data(), 1024, config, 44100.0f,
           psd.data(), freq.data(), &output_len);

    // First frequency should be DC (0 Hz)
    EXPECT_FLOAT_EQ(freq[0], 0.0f);

    // Last frequency should be Nyquist (sample_rate / 2)
    EXPECT_NEAR(freq[output_len - 1], 44100.0f / 2.0f, 100.0f);

    // Frequencies should be monotonically increasing
    for (size_t i = 1; i < output_len; ++i) {
        EXPECT_GT(freq[i], freq[i - 1]);
    }
}

// ============================================================================
// Sine Wave Detection
// ============================================================================

TEST_F(PSDTest, SineWavePeak) {
    const float sample_rate = 44100.0f;
    const float tone_freq = 1000.0f;  // 1 kHz

    auto signal = sine_wave(8192, tone_freq, sample_rate);

    WelchConfig config;
    config.segment_size = 1024;
    config.overlap = 512;
    config.window = WindowType::Hann;

    size_t output_len;
    std::vector<float> psd(513);
    std::vector<float> freq(513);

    pwelch(signal.data(), signal.size(), config, sample_rate,
           psd.data(), freq.data(), &output_len);

    // Find peak
    size_t peak_idx = 0;
    float peak_power = psd[0];
    for (size_t i = 1; i < output_len; ++i) {
        if (psd[i] > peak_power) {
            peak_power = psd[i];
            peak_idx = i;
        }
    }

    // Peak should be near tone frequency
    float peak_freq = freq[peak_idx];
    EXPECT_NEAR(peak_freq, tone_freq, sample_rate / config.segment_size);
}

TEST_F(PSDTest, MultipleSines) {
    const float sample_rate = 44100.0f;

    // Two tones at 500 Hz and 2000 Hz
    std::vector<float> signal(8192);
    for (size_t i = 0; i < signal.size(); ++i) {
        float t = static_cast<float>(i) / sample_rate;
        signal[i] = std::sin(constants::two_pi * 500.0f * t) +
                    0.5f * std::sin(constants::two_pi * 2000.0f * t);
    }

    WelchConfig config;
    config.segment_size = 1024;
    config.overlap = 512;
    config.window = WindowType::Hann;

    size_t output_len;
    std::vector<float> psd(513);
    std::vector<float> freq(513);

    pwelch(signal.data(), signal.size(), config, sample_rate,
           psd.data(), freq.data(), &output_len);

    // Find local maxima
    std::vector<size_t> peaks;
    for (size_t i = 1; i < output_len - 1; ++i) {
        if (psd[i] > psd[i-1] && psd[i] > psd[i+1] && psd[i] > 0.01f * psd[0]) {
            peaks.push_back(i);
        }
    }

    // Should have at least 2 peaks
    EXPECT_GE(peaks.size(), 2u);
}

// ============================================================================
// White Noise Tests
// ============================================================================

TEST_F(PSDTest, WhiteNoiseFlat) {
    auto signal = white_noise(16384, 1.0f);

    WelchConfig config;
    config.segment_size = 1024;
    config.overlap = 512;
    config.window = WindowType::Hann;

    size_t output_len;
    std::vector<float> psd(513);
    std::vector<float> freq(513);

    pwelch(signal.data(), signal.size(), config, 44100.0f,
           psd.data(), freq.data(), &output_len);

    // For white noise, PSD should be relatively flat
    // Compute mean and check variance is limited
    float mean_psd = 0.0f;
    for (size_t i = 10; i < output_len - 10; ++i) {  // Exclude edges
        mean_psd += psd[i];
    }
    mean_psd /= (output_len - 20);

    // Check that values don't deviate too much from mean
    float max_deviation = 0.0f;
    for (size_t i = 10; i < output_len - 10; ++i) {
        max_deviation = std::max(max_deviation, std::abs(psd[i] - mean_psd) / mean_psd);
    }

    // White noise PSD should be within factor of 10 of mean
    EXPECT_LT(max_deviation, 10.0f);
}

// ============================================================================
// Detrending
// ============================================================================

TEST_F(PSDTest, DetrendRemovesDC) {
    // Signal with DC offset
    std::vector<float> signal(4096);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = 5.0f + std::sin(constants::two_pi * 100.0f * i / 44100.0f);
    }

    WelchConfig config;
    config.segment_size = 1024;
    config.overlap = 512;
    config.detrend = true;

    size_t output_len;
    std::vector<float> psd(513);
    std::vector<float> freq(513);

    pwelch(signal.data(), signal.size(), config, 44100.0f,
           psd.data(), freq.data(), &output_len);

    // DC component should be minimal compared to AC peak
    float dc_power = psd[0];
    float max_ac = 0.0f;
    for (size_t i = 1; i < output_len; ++i) {
        max_ac = std::max(max_ac, psd[i]);
    }

    EXPECT_LT(dc_power, max_ac);
}

// ============================================================================
// Periodogram
// ============================================================================

TEST_F(PSDTest, Periodogram) {
    auto signal = sine_wave(1024, 1000.0f, 44100.0f);

    std::vector<float> psd(513);
    std::vector<float> freq(513);

    periodogram(signal.data(), signal.size(), WindowType::Hann, 44100.0f,
                psd.data(), freq.data());

    // Find peak
    size_t peak_idx = 0;
    float peak_power = psd[0];
    for (size_t i = 1; i < 513; ++i) {
        if (psd[i] > peak_power) {
            peak_power = psd[i];
            peak_idx = i;
        }
    }

    // Peak should be near 1000 Hz
    float peak_freq = freq[peak_idx];
    EXPECT_NEAR(peak_freq, 1000.0f, 100.0f);
}

// ============================================================================
// Spectrogram
// ============================================================================

TEST_F(PSDTest, Spectrogram) {
    // Chirp signal (frequency sweep)
    const float sample_rate = 44100.0f;
    const size_t len = 8192;

    std::vector<float> signal(len);
    for (size_t i = 0; i < len; ++i) {
        float t = static_cast<float>(i) / sample_rate;
        float freq = 100.0f + 4000.0f * t;  // Linear chirp
        signal[i] = std::sin(constants::two_pi * freq * t / 2.0f);
    }

    WelchConfig config;
    config.segment_size = 256;
    config.overlap = 192;
    config.window = WindowType::Hann;

    size_t num_frames = spectrogram_num_frames(len, config.segment_size, config.overlap);
    size_t num_freqs = config.segment_size / 2 + 1;

    std::vector<float> spec(num_frames * num_freqs);
    std::vector<float> freq(num_freqs);
    std::vector<float> times(num_frames);
    size_t actual_frames, actual_freqs;

    spectrogram(signal.data(), len, config, sample_rate,
                spec.data(), freq.data(), times.data(),
                &actual_frames, &actual_freqs);

    EXPECT_EQ(actual_frames, num_frames);
    EXPECT_EQ(actual_freqs, num_freqs);

    // Check time vector
    EXPECT_GE(times[0], 0.0f);
    for (size_t i = 1; i < actual_frames; ++i) {
        EXPECT_GT(times[i], times[i - 1]);  // Monotonically increasing
    }
}

// ============================================================================
// Output Size
// ============================================================================

TEST_F(PSDTest, OutputSize) {
    WelchConfig config;
    config.segment_size = 512;
    config.onesided = true;

    EXPECT_EQ(pwelch_output_size(config), 257u);

    config.onesided = false;
    EXPECT_EQ(pwelch_output_size(config), 512u);
}

// ============================================================================
// Different Window Types
// ============================================================================

TEST_F(PSDTest, DifferentWindows) {
    auto signal = white_noise(4096);

    std::vector<WindowType> windows = {
        WindowType::Rectangular,
        WindowType::Hann,
        WindowType::Hamming,
        WindowType::Blackman
    };

    for (WindowType win : windows) {
        WelchConfig config;
        config.segment_size = 256;
        config.overlap = 128;
        config.window = win;

        size_t output_len;
        std::vector<float> psd(129);
        std::vector<float> freq(129);

        pwelch(signal.data(), signal.size(), config, 44100.0f,
               psd.data(), freq.data(), &output_len);

        EXPECT_EQ(output_len, 129u);

        for (size_t i = 0; i < output_len; ++i) {
            EXPECT_FALSE(std::isnan(psd[i]));
            EXPECT_GE(psd[i], 0.0f);
        }
    }
}

// ============================================================================
// Parseval's Theorem
// ============================================================================

TEST_F(PSDTest, ParsevalsTheorem) {
    // Total power in time domain should equal integral of PSD
    auto signal = sine_wave(4096, 500.0f, 44100.0f, 1.0f);

    // Time domain power
    float time_power = 0.0f;
    for (float s : signal) {
        time_power += s * s;
    }
    time_power /= signal.size();

    // PSD power (approximate integral)
    WelchConfig config;
    config.segment_size = 256;
    config.overlap = 128;

    size_t output_len;
    std::vector<float> psd(129);
    std::vector<float> freq(129);

    pwelch(signal.data(), signal.size(), config, 44100.0f,
           psd.data(), freq.data(), &output_len);

    float psd_power = 0.0f;
    float df = freq[1] - freq[0];  // Frequency resolution
    for (size_t i = 0; i < output_len; ++i) {
        psd_power += psd[i] * df;
    }

    // Powers should be in same order of magnitude
    // (exact equality requires careful normalization)
    EXPECT_GT(psd_power, time_power * 0.1f);
    EXPECT_LT(psd_power, time_power * 10.0f);
}
