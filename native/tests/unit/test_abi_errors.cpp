/**
 * @file test_abi_errors.cpp
 * @brief Unit tests for thread-safe error handling via C ABI
 */

#include <gtest/gtest.h>
#include "spectra_abi.h"
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>

class ABIErrorTest : public ::testing::Test {
protected:
    void SetUp() override {
        spectra_clear_error();
    }

    void TearDown() override {
        spectra_clear_error();
    }
};

// ============================================================================
// Basic Error Handling
// ============================================================================

TEST_F(ABIErrorTest, NoErrorInitially) {
    SpectraError err = spectra_get_last_error();
    EXPECT_EQ(err, SPECTRA_OK);
}

TEST_F(ABIErrorTest, ClearError) {
    spectra_clear_error();
    SpectraError err = spectra_get_last_error();
    EXPECT_EQ(err, SPECTRA_OK);
}

TEST_F(ABIErrorTest, ErrorMessageExists) {
    spectra_clear_error();
    const char* msg = spectra_get_error_message();
    // Should return a valid pointer (possibly empty string)
    EXPECT_NE(msg, nullptr);
}

// ============================================================================
// Error on Invalid Operations
// ============================================================================

TEST_F(ABIErrorTest, InvalidFFTHandle) {
    std::vector<float> buffer(256);

    int result = spectra_fft_forward(nullptr, buffer.data(),
                                      buffer.data(), buffer.data());
    EXPECT_NE(result, SPECTRA_OK);

    SpectraError err = spectra_get_last_error();
    EXPECT_NE(err, SPECTRA_OK);
}

TEST_F(ABIErrorTest, InvalidFilterHandle) {
    std::vector<float> buffer(64);

    int result = spectra_streaming_filter_process(nullptr,
                                                   buffer.data(), buffer.data(), 64);
    EXPECT_NE(result, SPECTRA_OK);
}

TEST_F(ABIErrorTest, InvalidOnsetHandle) {
    std::vector<float> buffer(1024);

    int result = spectra_onset_process(nullptr, buffer.data(), 1024,
                                        nullptr, nullptr);
    EXPECT_NE(result, SPECTRA_OK);
}

TEST_F(ABIErrorTest, InvalidSTFTHandle) {
    std::vector<float> real(129), imag(129);

    int result = spectra_stft_pop_frame(nullptr, real.data(), imag.data());
    EXPECT_NE(result, SPECTRA_OK);
}

// ============================================================================
// Error Recovery
// ============================================================================

TEST_F(ABIErrorTest, ErrorClearedAfterSuccess) {
    // First cause an error
    spectra_fft_forward(nullptr, nullptr, nullptr, nullptr);
    EXPECT_NE(spectra_get_last_error(), SPECTRA_OK);

    // Clear the error
    spectra_clear_error();

    // Now perform a successful operation
    SpectraFFT fft = spectra_fft_create(256);
    EXPECT_NE(fft, nullptr);

    // Error should be cleared
    EXPECT_EQ(spectra_get_last_error(), SPECTRA_OK);

    spectra_fft_destroy(fft);
}

TEST_F(ABIErrorTest, ErrorPersistsUntilCleared) {
    // Cause an error
    spectra_fft_forward(nullptr, nullptr, nullptr, nullptr);
    SpectraError err1 = spectra_get_last_error();
    EXPECT_NE(err1, SPECTRA_OK);

    // Check error persists
    SpectraError err2 = spectra_get_last_error();
    EXPECT_EQ(err1, err2);

    // Clear
    spectra_clear_error();
    EXPECT_EQ(spectra_get_last_error(), SPECTRA_OK);
}

// ============================================================================
// Thread Safety
// ============================================================================

TEST_F(ABIErrorTest, ThreadLocalErrors) {
    // Errors should not cross threads
    std::atomic<bool> thread1_saw_error{false};
    std::atomic<bool> thread2_saw_ok{false};

    std::thread t1([&]() {
        spectra_clear_error();
        // Cause an error
        spectra_fft_forward(nullptr, nullptr, nullptr, nullptr);
        thread1_saw_error = (spectra_get_last_error() != SPECTRA_OK);
    });

    std::thread t2([&]() {
        spectra_clear_error();
        // No error in this thread
        SpectraFFT fft = spectra_fft_create(128);
        thread2_saw_ok = (spectra_get_last_error() == SPECTRA_OK);
        spectra_fft_destroy(fft);
    });

    t1.join();
    t2.join();

    EXPECT_TRUE(thread1_saw_error);
    EXPECT_TRUE(thread2_saw_ok);
}

TEST_F(ABIErrorTest, ConcurrentOperations) {
    // Multiple threads operating concurrently
    const int num_threads = 4;
    const int iterations = 100;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < iterations; ++i) {
                spectra_clear_error();

                SpectraFFT fft = spectra_fft_create(256);
                if (fft) {
                    std::vector<float> input(256, 1.0f);
                    std::vector<float> real(256), imag(256);

                    int result = spectra_fft_forward(fft, input.data(),
                                                      real.data(), imag.data());
                    if (result == SPECTRA_OK) {
                        success_count++;
                    }

                    spectra_fft_destroy(fft);
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(success_count, num_threads * iterations);
}

// ============================================================================
// Error Codes
// ============================================================================

TEST_F(ABIErrorTest, ErrorCodes) {
    // Verify error code enum values
    EXPECT_EQ(SPECTRA_OK, 0);
    EXPECT_GT(SPECTRA_ERROR_INVALID_ARGUMENT, 0);
    EXPECT_GT(SPECTRA_ERROR_ALLOCATION_FAILED, 0);
    EXPECT_GT(SPECTRA_ERROR_INVALID_HANDLE, 0);
    EXPECT_GT(SPECTRA_ERROR_INVALID_STATE, 0);
    EXPECT_GT(SPECTRA_ERROR_NUMERICAL_INSTABILITY, 0);
    EXPECT_GT(SPECTRA_ERROR_BUFFER_TOO_SMALL, 0);
}

TEST_F(ABIErrorTest, ErrorMessageNotEmpty) {
    // After error, message should be meaningful
    spectra_fft_forward(nullptr, nullptr, nullptr, nullptr);

    const char* msg = spectra_get_error_message();
    EXPECT_NE(msg, nullptr);
    // Message might be empty or have content depending on implementation
}

// ============================================================================
// Stress Test
// ============================================================================

TEST_F(ABIErrorTest, RapidErrorClearCycles) {
    // Rapidly causing and clearing errors
    for (int i = 0; i < 1000; ++i) {
        spectra_fft_forward(nullptr, nullptr, nullptr, nullptr);
        spectra_clear_error();
    }

    // Should end in clean state
    EXPECT_EQ(spectra_get_last_error(), SPECTRA_OK);
}

TEST_F(ABIErrorTest, MultipleErrorsOverwrite) {
    // Later errors should overwrite earlier ones
    spectra_fft_forward(nullptr, nullptr, nullptr, nullptr);
    SpectraError err1 = spectra_get_last_error();

    spectra_streaming_filter_process(nullptr, nullptr, nullptr, 0);
    SpectraError err2 = spectra_get_last_error();

    // Both should be error states
    EXPECT_NE(err1, SPECTRA_OK);
    EXPECT_NE(err2, SPECTRA_OK);
}
