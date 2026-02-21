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
    // Set state for DC steady-state
    // This is a simplified version
    for (auto& s : state_) {
        s = initial_value;
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

std::vector<BiquadCascade::Section> tf2sos(const FilterCoefficients& coeffs) {
    // Simplified: just wrap coefficients into single section if order <= 2
    std::vector<BiquadCascade::Section> sos;

    if (coeffs.b.size() <= 3 && coeffs.a.size() <= 3) {
        BiquadCascade::Section s;
        s.b0 = coeffs.b.size() > 0 ? coeffs.b[0] : 0.0f;
        s.b1 = coeffs.b.size() > 1 ? coeffs.b[1] : 0.0f;
        s.b2 = coeffs.b.size() > 2 ? coeffs.b[2] : 0.0f;
        s.a1 = coeffs.a.size() > 1 ? coeffs.a[1] : 0.0f;
        s.a2 = coeffs.a.size() > 2 ? coeffs.a[2] : 0.0f;
        sos.push_back(s);
    } else {
        // TODO: Implement proper pole-zero pairing for higher orders
        BiquadCascade::Section s = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        sos.push_back(s);
    }

    return sos;
}

} // namespace spectra
