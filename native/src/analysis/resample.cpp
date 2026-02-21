/**
 * @file resample.cpp
 * @brief Resampling implementations
 */

#include "spectra/analysis/resample.hpp"
#include "spectra/filters/fir_filter.hpp"
#include <cmath>
#include <algorithm>

namespace spectra {

void resample(const float* input, size_t input_len,
              int p, int q,
              float* output, size_t* output_len) {
    // Polyphase resampling with anti-aliasing filter
    const size_t out_len = resample_output_size(input_len, p, q);
    *output_len = out_len;

    // Design anti-aliasing filter
    const float cutoff = 0.5f / std::max(p, q);
    const int filter_len = 2 * std::max(p, q) * 10 + 1;
    std::vector<float> h(filter_len);

    // Windowed sinc
    const int M = filter_len - 1;
    for (int n = 0; n <= M; ++n) {
        const float nm = n - M / 2.0f;
        if (std::abs(nm) < 1e-6f) {
            h[n] = 2.0f * cutoff;
        } else {
            h[n] = std::sin(constants::two_pi * cutoff * nm) / (constants::pi * nm);
        }
        // Hann window
        h[n] *= 0.5f * (1.0f - std::cos(constants::two_pi * n / M));
    }

    // Scale for unity gain
    float sum = 0.0f;
    for (int n = 0; n < filter_len; ++n) sum += h[n];
    for (int n = 0; n < filter_len; ++n) h[n] /= sum;

    // Upsample, filter, downsample
    std::vector<float> upsampled(input_len * p, 0.0f);
    for (size_t i = 0; i < input_len; ++i) {
        upsampled[i * p] = input[i] * static_cast<float>(p);
    }

    std::vector<float> filtered(upsampled.size());
    fir_filter(h.data(), h.size(), upsampled.data(), filtered.data(), upsampled.size());

    // Downsample
    for (size_t i = 0; i < out_len; ++i) {
        const size_t idx = i * q;
        output[i] = idx < filtered.size() ? filtered[idx] : 0.0f;
    }
}

void upsample(const float* input, size_t input_len,
              int factor, float* output) {
    const size_t output_len = input_len * factor;

    for (size_t i = 0; i < output_len; ++i) {
        if (i % factor == 0) {
            output[i] = input[i / factor];
        } else {
            output[i] = 0.0f;
        }
    }
}

void downsample(const float* input, size_t input_len,
                int factor, float* output, int phase) {
    size_t out_idx = 0;
    for (size_t i = phase; i < input_len; i += factor) {
        output[out_idx++] = input[i];
    }
}

void decimate(const float* input, size_t input_len,
              int factor,
              float* output, size_t* output_len) {
    // Design lowpass filter at cutoff = 0.8 / factor
    const float cutoff = 0.8f / factor;
    const int filter_len = 8 * factor + 1;
    std::vector<float> h(filter_len);

    const int M = filter_len - 1;
    for (int n = 0; n <= M; ++n) {
        const float nm = n - M / 2.0f;
        if (std::abs(nm) < 1e-6f) {
            h[n] = 2.0f * cutoff;
        } else {
            h[n] = std::sin(constants::two_pi * cutoff * nm) / (constants::pi * nm);
        }
        h[n] *= 0.54f - 0.46f * std::cos(constants::two_pi * n / M);  // Hamming
    }

    // Filter
    std::vector<float> filtered(input_len);
    fir_filter(h.data(), h.size(), input, filtered.data(), input_len);

    // Downsample
    *output_len = (input_len + factor - 1) / factor;
    downsample(filtered.data(), input_len, factor, output, 0);
}

void interp(const float* input, size_t input_len,
            int factor, float* output) {
    // Design lowpass filter
    const float cutoff = 0.5f / factor;
    const int filter_len = 8 * factor + 1;
    std::vector<float> h(filter_len);

    const int M = filter_len - 1;
    for (int n = 0; n <= M; ++n) {
        const float nm = n - M / 2.0f;
        if (std::abs(nm) < 1e-6f) {
            h[n] = 2.0f * cutoff;
        } else {
            h[n] = std::sin(constants::two_pi * cutoff * nm) / (constants::pi * nm);
        }
        h[n] *= 0.54f - 0.46f * std::cos(constants::two_pi * n / M);
    }

    // Normalize for unity gain at interpolated points
    float sum = 0.0f;
    for (int n = 0; n < filter_len; ++n) sum += h[n];
    for (int n = 0; n < filter_len; ++n) h[n] *= factor / sum;

    // Upsample
    const size_t output_len = input_len * factor;
    std::vector<float> upsampled(output_len, 0.0f);
    for (size_t i = 0; i < input_len; ++i) {
        upsampled[i * factor] = input[i];
    }

    // Filter
    fir_filter(h.data(), h.size(), upsampled.data(), output, output_len);
}

void resample_linear(const float* input, size_t input_len,
                     float* output, size_t output_len) {
    const float ratio = static_cast<float>(input_len - 1) / static_cast<float>(output_len - 1);

    for (size_t i = 0; i < output_len; ++i) {
        const float pos = i * ratio;
        const size_t idx0 = static_cast<size_t>(pos);
        const size_t idx1 = std::min(idx0 + 1, input_len - 1);
        const float frac = pos - idx0;

        output[i] = input[idx0] * (1.0f - frac) + input[idx1] * frac;
    }
}

void resample_cubic(const float* input, size_t input_len,
                    float* output, size_t output_len) {
    const float ratio = static_cast<float>(input_len - 1) / static_cast<float>(output_len - 1);

    for (size_t i = 0; i < output_len; ++i) {
        const float pos = i * ratio;
        const int idx1 = static_cast<int>(pos);
        const float t = pos - idx1;

        // Clamp indices
        const int idx0 = std::max(0, idx1 - 1);
        const int idx2 = std::min(static_cast<int>(input_len - 1), idx1 + 1);
        const int idx3 = std::min(static_cast<int>(input_len - 1), idx1 + 2);

        const float p0 = input[idx0];
        const float p1 = input[idx1];
        const float p2 = input[idx2];
        const float p3 = input[idx3];

        // Catmull-Rom spline
        output[i] = 0.5f * ((2.0f * p1) +
                           (-p0 + p2) * t +
                           (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
                           (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
    }
}

} // namespace spectra
