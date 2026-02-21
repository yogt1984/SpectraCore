/**
 * @file stft.hpp
 * @brief Short-Time Fourier Transform
 */

#ifndef SPECTRA_FFT_STFT_HPP
#define SPECTRA_FFT_STFT_HPP

#include "../core/types.hpp"
#include "fft.hpp"
#include <memory>
#include <vector>

namespace spectra {

/**
 * @brief STFT configuration
 */
struct STFTConfig {
    size_t fft_size = 2048;                    ///< FFT size
    size_t hop_size = 512;                     ///< Hop size (overlap = fft_size - hop_size)
    WindowType window = WindowType::Hann;      ///< Window function
    bool center = true;                        ///< Pad signal to center frames
};

/**
 * @brief Short-Time Fourier Transform processor
 *
 * Supports both batch processing (entire signal) and streaming mode
 * (sample-by-sample with frame output).
 */
class STFT {
public:
    /**
     * @brief Construct STFT processor
     * @param config STFT configuration
     */
    explicit STFT(const STFTConfig& config = {});
    ~STFT();

    // Non-copyable, movable
    STFT(const STFT&) = delete;
    STFT& operator=(const STFT&) = delete;
    STFT(STFT&& other) noexcept;
    STFT& operator=(STFT&& other) noexcept;

    /**
     * @brief Process entire signal (batch mode)
     * @param input Input signal
     * @param input_size Number of input samples
     * @param output Output spectrogram (row-major: num_frames x spectrum_size)
     * @param num_frames Output: number of frames generated
     */
    void analyze(const float* input, size_t input_size,
                 Complex* output, size_t* num_frames);

    /**
     * @brief Synthesize signal from spectrogram (inverse STFT)
     * @param input Input spectrogram
     * @param num_frames Number of frames
     * @param output Output signal
     * @param output_size Output: number of samples generated
     */
    void synthesize(const Complex* input, size_t num_frames,
                    float* output, size_t* output_size);

    /**
     * @brief Push samples for streaming analysis
     * @param samples Input samples
     * @param count Number of samples
     */
    void push_samples(const float* samples, size_t count);

    /**
     * @brief Pop a completed frame (streaming mode)
     * @param frame Output frame (spectrum_size() elements)
     * @return true if a frame was available
     */
    bool pop_frame(Complex* frame);

    /**
     * @brief Get number of frames available for reading
     */
    [[nodiscard]] size_t frames_available() const;

    /**
     * @brief Reset streaming state
     */
    void reset();

    /**
     * @brief Calculate number of frames for a given input size
     */
    [[nodiscard]] size_t calc_num_frames(size_t input_size) const;

    /**
     * @brief Get spectrum size per frame
     */
    [[nodiscard]] size_t spectrum_size() const { return config_.fft_size / 2 + 1; }

    /**
     * @brief Get current configuration
     */
    [[nodiscard]] const STFTConfig& config() const { return config_; }

private:
    STFTConfig config_;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Compute spectrogram magnitude
 * @param stft_output Complex STFT output
 * @param magnitude Output magnitude spectrogram
 * @param num_frames Number of frames
 * @param spectrum_size Spectrum size per frame
 */
void stft_magnitude(const Complex* stft_output, float* magnitude,
                    size_t num_frames, size_t spectrum_size);

/**
 * @brief Compute spectrogram power (magnitude squared)
 */
void stft_power(const Complex* stft_output, float* power,
                size_t num_frames, size_t spectrum_size);

/**
 * @brief Compute spectrogram in dB
 */
void stft_db(const Complex* stft_output, float* db,
             size_t num_frames, size_t spectrum_size,
             float ref_value = 1.0f, float floor_db = -120.0f);

} // namespace spectra

#endif // SPECTRA_FFT_STFT_HPP
