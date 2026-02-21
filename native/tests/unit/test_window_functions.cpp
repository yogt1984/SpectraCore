/**
 * @file test_window_functions.cpp
 * @brief Unit tests for window functions
 */

#include <gtest/gtest.h>
#include "spectra/core/window_functions.hpp"
#include <cmath>

using namespace spectra;

class WindowFunctionsTest : public ::testing::Test {
protected:
    static constexpr size_t N = 64;
    std::vector<float> window;

    void SetUp() override {
        window.resize(N);
    }
};

TEST_F(WindowFunctionsTest, RectangularWindow) {
    generate_window(window.data(), N, WindowType::Rectangular);

    for (size_t i = 0; i < N; ++i) {
        EXPECT_FLOAT_EQ(window[i], 1.0f);
    }
}

TEST_F(WindowFunctionsTest, HannWindow) {
    generate_window(window.data(), N, WindowType::Hann);

    // Endpoints should be near zero
    EXPECT_NEAR(window[0], 0.0f, 0.001f);

    // Middle should be near 1
    EXPECT_NEAR(window[N / 2], 1.0f, 0.1f);

    // Should be symmetric
    for (size_t i = 0; i < N / 2; ++i) {
        EXPECT_NEAR(window[i], window[N - 1 - i], 1e-5f);
    }
}

TEST_F(WindowFunctionsTest, HammingWindow) {
    generate_window(window.data(), N, WindowType::Hamming);

    // Hamming window doesn't go to zero at endpoints
    EXPECT_NEAR(window[0], 0.08f, 0.01f);

    // Middle should be near 1
    EXPECT_NEAR(window[N / 2], 1.0f, 0.1f);

    // Should be symmetric
    for (size_t i = 0; i < N / 2; ++i) {
        EXPECT_NEAR(window[i], window[N - 1 - i], 1e-5f);
    }
}

TEST_F(WindowFunctionsTest, BlackmanWindow) {
    generate_window(window.data(), N, WindowType::Blackman);

    // Endpoints should be near zero
    EXPECT_NEAR(window[0], 0.0f, 0.01f);

    // Should be symmetric
    for (size_t i = 0; i < N / 2; ++i) {
        EXPECT_NEAR(window[i], window[N - 1 - i], 1e-5f);
    }
}

TEST_F(WindowFunctionsTest, ApplyWindow) {
    std::vector<float> signal(N, 2.0f);
    generate_window(window.data(), N, WindowType::Hann);

    apply_window(signal.data(), window.data(), N);

    for (size_t i = 0; i < N; ++i) {
        EXPECT_FLOAT_EQ(signal[i], 2.0f * window[i]);
    }
}

TEST_F(WindowFunctionsTest, VectorInterface) {
    auto win = generate_window(N, WindowType::Hann);
    EXPECT_EQ(win.size(), N);
    EXPECT_NEAR(win[0], 0.0f, 0.001f);
}

TEST_F(WindowFunctionsTest, CoherentGain) {
    EXPECT_FLOAT_EQ(window_coherent_gain(WindowType::Rectangular), 1.0f);
    EXPECT_FLOAT_EQ(window_coherent_gain(WindowType::Hann), 0.5f);
}

TEST_F(WindowFunctionsTest, AllWindowsNonNegative) {
    std::vector<WindowType> types = {
        WindowType::Rectangular,
        WindowType::Hann,
        WindowType::Hamming,
        WindowType::Blackman,
        WindowType::BlackmanHarris,
        WindowType::FlatTop
    };

    for (auto type : types) {
        generate_window(window.data(), N, type);
        for (size_t i = 0; i < N; ++i) {
            EXPECT_GE(window[i], 0.0f) << "Window type: " << static_cast<int>(type);
        }
    }
}
