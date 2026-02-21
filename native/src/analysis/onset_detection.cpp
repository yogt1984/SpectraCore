/**
 * @file onset_detection.cpp
 * @brief Onset detection implementations
 */

#include "spectra/analysis/onset_detection.hpp"
#include "spectra/core/window_functions.hpp"
#include <algorithm>
#include <cstring>

namespace spectra {

struct OnsetDetector::Impl {
    STFT stft;
    std::vector<float> prev_magnitude;
    std::vector<float> detection_function;
    float samples_processed = 0.0f;
    float last_onset_time = -1000.0f;
    float adaptive_threshold = 0.0f;

    Impl(const OnsetConfig& config)
        : stft(STFTConfig{config.fft_size, config.hop_size, config.window, false})
        , prev_magnitude(config.fft_size / 2 + 1, 0.0f)
    {}
};

OnsetDetector::OnsetDetector(const OnsetConfig& config, float sample_rate)
    : config_(config)
    , sample_rate_(sample_rate)
    , impl_(std::make_unique<Impl>(config))
{}

OnsetDetector::~OnsetDetector() = default;

OnsetDetector::OnsetDetector(OnsetDetector&& other) noexcept = default;
OnsetDetector& OnsetDetector::operator=(OnsetDetector&& other) noexcept = default;

void OnsetDetector::process(const float* samples, size_t count, OnsetCallback callback) {
    impl_->stft.push_samples(samples, count);

    std::vector<Complex> frame(impl_->stft.spectrum_size());
    std::vector<float> magnitude(impl_->stft.spectrum_size());

    while (impl_->stft.pop_frame(frame.data())) {
        // Compute magnitude
        for (size_t i = 0; i < magnitude.size(); ++i) {
            magnitude[i] = frame[i].magnitude();
        }

        // Compute spectral flux
        float flux = spectral_flux(magnitude.data(), impl_->prev_magnitude.data(),
                                   magnitude.size());

        // Update adaptive threshold (simple moving average)
        impl_->adaptive_threshold = 0.9f * impl_->adaptive_threshold + 0.1f * flux;

        // Check for onset
        const float threshold = config_.threshold * impl_->adaptive_threshold * 2.0f;
        const float current_time = impl_->samples_processed / sample_rate_;
        const float min_interval = config_.min_interval_ms / 1000.0f;

        if (flux > threshold &&
            current_time - impl_->last_onset_time > min_interval) {
            if (callback) {
                callback(current_time, flux / (threshold + 0.001f));
            }
            impl_->last_onset_time = current_time;
        }

        // Update state
        std::memcpy(impl_->prev_magnitude.data(), magnitude.data(),
                   magnitude.size() * sizeof(float));
        impl_->samples_processed += config_.hop_size;
    }

    impl_->samples_processed += count - (impl_->samples_processed - count);
}

std::vector<OnsetEvent> OnsetDetector::process(const float* samples, size_t count) {
    std::vector<OnsetEvent> onsets;
    process(samples, count, [&onsets](float time, float strength) {
        onsets.push_back({time, strength});
    });
    return onsets;
}

void OnsetDetector::reset() {
    impl_->stft.reset();
    std::fill(impl_->prev_magnitude.begin(), impl_->prev_magnitude.end(), 0.0f);
    impl_->samples_processed = 0.0f;
    impl_->last_onset_time = -1000.0f;
    impl_->adaptive_threshold = 0.0f;
}

void OnsetDetector::set_threshold(float threshold) {
    config_.threshold = threshold;
}

float OnsetDetector::threshold() const {
    return config_.threshold;
}

float spectral_flux(const float* current, const float* previous, size_t size) {
    float flux = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        const float diff = current[i] - previous[i];
        if (diff > 0.0f) {
            flux += diff;
        }
    }
    return flux;
}

float spectral_flux(const Complex* current, const Complex* previous, size_t size) {
    float flux = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        const float diff = current[i].magnitude() - previous[i].magnitude();
        if (diff > 0.0f) {
            flux += diff;
        }
    }
    return flux;
}

std::vector<OnsetEvent> detect_onsets(const float* signal, size_t len,
                                       const OnsetConfig& config,
                                       float sample_rate) {
    OnsetDetector detector(config, sample_rate);
    return detector.process(signal, len);
}

size_t pick_peaks(const float* detection_function, size_t len,
                  float threshold, size_t min_distance,
                  size_t* peaks) {
    size_t num_peaks = 0;
    size_t last_peak = 0;

    for (size_t i = 1; i < len - 1; ++i) {
        // Check if local maximum
        if (detection_function[i] > detection_function[i - 1] &&
            detection_function[i] > detection_function[i + 1] &&
            detection_function[i] > threshold &&
            (num_peaks == 0 || i - last_peak >= min_distance)) {
            peaks[num_peaks++] = i;
            last_peak = i;
        }
    }

    return num_peaks;
}

} // namespace spectra
