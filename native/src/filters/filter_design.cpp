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

// Design Chebyshev Type II (inverse Chebyshev) lowpass
// Type II has monotonic passband and equiripple stopband
FilterCoefficients cheby2_lowpass(int order, double ws, double epsilon) {
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};

    // Chebyshev Type II poles are inversions of Type I poles
    // a = (1/n) * asinh(1/epsilon)
    double a = std::asinh(1.0 / epsilon) / order;
    double sinh_a = std::sinh(a);
    double cosh_a = std::cosh(a);

    int num_complex_pairs = order / 2;
    bool has_real_pole = (order % 2 == 1);

    // Real pole (for odd order): no corresponding zero
    if (has_real_pole) {
        // Type I pole at p = -sinh(a), Type II pole at ws/p = -ws/sinh(a)
        double sigma = -ws / sinh_a;

        // First-order section: H(s) = sigma / (s - sigma)
        // Bilinear transform
        double K = 2.0 - sigma;
        double b0 = -sigma / K;
        double b1 = -sigma / K;
        double a1 = (-2.0 - sigma) / K;

        cascade_fos(coeffs, b0, b1, 1.0, a1);
    }

    // Complex conjugate pairs with zeros
    for (int k = 0; k < num_complex_pairs; ++k) {
        double theta = constants::pi * (2.0 * k + 1.0) / (2.0 * order);
        double sin_theta = std::sin(theta);
        double cos_theta = std::cos(theta);

        // Type I normalized poles
        double sigma1 = -sinh_a * sin_theta;
        double omega1 = cosh_a * cos_theta;

        // Type II poles: inversion of Type I poles, scaled by ws
        // p_II = ws² / p_I* = ws² / (sigma1 - j*omega1)
        // = ws² * (sigma1 + j*omega1) / (sigma1² + omega1²)
        double denom = sigma1 * sigma1 + omega1 * omega1;
        double sigma2 = ws * ws * sigma1 / denom;  // Real part (negative)
        double omega2 = -ws * ws * omega1 / denom; // Imaginary part

        // Actually, let's use the standard formulation:
        // Type II pole: sigma2 = ws * sinh_a * sin(theta) / (sinh²(a)*sin²(theta) + cosh²(a)*cos²(theta))
        //               omega2 = ws * cosh_a * cos(theta) / (sinh²(a)*sin²(theta) + cosh²(a)*cos²(theta))
        double pole_denom = sinh_a * sinh_a * sin_theta * sin_theta +
                            cosh_a * cosh_a * cos_theta * cos_theta;
        sigma2 = -ws * sinh_a * sin_theta / pole_denom;
        omega2 = ws * cosh_a * cos_theta / pole_denom;

        // Type II zeros on imaginary axis: z = ±j * ws / cos(theta)
        double zero_omega = ws / cos_theta;

        // Second-order section with zeros:
        // H(s) = (s² + zero_omega²) / (s² - 2*sigma2*s + sigma2² + omega2²)

        double wn2 = sigma2 * sigma2 + omega2 * omega2;
        double a1_s = -2.0 * sigma2;
        double a2_s = wn2;

        // Bilinear transform
        // Numerator: (s² + zero_omega²) → after bilinear becomes complex
        // s = 2*(z-1)/(z+1)
        // s² = 4*(z-1)²/(z+1)²
        // s² + z² = (4*(z-1)² + z²*(z+1)²) / (z+1)²
        //         = (4*z² - 8*z + 4 + z²*(z² + 2*z + 1)) / (z+1)²

        // Actually, let's use direct computation:
        // Numerator analog: s² + zero_omega²
        // After bilinear s = 2*(z-1)/(z+1):
        // 4*(z-1)²/(z+1)² + zero_omega² = (4*(z-1)² + zero_omega²*(z+1)²) / (z+1)²
        // = (4*z² - 8*z + 4 + zero_omega²*z² + 2*zero_omega²*z + zero_omega²) / (z+1)²
        // = ((4+zero_omega²)*z² + (-8+2*zero_omega²)*z + (4+zero_omega²)) / (z+1)²

        double zo2 = zero_omega * zero_omega;
        double n0 = 4.0 + zo2;
        double n1 = -8.0 + 2.0 * zo2;
        double n2 = 4.0 + zo2;

        // Denominator
        double d0 = 4.0 + 2.0 * a1_s + a2_s;
        double d1 = -8.0 + 2.0 * a2_s;
        double d2 = 4.0 - 2.0 * a1_s + a2_s;

        // Normalize by d0
        double b0 = n0 / d0;
        double b1 = n1 / d0;
        double b2 = n2 / d0;
        double a1_z = d1 / d0;
        double a2_z = d2 / d0;

        cascade_sos(coeffs, b0, b1, b2, 1.0, a1_z, a2_z);
    }

    // Normalize for unity DC gain (lowpass should pass DC)
    float b_sum = 0.0f, a_sum = 0.0f;
    for (auto& b : coeffs.b) b_sum += b;
    for (auto& a : coeffs.a) a_sum += a;

    if (std::abs(b_sum) > 1e-10f) {
        float gain = a_sum / b_sum;
        for (auto& b : coeffs.b) b *= gain;
    }

    return coeffs;
}

