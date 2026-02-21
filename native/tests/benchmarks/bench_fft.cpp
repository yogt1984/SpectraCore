/**
 * @file bench_fft.cpp
 * @brief FFT performance benchmarks
 */

#include "spectra/fft/fft.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>

using namespace spectra;

template<typename Func>
double benchmark(Func f, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        f();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / static_cast<double>(iterations);
}

int main() {
    std::cout << "FFT Benchmark\n";
    std::cout << "=============\n\n";

    std::vector<size_t> sizes = {256, 512, 1024, 2048, 4096, 8192};

    for (size_t N : sizes) {
        FFT fft(N);
        std::vector<float> signal(N);
        std::vector<Complex> spectrum(fft.spectrum_size());

        // Initialize signal
        for (size_t i = 0; i < N; ++i) {
            signal[i] = std::sin(0.01f * i) + 0.5f * std::cos(0.03f * i);
        }

        int iterations = 10000;
        double us_per_fft = benchmark([&]() {
            fft.forward(signal.data(), spectrum.data());
        }, iterations);

        std::cout << "N=" << N << ": " << us_per_fft << " us/FFT\n";
    }

    return 0;
}
