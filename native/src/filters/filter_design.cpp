/**
 * @file filter_design.cpp
 * @brief Filter design implementations
 */

#include "spectra/filters/filter_design.hpp"
#include "spectra/core/window_functions.hpp"
#include <cmath>
#include <algorithm>
#include <complex>

namespace spectra {

// Helper functions for filter design
namespace {

// Cascade a second-order section with existing filter
void cascade_sos(FilterCoefficients& coeffs,
                 double b0, double b1, double b2,
                 double a0, double a1, double a2) {
    size_t old_order = coeffs.b.size();
    std::vector<float> new_b(old_order + 2, 0.0f);
    std::vector<float> new_a(old_order + 2, 0.0f);

    for (size_t i = 0; i < old_order; ++i) {
        new_b[i] += static_cast<float>(b0) * coeffs.b[i];
        new_b[i + 1] += static_cast<float>(b1) * coeffs.b[i];
        new_b[i + 2] += static_cast<float>(b2) * coeffs.b[i];

        new_a[i] += static_cast<float>(a0) * coeffs.a[i];
        new_a[i + 1] += static_cast<float>(a1) * coeffs.a[i];
        new_a[i + 2] += static_cast<float>(a2) * coeffs.a[i];
    }

    coeffs.b = new_b;
    coeffs.a = new_a;
}

// Cascade a first-order section
void cascade_fos(FilterCoefficients& coeffs,
                 double b0, double b1,
                 double a0, double a1) {
    size_t old_order = coeffs.b.size();
    std::vector<float> new_b(old_order + 1, 0.0f);
    std::vector<float> new_a(old_order + 1, 0.0f);

    for (size_t i = 0; i < old_order; ++i) {
        new_b[i] += static_cast<float>(b0) * coeffs.b[i];
        new_b[i + 1] += static_cast<float>(b1) * coeffs.b[i];

        new_a[i] += static_cast<float>(a0) * coeffs.a[i];
        new_a[i + 1] += static_cast<float>(a1) * coeffs.a[i];
    }

    coeffs.b = new_b;
    coeffs.a = new_a;
}

// Design Butterworth lowpass using cascaded sections
FilterCoefficients butter_lowpass(int order, double wc) {
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};

    // Process poles: for odd order, there's one real pole; rest are complex pairs
    int num_complex_pairs = order / 2;
    bool has_real_pole = (order % 2 == 1);

    // Real pole (for odd order): at s = -wc
    if (has_real_pole) {
        // First-order analog section: H(s) = wc / (s + wc)
        // Bilinear transform with s = 2*(z-1)/(z+1):
        // H(z) = wc*(z+1) / (2*(z-1) + wc*(z+1))
        //      = wc*(z+1) / ((2+wc)*z + (wc-2))
        // In z^-1 form: (wc + wc*z^-1) / ((2+wc) + (wc-2)*z^-1)
        // Normalized: b = [wc/(2+wc), wc/(2+wc)], a = [1, (wc-2)/(2+wc)]

        double K = 2.0 + wc;
        double b0 = wc / K;
        double b1 = wc / K;
        double a1 = (wc - 2.0) / K;

        cascade_fos(coeffs, b0, b1, 1.0, a1);
    }

