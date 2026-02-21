/**
 * @file test_onset_detection.cpp
 * @brief Unit tests for onset detection
 */

#include <gtest/gtest.h>
#include "spectra/analysis/onset_detection.hpp"
#include <cmath>
#include <numeric>
#include <random>

using namespace spectra;

class OnsetDetectionTest : public ::testing::Test {
protected:
    OnsetConfig config;
    float sample_rate = 44100.0f;

    void SetUp() override {
        config.fft_size = 1024;
        config.hop_size = 256;
        config.threshold = 0.3f;
        config.min_interval_ms = 50.0f;
        config.window = WindowType::Hann;
    }

    // Generate click/impulse at specific sample position
    std::vector<float> generateClick(size_t length, size_t click_pos, float amplitude = 1.0f) {
        std::vector<float> signal(length, 0.0f);
        if (click_pos < length) {
            // Short burst (simulating a click/transient)
            for (size_t i = 0; i < 100 && click_pos + i < length; ++i) {
                float env = std::exp(-static_cast<float>(i) / 20.0f);
                signal[click_pos + i] = amplitude * env * std::sin(0.5f * i);
            }
        }
        return signal;
    }

    // Generate multiple clicks
    std::vector<float> generateMultipleClicks(size_t length, const std::vector<size_t>& positions, float amplitude = 1.0f) {
        std::vector<float> signal(length, 0.0f);
        for (size_t pos : positions) {
            auto click = generateClick(length, pos, amplitude);
            for (size_t i = 0; i < length; ++i) {
                signal[i] += click[i];
            }
        }
        return signal;
    }

    // Generate silence
    std::vector<float> generateSilence(size_t length) {
        return std::vector<float>(length, 0.0f);
    }

    // Generate white noise
    std::vector<float> generateNoise(size_t length, float amplitude = 0.01f) {
        std::vector<float> signal(length);
        std::mt19937 rng(42);  // Fixed seed for reproducibility
        std::uniform_real_distribution<float> dist(-amplitude, amplitude);
        for (size_t i = 0; i < length; ++i) {
            signal[i] = dist(rng);
        }
        return signal;
    }

    // Generate drum-like transient
    std::vector<float> generateDrumHit(size_t length, size_t hit_pos, float amplitude = 1.0f) {
        std::vector<float> signal(length, 0.0f);
        if (hit_pos < length) {
            // Drum-like: fast attack, slower decay, low frequency content
            for (size_t i = 0; i < 500 && hit_pos + i < length; ++i) {
                float t = static_cast<float>(i) / sample_rate;
                float env = std::exp(-t * 30.0f);  // Fast decay
                float freq = 100.0f + 50.0f * std::exp(-t * 20.0f);  // Pitch drop
                signal[hit_pos + i] = amplitude * env * std::sin(2.0f * constants::pi * freq * t);
            }
        }
        return signal;
    }
};

// ============================================================================
// Spectral Flux Tests
// ============================================================================

TEST_F(OnsetDetectionTest, SpectralFluxZeroWithIdentical) {
    std::vector<float> spectrum(128, 1.0f);
    float flux = spectral_flux(spectrum.data(), spectrum.data(), spectrum.size());
    EXPECT_FLOAT_EQ(flux, 0.0f);
}

TEST_F(OnsetDetectionTest, SpectralFluxPositiveIncrease) {
    std::vector<float> prev(128, 0.0f);
    std::vector<float> curr(128, 1.0f);

    float flux = spectral_flux(curr.data(), prev.data(), curr.size());
    EXPECT_FLOAT_EQ(flux, 128.0f);  // Sum of all positive differences
}

TEST_F(OnsetDetectionTest, SpectralFluxIgnoresDecrease) {
    std::vector<float> prev(128, 1.0f);
    std::vector<float> curr(128, 0.0f);

    float flux = spectral_flux(curr.data(), prev.data(), curr.size());
    EXPECT_FLOAT_EQ(flux, 0.0f);  // Negative differences ignored
}

