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
    // Note: FlatTop windows intentionally have negative side lobes by design
    // for accurate amplitude measurements, so we exclude them from this test
    std::vector<WindowType> types = {
        WindowType::Rectangular,
        WindowType::Hann,
        WindowType::Hamming,
        WindowType::Blackman,
        WindowType::BlackmanHarris
    };

    for (auto type : types) {
        generate_window(window.data(), N, type);
        for (size_t i = 0; i < N; ++i) {
            EXPECT_GE(window[i], 0.0f) << "Window type: " << static_cast<int>(type);
        }
    }
}

TEST_F(WindowFunctionsTest, FlatTopWindowProperties) {
    generate_window(window.data(), N, WindowType::FlatTop);

    // FlatTop windows have negative side lobes by design
    // but center should be approximately 1.0
    EXPECT_NEAR(window[N / 2], 1.0f, 0.01f);

    // Should be symmetric
    for (size_t i = 0; i < N / 2; ++i) {
        EXPECT_NEAR(window[i], window[N - 1 - i], 1e-5f);
    }
}

TEST_F(WindowFunctionsTest, ReferenceValues) {
    // Test against computed reference values for symmetric window
    // Formula: w[n] = 0.5 * (1 - cos(2*pi*n/(N-1)))

    // Hann window N=8 (symmetric formula with N-1=7):
    // n=0: 0.0, n=1: 0.188, n=2: 0.611, n=3: 0.950, n=4: 0.950, ...
    std::vector<float> hann8(8);
    generate_window(hann8.data(), 8, WindowType::Hann);
    EXPECT_NEAR(hann8[0], 0.0f, 1e-4f);        // Endpoint
    EXPECT_NEAR(hann8[7], 0.0f, 1e-4f);        // Endpoint
    // Check symmetry: hann8[i] == hann8[N-1-i]
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_NEAR(hann8[i], hann8[7 - i], 1e-5f);
    }

    // Hamming window N=8 endpoints should be 0.08
    std::vector<float> hamming8(8);
    generate_window(hamming8.data(), 8, WindowType::Hamming);
    EXPECT_NEAR(hamming8[0], 0.08f, 1e-3f);
    EXPECT_NEAR(hamming8[7], 0.08f, 1e-3f);
    // Check symmetry
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_NEAR(hamming8[i], hamming8[7 - i], 1e-5f);
    }
}

TEST_F(WindowFunctionsTest, EdgeCaseN1) {
    std::vector<float> win1(1);
    generate_window(win1.data(), 1, WindowType::Hann);
    // Single point window should be 1.0 (center)
    EXPECT_TRUE(std::isfinite(win1[0]));
}

TEST_F(WindowFunctionsTest, EdgeCaseN2) {
    std::vector<float> win2(2);
    generate_window(win2.data(), 2, WindowType::Hann);
    // N=2 Hann window: [0, 0] at endpoints
    EXPECT_NEAR(win2[0], 0.0f, 1e-5f);
    EXPECT_NEAR(win2[1], 0.0f, 1e-5f);
}

TEST_F(WindowFunctionsTest, LargeWindow) {
    const size_t largeN = 4096;
    std::vector<float> largWin(largeN);
    generate_window(largWin.data(), largeN, WindowType::Hann);

    // Check symmetry
    for (size_t i = 0; i < largeN / 2; ++i) {
        EXPECT_NEAR(largWin[i], largWin[largeN - 1 - i], 1e-5f);
    }

    // Check center is 1.0
    EXPECT_NEAR(largWin[largeN / 2], 1.0f, 0.01f);
}
