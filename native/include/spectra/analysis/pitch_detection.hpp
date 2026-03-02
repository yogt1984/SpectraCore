/**
 * @file pitch_detection.hpp
 * @brief Real-time pitch detection using YIN and autocorrelation algorithms
 *
 * Implements the YIN fundamental frequency estimation algorithm
 * (de Cheveigné & Kawahara, 2002) with autocorrelation fallback.
 */

#ifndef SPECTRA_PITCH_DETECTION_HPP
#define SPECTRA_PITCH_DETECTION_HPP

#include <vector>
#include <cstddef>
#include <string>

namespace spectra {

/**
 * @brief Result of pitch detection
 */
struct PitchResult {
    float frequency;      ///< Detected frequency in Hz (0 if no pitch detected)
    float confidence;     ///< Confidence score (0.0 to 1.0)
    bool voiced;          ///< True if signal is voiced (has clear pitch)
    float clarity;        ///< Pitch clarity metric (0.0 to 1.0)

    PitchResult() : frequency(0.0f), confidence(0.0f), voiced(false), clarity(0.0f) {}
};

/**
 * @brief Musical note information
 */
struct MusicalNote {
    std::string name;     ///< Note name (e.g., "A", "C#", "Bb")
    int octave;           ///< Octave number (e.g., 4 for middle C = C4)
    float cents;          ///< Deviation in cents (-50 to +50, 0 = perfectly in tune)
    float frequency;      ///< Exact frequency in Hz

    MusicalNote() : name(""), octave(0), cents(0.0f), frequency(0.0f) {}
};

/**
 * @brief Pitch detection method
 */
enum class PitchMethod {
    YIN,              ///< YIN algorithm (most accurate, default)
    Autocorrelation,  ///< Autocorrelation method (faster, less accurate)
    HPS,              ///< Harmonic Product Spectrum (good for harmonic signals)
    Auto              ///< Automatically choose best method
};

/**
 * @brief Real-time pitch detector
 *
 * Detects fundamental frequency (pitch) from audio signals using
 * YIN algorithm or autocorrelation method.
 *
 * Features:
 * - Sub-sample accuracy via parabolic interpolation
 * - Confidence scoring
 * - Voiced/unvoiced detection
 * - Musical note mapping
 * - Configurable frequency range
 *
 * Example:
 * @code
 * PitchDetector detector(44100, 2048);
 * PitchResult result = detector.detect(audioBuffer, bufferSize);
 * if (result.voiced) {
 *     std::cout << "Frequency: " << result.frequency << " Hz\n";
 *     std::cout << "Confidence: " << result.confidence << "\n";
 * }
 * @endcode
 */
class PitchDetector {
public:
    /**
     * @brief Construct a pitch detector
     *
     * @param sample_rate Sample rate in Hz
     * @param buffer_size Size of audio buffer (must be power of 2, default: 2048)
     * @param min_freq Minimum detectable frequency in Hz (default: 60 Hz)
     * @param max_freq Maximum detectable frequency in Hz (default: 2000 Hz)
     */
    PitchDetector(
        float sample_rate,
        size_t buffer_size = 2048,
        float min_freq = 60.0f,
        float max_freq = 2000.0f
    );

    /**
     * @brief Detect pitch from audio buffer
     *
     * @param buffer Audio samples
     * @param size Number of samples
     * @param method Detection method (default: YIN)
     * @return PitchResult with frequency, confidence, and voicing info
     */
    PitchResult detect(const float* buffer, size_t size, PitchMethod method = PitchMethod::YIN);

    /**
     * @brief Detect pitch and convert to musical note
     *
     * @param buffer Audio samples
     * @param size Number of samples
     * @param a4_freq Reference frequency for A4 (default: 440 Hz)
     * @param method Detection method (default: YIN)
     * @return MusicalNote with note name, octave, and cents deviation
     */
    MusicalNote detectNote(
        const float* buffer,
        size_t size,
        float a4_freq = 440.0f,
        PitchMethod method = PitchMethod::YIN
    );

    /**
     * @brief Set detection threshold for YIN algorithm
     *
     * @param threshold Threshold value (0.0 to 1.0, default: 0.1)
     *                  Lower = more sensitive but more false positives
     *                  Higher = less sensitive but fewer false positives
     */
    void setThreshold(float threshold);

    /**
     * @brief Set minimum confidence for voiced detection
     *
     * @param min_confidence Minimum confidence (0.0 to 1.0, default: 0.5)
     */
    void setMinConfidence(float min_confidence);

    /**
     * @brief Get current sample rate
     */
    float getSampleRate() const { return sample_rate_; }

    /**
     * @brief Get buffer size
     */
    size_t getBufferSize() const { return buffer_size_; }

private:
    // YIN algorithm implementation
    PitchResult detectYIN(const float* buffer, size_t size);

    // Autocorrelation implementation
    PitchResult detectAutocorrelation(const float* buffer, size_t size);

    // Harmonic Product Spectrum implementation
    PitchResult detectHPS(const float* buffer, size_t size);

    // Helper: Compute difference function (YIN step 1)
    void computeDifference(const float* buffer, size_t size, std::vector<float>& diff);

    // Helper: Compute cumulative mean normalized difference (YIN step 2)
    void computeCMND(const std::vector<float>& diff, std::vector<float>& cmnd);

    // Helper: Find absolute threshold (YIN step 3)
    int findThreshold(const std::vector<float>& cmnd, float threshold);

    // Helper: Parabolic interpolation for sub-sample accuracy (YIN step 4)
    float parabolicInterpolation(const std::vector<float>& data, int peak_index);

    // Helper: Autocorrelation computation
    void autocorrelate(const float* buffer, size_t size, std::vector<float>& autocorr);

    // Helper: Find peaks in autocorrelation
    int findFirstPeak(const std::vector<float>& autocorr, int min_lag, int max_lag);

    float sample_rate_;
    size_t buffer_size_;
    float min_freq_;
    float max_freq_;
    float threshold_;
    float min_confidence_;

    // Working buffers (reused to avoid allocation)
    std::vector<float> diff_buffer_;
    std::vector<float> cmnd_buffer_;
    std::vector<float> autocorr_buffer_;
};

/**
 * @brief Convert frequency to musical note
 *
 * @param frequency Frequency in Hz
 * @param a4_freq Reference frequency for A4 (default: 440 Hz)
 * @return MusicalNote with name, octave, and cents deviation
 */
MusicalNote frequencyToNote(float frequency, float a4_freq = 440.0f);

/**
 * @brief Convert musical note to frequency
 *
 * @param note_name Note name (e.g., "A", "C#", "Bb")
 * @param octave Octave number
 * @param a4_freq Reference frequency for A4 (default: 440 Hz)
 * @return Frequency in Hz
 */
float noteToFrequency(const std::string& note_name, int octave, float a4_freq = 440.0f);

/**
 * @brief Get note name from MIDI note number
 *
 * @param midi_note MIDI note number (0-127)
 * @return Note name with octave (e.g., "C4", "A#5")
 */
std::string midiNoteToName(int midi_note);

/**
 * @brief Convert frequency to MIDI note number
 *
 * @param frequency Frequency in Hz
 * @param a4_freq Reference frequency for A4 (default: 440 Hz)
 * @return MIDI note number (0-127, fractional for in-between notes)
 */
float frequencyToMIDI(float frequency, float a4_freq = 440.0f);

} // namespace spectra

#endif // SPECTRA_PITCH_DETECTION_HPP
