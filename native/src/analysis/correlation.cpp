/**
 * @file correlation.cpp
 * @brief Correlation implementations
 */

#include "spectra/analysis/correlation.hpp"
#include <cmath>
#include <algorithm>

namespace spectra {

void xcorr(const float* x, size_t x_len,
           const float* y, size_t y_len,
           float* output) {
    const size_t out_len = x_len + y_len - 1;

    for (size_t lag = 0; lag < out_len; ++lag) {
        float sum = 0.0f;
        const int offset = static_cast<int>(lag) - static_cast<int>(y_len - 1);

        for (size_t i = 0; i < x_len; ++i) {
            const int j = static_cast<int>(i) - offset;
            if (j >= 0 && static_cast<size_t>(j) < y_len) {
                sum += x[i] * y[j];
            }
        }
        output[lag] = sum;
    }
}

void xcorr(const float* x, size_t len, float* output) {
    xcorr(x, len, x, len, output);
}

void xcorr(const float* x, size_t x_len,
           const float* y, size_t y_len,
           float* output, size_t maxlag) {
    const size_t center = y_len - 1;

    for (size_t i = 0; i <= 2 * maxlag; ++i) {
        const size_t lag = center - maxlag + i;
        float sum = 0.0f;
        const int offset = static_cast<int>(lag) - static_cast<int>(y_len - 1);

        for (size_t j = 0; j < x_len; ++j) {
            const int k = static_cast<int>(j) - offset;
            if (k >= 0 && static_cast<size_t>(k) < y_len) {
                sum += x[j] * y[k];
            }
        }
        output[i] = sum;
    }
}

void xcorr_normalized(const float* x, size_t x_len,
                      const float* y, size_t y_len,
                      float* output) {
    // Compute unnormalized
    xcorr(x, x_len, y, y_len, output);

    // Compute norms
    float norm_x = 0.0f, norm_y = 0.0f;
    for (size_t i = 0; i < x_len; ++i) norm_x += x[i] * x[i];
    for (size_t i = 0; i < y_len; ++i) norm_y += y[i] * y[i];

    const float scale = std::sqrt(norm_x * norm_y);
    if (scale > 0.0f) {
        const size_t out_len = x_len + y_len - 1;
        for (size_t i = 0; i < out_len; ++i) {
            output[i] /= scale;
        }
    }
}

void xcorr_biased(const float* x, size_t x_len,
                  const float* y, size_t y_len,
                  float* output) {
    xcorr(x, x_len, y, y_len, output);

    const size_t N = std::max(x_len, y_len);
    const size_t out_len = x_len + y_len - 1;
    for (size_t i = 0; i < out_len; ++i) {
        output[i] /= static_cast<float>(N);
    }
}

void xcorr_unbiased(const float* x, size_t x_len,
                    const float* y, size_t y_len,
                    float* output) {
    xcorr(x, x_len, y, y_len, output);

    const size_t out_len = x_len + y_len - 1;
    const int center = static_cast<int>(y_len) - 1;

    for (size_t i = 0; i < out_len; ++i) {
        const int lag = static_cast<int>(i) - center;
        const size_t overlap = std::min(x_len, y_len) -
            static_cast<size_t>(std::abs(lag));
        if (overlap > 0) {
            output[i] /= static_cast<float>(overlap);
        }
    }
}

int find_max_correlation_lag(const float* correlation, size_t corr_len, size_t ref_len) {
    size_t max_idx = 0;
    float max_val = correlation[0];

    for (size_t i = 1; i < corr_len; ++i) {
        if (correlation[i] > max_val) {
            max_val = correlation[i];
            max_idx = i;
        }
    }

    return static_cast<int>(max_idx) - static_cast<int>(ref_len - 1);
}

float corrcoef(const float* x, const float* y, size_t len) {
    // Compute means
    float mean_x = 0.0f, mean_y = 0.0f;
    for (size_t i = 0; i < len; ++i) {
        mean_x += x[i];
        mean_y += y[i];
    }
    mean_x /= static_cast<float>(len);
    mean_y /= static_cast<float>(len);

    // Compute correlation
    float sum_xy = 0.0f, sum_xx = 0.0f, sum_yy = 0.0f;
    for (size_t i = 0; i < len; ++i) {
        const float dx = x[i] - mean_x;
        const float dy = y[i] - mean_y;
        sum_xy += dx * dy;
        sum_xx += dx * dx;
        sum_yy += dy * dy;
    }

    const float denom = std::sqrt(sum_xx * sum_yy);
    return denom > 0.0f ? sum_xy / denom : 0.0f;
}

} // namespace spectra