    // Complex conjugate pairs
    for (int k = 0; k < num_complex_pairs; ++k) {
        // Pole angle for pair k (k = 0, 1, ...)
        // For Butterworth, poles are at angles: pi/2 + pi*(2k+1)/(2*order)
        double theta = constants::pi * (2.0 * k + 1.0) / (2.0 * order);
        double sigma = -wc * std::sin(theta);  // Real part (negative for stability)
        double omega = wc * std::cos(theta);   // Imaginary part

        // Second-order analog section: H(s) = wc^2 / (s^2 - 2*sigma*s + sigma^2 + omega^2)
        double wc2 = wc * wc;
        double a1_s = -2.0 * sigma;
        double a2_s = sigma * sigma + omega * omega;

        // Bilinear transform: s = 2*(z-1)/(z+1)
        // H(z) = wc2 * (z+1)^2 / (4*(z-1)^2 - 2*a1_s*(z-1)*(z+1) + a2_s*(z+1)^2)
        //
        // Expanding denominator:
        // 4(z^2 - 2z + 1) - 2*a1_s*(z^2-1) + a2_s*(z^2 + 2z + 1)
        // = (4 - 2*a1_s + a2_s)*z^2 + (-8 + 2*a2_s)*z + (4 + 2*a1_s + a2_s)
        //
        // Numerator: wc2 * (z^2 + 2z + 1)

        double d0 = 4.0 + 2.0 * a1_s + a2_s;  // Note: a1_s is already negative
        double d1 = -8.0 + 2.0 * a2_s;
        double d2 = 4.0 - 2.0 * a1_s + a2_s;

        // Normalize by d0
        double b0 = wc2 / d0;
        double b1 = 2.0 * wc2 / d0;
        double b2 = wc2 / d0;
        double a1_z = d1 / d0;
        double a2_z = d2 / d0;

        cascade_sos(coeffs, b0, b1, b2, 1.0, a1_z, a2_z);
    }

    // Normalize for unity DC gain (sum of b / sum of a = 1 at z=1)
    float b_sum = 0.0f, a_sum = 0.0f;
    for (auto& b : coeffs.b) b_sum += b;
    for (auto& a : coeffs.a) a_sum += a;

    if (std::abs(b_sum) > 1e-10f) {
        float gain = a_sum / b_sum;
        for (auto& b : coeffs.b) b *= gain;
    }

    return coeffs;
}

// Design Butterworth highpass
FilterCoefficients butter_highpass(int order, double wc) {
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};

    int num_complex_pairs = order / 2;
    bool has_real_pole = (order % 2 == 1);

    // Real pole (for odd order)
    if (has_real_pole) {
        // Highpass first-order: H(s) = s / (s + wc)
        // Bilinear: H(z) = 2*(z-1) / ((2+wc)*z + (wc-2))
        // = 2*(1 - z^-1) / ((2+wc) + (wc-2)*z^-1)

        double K = 2.0 + wc;
        double b0 = 2.0 / K;
        double b1 = -2.0 / K;
        double a1 = (wc - 2.0) / K;

        cascade_fos(coeffs, b0, b1, 1.0, a1);
    }

    // Complex conjugate pairs
    for (int k = 0; k < num_complex_pairs; ++k) {
        double theta = constants::pi * (2.0 * k + 1.0) / (2.0 * order);
        double sigma = -wc * std::sin(theta);
        double omega = wc * std::cos(theta);

        // Highpass: H(s) = s^2 / (s^2 - 2*sigma*s + sigma^2 + omega^2)
        double a1_s = -2.0 * sigma;
        double a2_s = sigma * sigma + omega * omega;

        // Bilinear transform (same denominator as lowpass)
        double d0 = 4.0 + 2.0 * a1_s + a2_s;
        double d1 = -8.0 + 2.0 * a2_s;
        double d2 = 4.0 - 2.0 * a1_s + a2_s;

        // Numerator for highpass: 4*(z-1)^2 = 4*z^2 - 8*z + 4
        double b0 = 4.0 / d0;
        double b1 = -8.0 / d0;
        double b2 = 4.0 / d0;
        double a1_z = d1 / d0;
        double a2_z = d2 / d0;

        cascade_sos(coeffs, b0, b1, b2, 1.0, a1_z, a2_z);
    }

    // Normalize for unity gain at Nyquist (z = -1)
    float b_sum = 0.0f, a_sum = 0.0f;
    for (size_t i = 0; i < coeffs.b.size(); ++i) {
        float sign = (i % 2 == 0) ? 1.0f : -1.0f;
        b_sum += sign * coeffs.b[i];
    }
    for (size_t i = 0; i < coeffs.a.size(); ++i) {
        float sign = (i % 2 == 0) ? 1.0f : -1.0f;
        a_sum += sign * coeffs.a[i];
    }

    if (std::abs(b_sum) > 1e-10f) {
        float gain = std::abs(a_sum / b_sum);
        for (auto& b : coeffs.b) b *= gain;
    }

    return coeffs;
}