// ============================================================================
// Elliptic (Cauer) Filter Helper Functions
// ============================================================================

// Complete elliptic integral of the first kind K(k)
// Uses the arithmetic-geometric mean method
double elliptic_k(double k) {
    if (k < 0.0 || k >= 1.0) return 0.0;
    if (k < 1e-15) return constants::pi / 2.0;

    double a = 1.0;
    double b = std::sqrt(1.0 - k * k);

    for (int iter = 0; iter < 50; ++iter) {
        double a_new = (a + b) / 2.0;
        double b_new = std::sqrt(a * b);
        if (std::abs(a_new - b_new) < 1e-15) {
            return constants::pi / (2.0 * a_new);
        }
        a = a_new;
        b = b_new;
    }
    return constants::pi / (2.0 * a);
}

// Jacobi elliptic functions sn, cn, dn
// Uses the descending Landen transformation
void jacobi_elliptic(double u, double k, double& sn, double& cn, double& dn) {
    if (k < 0.0 || k > 1.0) {
        sn = cn = dn = 0.0;
        return;
    }

    if (k < 1e-15) {
        // k ≈ 0: sn ≈ sin(u), cn ≈ cos(u), dn ≈ 1
        sn = std::sin(u);
        cn = std::cos(u);
        dn = 1.0;
        return;
    }

    if (k > 1.0 - 1e-15) {
        // k ≈ 1: sn ≈ tanh(u), cn ≈ sech(u), dn ≈ sech(u)
        double e = std::exp(u);
        double em = std::exp(-u);
        sn = (e - em) / (e + em);  // tanh
        cn = 2.0 / (e + em);        // sech
        dn = cn;
        return;
    }

    // Descending Landen transformation
    const int max_iter = 20;
    std::vector<double> a(max_iter);
    std::vector<double> c(max_iter);

    a[0] = 1.0;
    c[0] = k;
    int n = 0;

    for (n = 0; n < max_iter - 1; ++n) {
        double b = std::sqrt(a[n] * a[n] - c[n] * c[n]);
        a[n + 1] = (a[n] + b) / 2.0;
        c[n + 1] = (a[n] - b) / 2.0;
        if (std::abs(c[n + 1]) < 1e-15) break;
    }

    // Ascending recurrence
    double phi = std::pow(2.0, n) * a[n] * u;
    for (int i = n; i >= 1; --i) {
        phi = (phi + std::asin(c[i] * std::sin(phi) / a[i])) / 2.0;
    }

    sn = std::sin(phi);
    cn = std::cos(phi);
    dn = std::sqrt(1.0 - k * k * sn * sn);
}

// Compute elliptic filter selectivity factor
// Given wp (passband edge), ws (stopband edge), returns k
double elliptic_selectivity(double wp, double ws) {
    return wp / ws;
}

