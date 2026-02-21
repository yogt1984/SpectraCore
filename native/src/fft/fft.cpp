/**
 * @file fft.cpp
 * @brief FFT implementation wrapping KissFFT
 */

#include "spectra/fft/fft.hpp"
#include "kiss_fftr.h"
#include <cmath>

namespace spectra {

struct FFT::Impl {
    kiss_fftr_cfg cfg_forward;
    kiss_fftr_cfg cfg_inverse;
    std::vector<kiss_fft_cpx> spectrum_buffer;

    Impl(size_t size) {
        cfg_forward = kiss_fftr_alloc(static_cast<int>(size), 0, nullptr, nullptr);
        cfg_inverse = kiss_fftr_alloc(static_cast<int>(size), 1, nullptr, nullptr);
        spectrum_buffer.resize(size / 2 + 1);
    }

    ~Impl() {
        kiss_fft_free(cfg_forward);
        kiss_fft_free(cfg_inverse);
    }
};

FFT::FFT(size_t size) : size_(size), impl_(std::make_unique<Impl>(size)) {}

FFT::~FFT() = default;

FFT::FFT(FFT&& other) noexcept = default;
FFT& FFT::operator=(FFT&& other) noexcept = default;

void FFT::forward(const float* input, Complex* output) {
    kiss_fftr(impl_->cfg_forward, input, impl_->spectrum_buffer.data());

    for (size_t i = 0; i < spectrum_size(); ++i) {
        output[i].real = impl_->spectrum_buffer[i].r;
        output[i].imag = impl_->spectrum_buffer[i].i;
    }
}

void FFT::inverse(const Complex* input, float* output) {
    for (size_t i = 0; i < spectrum_size(); ++i) {
        impl_->spectrum_buffer[i].r = input[i].real;
        impl_->spectrum_buffer[i].i = input[i].imag;
    }

    kiss_fftri(impl_->cfg_inverse, impl_->spectrum_buffer.data(), output);

    // Normalize
    const float scale = 1.0f / static_cast<float>(size_);
    for (size_t i = 0; i < size_; ++i) {
        output[i] *= scale;
    }
}

void FFT::forward(const float* input, float* output_real, float* output_imag) {
    kiss_fftr(impl_->cfg_forward, input, impl_->spectrum_buffer.data());

    for (size_t i = 0; i < spectrum_size(); ++i) {
        output_real[i] = impl_->spectrum_buffer[i].r;
        output_imag[i] = impl_->spectrum_buffer[i].i;
    }
}

void FFT::inverse(const float* input_real, const float* input_imag, float* output) {
    for (size_t i = 0; i < spectrum_size(); ++i) {
        impl_->spectrum_buffer[i].r = input_real[i];
        impl_->spectrum_buffer[i].i = input_imag[i];
    }

    kiss_fftri(impl_->cfg_inverse, impl_->spectrum_buffer.data(), output);

    const float scale = 1.0f / static_cast<float>(size_);
    for (size_t i = 0; i < size_; ++i) {
        output[i] *= scale;
    }
}

void FFT::magnitude_spectrum(const float* input, float* output) {
    kiss_fftr(impl_->cfg_forward, input, impl_->spectrum_buffer.data());

    for (size_t i = 0; i < spectrum_size(); ++i) {
        const float r = impl_->spectrum_buffer[i].r;
        const float im = impl_->spectrum_buffer[i].i;
        output[i] = std::sqrt(r * r + im * im);
    }
}

void FFT::power_spectrum(const float* input, float* output) {
    kiss_fftr(impl_->cfg_forward, input, impl_->spectrum_buffer.data());

    for (size_t i = 0; i < spectrum_size(); ++i) {
        const float r = impl_->spectrum_buffer[i].r;
        const float im = impl_->spectrum_buffer[i].i;
        output[i] = r * r + im * im;
    }
}

void compute_magnitude(const Complex* spectrum, float* magnitude, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        magnitude[i] = spectrum[i].magnitude();
    }
}

void compute_phase(const Complex* spectrum, float* phase, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        phase[i] = spectrum[i].phase();
    }
}

void magnitude_to_db(const float* magnitude, float* db, size_t size, float ref_value) {
    const float min_db = -120.0f;
    for (size_t i = 0; i < size; ++i) {
        if (magnitude[i] > 0.0f) {
            db[i] = 20.0f * std::log10(magnitude[i] / ref_value);
            if (db[i] < min_db) db[i] = min_db;
        } else {
            db[i] = min_db;
        }
    }
}

} // namespace spectra