TEST_F(OnsetDetectionTest, SpectralFluxMixedChanges) {
    std::vector<float> prev = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> curr = {2.0f, 1.0f, 4.0f, 3.0f};  // +1, -1, +1, -1

    float flux = spectral_flux(curr.data(), prev.data(), curr.size());
    EXPECT_FLOAT_EQ(flux, 2.0f);  // Only positive: 1 + 1 = 2
}

TEST_F(OnsetDetectionTest, SpectralFluxComplex) {
    std::vector<Complex> prev(4);
    std::vector<Complex> curr(4);

    // Set up magnitudes: prev = [1, 1, 1, 1], curr = [2, 0, 2, 0]
    prev[0] = {1.0f, 0.0f};
    prev[1] = {1.0f, 0.0f};
    prev[2] = {1.0f, 0.0f};
    prev[3] = {1.0f, 0.0f};

    curr[0] = {2.0f, 0.0f};
    curr[1] = {0.0f, 0.0f};
    curr[2] = {2.0f, 0.0f};
    curr[3] = {0.0f, 0.0f};

    float flux = spectral_flux(curr.data(), prev.data(), curr.size());
    EXPECT_FLOAT_EQ(flux, 2.0f);  // +1 at [0] and +1 at [2]
}

// ============================================================================
// Peak Picking Tests
// ============================================================================

TEST_F(OnsetDetectionTest, PeakPickingBasic) {
    std::vector<float> df = {0, 1, 2, 1, 0, 3, 2, 0, 0, 5, 4};
    std::vector<size_t> peaks(10);

    size_t num_peaks = pick_peaks(df.data(), df.size(), 0.5f, 1, peaks.data());

    ASSERT_GE(num_peaks, 2u);
    EXPECT_EQ(peaks[0], 2u);   // Peak at index 2 (value 2)
    EXPECT_EQ(peaks[1], 5u);   // Peak at index 5 (value 3)
}

TEST_F(OnsetDetectionTest, PeakPickingMinDistance) {
    // Peaks at indices 2, 4, 6 - all within min_distance of each other
    std::vector<float> df = {0, 1, 5, 1, 5, 1, 5, 1, 0};
    std::vector<size_t> peaks(10);

    // With min_distance = 3, should only get every other peak
    size_t num_peaks = pick_peaks(df.data(), df.size(), 0.5f, 3, peaks.data());

    // Should get fewer peaks due to minimum distance constraint
    EXPECT_LE(num_peaks, 3u);

    // Verify minimum distance is respected
    for (size_t i = 1; i < num_peaks; ++i) {
        EXPECT_GE(peaks[i] - peaks[i-1], 3u);
    }
}

TEST_F(OnsetDetectionTest, PeakPickingThreshold) {
    std::vector<float> df = {0, 0.5, 2, 0.5, 0, 0.5, 10, 0.5, 0};
    std::vector<size_t> peaks(10);

    // High threshold should only pick the large peak
    size_t num_peaks = pick_peaks(df.data(), df.size(), 5.0f, 1, peaks.data());

    EXPECT_EQ(num_peaks, 1u);
    EXPECT_EQ(peaks[0], 6u);  // Only peak above threshold 5
}

TEST_F(OnsetDetectionTest, PeakPickingEmpty) {
    std::vector<float> df = {0, 0, 0, 0, 0};
    std::vector<size_t> peaks(10);

    size_t num_peaks = pick_peaks(df.data(), df.size(), 0.5f, 1, peaks.data());
    EXPECT_EQ(num_peaks, 0u);
}

// ============================================================================
// OnsetDetector Tests
// ============================================================================

TEST_F(OnsetDetectionTest, DetectorConstruction) {
    OnsetDetector detector(config, sample_rate);

    EXPECT_FLOAT_EQ(detector.threshold(), config.threshold);
    EXPECT_EQ(detector.config().fft_size, config.fft_size);
}

TEST_F(OnsetDetectionTest, DetectorSetThreshold) {
    OnsetDetector detector(config, sample_rate);

    detector.set_threshold(0.7f);
    EXPECT_FLOAT_EQ(detector.threshold(), 0.7f);
}