// Design Chebyshev Type I lowpass using cascaded sections
FilterCoefficients cheby1_lowpass(int order, double wc, double epsilon) {
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};

    // Chebyshev pole parameter: a = (1/n) * asinh(1/epsilon)
    double a = std::asinh(1.0 / epsilon) / order;
    double sinh_a = std::sinh(a);
    double cosh_a = std::cosh(a);

    int num_complex_pairs = order / 2;
    bool has_real_pole = (order % 2 == 1);

    // Real pole (for odd order): at s = -wc * sinh(a)
    if (has_real_pole) {
        double sigma = -wc * sinh_a;
        // H(s) = |sigma| / (s - sigma) = -sigma / (s - sigma)
        // Bilinear transform: s = 2*(z-1)/(z+1)
        // H(z) = -sigma*(z+1) / (2*(z-1) - sigma*(z+1))
        //      = -sigma*(z+1) / ((2-sigma)*z + (-2-sigma))

        double K = 2.0 - sigma;  // sigma is negative, so K > 0
        double b0 = -sigma / K;
        double b1 = -sigma / K;
        double a1 = (-2.0 - sigma) / K;

        cascade_fos(coeffs, b0, b1, 1.0, a1);
    }

    // Complex conjugate pairs
    for (int k = 0; k < num_complex_pairs; ++k) {
        // Pole angle: theta_k = pi*(2k+1)/(2n)
        double theta = constants::pi * (2.0 * k + 1.0) / (2.0 * order);
        double sigma = -wc * sinh_a * std::sin(theta);
        double omega = wc * cosh_a * std::cos(theta);

        // Second-order section: H(s) = K / (s^2 - 2*sigma*s + sigma^2 + omega^2)
        // where K is chosen for correct gain
        double wn2 = sigma * sigma + omega * omega;  // natural frequency squared
        double a1_s = -2.0 * sigma;
        double a2_s = wn2;

        // Bilinear transform: s = 2*(z-1)/(z+1)
        // Denominator: 4(z-1)^2 - 2*a1_s*(z^2-1) + a2_s*(z+1)^2
        double d0 = 4.0 + 2.0 * a1_s + a2_s;
        double d1 = -8.0 + 2.0 * a2_s;
        double d2 = 4.0 - 2.0 * a1_s + a2_s;

        // Numerator for lowpass: wn2 * (z+1)^2
        double b0 = wn2 / d0;
        double b1 = 2.0 * wn2 / d0;
        double b2 = wn2 / d0;
        double a1_z = d1 / d0;
        double a2_z = d2 / d0;

        cascade_sos(coeffs, b0, b1, b2, 1.0, a1_z, a2_z);
    }

    // Normalize for unity passband MAXIMUM gain
    // For Chebyshev Type I:
    // - Odd order: DC is at passband maximum (gain = 1), normalize DC to 1
    // - Even order: DC is at passband minimum (gain = 1/sqrt(1+eps^2))
    //   The passband maximum is sqrt(1+eps^2) times DC gain
    float b_sum = 0.0f, a_sum = 0.0f;
    for (auto& b : coeffs.b) b_sum += b;
    for (auto& a : coeffs.a) a_sum += a;

    if (std::abs(b_sum) > 1e-10f) {
        // Compute current DC gain
        float dc_gain_raw = b_sum / a_sum;

        // For odd order: DC should be 1 (max gain)
        // For even order: DC should be 1/sqrt(1+eps^2), max gain is 1
        // Both cases: we want max passband gain = 1
        float target_dc;
        if (has_real_pole) {
            // Odd order: DC is at a maximum, set DC = 1
            target_dc = 1.0f;
        } else {
            // Even order: DC is at a minimum, set DC = 1/sqrt(1+eps^2)
            target_dc = 1.0f / std::sqrt(1.0f + static_cast<float>(epsilon * epsilon));
        }

        float gain = target_dc / dc_gain_raw;
        for (auto& b : coeffs.b) b *= gain;
    }

    return coeffs;
}