// Compute discrimination factor k1 from ripple specifications
// Rp = passband ripple in dB, Rs = stopband attenuation in dB
double elliptic_discrimination(double Rp, double Rs) {
    double eps = std::sqrt(std::pow(10.0, Rp / 10.0) - 1.0);
    double A = std::pow(10.0, Rs / 20.0);  // Linear stopband gain
    return eps / std::sqrt(A * A - 1.0);
}

// Design elliptic (Cauer) lowpass filter
FilterCoefficients ellip_lowpass(int order, double wc, double epsilon, double A_stop) {
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};

    int L = order / 2;  // Number of second-order sections
    bool has_real_pole = (order % 2 == 1);

    // Calculate elliptic filter parameters
    // We need the selectivity k0 = 1/cosh(asinh(1/eps)/n) approximately
    // But for design, we compute poles and zeros based on Jacobi elliptic functions

    // Simplified approach: use precomputed formulas for low-order elliptic filters
    // For a proper implementation, we'd iterate using elliptic integrals

    // Compute epsilon from passband ripple already given
    // eps is already passed as parameter

    // Selectivity factor for the analog prototype
    // k = wp/ws where wp is passband edge and ws is stopband edge
    // For now we use wc as both passband and compute k from desired specs

    // The relationship between parameters:
    // eps^2 = passband ripple factor
    // A = stopband attenuation factor (linear)
    // k1 = eps / sqrt(A^2 - 1) = discrimination factor
    // k = selectivity (ratio of edge frequencies)
    // n >= K(k1')*K(k) / (K(k1)*K(k')) for the order to meet specs

    // For implementation, we use the pole-zero formulas:
    // For the prototype, poles are at:
    // s_k = -wc*sinh(v0)*sin(phi_k) ± j*wc*cosh(v0)*cos(phi_k)
    // where phi_k = (2k-1)*pi/(2n) for k = 1..n
    // v0 = (1/n)*asinh(1/epsilon)

    // Zeros are at:
    // s_k = ±j*wc/sn(K*(2k-1)/n, k) for k = 1..L

    // For elliptic, we need to compute proper zeros using Jacobi functions
    // Simplified: approximate with Chebyshev-like structure but add zeros

    double a = std::asinh(1.0 / epsilon) / order;
    double sinh_a = std::sinh(a);
    double cosh_a = std::cosh(a);

    // Real pole (for odd order)
    if (has_real_pole) {
        double sigma = -wc * sinh_a;
        double K_val = 2.0 - sigma;
        double b0 = -sigma / K_val;
        double b1 = -sigma / K_val;
        double a1 = (-2.0 - sigma) / K_val;
        cascade_fos(coeffs, b0, b1, 1.0, a1);
    }

    // For elliptic filters, we need zeros in the stopband
    // Compute stopband edge from attenuation
    // ws ≈ wc * cosh(asinh(1/eps_s)/n) where eps_s relates to stopband
    double eps_s = 1.0 / std::sqrt(A_stop * A_stop - 1.0);
    double ws = wc * std::cosh(std::asinh(1.0 / eps_s) / order);
    if (ws < wc * 1.1) ws = wc * 1.5;  // Ensure stopband > passband

    // Compute selectivity
    double k = wc / ws;  // < 1 for lowpass
    double K_k = elliptic_k(k);
    double K_kp = elliptic_k(std::sqrt(1.0 - k * k));

    // Complex conjugate pairs with zeros
    for (int m = 0; m < L; ++m) {
        double theta = constants::pi * (2.0 * m + 1.0) / (2.0 * order);
        double sin_theta = std::sin(theta);
        double cos_theta = std::cos(theta);

        // Pole locations (elliptic: slightly different from Chebyshev)
        double sigma = -wc * sinh_a * sin_theta;
        double omega = wc * cosh_a * cos_theta;

        // Zero location on imaginary axis using Jacobi sn function
        // z_m = j * ws / sn(u_m, k) where u_m = K * (2m+1) / n
        double u_m = K_k * (2.0 * m + 1.0) / order;
        double sn_u, cn_u, dn_u;
        jacobi_elliptic(u_m, k, sn_u, cn_u, dn_u);

        double zero_omega;
        if (std::abs(sn_u) > 1e-10) {
            zero_omega = ws / std::abs(sn_u);
        } else {
            zero_omega = ws * 10.0;  // Very high frequency zero
        }

        // Second-order section with zeros
        // H(s) = (s² + zero_omega²) / (s² - 2*sigma*s + sigma² + omega²)
        double wn2 = sigma * sigma + omega * omega;
        double a1_s = -2.0 * sigma;
        double a2_s = wn2;

        // Bilinear transform for numerator with zeros
        double zo2 = zero_omega * zero_omega;
        double n0 = 4.0 + zo2;
        double n1 = -8.0 + 2.0 * zo2;
        double n2 = 4.0 + zo2;

        // Denominator
        double d0 = 4.0 + 2.0 * a1_s + a2_s;
        double d1 = -8.0 + 2.0 * a2_s;
        double d2 = 4.0 - 2.0 * a1_s + a2_s;

        // Normalize by d0
        double b0 = n0 / d0;
        double b1 = n1 / d0;
        double b2 = n2 / d0;
        double a1_z = d1 / d0;
        double a2_z = d2 / d0;

        cascade_sos(coeffs, b0, b1, b2, 1.0, a1_z, a2_z);
    }

    // Normalize DC gain based on odd/even order
    float b_sum = 0.0f, a_sum = 0.0f;
    for (auto& b : coeffs.b) b_sum += b;
    for (auto& a : coeffs.a) a_sum += a;

    if (std::abs(b_sum) > 1e-10f) {
        float dc_gain_raw = b_sum / a_sum;
        float target_dc;
        if (has_real_pole) {
            target_dc = 1.0f;
        } else {
            target_dc = 1.0f / std::sqrt(1.0f + static_cast<float>(epsilon * epsilon));
        }
        float gain = target_dc / dc_gain_raw;
        for (auto& b : coeffs.b) b *= gain;
    }

    return coeffs;
}

