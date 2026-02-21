/**
 * @file fir_filter.hpp
 * @brief FIR filter implementation
 */

#ifndef SPECTRA_FILTERS_FIR_FILTER_HPP
#define SPECTRA_FILTERS_FIR_FILTER_HPP

#include "../core/types.hpp"
#include <vector>
#include <memory>

namespace spectra {

/**
 * @brief Stateless FIR filter application
 *
 * Applies FIR filter to entire signal at once.
 *
 * @param b Filter coefficients (length M)
 * @param input Input signal
 * @param output Output signal (same length as input)
 * @param len Signal length
 */
void fir_filter(const float* b, size_t b_len,
                const float* input, float* output, size_t len);

/**
 * @brief FIR convolution
 *
 * Full convolution: output length = input_len + kernel_len - 1
 *
 * @param input Input signal
 * @param input_len Input length
 * @param kernel Filter kernel
 * @param kernel_len Kernel length
 * @param output Output signal
 */
void fir_convolve(const float* input, size_t input_len,
                  const float* kernel, size_t kernel_len,
                  float* output);

/**
 * @brief Streaming FIR filter with internal state
 *
 * Maintains delay line state between calls for real-time processing.
 */
class StreamingFIRFilter {
public:
    /**
     * @brief Construct streaming FIR filter
     * @param coefficients Filter coefficients
     */
    explicit StreamingFIRFilter(const std::vector<float>& coefficients);
    ~StreamingFIRFilter();

    // Non-copyable, movable
    StreamingFIRFilter(const StreamingFIRFilter&) = delete;
    StreamingFIRFilter& operator=(const StreamingFIRFilter&) = delete;
    StreamingFIRFilter(StreamingFIRFilter&& other) noexcept;
    StreamingFIRFilter& operator=(StreamingFIRFilter&& other) noexcept;

    /**
     * @brief Process a block of samples
     * @param input Input samples
     * @param output Output samples (same length as input)
     * @param count Number of samples
     */
    void process(const float* input, float* output, size_t count);

    /**
     * @brief Process single sample
     * @param sample Input sample
     * @return Filtered output sample
     */
    float process_sample(float sample);

    /**
     * @brief Reset filter state (clear delay line)
     */
    void reset();

    /**
     * @brief Get filter order
     */
    [[nodiscard]] size_t order() const { return coefficients_.size() - 1; }

    /**
     * @brief Get filter length (number of taps)
     */
    [[nodiscard]] size_t length() const { return coefficients_.size(); }

    /**
     * @brief Get group delay in samples
     */
    [[nodiscard]] float group_delay() const {
        return static_cast<float>(coefficients_.size() - 1) / 2.0f;
    }

private:
    std::vector<float> coefficients_;
    std::vector<float> delay_line_;
    size_t delay_index_ = 0;
};

} // namespace spectra

#endif // SPECTRA_FILTERS_FIR_FILTER_HPP
