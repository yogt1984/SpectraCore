/**
 * @file window_functions.cpp
 * @brief Window function implementations
 */

#include "spectra/core/window_functions.hpp"
#include <cmath>

namespace spectra {

std::vector<float> generate_window(size_t size, WindowType type) {
    std::vector<float> window(size);
    generate_window(window.data(), size, type);
    return window;
}

void generate_window(float* output, size_t size, WindowType type) {
    const float N = static_cast<float>(size);

    switch (type) {
        case WindowType::Rectangular:
            for (size_t i = 0; i < size; ++i) {
                output[i] = 1.0f;
            }
            break;

        case WindowType::Hann:
            for (size_t i = 0; i < size; ++i) {
                output[i] = 0.5f * (1.0f - std::cos(constants::two_pi * i / (N - 1)));
            }
            break;

        case WindowType::Hamming:
            for (size_t i = 0; i < size; ++i) {
                output[i] = 0.54f - 0.46f * std::cos(constants::two_pi * i / (N - 1));
            }
            break;

        case WindowType::Blackman:
            for (size_t i = 0; i < size; ++i) {
                const float n = static_cast<float>(i);
                float val = 0.42f
                          - 0.5f * std::cos(constants::two_pi * n / (N - 1))
                          + 0.08f * std::cos(2.0f * constants::two_pi * n / (N - 1));
                // Clamp tiny negative values from floating-point precision
                output[i] = std::max(0.0f, val);
            }
            break;

        case WindowType::BlackmanHarris:
            for (size_t i = 0; i < size; ++i) {
                const float n = static_cast<float>(i);
                output[i] = 0.35875f
                          - 0.48829f * std::cos(constants::two_pi * n / (N - 1))
                          + 0.14128f * std::cos(2.0f * constants::two_pi * n / (N - 1))
                          - 0.01168f * std::cos(3.0f * constants::two_pi * n / (N - 1));
            }
            break;

        case WindowType::FlatTop:
            for (size_t i = 0; i < size; ++i) {
                const float n = static_cast<float>(i);
                output[i] = 0.21557895f
                          - 0.41663158f * std::cos(constants::two_pi * n / (N - 1))
                          + 0.277263158f * std::cos(2.0f * constants::two_pi * n / (N - 1))
                          - 0.083578947f * std::cos(3.0f * constants::two_pi * n / (N - 1))
                          + 0.006947368f * std::cos(4.0f * constants::two_pi * n / (N - 1));
            }
            break;
    }
}

void apply_window(float* data, size_t size, WindowType type) {
    std::vector<float> window = generate_window(size, type);
    apply_window(data, window.data(), size);
}

void apply_window(float* data, const float* window, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        data[i] *= window[i];
    }
}

float window_coherent_gain(WindowType type) {
    switch (type) {
        case WindowType::Rectangular:    return 1.0f;
        case WindowType::Hann:           return 0.5f;
        case WindowType::Hamming:        return 0.54f;
        case WindowType::Blackman:       return 0.42f;
        case WindowType::BlackmanHarris: return 0.35875f;
        case WindowType::FlatTop:        return 0.21557895f;
        default:                         return 1.0f;
    }
}

float window_enbw(WindowType type) {
    switch (type) {
        case WindowType::Rectangular:    return 1.0f;
        case WindowType::Hann:           return 1.5f;
        case WindowType::Hamming:        return 1.36f;
        case WindowType::Blackman:       return 1.73f;
        case WindowType::BlackmanHarris: return 2.0f;
        case WindowType::FlatTop:        return 3.77f;
        default:                         return 1.0f;
    }
}

} // namespace spectra