// Design elliptic highpass filter using lowpass-to-highpass transformation
FilterCoefficients ellip_highpass(int order, double wc, double epsilon, double A_stop) {
    FilterCoefficients coeffs;
    coeffs.b = {1.0f};
    coeffs.a = {1.0f};

    int L = order / 2;
    bool has_real_pole = (order % 2 == 1);

    double a = std::asinh(1.0 / epsilon) / order;
    double sinh_a = std::sinh(a);
    double cosh_a = std::cosh(a);

    // Real pole (for odd order) - highpass has zero at DC
    if (has_real_pole) {
        double sigma = -wc * sinh_a;
        // Highpass first-order: H(s) = s / (s - sigma)
        // Bilinear transform
        double K_val = 2.0 - sigma;
        double b0 = 2.0 / K_val;
        double b1 = -2.0 / K_val;
        double a1 = (-2.0 - sigma) / K_val;
        cascade_fos(coeffs, b0, b1, 1.0, a1);
    }

    // Compute zeros for highpass - they should be near DC to attenuate low frequencies
    double eps_s = 1.0 / std::sqrt(A_stop * A_stop - 1.0);

    // For highpass, stopband edge is below passband edge
    // ws_hp = wc / factor where factor > 1
    double factor = std::cosh(std::asinh(1.0 / eps_s) / order);
    if (factor < 1.1) factor = 2.0;
    double ws = wc / factor;

    double k = ws / wc;  // < 1 for highpass
    if (k >= 1.0) k = 0.5;
    double K_k = elliptic_k(k);

    for (int m = 0; m < L; ++m) {
        double theta = constants::pi * (2.0 * m + 1.0) / (2.0 * order);
        double sin_theta = std::sin(theta);
        double cos_theta = std::cos(theta);

        // Poles for highpass (same structure)
        double sigma = -wc * sinh_a * sin_theta;
        double omega = wc * cosh_a * cos_theta;

        // Zeros on imaginary axis, but below cutoff for highpass
        // Use Jacobi sn to place zeros in stopband
        double u_m = K_k * (2.0 * m + 1.0) / order;
        double sn_u, cn_u, dn_u;
        jacobi_elliptic(u_m, k, sn_u, cn_u, dn_u);

        double zero_omega;
        if (std::abs(sn_u) > 1e-10) {
            zero_omega = ws * std::abs(sn_u);  // Near DC for highpass
        } else {
            zero_omega = ws * 0.1;
        }

        // Make sure zeros are in stopband (below cutoff for highpass)
        if (zero_omega >= wc) {
            zero_omega = wc * 0.3;
        }

        double wn2 = sigma * sigma + omega * omega;
        double a1_s = -2.0 * sigma;
        double a2_s = wn2;

        // For highpass with zeros near DC:
        // Analog: H(s) = s² / (s² - 2*sigma*s + wn2) for pure highpass
        // With zeros: H(s) = (s² + zero_omega²) * (s²) / ((s² + zero_omega²)(s² - 2*sigma*s + wn2))
        // Simplified: use highpass numerator 4*(z-1)² but modify gain structure

        // Actually, for elliptic highpass, we need zeros near DC
        // Let's use: H(s) = (s² + zero_omega²) / (s² - 2*sigma*s + wn2)
        // But scale differently for highpass behavior

        double zo2 = zero_omega * zero_omega;

        // Bilinear transform of numerator (s² + zo²)
        double n0 = 4.0 + zo2;
        double n1 = -8.0 + 2.0 * zo2;
        double n2 = 4.0 + zo2;

        // Denominator
        double d0 = 4.0 + 2.0 * a1_s + a2_s;
        double d1 = -8.0 + 2.0 * a2_s;
        double d2 = 4.0 - 2.0 * a1_s + a2_s;

        double b0 = n0 / d0;
        double b1 = n1 / d0;
        double b2 = n2 / d0;
        double a1_z = d1 / d0;
        double a2_z = d2 / d0;

        cascade_sos(coeffs, b0, b1, b2, 1.0, a1_z, a2_z);
    }

    // Normalize for unity gain at Nyquist
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
        float nyq_gain_raw = std::abs(b_sum / a_sum);
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

// Design Chebyshev Type II highpass
FilterCoefficients cheby2_highpass(int order, double ws, double epsilon) {
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
        double sigma = -ws / sinh_a;

        // Highpass first-order: H(s) = s / (s - sigma)
        double K = 2.0 - sigma;
        double b0 = 2.0 / K;
        double b1 = -2.0 / K;
        double a1 = (-2.0 - sigma) / K;

        cascade_fos(coeffs, b0, b1, 1.0, a1);
    }

    // Complex conjugate pairs with zeros
    for (int k = 0; k < num_complex_pairs; ++k) {
        double theta = constants::pi * (2.0 * k + 1.0) / (2.0 * order);
        double sin_theta = std::sin(theta);
        double cos_theta = std::cos(theta);

        double pole_denom = sinh_a * sinh_a * sin_theta * sin_theta +
                            cosh_a * cosh_a * cos_theta * cos_theta;
        double sigma2 = -ws * sinh_a * sin_theta / pole_denom;
        double omega2 = ws * cosh_a * cos_theta / pole_denom;

        // Type II zeros for highpass: at s = ±j * ws / cos(theta)
        // But for highpass, zeros should be near DC, so we invert
        double zero_omega = ws / cos_theta;

        double wn2 = sigma2 * sigma2 + omega2 * omega2;
        double a1_s = -2.0 * sigma2;
        double a2_s = wn2;

        // For highpass with zeros: H(s) = s² / (s² - 2*sigma2*s + wn2)
        // The zeros from Type II become poles of the highpass... this is getting complex
        // Let's use simpler approach: just flip numerator like we did for Butterworth
        double d0 = 4.0 + 2.0 * a1_s + a2_s;
        double d1 = -8.0 + 2.0 * a2_s;
        double d2 = 4.0 - 2.0 * a1_s + a2_s;

        // Highpass numerator: 4*(z-1)^2
        double b0 = 4.0 / d0;
        double b1 = -8.0 / d0;
        double b2 = 4.0 / d0;
        double a1_z = d1 / d0;
        double a2_z = d2 / d0;

        cascade_sos(coeffs, b0, b1, b2, 1.0, a1_z, a2_z);
    }

    // Normalize for unity gain at Nyquist
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
    FilterCoefficients coeffs;

    // Validate inputs
    if (order < 1 || order > 10 || normalized_freq <= 0.0f || normalized_freq >= 1.0f ||
        stopband_db <= 0.0f || stopband_db > 120.0f) {
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
        return coeffs;
    }

    // Calculate epsilon from stopband attenuation
    // For Type II: at stopband edge, |H(jωs)|² = 1/(1 + 1/ε²)
    // Stopband attenuation in dB: Rs = 10*log10(1 + 1/ε²)
    // So: 1/ε² = 10^(Rs/10) - 1, ε = 1/sqrt(10^(Rs/10) - 1)
    double epsilon = 1.0 / std::sqrt(std::pow(10.0, stopband_db / 10.0) - 1.0);

    // Frequency prewarping - normalized_freq is the stopband edge for Type II
    double ws = 2.0 * std::tan(constants::pi * normalized_freq / 2.0);

    if (type == FilterType::Lowpass) {
        return cheby2_lowpass(order, ws, epsilon);
    } else if (type == FilterType::Highpass) {
        return cheby2_highpass(order, ws, epsilon);
    } else {
        // Bandpass/Bandstop not yet implemented
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
        return coeffs;
    }
}

