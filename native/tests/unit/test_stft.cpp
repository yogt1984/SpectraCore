/**
 * @file test_stft.cpp
 * @brief Unit tests for STFT (Short-Time Fourier Transform)
 */

#include <gtest/gtest.h>
#include "spectra/fft/stft.hpp"
#include "spectra/core/window_functions.hpp"
#include <cmath>
#include <numeric>

using namespace spectra;

class STFTTest : public ::testing::Test {
protected:
    STFTConfig config;

    void SetUp() override {
        config.fft_size = 256;
        config.hop_size = 64;
        config.window = WindowType::Hann;
    }
};

TEST_F(STFTTest, BasicConstruction) {
    STFT stft(config);
    EXPECT_EQ(stft.spectrum_size(), config.fft_size / 2 + 1);
    EXPECT_EQ(stft.config().fft_size, config.fft_size);
    EXPECT_EQ(stft.config().hop_size, config.hop_size);
}

TEST_F(STFTTest, CalcNumFrames) {
    STFT stft(config);

    // Formula: 1 + (input_size - fft_size) / hop_size
    EXPECT_EQ(stft.calc_num_frames(256), 1u);   // Exactly one frame
    EXPECT_EQ(stft.calc_num_frames(320), 2u);   // 256 + 64 = 320
    EXPECT_EQ(stft.calc_num_frames(384), 3u);   // 256 + 64*2 = 384
    EXPECT_EQ(stft.calc_num_frames(1024), 13u); // 1 + (1024-256)/64 = 13

    // Less than fft_size
    EXPECT_EQ(stft.calc_num_frames(128), 0u);
    EXPECT_EQ(stft.calc_num_frames(0), 0u);
}

TEST_F(STFTTest, AnalyzeDCSignal) {
    STFT stft(config);

    // DC signal
    std::vector<float> signal(1024, 1.0f);
    size_t num_frames;
    size_t expected_frames = stft.calc_num_frames(signal.size());

    std::vector<Complex> output(expected_frames * stft.spectrum_size());
    stft.analyze(signal.data(), signal.size(), output.data(), &num_frames);

    EXPECT_EQ(num_frames, expected_frames);

    // DC component should be dominant in each frame
    for (size_t f = 0; f < num_frames; ++f) {
        const Complex* frame = output.data() + f * stft.spectrum_size();
        float dc_mag = frame[0].magnitude();

        // DC should have significant energy
        EXPECT_GT(dc_mag, 0.0f);

        // Find max bin - should be DC (bin 0) for DC signal
        float max_mag = 0.0f;
        size_t max_bin = 0;
        for (size_t i = 0; i < stft.spectrum_size(); ++i) {
            float mag = frame[i].magnitude();
            if (mag > max_mag) {
                max_mag = mag;
                max_bin = i;
            }
        }
        EXPECT_EQ(max_bin, 0u) << "Frame " << f << ": DC bin should have maximum energy";
    }
}

TEST_F(STFTTest, AnalyzeSineWave) {
    STFT stft(config);

    // Generate sine wave at known frequency
    const size_t target_bin = 8;
    const float freq = static_cast<float>(target_bin) / config.fft_size;

    std::vector<float> signal(1024);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = std::sin(2.0f * constants::pi * freq * i);
    }

    size_t num_frames;
    std::vector<Complex> output(stft.calc_num_frames(signal.size()) * stft.spectrum_size());
    stft.analyze(signal.data(), signal.size(), output.data(), &num_frames);

    // Each frame should have peak at target bin
    for (size_t f = 0; f < num_frames; ++f) {
        const Complex* frame = output.data() + f * stft.spectrum_size();

        float max_mag = 0.0f;
        size_t max_bin = 0;
        for (size_t i = 0; i < stft.spectrum_size(); ++i) {
            float mag = frame[i].magnitude();
            if (mag > max_mag) {
                max_mag = mag;
                max_bin = i;
            }
        }

        EXPECT_EQ(max_bin, target_bin) << "Frame " << f;
    }
}

TEST_F(STFTTest, SynthesizeReconstruction) {
    STFT stft(config);

    // Create test signal
    std::vector<float> signal(1024);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = std::sin(0.1f * i) + 0.5f * std::cos(0.3f * i);
    }

    // Analyze
    size_t num_frames;
    std::vector<Complex> spectrum(stft.calc_num_frames(signal.size()) * stft.spectrum_size());
    stft.analyze(signal.data(), signal.size(), spectrum.data(), &num_frames);

    // Synthesize
    size_t output_size;
    std::vector<float> reconstructed(signal.size() * 2);  // Extra space
    stft.synthesize(spectrum.data(), num_frames, reconstructed.data(), &output_size);

    EXPECT_GT(output_size, 0u);
}

