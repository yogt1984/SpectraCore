/**
 * @file pitch_detection.cpp
 * @brief Implementation of pitch detection algorithms
 */

#include "spectra/analysis/pitch_detection.hpp"
#include <cmath>
#include <algorithm>
#include <limits>

namespace spectra {

namespace {

// Note names (chromatic scale)
const char* NOTE_NAMES[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// Number of semitones in an octave
const int SEMITONES_PER_OCTAVE = 12;

// MIDI note number for A4
const int MIDI_A4 = 69;

} // anonymous namespace

// ============================================================================
// PitchDetector Implementation
// ============================================================================

PitchDetector::PitchDetector(
    float sample_rate,
    size_t buffer_size,
    float min_freq,
    float max_freq
)
    : sample_rate_(sample_rate)
    , buffer_size_(buffer_size)
    , min_freq_(min_freq)
    , max_freq_(max_freq)
    , threshold_(0.1f)
    , min_confidence_(0.5f)
{
    // Preallocate working buffers
    diff_buffer_.resize(buffer_size / 2);
    cmnd_buffer_.resize(buffer_size / 2);
    autocorr_buffer_.resize(buffer_size);
}

PitchResult PitchDetector::detect(const float* buffer, size_t size, PitchMethod method) {
    if (buffer == nullptr || size == 0) {
        return PitchResult();
    }

    switch (method) {
        case PitchMethod::YIN:
            return detectYIN(buffer, size);

        case PitchMethod::Autocorrelation:
            return detectAutocorrelation(buffer, size);

        case PitchMethod::HPS:
            return detectHPS(buffer, size);

        case PitchMethod::Auto:
            // Try YIN first, fall back to autocorrelation
            {
                PitchResult yin_result = detectYIN(buffer, size);
                if (yin_result.confidence > min_confidence_) {
                    return yin_result;
                }
                return detectAutocorrelation(buffer, size);
            }

        default:
            return detectYIN(buffer, size);
    }
}

MusicalNote PitchDetector::detectNote(
    const float* buffer,
    size_t size,
    float a4_freq,
    PitchMethod method
) {
    PitchResult result = detect(buffer, size, method);

    if (!result.voiced || result.frequency <= 0.0f) {
        return MusicalNote();
    }

    return frequencyToNote(result.frequency, a4_freq);
}

void PitchDetector::setThreshold(float threshold) {
    threshold_ = std::max(0.0f, std::min(1.0f, threshold));
}

void PitchDetector::setMinConfidence(float min_confidence) {
    min_confidence_ = std::max(0.0f, std::min(1.0f, min_confidence));
}

// ============================================================================
// YIN Algorithm Implementation
// ============================================================================

PitchResult PitchDetector::detectYIN(const float* buffer, size_t size) {
    PitchResult result;

    // Step 1: Compute difference function
    size_t half_size = size / 2;
    computeDifference(buffer, size, diff_buffer_);

    // Step 2: Compute cumulative mean normalized difference
    computeCMND(diff_buffer_, cmnd_buffer_);

    // Step 3: Absolute threshold
    int min_lag = static_cast<int>(sample_rate_ / max_freq_);
    int max_lag = static_cast<int>(sample_rate_ / min_freq_);
    max_lag = std::min(max_lag, static_cast<int>(half_size - 1));

    int tau = findThreshold(cmnd_buffer_, threshold_);

    if (tau == -1 || tau < min_lag || tau >= max_lag) {
        // No pitch detected
        result.voiced = false;
        result.confidence = 0.0f;
        return result;
    }

    // Step 4: Parabolic interpolation for sub-sample accuracy
    float interpolated_tau = parabolicInterpolation(cmnd_buffer_, tau);

    // Convert period to frequency
    result.frequency = sample_rate_ / interpolated_tau;

    // Confidence and clarity from CMND value
    float cmnd_value = cmnd_buffer_[tau];
    result.confidence = 1.0f - cmnd_value;
    result.clarity = result.confidence;

    // Voiced detection based on confidence
    result.voiced = result.confidence >= min_confidence_;

    return result;
}

void PitchDetector::computeDifference(
    const float* buffer,
    size_t size,
    std::vector<float>& diff
) {
    size_t max_tau = size / 2;

    for (size_t tau = 0; tau < max_tau; ++tau) {
        float sum = 0.0f;

        for (size_t j = 0; j < max_tau; ++j) {
            float delta = buffer[j] - buffer[j + tau];
            sum += delta * delta;
        }

        diff[tau] = sum;
    }
}

void PitchDetector::computeCMND(
    const std::vector<float>& diff,
    std::vector<float>& cmnd
) {
    cmnd[0] = 1.0f; // Special case for tau = 0

    float running_sum = 0.0f;

    for (size_t tau = 1; tau < diff.size(); ++tau) {
        running_sum += diff[tau];

        if (running_sum == 0.0f) {
            cmnd[tau] = 1.0f;
        } else {
            cmnd[tau] = diff[tau] / (running_sum / tau);
        }
    }
}

int PitchDetector::findThreshold(const std::vector<float>& cmnd, float threshold) {
    // Find first tau where cmnd < threshold
    for (size_t tau = 1; tau < cmnd.size(); ++tau) {
        if (cmnd[tau] < threshold) {
            // Search for local minimum after this point
            size_t min_tau = tau;
            float min_value = cmnd[tau];

            // Look ahead for a better minimum
            for (size_t t = tau + 1; t < std::min(cmnd.size(), tau + 50); ++t) {
                if (cmnd[t] < min_value) {
                    min_value = cmnd[t];
                    min_tau = t;
                } else if (cmnd[t] > min_value * 1.3f) {
                    // Gone too far upward, use current minimum
                    break;
                }
            }

            return static_cast<int>(min_tau);
        }
    }

    return -1; // No period found
}

float PitchDetector::parabolicInterpolation(const std::vector<float>& data, int peak_index) {
    if (peak_index <= 0 || peak_index >= static_cast<int>(data.size()) - 1) {
        return static_cast<float>(peak_index);
    }

    float alpha = data[peak_index - 1];
    float beta = data[peak_index];
    float gamma = data[peak_index + 1];

    // Parabolic interpolation formula
    float offset = 0.5f * (alpha - gamma) / (alpha - 2.0f * beta + gamma);

    return peak_index + offset;
}

// ============================================================================
// Autocorrelation Implementation
// ============================================================================

PitchResult PitchDetector::detectAutocorrelation(const float* buffer, size_t size) {
    PitchResult result;

    // Compute autocorrelation
    autocorrelate(buffer, size, autocorr_buffer_);

    // Find first peak after initial descent
    int min_lag = static_cast<int>(sample_rate_ / max_freq_);
    int max_lag = static_cast<int>(sample_rate_ / min_freq_);
    max_lag = std::min(max_lag, static_cast<int>(size - 1));

    int peak = findFirstPeak(autocorr_buffer_, min_lag, max_lag);

    if (peak == -1) {
        result.voiced = false;
        result.confidence = 0.0f;
        return result;
    }

    // Parabolic interpolation for sub-sample accuracy
    float interpolated_peak = parabolicInterpolation(autocorr_buffer_, peak);

    // Convert lag to frequency
    result.frequency = sample_rate_ / interpolated_peak;

    // Confidence from autocorrelation peak height
    float peak_value = autocorr_buffer_[peak];
    float zero_lag_value = autocorr_buffer_[0];

    if (zero_lag_value > 0.0f) {
        result.confidence = peak_value / zero_lag_value;
        result.clarity = result.confidence;
    } else {
        result.confidence = 0.0f;
        result.clarity = 0.0f;
    }

    result.voiced = result.confidence >= min_confidence_;

    return result;
}

void PitchDetector::autocorrelate(
    const float* buffer,
    size_t size,
    std::vector<float>& autocorr
) {
    for (size_t lag = 0; lag < size; ++lag) {
        float sum = 0.0f;

        for (size_t i = 0; i < size - lag; ++i) {
            sum += buffer[i] * buffer[i + lag];
        }

        autocorr[lag] = sum;
    }
}

int PitchDetector::findFirstPeak(
    const std::vector<float>& autocorr,
    int min_lag,
    int max_lag
) {
    // Skip initial descent (first few samples might have high autocorrelation)
    int search_start = std::max(1, min_lag);

    float max_peak = -std::numeric_limits<float>::infinity();
    int peak_index = -1;

    for (int lag = search_start; lag < max_lag; ++lag) {
        // Simple peak finding: higher than neighbors
        if (lag > 0 && lag < static_cast<int>(autocorr.size()) - 1) {
            if (autocorr[lag] > autocorr[lag - 1] &&
                autocorr[lag] > autocorr[lag + 1] &&
                autocorr[lag] > max_peak) {
                max_peak = autocorr[lag];
                peak_index = lag;
            }
        }
    }

    return peak_index;
}

// ============================================================================
// Harmonic Product Spectrum (HPS) Implementation
// ============================================================================

PitchResult PitchDetector::detectHPS(const float* buffer, size_t size) {
    // HPS is best used with frequency domain data
    // For now, fall back to autocorrelation
    // TODO: Implement proper FFT-based HPS
    return detectAutocorrelation(buffer, size);
}

// ============================================================================
// Musical Note Conversion Functions
// ============================================================================

MusicalNote frequencyToNote(float frequency, float a4_freq) {
    MusicalNote note;

    if (frequency <= 0.0f) {
        return note;
    }

    note.frequency = frequency;

    // Convert frequency to MIDI note number (fractional)
    float midi_note = frequencyToMIDI(frequency, a4_freq);

    // Round to nearest semitone
    int midi_int = static_cast<int>(std::round(midi_note));

    // Calculate cents deviation
    note.cents = (midi_note - midi_int) * 100.0f;

    // Extract note name and octave
    int note_class = midi_int % SEMITONES_PER_OCTAVE;
    note.octave = (midi_int / SEMITONES_PER_OCTAVE) - 1;
    note.name = NOTE_NAMES[note_class];

    return note;
}

float noteToFrequency(const std::string& note_name, int octave, float a4_freq) {
    // Find note index in chromatic scale
    int note_index = -1;

    for (int i = 0; i < SEMITONES_PER_OCTAVE; ++i) {
        if (note_name == NOTE_NAMES[i]) {
            note_index = i;
            break;
        }
    }

    if (note_index == -1) {
        return 0.0f; // Invalid note name
    }

    // Calculate MIDI note number
    int midi_note = (octave + 1) * SEMITONES_PER_OCTAVE + note_index;

    // Convert to frequency
    float semitones_from_a4 = midi_note - MIDI_A4;
    return a4_freq * std::pow(2.0f, semitones_from_a4 / 12.0f);
}

std::string midiNoteToName(int midi_note) {
    if (midi_note < 0 || midi_note > 127) {
        return "";
    }

    int note_class = midi_note % SEMITONES_PER_OCTAVE;
    int octave = (midi_note / SEMITONES_PER_OCTAVE) - 1;

    return std::string(NOTE_NAMES[note_class]) + std::to_string(octave);
}

float frequencyToMIDI(float frequency, float a4_freq) {
    if (frequency <= 0.0f || a4_freq <= 0.0f) {
        return 0.0f;
    }

    // MIDI note = 69 + 12 * log2(frequency / 440)
    return MIDI_A4 + 12.0f * std::log2(frequency / a4_freq);
}

} // namespace spectra