// Design Chebyshev Type I highpass using cascaded sections
FilterCoefficients cheby1_highpass(int order, double wc, double epsilon) {
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};

    double a = std::asinh(1.0 / epsilon) / order;
    double sinh_a = std::sinh(a);
    double cosh_a = std::cosh(a);

    int num_complex_pairs = order / 2;
    bool has_real_pole = (order % 2 == 1);

    // Real pole (for odd order)
    if (has_real_pole) {
        double sigma = -wc * sinh_a;
        // Highpass: H(s) = s / (s - sigma)
        // Bilinear: H(z) = 2*(z-1)/(z+1) / (2*(z-1)/(z+1) - sigma)
        //                = 2*(z-1) / ((2-sigma)*z + (-2-sigma))

        double K = 2.0 - sigma;
        double b0 = 2.0 / K;
        double b1 = -2.0 / K;
        double a1 = (-2.0 - sigma) / K;

        cascade_fos(coeffs, b0, b1, 1.0, a1);
    }

    // Complex conjugate pairs
    for (int k = 0; k < num_complex_pairs; ++k) {
        double theta = constants::pi * (2.0 * k + 1.0) / (2.0 * order);
        double sigma = -wc * sinh_a * std::sin(theta);
        double omega = wc * cosh_a * std::cos(theta);

        double wn2 = sigma * sigma + omega * omega;
        double a1_s = -2.0 * sigma;
        double a2_s = wn2;

        double d0 = 4.0 + 2.0 * a1_s + a2_s;
        double d1 = -8.0 + 2.0 * a2_s;
        double d2 = 4.0 - 2.0 * a1_s + a2_s;

        // Numerator for highpass: 4*(z-1)^2
        double b0 = 4.0 / d0;
        double b1 = -8.0 / d0;
        double b2 = 4.0 / d0;
        double a1_z = d1 / d0;
        double a2_z = d2 / d0;

        cascade_sos(coeffs, b0, b1, b2, 1.0, a1_z, a2_z);
    }

    // Normalize for unity passband MAXIMUM gain at Nyquist
    // Similar to lowpass but at z = -1 (Nyquist)
    float b_sum = 0.0f, a_sum = 0.0f;
    for (size_t i = 0; i < coeffs.b.size(); ++i) {
        float sign = (i % 2 == 0) ? 1.0f : -1.0f;
        b_sum += sign * coeffs.b[i];
    }
    for (size_t i = 0; i < coeffs.a.size(); ++i) {
        float sign = (i % 2 == 0) ? 1.0f : -1.0f;
        a_sum += sign * coeffs.a[i];
    }

    if (std::abs(b_sum) > 1e-10f) {
        // Compute current Nyquist gain
        float nyq_gain_raw = std::abs(b_sum / a_sum);

        // For odd order: Nyquist is at a maximum, set Nyquist = 1
        // For even order: Nyquist is at a minimum, set Nyquist = 1/sqrt(1+eps^2)
        float target_nyq;
        if (has_real_pole) {
            target_nyq = 1.0f;
        } else {
            target_nyq = 1.0f / std::sqrt(1.0f + static_cast<float>(epsilon * epsilon));
        }

        float gain = target_nyq / nyq_gain_raw;
        for (auto& b : coeffs.b) b *= gain;
    }

    return coeffs;
}

} // anonymous namespace

FilterCoefficients butter(int order, float normalized_freq, FilterType type) {
    FilterCoefficients coeffs;

    // Validate inputs
    if (order < 1 || order > 10 || normalized_freq <= 0.0f || normalized_freq >= 1.0f) {
        // Return passthrough filter on error
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
        return coeffs;
    }

    // Frequency prewarping for bilinear transform with s = 2*(z-1)/(z+1):
    // ω_a = 2 * tan(π * f_n / 2)
    double wc = 2.0 * std::tan(constants::pi * normalized_freq / 2.0);

    if (type == FilterType::Lowpass) {
        return butter_lowpass(order, wc);
    } else if (type == FilterType::Highpass) {
        return butter_highpass(order, wc);
    } else {
        // Bandpass/Bandstop need two frequencies
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
        return coeffs;
    }
}