TEST_F(STFTTest, StreamingMatchesBatch) {
    STFT batch_stft(config);
    STFT stream_stft(config);

    // Create test signal
    std::vector<float> signal(1024);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = std::sin(0.05f * i) * std::cos(0.02f * i);
    }

    // Batch processing
    size_t batch_num_frames;
    std::vector<Complex> batch_output(batch_stft.calc_num_frames(signal.size()) * batch_stft.spectrum_size());
    batch_stft.analyze(signal.data(), signal.size(), batch_output.data(), &batch_num_frames);

    // Streaming processing - push all samples
    stream_stft.push_samples(signal.data(), signal.size());

    // Pop frames and compare
    std::vector<Complex> stream_frame(stream_stft.spectrum_size());
    size_t stream_frame_count = 0;

    while (stream_stft.pop_frame(stream_frame.data())) {
        ASSERT_LT(stream_frame_count, batch_num_frames);

        const Complex* batch_frame = batch_output.data() + stream_frame_count * batch_stft.spectrum_size();

        for (size_t i = 0; i < stream_stft.spectrum_size(); ++i) {
            EXPECT_NEAR(stream_frame[i].real, batch_frame[i].real, 1e-5f)
                << "Frame " << stream_frame_count << ", bin " << i;
            EXPECT_NEAR(stream_frame[i].imag, batch_frame[i].imag, 1e-5f)
                << "Frame " << stream_frame_count << ", bin " << i;
        }

        ++stream_frame_count;
    }

    EXPECT_EQ(stream_frame_count, batch_num_frames);
}

TEST_F(STFTTest, StreamingChunked) {
    STFT stft(config);

    // Create test signal
    std::vector<float> signal(2048);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = std::sin(0.1f * i);
    }

    // Push in small chunks
    const size_t chunk_size = 100;
    std::vector<Complex> frames;

    for (size_t offset = 0; offset < signal.size(); offset += chunk_size) {
        size_t count = std::min(chunk_size, signal.size() - offset);
        stft.push_samples(signal.data() + offset, count);

        // Pop any available frames
        Complex frame[129];  // spectrum_size for fft_size=256
        while (stft.pop_frame(frame)) {
            for (size_t i = 0; i < stft.spectrum_size(); ++i) {
                frames.push_back(frame[i]);
            }
        }
    }

    // Should have generated frames
    size_t expected_frames = stft.calc_num_frames(signal.size());
    EXPECT_EQ(frames.size() / stft.spectrum_size(), expected_frames);
}

TEST_F(STFTTest, StreamingReset) {
    STFT stft(config);

    std::vector<float> signal(512);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = static_cast<float>(i);
    }

    stft.push_samples(signal.data(), signal.size());
    EXPECT_GT(stft.frames_available(), 0u);

    stft.reset();
    EXPECT_EQ(stft.frames_available(), 0u);

    // Should be able to push again after reset
    stft.push_samples(signal.data(), signal.size());
    EXPECT_GT(stft.frames_available(), 0u);
}

TEST_F(STFTTest, OverlapAddReconstruction) {
    // Use 75% overlap for good reconstruction
    STFTConfig ola_config;
    ola_config.fft_size = 256;
    ola_config.hop_size = 64;  // 75% overlap
    ola_config.window = WindowType::Hann;

    STFT stft(ola_config);

    // Create test signal (avoid edges for comparison)
    const size_t signal_len = 2048;
    std::vector<float> signal(signal_len);
    for (size_t i = 0; i < signal_len; ++i) {
        signal[i] = std::sin(0.1f * i) + 0.3f * std::cos(0.25f * i);
    }

    // Analyze
    size_t num_frames;
    std::vector<Complex> spectrum(stft.calc_num_frames(signal_len) * stft.spectrum_size());
    stft.analyze(signal.data(), signal_len, spectrum.data(), &num_frames);

    // Synthesize
    size_t output_size;
    std::vector<float> reconstructed(signal_len * 2);
    stft.synthesize(spectrum.data(), num_frames, reconstructed.data(), &output_size);

    // Calculate window sum for normalization (OLA normalization factor)
    std::vector<float> window = generate_window(ola_config.fft_size, ola_config.window);
    std::vector<float> window_sum(output_size, 0.0f);
    for (size_t f = 0; f < num_frames; ++f) {
        size_t start = f * ola_config.hop_size;
        for (size_t i = 0; i < ola_config.fft_size && start + i < output_size; ++i) {
            window_sum[start + i] += window[i] * window[i];
        }
    }

    // Normalize reconstructed signal
    for (size_t i = 0; i < output_size; ++i) {
        if (window_sum[i] > 1e-8f) {
            reconstructed[i] /= window_sum[i];
        }
    }

    // Compare in the "safe" region (avoid edge effects)
    const size_t margin = ola_config.fft_size;
    float max_error = 0.0f;

    for (size_t i = margin; i < std::min(signal_len, output_size) - margin; ++i) {
        float error = std::abs(reconstructed[i] - signal[i]);
        max_error = std::max(max_error, error);
    }

    EXPECT_LT(max_error, 1e-4f) << "Max reconstruction error: " << max_error;
}

