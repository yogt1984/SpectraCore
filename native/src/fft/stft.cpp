/**
 * @file stft.cpp
 * @brief STFT implementation
 */

#include "spectra/fft/stft.hpp"
#include "spectra/core/window_functions.hpp"
#include <cstring>

namespace spectra {

struct STFT::Impl {
    FFT fft;
    std::vector<float> window;
    std::vector<float> frame_buffer;
    std::vector<Complex> output_frames;
    size_t buffer_pos = 0;
    size_t frames_ready = 0;

    Impl(const STFTConfig& config)
        : fft(config.fft_size)
        , window(generate_window(config.fft_size, config.window))
        , frame_buffer(config.fft_size, 0.0f)
    {}
};

STFT::STFT(const STFTConfig& config)
    : config_(config)
    , impl_(std::make_unique<Impl>(config))
{}

STFT::~STFT() = default;

STFT::STFT(STFT&& other) noexcept = default;
STFT& STFT::operator=(STFT&& other) noexcept = default;

void STFT::analyze(const float* input, size_t input_size,
                   Complex* output, size_t* num_frames) {
    const size_t n_frames = calc_num_frames(input_size);
    *num_frames = n_frames;

    std::vector<float> windowed_frame(config_.fft_size);

    for (size_t frame = 0; frame < n_frames; ++frame) {
        const size_t start = frame * config_.hop_size;

        // Copy and window
        for (size_t i = 0; i < config_.fft_size; ++i) {
            if (start + i < input_size) {
                windowed_frame[i] = input[start + i] * impl_->window[i];
            } else {
                windowed_frame[i] = 0.0f;
            }
        }

        // FFT
        impl_->fft.forward(windowed_frame.data(), output + frame * spectrum_size());
    }
}

void STFT::synthesize(const Complex* input, size_t num_frames,
                      float* output, size_t* output_size) {
    const size_t out_len = (num_frames - 1) * config_.hop_size + config_.fft_size;
    *output_size = out_len;

    std::memset(output, 0, out_len * sizeof(float));
    std::vector<float> frame(config_.fft_size);

    for (size_t f = 0; f < num_frames; ++f) {
        impl_->fft.inverse(input + f * spectrum_size(), frame.data());

        const size_t start = f * config_.hop_size;
        for (size_t i = 0; i < config_.fft_size && start + i < out_len; ++i) {
            output[start + i] += frame[i] * impl_->window[i];
        }
    }
}

void STFT::push_samples(const float* samples, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        impl_->frame_buffer[impl_->buffer_pos++] = samples[i];

        if (impl_->buffer_pos >= config_.fft_size) {
            // Process frame
            std::vector<float> windowed(config_.fft_size);
            for (size_t j = 0; j < config_.fft_size; ++j) {
                windowed[j] = impl_->frame_buffer[j] * impl_->window[j];
            }

            impl_->output_frames.resize(impl_->output_frames.size() + spectrum_size());
            impl_->fft.forward(windowed.data(),
                impl_->output_frames.data() + (impl_->frames_ready * spectrum_size()));
            impl_->frames_ready++;

            // Shift buffer by hop size
            std::memmove(impl_->frame_buffer.data(),
                        impl_->frame_buffer.data() + config_.hop_size,
                        (config_.fft_size - config_.hop_size) * sizeof(float));
            impl_->buffer_pos = config_.fft_size - config_.hop_size;
        }
    }
}

bool STFT::pop_frame(Complex* frame) {
    if (impl_->frames_ready == 0) return false;

    std::memcpy(frame, impl_->output_frames.data(), spectrum_size() * sizeof(Complex));
    impl_->output_frames.erase(impl_->output_frames.begin(),
                                impl_->output_frames.begin() + spectrum_size());
    impl_->frames_ready--;
    return true;
}

size_t STFT::frames_available() const {
    return impl_->frames_ready;
}

void STFT::reset() {
    impl_->buffer_pos = 0;
    impl_->frames_ready = 0;
    impl_->output_frames.clear();
    std::fill(impl_->frame_buffer.begin(), impl_->frame_buffer.end(), 0.0f);
}

size_t STFT::calc_num_frames(size_t input_size) const {
    if (input_size < config_.fft_size) return 0;
    return 1 + (input_size - config_.fft_size) / config_.hop_size;
}

void stft_magnitude(const Complex* stft_output, float* magnitude,
                    size_t num_frames, size_t spectrum_size) {
    for (size_t i = 0; i < num_frames * spectrum_size; ++i) {
        magnitude[i] = stft_output[i].magnitude();
    }
}

void stft_power(const Complex* stft_output, float* power,
                size_t num_frames, size_t spectrum_size) {
    for (size_t i = 0; i < num_frames * spectrum_size; ++i) {
        power[i] = stft_output[i].magnitude_squared();
    }
}

void stft_db(const Complex* stft_output, float* db,
             size_t num_frames, size_t spectrum_size,
             float ref_value, float floor_db) {
    for (size_t i = 0; i < num_frames * spectrum_size; ++i) {
        float mag = stft_output[i].magnitude();
        if (mag > 0.0f) {
            db[i] = 20.0f * std::log10(mag / ref_value);
            if (db[i] < floor_db) db[i] = floor_db;
        } else {
            db[i] = floor_db;
        }
    }
}

} // namespace spectra
