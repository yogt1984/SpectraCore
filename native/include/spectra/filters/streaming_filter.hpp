/**
 * @file streaming_filter.hpp
 * @brief Stateful streaming IIR filter for real-time processing
 */

#ifndef SPECTRA_FILTERS_STREAMING_FILTER_HPP
#define SPECTRA_FILTERS_STREAMING_FILTER_HPP

#include "../core/types.hpp"
#include <vector>

namespace spectra {

/**
 * @brief Streaming IIR filter with internal state
 *
 * Maintains filter state between process() calls for real-time applications.
 * Uses Direct Form II Transposed structure for numerical stability.
 */
class StreamingIIRFilter {
public:
    /**
     * @brief Construct streaming IIR filter
     * @param b Numerator (feedforward) coefficients
     * @param a Denominator (feedback) coefficients
     */
    StreamingIIRFilter(const std::vector<float>& b, const std::vector<float>& a);

    /**
     * @brief Construct from FilterCoefficients
     */
    explicit StreamingIIRFilter(const FilterCoefficients& coeffs);

    ~StreamingIIRFilter() = default;

    // Copyable
    StreamingIIRFilter(const StreamingIIRFilter&) = default;
    StreamingIIRFilter& operator=(const StreamingIIRFilter&) = default;

    // Movable
    StreamingIIRFilter(StreamingIIRFilter&&) noexcept = default;
    StreamingIIRFilter& operator=(StreamingIIRFilter&&) noexcept = default;

    /**
     * @brief Process a block of samples
     * @param input Input samples
     * @param output Output samples (must be same size as input)
     * @param count Number of samples to process
     */
    void process(const float* input, float* output, size_t count);

    /**
     * @brief Process single sample
     * @param sample Input sample
     * @return Filtered output sample
     */
    float process_sample(float sample);

    /**
     * @brief Process in-place
     * @param data Input/output buffer
     * @param count Number of samples
     */
    void process_inplace(float* data, size_t count);

    /**
     * @brief Reset filter state to zero
     */
    void reset();

    /**
     * @brief Set filter state for steady-state startup
     * @param initial_value Expected DC level of input signal
     */
    void set_steady_state(float initial_value);

    /**
     * @brief Get filter order
     */
    [[nodiscard]] size_t order() const {
        return std::max(b_.size(), a_.size()) - 1;
    }

    /**
     * @brief Get numerator coefficients
     */
    [[nodiscard]] const std::vector<float>& b() const { return b_; }

    /**
     * @brief Get denominator coefficients
     */
    [[nodiscard]] const std::vector<float>& a() const { return a_; }

private:
    std::vector<float> b_;      // Numerator coefficients
    std::vector<float> a_;      // Denominator coefficients
    std::vector<float> state_;  // Filter state (Direct Form II Transposed)

    void normalize_coefficients();
};

/**
 * @brief Cascaded biquad filter for higher-order filtering
 *
 * Implements filter as cascade of second-order sections (SOS) for
 * improved numerical stability with high-order filters.
 */
class BiquadCascade {
public:
    /**
     * @brief Single biquad section coefficients
     */
    struct Section {
        float b0, b1, b2;  // Numerator
        float a1, a2;      // Denominator (a0 = 1)
    };

    /**
     * @brief Construct from second-order sections
     * @param sections Vector of biquad sections
     */
    explicit BiquadCascade(const std::vector<Section>& sections);

    /**
     * @brief Construct from filter coefficients (auto-convert to SOS)
     */
    explicit BiquadCascade(const FilterCoefficients& coeffs);

    /**
     * @brief Process a block of samples
     */
    void process(const float* input, float* output, size_t count);

    /**
     * @brief Process single sample
     */
    float process_sample(float sample);

    /**
     * @brief Reset all section states
     */
    void reset();

    /**
     * @brief Get number of sections
     */
    [[nodiscard]] size_t num_sections() const { return sections_.size(); }

private:
    struct SectionState {
        Section coeffs;
        float z1 = 0.0f, z2 = 0.0f;  // State variables
    };

    std::vector<SectionState> sections_;
};

/**
 * @brief Convert transfer function to second-order sections
 *
 * Equivalent to MATLAB: sos = tf2sos(b, a)
 */
std::vector<BiquadCascade::Section> tf2sos(const FilterCoefficients& coeffs);

} // namespace spectra

#endif // SPECTRA_FILTERS_STREAMING_FILTER_HPP
