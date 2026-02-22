/**
 * @file iir_filter.cpp
 * @brief IIR filter implementation
 */

#include "spectra/filters/iir_filter.hpp"
#include <algorithm>
#include <cstring>

namespace spectra {

void lfilter(const float* b, size_t b_len,
             const float* a, size_t a_len,
             const float* input, float* output, size_t len) {
    if (len == 0 || b_len == 0 || a_len == 0) return;

    // Direct Form II Transposed
    const size_t order = std::max(b_len, a_len) - 1;

    // Handle zero-order case (pure gain)
    if (order == 0) {
        const float gain = b[0] / a[0];
        for (size_t n = 0; n < len; ++n) {
            output[n] = gain * input[n];
        }
        return;
    }

    std::vector<float> state(order, 0.0f);

    // Normalize by a[0]
    const float a0 = a[0];

    for (size_t n = 0; n < len; ++n) {
        float y = (b[0] / a0) * input[n] + state[0];

        for (size_t i = 0; i < order - 1; ++i) {
            const float bi = (i + 1 < b_len) ? b[i + 1] / a0 : 0.0f;
            const float ai = (i + 1 < a_len) ? a[i + 1] / a0 : 0.0f;
            state[i] = bi * input[n] - ai * y + state[i + 1];
        }

        const float bn = (order < b_len) ? b[order] / a0 : 0.0f;
        const float an = (order < a_len) ? a[order] / a0 : 0.0f;
        state[order - 1] = bn * input[n] - an * y;

        output[n] = y;
    }
}

void lfilter_zi(const float* b, size_t b_len,
                const float* a, size_t a_len,
                const float* input, float* output, size_t len,
                const float* zi, float* zf) {
    if (len == 0 || b_len == 0 || a_len == 0) return;

    const size_t order = std::max(b_len, a_len) - 1;

    // Handle zero-order case (pure gain)
    if (order == 0) {
        const float gain = b[0] / a[0];
        for (size_t n = 0; n < len; ++n) {
            output[n] = gain * input[n];
        }
        return;
    }

    std::vector<float> state(order);

    if (zi) {
        std::memcpy(state.data(), zi, order * sizeof(float));
    } else {
        std::fill(state.begin(), state.end(), 0.0f);
    }

    const float a0 = a[0];

    for (size_t n = 0; n < len; ++n) {
        float y = (b[0] / a0) * input[n] + state[0];

        for (size_t i = 0; i < order - 1; ++i) {
            const float bi = (i + 1 < b_len) ? b[i + 1] / a0 : 0.0f;
            const float ai = (i + 1 < a_len) ? a[i + 1] / a0 : 0.0f;
            state[i] = bi * input[n] - ai * y + state[i + 1];
        }

        const float bn = (order < b_len) ? b[order] / a0 : 0.0f;
        const float an = (order < a_len) ? a[order] / a0 : 0.0f;
        state[order - 1] = bn * input[n] - an * y;

        output[n] = y;
    }

    if (zf) {
        std::memcpy(zf, state.data(), order * sizeof(float));
    }
}

void filtfilt(const float* b, size_t b_len,
              const float* a, size_t a_len,
              const float* input, float* output, size_t len) {
    if (len == 0) return;

    // Pad length
    const size_t order = std::max(b_len, a_len) - 1;
    const size_t pad_len = 3 * order;

    // Create padded signal with reflected edges
    std::vector<float> padded(len + 2 * pad_len);

    // Reflect at start
    for (size_t i = 0; i < pad_len; ++i) {
        padded[i] = 2.0f * input[0] - input[pad_len - i];
    }

    // Copy signal
    std::memcpy(padded.data() + pad_len, input, len * sizeof(float));

    // Reflect at end
    for (size_t i = 0; i < pad_len; ++i) {
        padded[len + pad_len + i] = 2.0f * input[len - 1] - input[len - 2 - i];
    }

    // Forward filter
    std::vector<float> forward_out(padded.size());
    lfilter(b, b_len, a, a_len, padded.data(), forward_out.data(), padded.size());

    // Reverse
    std::reverse(forward_out.begin(), forward_out.end());

    // Backward filter
    std::vector<float> backward_out(padded.size());
    lfilter(b, b_len, a, a_len, forward_out.data(), backward_out.data(), padded.size());

    // Reverse again and extract
    std::reverse(backward_out.begin(), backward_out.end());
    std::memcpy(output, backward_out.data() + pad_len, len * sizeof(float));
}

void lfilter_ic(const float* b, size_t b_len,
                const float* a, size_t a_len,
                float* zi) {
    if (b_len == 0 || a_len == 0) return;

    // Compute initial conditions for step response
    const size_t order = std::max(b_len, a_len) - 1;

    // Zero-order filter has no state
    if (order == 0) return;

    // Sum of b coefficients
    float sum_b = 0.0f;
    for (size_t i = 0; i < b_len; ++i) {
        sum_b += b[i];
    }

    // Sum of a coefficients
    float sum_a = 0.0f;
    for (size_t i = 0; i < a_len; ++i) {
        sum_a += a[i];
    }

    // DC gain
    const float dc_gain = (std::abs(sum_a) > 1e-10f) ? sum_b / sum_a : 0.0f;

    // Simple approximation: set all zi to dc_gain
    for (size_t i = 0; i < order; ++i) {
        zi[i] = dc_gain;
    }
}

} // namespace spectra
