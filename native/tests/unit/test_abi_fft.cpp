/**
 * @file test_abi_fft.cpp
 * @brief Unit tests for FFT C ABI functions
 */

#include <gtest/gtest.h>
#include "spectra_abi.h"
#include <cmath>
#include <vector>

class ABIFFTTest : public ::testing::Test {
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
// Creation and Destruction
// ============================================================================

TEST_F(ABIFFTTest, CreateDestroy) {
    SpectraFFT fft = spectra_fft_create(256);
    EXPECT_NE(fft, nullptr);
    spectra_fft_destroy(fft);
}

TEST_F(ABIFFTTest, CreateVariousSizes) {
    std::vector<int> sizes = {64, 128, 256, 512, 1024, 2048};

    for (int size : sizes) {
        SpectraFFT fft = spectra_fft_create(size);
        EXPECT_NE(fft, nullptr) << "Failed to create FFT of size " << size;
        EXPECT_EQ(spectra_fft_size(fft), size);
        spectra_fft_destroy(fft);
    }
}

TEST_F(ABIFFTTest, SpectrumSize) {
    SpectraFFT fft = spectra_fft_create(256);
    EXPECT_NE(fft, nullptr);

    // For real FFT, spectrum size is N/2 + 1
    int spectrum_size = spectra_fft_spectrum_size(fft);
    EXPECT_EQ(spectrum_size, 129);

    spectra_fft_destroy(fft);
}

TEST_F(ABIFFTTest, DestroyNull) {
    // Should not crash
    spectra_fft_destroy(nullptr);
}

// ============================================================================
// Forward Transform
// ============================================================================

TEST_F(ABIFFTTest, ForwardBasic) {
    const int size = 256;
    SpectraFFT fft = spectra_fft_create(size);
    ASSERT_NE(fft, nullptr);

    std::vector<float> input(size, 0.0f);
    std::vector<float> output_real(size);
    std::vector<float> output_imag(size);

    // DC signal
    for (int i = 0; i < size; ++i) {
        input[i] = 1.0f;
    }

    int result = spectra_fft_forward(fft, input.data(),
                                      output_real.data(), output_imag.data());
    EXPECT_EQ(result, SPECTRA_OK);

    // DC component should be large
    EXPECT_GT(output_real[0], 0.0f);

    // Other components should be near zero
    for (int i = 1; i < size / 2; ++i) {
        EXPECT_NEAR(output_real[i], 0.0f, 0.01f);
        EXPECT_NEAR(output_imag[i], 0.0f, 0.01f);
    }

    spectra_fft_destroy(fft);
}

TEST_F(ABIFFTTest, ForwardSineWave) {
    const int size = 256;
    const int bin = 10;  // Frequency bin
    SpectraFFT fft = spectra_fft_create(size);
    ASSERT_NE(fft, nullptr);

    std::vector<float> input(size);
    std::vector<float> output_real(size);
    std::vector<float> output_imag(size);

    // Generate sine wave at specific frequency bin
    for (int i = 0; i < size; ++i) {
        input[i] = std::sin(2.0f * PI * bin * i / size);
    }

    int result = spectra_fft_forward(fft, input.data(),
                                      output_real.data(), output_imag.data());
    EXPECT_EQ(result, SPECTRA_OK);

    // Find peak
    int peak_bin = 0;
    float peak_mag = 0.0f;
    for (int i = 0; i < size / 2; ++i) {
        float mag = std::sqrt(output_real[i] * output_real[i] +
                              output_imag[i] * output_imag[i]);
        if (mag > peak_mag) {
            peak_mag = mag;
            peak_bin = i;
        }
    }

    EXPECT_EQ(peak_bin, bin);

    spectra_fft_destroy(fft);
}

TEST_F(ABIFFTTest, ForwardNoNaN) {
    const int size = 512;
    SpectraFFT fft = spectra_fft_create(size);
    ASSERT_NE(fft, nullptr);

    std::vector<float> input(size);
    std::vector<float> output_real(size);
    std::vector<float> output_imag(size);

    // Random-ish input
    for (int i = 0; i < size; ++i) {
        input[i] = std::sin(i * 0.1f) + std::cos(i * 0.3f);
    }

    spectra_fft_forward(fft, input.data(), output_real.data(), output_imag.data());

    for (int i = 0; i < size; ++i) {
        EXPECT_FALSE(std::isnan(output_real[i]));
        EXPECT_FALSE(std::isnan(output_imag[i]));
        EXPECT_FALSE(std::isinf(output_real[i]));
        EXPECT_FALSE(std::isinf(output_imag[i]));
    }

    spectra_fft_destroy(fft);
}

// ============================================================================
// Inverse Transform
// ============================================================================

TEST_F(ABIFFTTest, InverseBasic) {
    const int size = 256;
    SpectraFFT fft = spectra_fft_create(size);
    ASSERT_NE(fft, nullptr);

    std::vector<float> input_real(size, 0.0f);
    std::vector<float> input_imag(size, 0.0f);
    std::vector<float> output(size);

    // DC only
    input_real[0] = static_cast<float>(size);

    int result = spectra_fft_inverse(fft, input_real.data(),
                                      input_imag.data(), output.data());
    EXPECT_EQ(result, SPECTRA_OK);

    // Output should be constant
    for (int i = 0; i < size; ++i) {
        EXPECT_NEAR(output[i], 1.0f, 0.1f);
    }

    spectra_fft_destroy(fft);
}

TEST_F(ABIFFTTest, RoundTrip) {
    const int size = 256;
    SpectraFFT fft = spectra_fft_create(size);
    ASSERT_NE(fft, nullptr);

    std::vector<float> original(size);
    std::vector<float> freq_real(size);
    std::vector<float> freq_imag(size);
    std::vector<float> recovered(size);

    // Generate test signal
    for (int i = 0; i < size; ++i) {
        original[i] = std::sin(2.0f * PI * 5 * i / size) +
                      0.5f * std::cos(2.0f * PI * 20 * i / size);
    }

    // Forward
    spectra_fft_forward(fft, original.data(), freq_real.data(), freq_imag.data());

    // Inverse
    spectra_fft_inverse(fft, freq_real.data(), freq_imag.data(), recovered.data());

    // Should match original
    float max_error = 0.0f;
    for (int i = 0; i < size; ++i) {
        float error = std::abs(recovered[i] - original[i]);
        max_error = std::max(max_error, error);
    }

    EXPECT_LT(max_error, 1e-4f);

    spectra_fft_destroy(fft);
}

// ============================================================================
// Error Handling
// ============================================================================

TEST_F(ABIFFTTest, InvalidHandle) {
    std::vector<float> buffer(256);

    int result = spectra_fft_forward(nullptr, buffer.data(),
                                      buffer.data(), buffer.data());
    EXPECT_NE(result, SPECTRA_OK);

    result = spectra_fft_inverse(nullptr, buffer.data(),
                                  buffer.data(), buffer.data());
    EXPECT_NE(result, SPECTRA_OK);
}

TEST_F(ABIFFTTest, SizeFromHandle) {
    SpectraFFT fft = spectra_fft_create(512);
    ASSERT_NE(fft, nullptr);

    EXPECT_EQ(spectra_fft_size(fft), 512);

    spectra_fft_destroy(fft);
}

// ============================================================================
// Window Functions via ABI
// ============================================================================

TEST_F(ABIFFTTest, GenerateWindow) {
    const int size = 256;
    std::vector<float> window(size);

    int result = spectra_generate_window(window.data(), size, SPECTRA_WINDOW_HANN);
    EXPECT_EQ(result, SPECTRA_OK);

    // Check window properties
    EXPECT_NEAR(window[0], 0.0f, 0.01f);  // Edge is zero
    EXPECT_NEAR(window[size / 2], 1.0f, 0.01f);  // Center is ~1
}

TEST_F(ABIFFTTest, ApplyWindow) {
    const int size = 256;
    std::vector<float> data(size, 1.0f);

    int result = spectra_apply_window(data.data(), size, SPECTRA_WINDOW_HAMMING);
    EXPECT_EQ(result, SPECTRA_OK);

    // After windowing, edges should be attenuated
    EXPECT_LT(data[0], 0.2f);
    EXPECT_GT(data[size / 2], 0.8f);
}

TEST_F(ABIFFTTest, DifferentWindowTypes) {
    const int size = 128;
    std::vector<float> window(size);

    std::vector<SpectraWindowType> types = {
        SPECTRA_WINDOW_RECTANGULAR,
        SPECTRA_WINDOW_HANN,
        SPECTRA_WINDOW_HAMMING,
        SPECTRA_WINDOW_BLACKMAN,
        SPECTRA_WINDOW_BLACKMAN_HARRIS,
        SPECTRA_WINDOW_FLAT_TOP
    };

    for (auto type : types) {
        int result = spectra_generate_window(window.data(), size, type);
        EXPECT_EQ(result, SPECTRA_OK);

        // All windows should have finite values
        for (int i = 0; i < size; ++i) {
            EXPECT_FALSE(std::isnan(window[i]));
            EXPECT_FALSE(std::isinf(window[i]));
        }
    }
}

// ============================================================================
// STFT via ABI
// ============================================================================

TEST_F(ABIFFTTest, STFTCreateDestroy) {
    SpectraSTFTConfig config;
    config.fft_size = 256;
    config.hop_size = 128;
    config.window = SPECTRA_WINDOW_HANN;
    config.center = 1;

    SpectraSTFT stft = spectra_stft_create(&config);
    EXPECT_NE(stft, nullptr);

    spectra_stft_destroy(stft);
}

TEST_F(ABIFFTTest, STFTNumFrames) {
    int num_frames = spectra_stft_num_frames(1024, 256, 128);
    EXPECT_GT(num_frames, 0);
}

TEST_F(ABIFFTTest, STFTAnalyze) {
    SpectraSTFTConfig config;
    config.fft_size = 256;
    config.hop_size = 128;
    config.window = SPECTRA_WINDOW_HANN;
    config.center = 0;

    SpectraSTFT stft = spectra_stft_create(&config);
    ASSERT_NE(stft, nullptr);

    const int input_size = 1024;
    std::vector<float> input(input_size);

    // Generate test signal
    for (int i = 0; i < input_size; ++i) {
        input[i] = std::sin(2.0f * PI * 10 * i / input_size);
    }

    int expected_frames = spectra_stft_num_frames(input_size, 256, 128);
    int spectrum_size = 256 / 2 + 1;

    std::vector<float> output_real(expected_frames * spectrum_size);
    std::vector<float> output_imag(expected_frames * spectrum_size);
    int actual_frames;

    int result = spectra_stft_analyze(stft, input.data(), input_size,
                                       output_real.data(), output_imag.data(),
                                       &actual_frames);
    EXPECT_EQ(result, SPECTRA_OK);
    EXPECT_GT(actual_frames, 0);

    // Check for valid output
    for (int i = 0; i < actual_frames * spectrum_size; ++i) {
        EXPECT_FALSE(std::isnan(output_real[i]));
        EXPECT_FALSE(std::isnan(output_imag[i]));
    }

    spectra_stft_destroy(stft);
}

TEST_F(ABIFFTTest, STFTStreaming) {
    SpectraSTFTConfig config;
    config.fft_size = 256;
    config.hop_size = 128;
    config.window = SPECTRA_WINDOW_HANN;
    config.center = 0;

    SpectraSTFT stft = spectra_stft_create(&config);
    ASSERT_NE(stft, nullptr);

    std::vector<float> samples(512);
    for (int i = 0; i < 512; ++i) {
        samples[i] = std::sin(2.0f * PI * 5 * i / 512);
    }

    // Push samples
    int result = spectra_stft_push_samples(stft, samples.data(), 512);
    EXPECT_EQ(result, SPECTRA_OK);

    // Check frames available
    int available = spectra_stft_frames_available(stft);
    EXPECT_GE(available, 0);

    // Pop frames if available
    if (available > 0) {
        int spectrum_size = 256 / 2 + 1;
        std::vector<float> real_out(spectrum_size);
        std::vector<float> imag_out(spectrum_size);

        result = spectra_stft_pop_frame(stft, real_out.data(), imag_out.data());
        EXPECT_EQ(result, SPECTRA_OK);
    }

    spectra_stft_destroy(stft);
}

// ============================================================================
// Version Info
// ============================================================================

TEST_F(ABIFFTTest, VersionInfo) {
    int major = spectra_version_major();
    int minor = spectra_version_minor();
    int patch = spectra_version_patch();
    const char* version_str = spectra_version_string();

    EXPECT_GE(major, 0);
    EXPECT_GE(minor, 0);
    EXPECT_GE(patch, 0);
    EXPECT_NE(version_str, nullptr);
}
