/**
 * @file types.hpp
 * @brief Core type definitions for SpectraCore
 */

#ifndef SPECTRA_CORE_TYPES_HPP
#define SPECTRA_CORE_TYPES_HPP

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <array>
#include <vector>
#include <cassert>

namespace spectra {

/// Audio sample type (32-bit float)
using Sample = float;

/// Complex number representation
struct Complex {
    float real = 0.0f;
    float imag = 0.0f;

    Complex() = default;
    Complex(float r, float i) : real(r), imag(i) {}

    [[nodiscard]] float magnitude() const {
        return std::sqrt(real * real + imag * imag);
    }

    [[nodiscard]] float magnitude_squared() const {
        return real * real + imag * imag;
    }

    [[nodiscard]] float phase() const {
        return std::atan2(imag, real);
    }

    Complex operator+(const Complex& other) const {
        return {real + other.real, imag + other.imag};
    }

    Complex operator-(const Complex& other) const {
        return {real - other.real, imag - other.imag};
    }

    Complex operator*(const Complex& other) const {
        return {
            real * other.real - imag * other.imag,
            real * other.imag + imag * other.real
        };
    }

    Complex operator*(float scalar) const {
        return {real * scalar, imag * scalar};
    }

    Complex& operator+=(const Complex& other) {
        real += other.real;
        imag += other.imag;
        return *this;
    }

    Complex conjugate() const {
        return {real, -imag};
    }
};

/// Filter type enumeration
enum class FilterType {
    Lowpass,
    Highpass,
    Bandpass,
    Bandstop
};

/// Window function type enumeration
enum class WindowType {
    Rectangular,
    Hann,
    Hamming,
    Blackman,
    BlackmanHarris,
    FlatTop
};

/// Filter coefficients (numerator and denominator)
struct FilterCoefficients {
    std::vector<float> b;  ///< Numerator (feedforward) coefficients
    std::vector<float> a;  ///< Denominator (feedback) coefficients

    [[nodiscard]] size_t order() const {
        return std::max(b.size(), a.size()) - 1;
    }
};

/// Result type for operations that can fail
template<typename T>
struct Result {
    T value;
    bool ok = true;
    const char* error = nullptr;

    static Result success(T val) {
        return {std::move(val), true, nullptr};
    }

    static Result failure(const char* err) {
        return {{}, false, err};
    }

    explicit operator bool() const { return ok; }
};

/// Constants
namespace constants {
    constexpr float pi = 3.14159265358979323846f;
    constexpr float two_pi = 2.0f * pi;
    constexpr float epsilon = 1e-7f;
}

} // namespace spectra

#endif // SPECTRA_CORE_TYPES_HPP
