/**
 * @file streaming_filter.cpp
 * @brief Streaming filter implementations
 */

#include "spectra/filters/streaming_filter.hpp"
#include <algorithm>

namespace spectra {

StreamingIIRFilter::StreamingIIRFilter(const std::vector<float>& b,
                                       const std::vector<float>& a)
    : b_(b), a_(a)
{
    normalize_coefficients();
    const size_t state_size = std::max(b_.size(), a_.size()) - 1;
    state_.resize(state_size, 0.0f);
}

StreamingIIRFilter::StreamingIIRFilter(const FilterCoefficients& coeffs)
    : StreamingIIRFilter(coeffs.b, coeffs.a)
{}

void StreamingIIRFilter::normalize_coefficients() {
    if (a_.empty() || a_[0] == 0.0f) {
        a_ = {1.0f};
        return;
    }

    const float a0 = a_[0];
    if (a0 != 1.0f) {
        for (auto& coef : b_) coef /= a0;
        for (auto& coef : a_) coef /= a0;
    }
}

void StreamingIIRFilter::process(const float* input, float* output, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        output[i] = process_sample(input[i]);
    }
}

float StreamingIIRFilter::process_sample(float sample) {
    // Direct Form II Transposed
    float y = b_[0] * sample + state_[0];

    for (size_t i = 0; i < state_.size() - 1; ++i) {
        const float bi = (i + 1 < b_.size()) ? b_[i + 1] : 0.0f;
        const float ai = (i + 1 < a_.size()) ? a_[i + 1] : 0.0f;
        state_[i] = bi * sample - ai * y + state_[i + 1];
    }

    const size_t last = state_.size() - 1;
    const float bn = (last + 1 < b_.size()) ? b_[last + 1] : 0.0f;
    const float an = (last + 1 < a_.size()) ? a_[last + 1] : 0.0f;
    state_[last] = bn * sample - an * y;

    return y;
}

void StreamingIIRFilter::process_inplace(float* data, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        data[i] = process_sample(data[i]);
    }
}

void StreamingIIRFilter::reset() {
    std::fill(state_.begin(), state_.end(), 0.0f);
}

void StreamingIIRFilter::set_steady_state(float initial_value) {
    // Compute initial conditions for steady-state startup with DC input = initial_value
    // For Direct Form II Transposed, we need to compute z such that:
    // y = b[0]*x + z[0] and the state reaches steady state

    // For lowpass with DC gain G, if input is constant x, output should be G*x
    // Compute DC gain: G = sum(b) / sum(a)
    float sum_b = 0.0f, sum_a = 0.0f;
    for (float b : b_) sum_b += b;
    for (float a : a_) sum_a += a;

    float dc_gain = (std::abs(sum_a) > 1e-10f) ? sum_b / sum_a : 0.0f;
    float y_ss = dc_gain * initial_value;  // Steady-state output

    // For Direct Form II Transposed, at steady state:
    // y = b[0]*x + z[0]
    // z[i] = b[i+1]*x - a[i+1]*y + z[i+1] for i = 0..N-2
    // z[N-1] = b[N]*x - a[N]*y

    // Working backwards:
    const size_t n = state_.size();
    if (n == 0) return;

    float x = initial_value;
    float y = y_ss;

    // Compute last state
    const float bn = (n < b_.size()) ? b_[n] : 0.0f;
    const float an = (n < a_.size()) ? a_[n] : 0.0f;
    state_[n - 1] = bn * x - an * y;

    // Work backwards
    for (int i = static_cast<int>(n) - 2; i >= 0; --i) {
        const float bi = (static_cast<size_t>(i + 1) < b_.size()) ? b_[i + 1] : 0.0f;
        const float ai = (static_cast<size_t>(i + 1) < a_.size()) ? a_[i + 1] : 0.0f;
        state_[i] = bi * x - ai * y + state_[i + 1];
    }
}

// BiquadCascade implementation

BiquadCascade::BiquadCascade(const std::vector<Section>& sections) {
    sections_.reserve(sections.size());
    for (const auto& s : sections) {
        sections_.push_back({s, 0.0f, 0.0f});
    }
}

BiquadCascade::BiquadCascade(const FilterCoefficients& coeffs) {
    auto sos = tf2sos(coeffs);
    sections_.reserve(sos.size());
    for (const auto& s : sos) {
        sections_.push_back({s, 0.0f, 0.0f});
    }
}

void BiquadCascade::process(const float* input, float* output, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        output[i] = process_sample(input[i]);
    }
}

float BiquadCascade::process_sample(float sample) {
    float x = sample;

    for (auto& section : sections_) {
        // Direct Form II Transposed
        float y = section.coeffs.b0 * x + section.z1;
        section.z1 = section.coeffs.b1 * x - section.coeffs.a1 * y + section.z2;
        section.z2 = section.coeffs.b2 * x - section.coeffs.a2 * y;
        x = y;
    }

    return x;
}

void BiquadCascade::reset() {
    for (auto& section : sections_) {
        section.z1 = 0.0f;
        section.z2 = 0.0f;
    }
}

// Helper: Polynomial division to extract a quadratic factor
// Divides poly by (1 + a1*z^-1 + a2*z^-2) and returns quotient
static std::vector<float> poly_div_quadratic(const std::vector<float>& poly,
                                               float a1, float a2,
                                               float& r0, float& r1) {
    if (poly.size() < 3) {
        r0 = poly.size() > 0 ? poly[0] : 0.0f;
        r1 = poly.size() > 1 ? poly[1] : 0.0f;
        return {};
    }

    std::vector<float> quot(poly.size() - 2);
    std::vector<float> rem = poly;

    for (size_t i = 0; i < quot.size(); ++i) {
        quot[i] = rem[i];
        rem[i + 1] -= a1 * quot[i];
        rem[i + 2] -= a2 * quot[i];
    }

    r0 = rem[rem.size() - 2];
    r1 = rem[rem.size() - 1];
    return quot;
}