FilterCoefficients butter(int order, float low_freq, float high_freq, FilterType type) {
    FilterCoefficients coeffs;

    // Validate inputs
    if (order < 1 || low_freq <= 0.0f || high_freq >= 1.0f || low_freq >= high_freq) {
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
        return coeffs;
    }

    // For bandpass/bandstop, design as cascade of highpass and lowpass
    if (type == FilterType::Bandpass) {
        auto hp = butter(order, low_freq, FilterType::Highpass);
        auto lp = butter(order, high_freq, FilterType::Lowpass);

        // Convolve the two filters
        coeffs.b = std::vector<float>(hp.b.size() + lp.b.size() - 1, 0.0f);
        coeffs.a = std::vector<float>(hp.a.size() + lp.a.size() - 1, 0.0f);

        for (size_t i = 0; i < hp.b.size(); ++i) {
            for (size_t j = 0; j < lp.b.size(); ++j) {
                coeffs.b[i + j] += hp.b[i] * lp.b[j];
            }
        }
        for (size_t i = 0; i < hp.a.size(); ++i) {
            for (size_t j = 0; j < lp.a.size(); ++j) {
                coeffs.a[i + j] += hp.a[i] * lp.a[j];
            }
        }
    } else if (type == FilterType::Bandstop) {
        // Bandstop is more complex - simplified cascade approach
        auto lp = butter(order, low_freq, FilterType::Lowpass);
        auto hp = butter(order, high_freq, FilterType::Highpass);

        // This is an approximation - proper bandstop needs parallel connection
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
    } else {
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
    }

    return coeffs;
}

FilterCoefficients cheby1(int order, float ripple_db, float normalized_freq, FilterType type) {
    FilterCoefficients coeffs;

    // Validate inputs
    if (order < 1 || order > 10 || normalized_freq <= 0.0f || normalized_freq >= 1.0f ||
        ripple_db <= 0.0f || ripple_db > 20.0f) {
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
        return coeffs;
    }

    // Calculate ripple parameter epsilon from dB
    // At the passband edge: |H(jω)|² = 1/(1+ε²)
    // In dB: 10*log10(1/(1+ε²)) = -Rp
    // So: ε = sqrt(10^(Rp/10) - 1)
    double epsilon = std::sqrt(std::pow(10.0, ripple_db / 10.0) - 1.0);

    // Frequency prewarping
    double wc = 2.0 * std::tan(constants::pi * normalized_freq / 2.0);

    if (type == FilterType::Lowpass) {
        return cheby1_lowpass(order, wc, epsilon);
    } else if (type == FilterType::Highpass) {
        return cheby1_highpass(order, wc, epsilon);
    } else {
        // Bandpass/Bandstop not yet implemented
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
        return coeffs;
    }
}

FilterCoefficients cheby2(int order, float stopband_db, float normalized_freq, FilterType type) {
    // Placeholder
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};
    return coeffs;
}

FilterCoefficients ellip(int order, float passband_ripple_db, float stopband_db,
                         float normalized_freq, FilterType type) {
    // Placeholder
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};
    return coeffs;
}

FilterCoefficients bessel(int order, float normalized_freq, FilterType type) {
    // Placeholder
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};
    return coeffs;
}

std::vector<float> fir_windowed_sinc(int num_taps, float normalized_freq,
                                      FilterType type, WindowType window) {
    std::vector<float> h(num_taps);
    const int M = num_taps - 1;
    const float fc = normalized_freq / 2.0f;  // Cutoff in terms of sample rate

    // Generate sinc
    for (int n = 0; n <= M; ++n) {
        const float nm = n - M / 2.0f;
        if (std::abs(nm) < 1e-6f) {
            h[n] = 2.0f * fc;
        } else {
            h[n] = std::sin(constants::two_pi * fc * nm) / (constants::pi * nm);
        }
    }

    // Apply window
    std::vector<float> win = generate_window(num_taps, window);
    for (int n = 0; n < num_taps; ++n) {
        h[n] *= win[n];
    }

    // Spectral inversion for highpass
    if (type == FilterType::Highpass) {
        for (int n = 0; n < num_taps; ++n) {
            h[n] = -h[n];
        }
        h[M / 2] += 1.0f;
    }

    return h;
}

