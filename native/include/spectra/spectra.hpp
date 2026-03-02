/**
 * @file spectra.hpp
 * @brief Main include header for SpectraCore DSP library
 */

#ifndef SPECTRA_HPP
#define SPECTRA_HPP

// Core
#include "core/types.hpp"
#include "core/ring_buffer.hpp"
#include "core/window_functions.hpp"

// FFT
#include "fft/fft.hpp"
#include "fft/stft.hpp"

// Filters
#include "filters/filter_design.hpp"
#include "filters/fir_filter.hpp"
#include "filters/iir_filter.hpp"
#include "filters/streaming_filter.hpp"

// Analysis
#include "analysis/correlation.hpp"
#include "analysis/psd.hpp"
#include "analysis/hilbert.hpp"
#include "analysis/resample.hpp"
#include "analysis/onset_detection.hpp"
#include "analysis/pitch_detection.hpp"

#endif // SPECTRA_HPP