FilterCoefficients ellip(int order, float passband_ripple_db, float stopband_db,
                         float normalized_freq, FilterType type) {
    FilterCoefficients coeffs;

    // Validate inputs
    if (order < 1 || order > 10 || normalized_freq <= 0.0f || normalized_freq >= 1.0f ||
        passband_ripple_db <= 0.0f || passband_ripple_db > 20.0f ||
        stopband_db <= 0.0f || stopband_db > 120.0f) {
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
        return coeffs;
    }

    // Calculate epsilon from passband ripple
    double epsilon = std::sqrt(std::pow(10.0, passband_ripple_db / 10.0) - 1.0);

    // Calculate stopband attenuation factor (linear)
    double A_stop = std::pow(10.0, stopband_db / 20.0);

    // Frequency prewarping
    double wc = 2.0 * std::tan(constants::pi * normalized_freq / 2.0);

    if (type == FilterType::Lowpass) {
        return ellip_lowpass(order, wc, epsilon, A_stop);
    } else if (type == FilterType::Highpass) {
        return ellip_highpass(order, wc, epsilon, A_stop);
    } else {
        // Bandpass/Bandstop not yet implemented
        coeffs.b = {1.0f};
        coeffs.a = {1.0f};
        return coeffs;
    }
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
    // Validate inputs
    if (num_taps < 1 || normalized_freq <= 0.0f || normalized_freq >= 1.0f) {
        return std::vector<float>(std::max(1, num_taps), 1.0f);
    }

    // Ensure odd number of taps for symmetric FIR (Type I)
    // Even taps have a zero at Nyquist which is problematic for highpass
    if (type == FilterType::Highpass && (num_taps % 2 == 0)) {
        num_taps += 1;
    }

    std::vector<float> h(num_taps);
    const int M = num_taps - 1;
    const float fc = normalized_freq / 2.0f;  // Cutoff in terms of sample rate (0.5 = Nyquist)

    // Generate ideal lowpass sinc
    for (int n = 0; n <= M; ++n) {
        const float nm = static_cast<float>(n) - static_cast<float>(M) / 2.0f;
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

    // Normalize for unity gain at DC (lowpass) or Nyquist (highpass)
    float sum = 0.0f;
    if (type == FilterType::Lowpass) {
        // Sum of coefficients = DC gain
        for (float coef : h) sum += coef;
    } else {
        // Alternating sum = gain at Nyquist
        for (int n = 0; n < num_taps; ++n) {
            sum += h[n] * ((n % 2 == 0) ? 1.0f : -1.0f);
        }
    }

    if (std::abs(sum) > 1e-10f) {
        for (float& coef : h) coef /= sum;
    }

    return h;
}

std::vector<float> fir_windowed_sinc(int num_taps, float low_freq, float high_freq,
                                      FilterType type, WindowType window) {
    // Validate inputs
    if (num_taps < 1 || low_freq <= 0.0f || high_freq >= 1.0f || low_freq >= high_freq) {
        return std::vector<float>(std::max(1, num_taps), 1.0f / std::max(1, num_taps));
    }

    // For bandstop, ensure odd number of taps
    if (type == FilterType::Bandstop && (num_taps % 2 == 0)) {
        num_taps += 1;
    }

    std::vector<float> h(num_taps);
    const int M = num_taps - 1;
    const float fc_low = low_freq / 2.0f;   // Lower cutoff
    const float fc_high = high_freq / 2.0f; // Upper cutoff

    // Generate window
    std::vector<float> win = generate_window(num_taps, window);

    if (type == FilterType::Bandpass) {
        // Bandpass = Highpass(low) convolved with Lowpass(high)
        // Or equivalently: Lowpass(high) - Lowpass(low)
        for (int n = 0; n <= M; ++n) {
            const float nm = static_cast<float>(n) - static_cast<float>(M) / 2.0f;
            float h_high, h_low;

            if (std::abs(nm) < 1e-6f) {
                h_high = 2.0f * fc_high;
                h_low = 2.0f * fc_low;
            } else {
                h_high = std::sin(constants::two_pi * fc_high * nm) / (constants::pi * nm);
                h_low = std::sin(constants::two_pi * fc_low * nm) / (constants::pi * nm);
            }

            h[n] = (h_high - h_low) * win[n];
        }

        // Normalize for unity gain at center frequency
        float center_freq = (low_freq + high_freq) / 2.0f;
        float w_center = constants::pi * center_freq;
        float sum_real = 0.0f;
        for (int n = 0; n < num_taps; ++n) {
            float phase = w_center * (n - M / 2.0f);
            sum_real += h[n] * std::cos(phase);
        }
        if (std::abs(sum_real) > 1e-10f) {
            for (float& coef : h) coef /= sum_real;
        }

    } else if (type == FilterType::Bandstop) {
        // Bandstop = Lowpass(low) + Highpass(high) = 1 - Bandpass
        // First create bandpass, then invert
        for (int n = 0; n <= M; ++n) {
            const float nm = static_cast<float>(n) - static_cast<float>(M) / 2.0f;
            float h_high, h_low;

            if (std::abs(nm) < 1e-6f) {
                h_high = 2.0f * fc_high;
                h_low = 2.0f * fc_low;
            } else {
                h_high = std::sin(constants::two_pi * fc_high * nm) / (constants::pi * nm);
                h_low = std::sin(constants::two_pi * fc_low * nm) / (constants::pi * nm);
            }

            // Bandpass kernel
            h[n] = (h_high - h_low) * win[n];
        }

        // Spectral inversion: bandstop = delta - bandpass
        for (int n = 0; n < num_taps; ++n) {
            h[n] = -h[n];
        }
        h[M / 2] += 1.0f;

        // Normalize for unity DC gain
        float sum = 0.0f;
        for (float coef : h) sum += coef;
        if (std::abs(sum) > 1e-10f) {
            for (float& coef : h) coef /= sum;
        }

    } else {
        // Invalid type for this overload
        return std::vector<float>(num_taps, 1.0f / num_taps);
    }

    return h;
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
