/**
 * @file fir_filter.cpp
 * @brief FIR filter implementation
 */

#include "spectra/filters/fir_filter.hpp"

namespace spectra {

void fir_filter(const float* b, size_t b_len,
                const float* input, float* output, size_t len) {
    for (size_t n = 0; n < len; ++n) {
        float sum = 0.0f;
        for (size_t k = 0; k < b_len; ++k) {
            if (n >= k) {
                sum += b[k] * input[n - k];
            }
        }
        output[n] = sum;
    }
}

void fir_convolve(const float* input, size_t input_len,
                  const float* kernel, size_t kernel_len,
                  float* output) {
    const size_t output_len = input_len + kernel_len - 1;

    for (size_t n = 0; n < output_len; ++n) {
        float sum = 0.0f;
        for (size_t k = 0; k < kernel_len; ++k) {
            if (n >= k && n - k < input_len) {
                sum += kernel[k] * input[n - k];
            }
        }
        output[n] = sum;
    }
}

StreamingFIRFilter::StreamingFIRFilter(const std::vector<float>& coefficients)
    : coefficients_(coefficients)
    , delay_line_(coefficients.size(), 0.0f)
    , delay_index_(0)
{}

StreamingFIRFilter::~StreamingFIRFilter() = default;

StreamingFIRFilter::StreamingFIRFilter(StreamingFIRFilter&& other) noexcept = default;
StreamingFIRFilter& StreamingFIRFilter::operator=(StreamingFIRFilter&& other) noexcept = default;

void StreamingFIRFilter::process(const float* input, float* output, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        output[i] = process_sample(input[i]);
    }
}

float StreamingFIRFilter::process_sample(float sample) {
    // Insert new sample into delay line
    delay_line_[delay_index_] = sample;

    // Compute output
    float sum = 0.0f;
    size_t idx = delay_index_;
    for (size_t k = 0; k < coefficients_.size(); ++k) {
        sum += coefficients_[k] * delay_line_[idx];
        if (idx == 0) {
            idx = coefficients_.size() - 1;
        } else {
            --idx;
        }
    }

    // Advance delay line index
    delay_index_ = (delay_index_ + 1) % coefficients_.size();

    return sum;
}

void StreamingFIRFilter::reset() {
    std::fill(delay_line_.begin(), delay_line_.end(), 0.0f);
    delay_index_ = 0;
}

} // namespace spectra