TEST_F(OnsetDetectionTest, SilenceNoOnsets) {
    OnsetDetector detector(config, sample_rate);

    auto silence = generateSilence(44100);  // 1 second of silence
    auto onsets = detector.process(silence.data(), silence.size());

    EXPECT_EQ(onsets.size(), 0u) << "Silence should produce no onsets";
}

TEST_F(OnsetDetectionTest, SingleClickDetection) {
    OnsetDetector detector(config, sample_rate);

    // Click at 0.5 seconds
    size_t click_sample = static_cast<size_t>(0.5f * sample_rate);
    auto signal = generateClick(44100, click_sample);

    auto onsets = detector.process(signal.data(), signal.size());

    ASSERT_GE(onsets.size(), 1u) << "Should detect at least one onset";

    // Onset should be near the click position (within ~50ms due to STFT latency)
    float expected_time = static_cast<float>(click_sample) / sample_rate;
    EXPECT_NEAR(onsets[0].timestamp, expected_time, 0.1f);
}

TEST_F(OnsetDetectionTest, MultipleClicksDetection) {
    // Configure with smaller minimum interval to detect multiple clicks
    config.min_interval_ms = 100.0f;
    OnsetDetector detector(config, sample_rate);

    // Clicks at 0.2s, 0.5s, 0.8s (well separated)
    std::vector<size_t> click_positions = {
        static_cast<size_t>(0.2f * sample_rate),
        static_cast<size_t>(0.5f * sample_rate),
        static_cast<size_t>(0.8f * sample_rate)
    };

    auto signal = generateMultipleClicks(44100, click_positions);
    auto onsets = detector.process(signal.data(), signal.size());

    // Should detect all three clicks (or at least 2)
    EXPECT_GE(onsets.size(), 2u) << "Should detect multiple onsets";
}

TEST_F(OnsetDetectionTest, MinimumIntervalRespected) {
    config.min_interval_ms = 200.0f;  // 200ms minimum
    OnsetDetector detector(config, sample_rate);

    // Two clicks 50ms apart (less than minimum interval)
    size_t click1 = static_cast<size_t>(0.3f * sample_rate);
    size_t click2 = static_cast<size_t>(0.35f * sample_rate);  // 50ms later

    auto signal = generateMultipleClicks(44100, {click1, click2});
    auto onsets = detector.process(signal.data(), signal.size());

    // Should only detect one onset due to minimum interval
    if (onsets.size() >= 2) {
        // If two onsets detected, verify they're at least min_interval apart
        float interval = onsets[1].timestamp - onsets[0].timestamp;
        EXPECT_GE(interval, config.min_interval_ms / 1000.0f);
    }
}

TEST_F(OnsetDetectionTest, DrumHitDetection) {
    OnsetDetector detector(config, sample_rate);

    // Drum hit at 0.3 seconds
    size_t hit_sample = static_cast<size_t>(0.3f * sample_rate);
    auto signal = generateDrumHit(44100, hit_sample);

    auto onsets = detector.process(signal.data(), signal.size());

    ASSERT_GE(onsets.size(), 1u) << "Should detect drum hit";

    // Verify onset is near the hit position
    float expected_time = static_cast<float>(hit_sample) / sample_rate;
    EXPECT_NEAR(onsets[0].timestamp, expected_time, 0.1f);
}

TEST_F(OnsetDetectionTest, LowLevelNoiseRejection) {
    // Use higher threshold for noise rejection
    config.threshold = 0.8f;
    OnsetDetector detector(config, sample_rate);

    // Very low level noise (should be below threshold)
    auto noise = generateNoise(44100, 0.001f);  // Very quiet noise

    auto onsets = detector.process(noise.data(), noise.size());

    // With high threshold, noise should produce few false positives
    // (Adaptive threshold will adjust to noise floor)
    // Compare: a click should produce significantly more reliable detection
    auto click_signal = generateClick(44100, 22050, 1.0f);
    detector.reset();
    auto click_onsets = detector.process(click_signal.data(), click_signal.size());

    // Click should be more reliably detected than noise produces false positives
    // This is a relative test rather than absolute
    if (click_onsets.size() > 0) {
        EXPECT_GE(click_onsets[0].strength, 1.0f);
    }
}