TEST_F(STFTTest, STFTMagnitude) {
    STFT stft(config);

    // DC signal
    std::vector<float> signal(512, 1.0f);
    size_t num_frames;
    std::vector<Complex> spectrum(stft.calc_num_frames(signal.size()) * stft.spectrum_size());
    stft.analyze(signal.data(), signal.size(), spectrum.data(), &num_frames);

    std::vector<float> magnitude(num_frames * stft.spectrum_size());
    stft_magnitude(spectrum.data(), magnitude.data(), num_frames, stft.spectrum_size());

    // Verify magnitude computation
    for (size_t i = 0; i < num_frames * stft.spectrum_size(); ++i) {
        float expected = spectrum[i].magnitude();
        EXPECT_FLOAT_EQ(magnitude[i], expected);
    }
}

TEST_F(STFTTest, STFTPower) {
    STFT stft(config);

    std::vector<float> signal(512);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = std::sin(0.1f * i);
    }

    size_t num_frames;
    std::vector<Complex> spectrum(stft.calc_num_frames(signal.size()) * stft.spectrum_size());
    stft.analyze(signal.data(), signal.size(), spectrum.data(), &num_frames);

    std::vector<float> power(num_frames * stft.spectrum_size());
    stft_power(spectrum.data(), power.data(), num_frames, stft.spectrum_size());

    // Verify power = magnitude^2
    for (size_t i = 0; i < num_frames * stft.spectrum_size(); ++i) {
        float expected = spectrum[i].magnitude_squared();
        EXPECT_FLOAT_EQ(power[i], expected);
    }
}

TEST_F(STFTTest, STFTdB) {
    STFT stft(config);

    std::vector<float> signal(512);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = std::sin(0.1f * i);
    }

    size_t num_frames;
    std::vector<Complex> spectrum(stft.calc_num_frames(signal.size()) * stft.spectrum_size());
    stft.analyze(signal.data(), signal.size(), spectrum.data(), &num_frames);

    std::vector<float> db(num_frames * stft.spectrum_size());
    stft_db(spectrum.data(), db.data(), num_frames, stft.spectrum_size(), 1.0f, -120.0f);

    // Verify dB computation
    for (size_t i = 0; i < num_frames * stft.spectrum_size(); ++i) {
        float mag = spectrum[i].magnitude();
        float expected_db;
        if (mag > 0.0f) {
            expected_db = 20.0f * std::log10(mag);
            if (expected_db < -120.0f) expected_db = -120.0f;
        } else {
            expected_db = -120.0f;
        }
        EXPECT_NEAR(db[i], expected_db, 1e-4f);
    }
}

TEST_F(STFTTest, DifferentWindowTypes) {
    std::vector<WindowType> windows = {
        WindowType::Rectangular,
        WindowType::Hann,
        WindowType::Hamming,
        WindowType::Blackman
    };

    std::vector<float> signal(1024);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = std::sin(0.1f * i);
    }

    for (auto win_type : windows) {
        STFTConfig win_config;
        win_config.fft_size = 256;
        win_config.hop_size = 64;
        win_config.window = win_type;

        STFT stft(win_config);

        size_t num_frames;
        std::vector<Complex> spectrum(stft.calc_num_frames(signal.size()) * stft.spectrum_size());

        // Should not crash with any window type
        EXPECT_NO_THROW(stft.analyze(signal.data(), signal.size(), spectrum.data(), &num_frames));
        EXPECT_GT(num_frames, 0u) << "Window type: " << static_cast<int>(win_type);
    }
}

TEST_F(STFTTest, DifferentFFTSizes) {
    std::vector<size_t> fft_sizes = {128, 256, 512, 1024, 2048};

    std::vector<float> signal(4096);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = std::sin(0.05f * i);
    }

    for (size_t fft_size : fft_sizes) {
        STFTConfig size_config;
        size_config.fft_size = fft_size;
        size_config.hop_size = fft_size / 4;

        STFT stft(size_config);

        EXPECT_EQ(stft.spectrum_size(), fft_size / 2 + 1);

        size_t num_frames;
        std::vector<Complex> spectrum(stft.calc_num_frames(signal.size()) * stft.spectrum_size());
        stft.analyze(signal.data(), signal.size(), spectrum.data(), &num_frames);

        EXPECT_EQ(num_frames, stft.calc_num_frames(signal.size()));
    }
}

TEST_F(STFTTest, EmptyInput) {
    STFT stft(config);

    size_t num_frames;
    std::vector<Complex> output(1);  // Minimal buffer

    // Empty input
    stft.analyze(nullptr, 0, output.data(), &num_frames);
    EXPECT_EQ(num_frames, 0u);
}

TEST_F(STFTTest, ShortInput) {
    STFT stft(config);

    // Input shorter than FFT size
    std::vector<float> short_signal(100, 1.0f);
    size_t num_frames;
    std::vector<Complex> output(stft.spectrum_size());

    stft.analyze(short_signal.data(), short_signal.size(), output.data(), &num_frames);
    EXPECT_EQ(num_frames, 0u);
}
