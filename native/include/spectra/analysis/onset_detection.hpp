/**
 * @file onset_detection.hpp
 * @brief Audio onset detection using spectral flux
 */

#ifndef SPECTRA_ANALYSIS_ONSET_DETECTION_HPP
#define SPECTRA_ANALYSIS_ONSET_DETECTION_HPP

#include "../core/types.hpp"
#include "../fft/stft.hpp"
#include <memory>
#include <functional>
#include <vector>

namespace spectra {

/**
 * @brief Onset detection configuration
 */
struct OnsetConfig {
    size_t fft_size = 2048;         ///< FFT size
    size_t hop_size = 512;          ///< Hop size
    float threshold = 0.5f;         ///< Detection threshold (0 to 1)
    float min_interval_ms = 50.0f;  ///< Minimum interval between onsets in ms
    WindowType window = WindowType::Hann;
};

/**
 * @brief Onset event data
 */
struct OnsetEvent {
    float timestamp;    ///< Time in seconds
    float strength;     ///< Detection strength (normalized)
};

/**
 * @brief Callback for onset detection
 */
using OnsetCallback = std::function<void(float timestamp, float strength)>;

/**
 * @brief Real-time onset detector using spectral flux
 *
 * Detects note onsets in audio streams by computing the positive
 * spectral flux (rate of increase in spectral energy).
 */
class OnsetDetector {
public:
    /**
     * @brief Construct onset detector
     * @param config Detection configuration
     * @param sample_rate Audio sample rate in Hz
     */
    OnsetDetector(const OnsetConfig& config, float sample_rate);
    ~OnsetDetector();

    // Non-copyable, movable
    OnsetDetector(const OnsetDetector&) = delete;
    OnsetDetector& operator=(const OnsetDetector&) = delete;
    OnsetDetector(OnsetDetector&&) noexcept;
    OnsetDetector& operator=(OnsetDetector&&) noexcept;

    /**
     * @brief Process audio samples
     *
     * Calls callback for each detected onset.
     *
     * @param samples Audio samples
     * @param count Number of samples
     * @param callback Callback for onset events
     */
    void process(const float* samples, size_t count, OnsetCallback callback);

    /**
     * @brief Process and return detected onsets
     *
     * @param samples Audio samples
     * @param count Number of samples
     * @return Vector of detected onsets
     */
    std::vector<OnsetEvent> process(const float* samples, size_t count);

    /**
     * @brief Reset detector state
     */
    void reset();

    /**
     * @brief Set detection threshold
     * @param threshold New threshold (0 to 1)
     */
    void set_threshold(float threshold);

    /**
     * @brief Get current threshold
     */
    [[nodiscard]] float threshold() const;

    /**
     * @brief Get current configuration
     */
    [[nodiscard]] const OnsetConfig& config() const { return config_; }

private:
    OnsetConfig config_;
    float sample_rate_;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Compute spectral flux from two consecutive spectra
 *
 * Spectral flux is the sum of positive differences between
 * consecutive magnitude spectra.
 *
 * @param current Current magnitude spectrum
 * @param previous Previous magnitude spectrum
 * @param size Spectrum size
 * @return Spectral flux value
 */
float spectral_flux(const float* current, const float* previous, size_t size);

/**
 * @brief Compute spectral flux (complex input)
 */
float spectral_flux(const Complex* current, const Complex* previous, size_t size);

/**
 * @brief Offline onset detection
 *
 * Process entire signal and return all onsets.
 *
 * @param signal Audio signal
 * @param len Signal length
 * @param config Detection configuration
 * @param sample_rate Sample rate in Hz
 * @return Vector of onset events
 */
std::vector<OnsetEvent> detect_onsets(const float* signal, size_t len,
                                       const OnsetConfig& config,
                                       float sample_rate);

/**
 * @brief Peak picking for onset detection function
 *
 * @param detection_function Onset detection function values
 * @param len Length of detection function
 * @param threshold Threshold for peak detection
 * @param min_distance Minimum distance between peaks (in samples)
 * @param peaks Output: indices of detected peaks
 * @return Number of peaks found
 */
size_t pick_peaks(const float* detection_function, size_t len,
                  float threshold, size_t min_distance,
                  size_t* peaks);

} // namespace spectra

#endif // SPECTRA_ANALYSIS_ONSET_DETECTION_HPP