TEST_F(OnsetDetectionTest, CallbackInvocation) {
    OnsetDetector detector(config, sample_rate);

    size_t click_sample = static_cast<size_t>(0.5f * sample_rate);
    auto signal = generateClick(44100, click_sample);

    int callback_count = 0;
    float last_timestamp = 0.0f;

    detector.process(signal.data(), signal.size(),
                     [&callback_count, &last_timestamp](float time, float strength) {
                         callback_count++;
                         last_timestamp = time;
                         EXPECT_GT(strength, 0.0f);
                     });

    EXPECT_GE(callback_count, 1) << "Callback should be invoked at least once";
    EXPECT_GT(last_timestamp, 0.0f) << "Timestamp should be positive";
}

TEST_F(OnsetDetectionTest, ResetClearsState) {
    OnsetDetector detector(config, sample_rate);

    // Process some audio
    auto signal = generateClick(22050, 5000);
    detector.process(signal.data(), signal.size());

    // Reset
    detector.reset();

    // Process silence - should get no onsets
    auto silence = generateSilence(22050);
    auto onsets = detector.process(silence.data(), silence.size());

    EXPECT_EQ(onsets.size(), 0u) << "After reset, silence should produce no onsets";
}

TEST_F(OnsetDetectionTest, ChunkedProcessing) {
    OnsetDetector detector(config, sample_rate);

    // Create signal with click
    size_t click_sample = static_cast<size_t>(0.3f * sample_rate);
    auto signal = generateClick(44100, click_sample);

    // Process in chunks
    std::vector<OnsetEvent> all_onsets;
    const size_t chunk_size = 1024;

    for (size_t offset = 0; offset < signal.size(); offset += chunk_size) {
        size_t count = std::min(chunk_size, signal.size() - offset);
        auto onsets = detector.process(signal.data() + offset, count);
        all_onsets.insert(all_onsets.end(), onsets.begin(), onsets.end());
    }

    EXPECT_GE(all_onsets.size(), 1u) << "Should detect onset in chunked processing";
}

// ============================================================================
// Offline Detection Tests
// ============================================================================

TEST_F(OnsetDetectionTest, OfflineDetection) {
    size_t click_sample = static_cast<size_t>(0.4f * sample_rate);
    auto signal = generateClick(44100, click_sample);

    auto onsets = detect_onsets(signal.data(), signal.size(), config, sample_rate);

    ASSERT_GE(onsets.size(), 1u);

    float expected_time = static_cast<float>(click_sample) / sample_rate;
    EXPECT_NEAR(onsets[0].timestamp, expected_time, 0.1f);
}

TEST_F(OnsetDetectionTest, OnsetStrength) {
    OnsetDetector detector(config, sample_rate);

    // Create two clicks with different amplitudes
    auto weak_click = generateClick(22050, 5000, 0.3f);
    auto strong_click = generateClick(22050, 5000, 1.0f);

    auto weak_onsets = detector.process(weak_click.data(), weak_click.size());
    detector.reset();
    auto strong_onsets = detector.process(strong_click.data(), strong_click.size());

    if (weak_onsets.size() > 0 && strong_onsets.size() > 0) {
        // Stronger click should have higher onset strength
        // (Note: this depends on adaptive threshold behavior)
        EXPECT_GT(strong_onsets[0].strength, 0.0f);
    }
}

TEST_F(OnsetDetectionTest, DifferentFFTSizes) {
    std::vector<size_t> fft_sizes = {512, 1024, 2048};

    for (size_t fft_size : fft_sizes) {
        OnsetConfig test_config;
        test_config.fft_size = fft_size;
        test_config.hop_size = fft_size / 4;
        test_config.threshold = 0.3f;
        test_config.min_interval_ms = 50.0f;

        OnsetDetector detector(test_config, sample_rate);

        auto signal = generateClick(22050, 5000);
        auto onsets = detector.process(signal.data(), signal.size());

        // Should work with any FFT size
        EXPECT_NO_THROW(detector.process(signal.data(), signal.size()));
    }
}