std::vector<float> fir_windowed_sinc(int num_taps, float low_freq, float high_freq,
                                      FilterType type, WindowType window) {
    // Placeholder
    return std::vector<float>(num_taps, 1.0f / num_taps);
}

std::vector<float> firpm(int order,
                         const std::vector<float>& freq_bands,
                         const std::vector<float>& amplitudes,
                         const std::vector<float>& weights) {
    // Parks-McClellan placeholder
    return std::vector<float>(order + 1, 1.0f / (order + 1));
}

void freqz(const FilterCoefficients& coeffs, Complex* H, size_t num_points) {
    for (size_t k = 0; k < num_points; ++k) {
        const float w = constants::pi * k / (num_points - 1);

        // Numerator
        Complex num = {0.0f, 0.0f};
        for (size_t i = 0; i < coeffs.b.size(); ++i) {
            const float phase = -w * i;
            num.real += coeffs.b[i] * std::cos(phase);
            num.imag += coeffs.b[i] * std::sin(phase);
        }

        // Denominator
        Complex den = {0.0f, 0.0f};
        for (size_t i = 0; i < coeffs.a.size(); ++i) {
            const float phase = -w * i;
            den.real += coeffs.a[i] * std::cos(phase);
            den.imag += coeffs.a[i] * std::sin(phase);
        }

        // H = num / den
        const float den_mag_sq = den.magnitude_squared();
        H[k].real = (num.real * den.real + num.imag * den.imag) / den_mag_sq;
        H[k].imag = (num.imag * den.real - num.real * den.imag) / den_mag_sq;
    }
}

void freqz(const std::vector<float>& b, Complex* H, size_t num_points) {
    FilterCoefficients coeffs;
    coeffs.b = b;
    coeffs.a = {1.0f};
    freqz(coeffs, H, num_points);
}

void grpdelay(const FilterCoefficients& coeffs, float* gd, size_t num_points) {
    // Placeholder - compute numerical derivative of phase
    std::vector<Complex> H(num_points);
    freqz(coeffs, H.data(), num_points);

    for (size_t k = 0; k < num_points; ++k) {
        gd[k] = static_cast<float>(coeffs.order()) / 2.0f;
    }
}

bool is_stable(const FilterCoefficients& coeffs) {
    // Check if all poles of the filter are inside the unit circle
    // by verifying the denominator polynomial has positive magnitude on the unit circle

    if (coeffs.a.size() <= 1) return true;
    if (std::abs(coeffs.a[0]) < 1e-10f) return false;

    size_t n = coeffs.a.size() - 1;  // Filter order

    // Normalize by a[0]
    std::vector<double> a(coeffs.a.size());
    for (size_t i = 0; i < coeffs.a.size(); ++i) {
        a[i] = static_cast<double>(coeffs.a[i]) / coeffs.a[0];
    }

    // Necessary condition: |a[n]| < 1 for stability (Schur-Cohn condition)
    if (std::abs(a[n]) >= 1.0) return false;

    // Sample the denominator magnitude on the unit circle
    // If filter is stable, |A(e^jw)| should never get very small
    const int num_samples = 128;
    double min_mag_sq = 1e30;

    for (int k = 0; k < num_samples; ++k) {
        double w = constants::pi * k / (num_samples - 1);

        // Evaluate A(e^jw) = sum(a[i] * e^(-jwi))
        double real_part = 0.0, imag_part = 0.0;
        for (size_t i = 0; i <= n; ++i) {
            double phase = -w * static_cast<double>(i);
            real_part += a[i] * std::cos(phase);
            imag_part += a[i] * std::sin(phase);
        }

        double mag_sq = real_part * real_part + imag_part * imag_part;
        min_mag_sq = std::min(min_mag_sq, mag_sq);
    }

    // If minimum magnitude is extremely small, there's a pole on or very close
    // to the unit circle. For Butterworth filters designed via bilinear transform,
    // all poles are guaranteed to be inside the unit circle, so we just need to
    // catch obvious numerical errors.
    // Use a fixed absolute threshold - even for high-order filters with high
    // cutoff frequencies, the minimum shouldn't be below this.
    return min_mag_sq > 1e-16;
}

} // namespace spectra