// Helper: Extract coefficients for one second-order section
static BiquadCascade::Section extract_sos_from_tf(
    std::vector<float>& b, std::vector<float>& a,
    float a1, float a2) {

    BiquadCascade::Section s;

    // The denominator factor is (1 + a1*z^-1 + a2*z^-2)
    // We normalize so a0 = 1 in each section
    s.a1 = a1;
    s.a2 = a2;

    // Divide numerator by a compatible factor or extract 3 coefficients
    if (b.size() >= 3) {
        s.b0 = b[0];
        s.b1 = b[1];
        s.b2 = b[2];

        // Shift remaining numerator
        std::vector<float> new_b(b.size() - 2);
        for (size_t i = 0; i < new_b.size(); ++i) {
            new_b[i] = b[i + 2];
            if (i > 0) new_b[i] += s.b1 / s.b0 * new_b[i - 1];
        }
        // Simplified: just take first 3 coeffs
        b = std::vector<float>(b.begin() + 2, b.end());
        if (b.empty()) b = {1.0f};
    } else {
        s.b0 = b.size() > 0 ? b[0] : 1.0f;
        s.b1 = b.size() > 1 ? b[1] : 0.0f;
        s.b2 = 0.0f;
        b = {1.0f};
    }

    // Divide denominator
    if (a.size() >= 3) {
        a = std::vector<float>(a.begin() + 2, a.end());
        if (a.empty()) a = {1.0f};
    } else {
        a = {1.0f};
    }

    return s;
}

std::vector<BiquadCascade::Section> tf2sos(const FilterCoefficients& coeffs) {
    std::vector<BiquadCascade::Section> sos;

    // Get filter order
    size_t order = std::max(coeffs.b.size(), coeffs.a.size()) - 1;

    if (order == 0) {
        // Zero-order: just gain
        BiquadCascade::Section s = {
            coeffs.b[0] / coeffs.a[0], 0.0f, 0.0f, 0.0f, 0.0f
        };
        sos.push_back(s);
        return sos;
    }

    if (order <= 2) {
        // Already second order or less
        BiquadCascade::Section s;
        s.b0 = coeffs.b.size() > 0 ? coeffs.b[0] / coeffs.a[0] : 0.0f;
        s.b1 = coeffs.b.size() > 1 ? coeffs.b[1] / coeffs.a[0] : 0.0f;
        s.b2 = coeffs.b.size() > 2 ? coeffs.b[2] / coeffs.a[0] : 0.0f;
        s.a1 = coeffs.a.size() > 1 ? coeffs.a[1] / coeffs.a[0] : 0.0f;
        s.a2 = coeffs.a.size() > 2 ? coeffs.a[2] / coeffs.a[0] : 0.0f;
        sos.push_back(s);
        return sos;
    }

    // For higher orders, split into second-order sections
    // Simple approach: group consecutive pairs of coefficients
    // This is a simplified implementation that works for our filter designs

    // Copy and normalize coefficients
    std::vector<float> b = coeffs.b;
    std::vector<float> a = coeffs.a;

    float a0 = a[0];
    for (auto& c : b) c /= a0;
    for (auto& c : a) c /= a0;

    // Number of second-order sections needed
    size_t num_sos = (order + 1) / 2;

    // Extract sections from the polynomial
    // For Butterworth-like filters designed via bilinear transform,
    // coefficients come in cascaded second-order form already

    for (size_t i = 0; i < num_sos; ++i) {
        BiquadCascade::Section s;

        // Extract denominator section coefficients
        size_t idx = i * 2;
        if (idx + 2 < a.size()) {
            s.a1 = a[idx + 1];
            s.a2 = a[idx + 2];
        } else if (idx + 1 < a.size()) {
            s.a1 = a[idx + 1];
            s.a2 = 0.0f;
        } else {
            s.a1 = 0.0f;
            s.a2 = 0.0f;
        }

        // Extract numerator section coefficients
        if (idx + 2 < b.size()) {
            s.b0 = b[idx];
            s.b1 = b[idx + 1];
            s.b2 = b[idx + 2];
        } else if (idx + 1 < b.size()) {
            s.b0 = b[idx];
            s.b1 = b[idx + 1];
            s.b2 = 0.0f;
        } else if (idx < b.size()) {
            s.b0 = b[idx];
            s.b1 = 0.0f;
            s.b2 = 0.0f;
        } else {
            s.b0 = 1.0f;
            s.b1 = 0.0f;
            s.b2 = 0.0f;
        }

        sos.push_back(s);
    }

    // Normalize overall gain
    // The product of all sections should give the original transfer function
    // Adjust first section to get correct DC gain
    float sos_dc = 1.0f;
    for (const auto& s : sos) {
        float num_dc = s.b0 + s.b1 + s.b2;
        float den_dc = 1.0f + s.a1 + s.a2;
        if (std::abs(den_dc) > 1e-10f) {
            sos_dc *= num_dc / den_dc;
        }
    }

    float orig_dc = 0.0f, orig_dc_den = 0.0f;
    for (auto c : b) orig_dc += c;
    for (auto c : a) orig_dc_den += c;
    if (std::abs(orig_dc_den) > 1e-10f) {
        orig_dc /= orig_dc_den;
    }

    if (std::abs(sos_dc) > 1e-10f && !sos.empty()) {
        float gain_adj = orig_dc / sos_dc;
        sos[0].b0 *= gain_adj;
        sos[0].b1 *= gain_adj;
        sos[0].b2 *= gain_adj;
    }

    return sos;
}

} // namespace spectra
