/**
 * @file test_pitch_detection.cpp
 * @brief Unit tests for pitch detection algorithms
 */

#include <gtest/gtest.h>
#include "spectra/analysis/pitch_detection.hpp"
#include <cmath>
#include <vector>

using namespace spectra;

class PitchDetectionTest : public ::testing::Test {
protected:
    static constexpr float SAMPLE_RATE = 44100.0f;
    static constexpr size_t BUFFER_SIZE = 2048;
    static constexpr float PI = 3.14159265358979323846f;

    // Helper to generate a sine wave
    std::vector<float> generateSineWave(float frequency, size_t numSamples, float amplitude = 1.0f) {
        std::vector<float> buffer(numSamples);
        float angularFreq = 2.0f * PI * frequency / SAMPLE_RATE;

        for (size_t i = 0; i < numSamples; ++i) {
            buffer[i] = amplitude * std::sin(angularFreq * i);
        }

        return buffer;
    }

    // Helper to generate silence
    std::vector<float> generateSilence(size_t numSamples) {
        return std::vector<float>(numSamples, 0.0f);
    }

    // Helper to generate white noise
    std::vector<float> generateNoise(size_t numSamples, float amplitude = 0.1f) {
        std::vector<float> buffer(numSamples);

        for (size_t i = 0; i < numSamples; ++i) {
            buffer[i] = amplitude * (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f);
        }

        return buffer;
    }
};

// ============================================================================
// YIN Algorithm Tests
// ============================================================================

TEST_F(PitchDetectionTest, YIN_DetectA440) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(440.0f, BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    EXPECT_TRUE(result.voiced);
    EXPECT_NEAR(result.frequency, 440.0f, 5.0f);  // Within 5 Hz
    EXPECT_GT(result.confidence, 0.7f);  // High confidence
}

TEST_F(PitchDetectionTest, YIN_DetectMiddleC) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    // Middle C = C4 = ~261.63 Hz
    auto buffer = generateSineWave(261.63f, BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    EXPECT_TRUE(result.voiced);
    EXPECT_NEAR(result.frequency, 261.63f, 5.0f);
    EXPECT_GT(result.confidence, 0.7f);
}

TEST_F(PitchDetectionTest, YIN_DetectLowE) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    // Low E (guitar) = E2 = ~82.41 Hz
    auto buffer = generateSineWave(82.41f, BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    EXPECT_TRUE(result.voiced);
    EXPECT_NEAR(result.frequency, 82.41f, 3.0f);
    EXPECT_GT(result.confidence, 0.5f);
}

TEST_F(PitchDetectionTest, YIN_DetectHighE) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    // High E (guitar) = E4 = ~329.63 Hz
    auto buffer = generateSineWave(329.63f, BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    EXPECT_TRUE(result.voiced);
    EXPECT_NEAR(result.frequency, 329.63f, 5.0f);
    EXPECT_GT(result.confidence, 0.7f);
}

TEST_F(PitchDetectionTest, YIN_RejectSilence) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSilence(BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    EXPECT_FALSE(result.voiced);
    EXPECT_LT(result.confidence, 0.5f);
}

TEST_F(PitchDetectionTest, YIN_RejectNoise) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateNoise(BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    // Noise should either be rejected or have very low confidence
    if (result.voiced) {
        EXPECT_LT(result.confidence, 0.6f);
    }
}

TEST_F(PitchDetectionTest, YIN_ThresholdAdjustment) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(440.0f, BUFFER_SIZE, 0.3f);  // Lower amplitude

    // Lower threshold (more sensitive)
    detector.setThreshold(0.05f);
    auto result1 = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    // Higher threshold (less sensitive)
    detector.setThreshold(0.2f);
    auto result2 = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    // Both should detect, but first should have higher confidence
    EXPECT_TRUE(result1.voiced);
    EXPECT_GE(result1.confidence, result2.confidence);
}

TEST_F(PitchDetectionTest, YIN_MinConfidenceAdjustment) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(440.0f, BUFFER_SIZE, 0.5f);

    // Low min confidence
    detector.setMinConfidence(0.3f);
    auto result1 = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    // High min confidence
    detector.setMinConfidence(0.9f);
    auto result2 = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    // First should be voiced, second may not be
    EXPECT_TRUE(result1.voiced);
}

// ============================================================================
// Autocorrelation Tests
// ============================================================================

TEST_F(PitchDetectionTest, Autocorr_DetectA440) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(440.0f, BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::Autocorrelation);

    EXPECT_TRUE(result.voiced);
    EXPECT_NEAR(result.frequency, 440.0f, 10.0f);  // Less accurate than YIN
    EXPECT_GT(result.confidence, 0.6f);
}

TEST_F(PitchDetectionTest, Autocorr_DetectMiddleC) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(261.63f, BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::Autocorrelation);

    EXPECT_TRUE(result.voiced);
    EXPECT_NEAR(result.frequency, 261.63f, 10.0f);
    EXPECT_GT(result.confidence, 0.6f);
}

TEST_F(PitchDetectionTest, Autocorr_RejectSilence) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSilence(BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::Autocorrelation);

    EXPECT_FALSE(result.voiced);
}

// ============================================================================
// Musical Note Detection Tests
// ============================================================================

TEST_F(PitchDetectionTest, Note_DetectA4) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(440.0f, BUFFER_SIZE);
    auto note = detector.detectNote(buffer.data(), buffer.size(), 440.0f, PitchMethod::YIN);

    EXPECT_EQ(note.name, "A");
    EXPECT_EQ(note.octave, 4);
    EXPECT_NEAR(note.cents, 0.0f, 20.0f);  // Within 20 cents of perfect tuning
    EXPECT_NEAR(note.frequency, 440.0f, 5.0f);
}

TEST_F(PitchDetectionTest, Note_DetectC4) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(261.63f, BUFFER_SIZE);
    auto note = detector.detectNote(buffer.data(), buffer.size(), 440.0f, PitchMethod::YIN);

    EXPECT_EQ(note.name, "C");
    EXPECT_EQ(note.octave, 4);
    EXPECT_NEAR(note.cents, 0.0f, 20.0f);
}

TEST_F(PitchDetectionTest, Note_DetectE2) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(82.41f, BUFFER_SIZE);
    auto note = detector.detectNote(buffer.data(), buffer.size(), 440.0f, PitchMethod::YIN);

    EXPECT_EQ(note.name, "E");
    EXPECT_EQ(note.octave, 2);
    EXPECT_NEAR(note.cents, 0.0f, 20.0f);
}

TEST_F(PitchDetectionTest, Note_DetectSharpNote) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    // F#4 = ~369.99 Hz
    auto buffer = generateSineWave(369.99f, BUFFER_SIZE);
    auto note = detector.detectNote(buffer.data(), buffer.size(), 440.0f, PitchMethod::YIN);

    EXPECT_EQ(note.name, "F#");
    EXPECT_EQ(note.octave, 4);
    EXPECT_NEAR(note.cents, 0.0f, 20.0f);
}

TEST_F(PitchDetectionTest, Note_AlternativeTuning) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    // Using A4 = 432 Hz (alternative tuning)
    auto buffer = generateSineWave(432.0f, BUFFER_SIZE);
    auto note = detector.detectNote(buffer.data(), buffer.size(), 432.0f, PitchMethod::YIN);

    EXPECT_EQ(note.name, "A");
    EXPECT_EQ(note.octave, 4);
    EXPECT_NEAR(note.cents, 0.0f, 20.0f);
}

// ============================================================================
// Musical Note Conversion Tests
// ============================================================================

TEST_F(PitchDetectionTest, Convert_FrequencyToNote_A440) {
    auto note = frequencyToNote(440.0f, 440.0f);

    EXPECT_EQ(note.name, "A");
    EXPECT_EQ(note.octave, 4);
    EXPECT_NEAR(note.cents, 0.0f, 1.0f);
    EXPECT_NEAR(note.frequency, 440.0f, 0.1f);
}

TEST_F(PitchDetectionTest, Convert_FrequencyToNote_C4) {
    auto note = frequencyToNote(261.63f, 440.0f);

    EXPECT_EQ(note.name, "C");
    EXPECT_EQ(note.octave, 4);
    EXPECT_NEAR(note.cents, 0.0f, 5.0f);
}

TEST_F(PitchDetectionTest, Convert_FrequencyToNote_Detuned) {
    // 445 Hz is ~20 cents sharp of A4
    auto note = frequencyToNote(445.0f, 440.0f);

    EXPECT_EQ(note.name, "A");
    EXPECT_EQ(note.octave, 4);
    EXPECT_GT(note.cents, 15.0f);
    EXPECT_LT(note.cents, 25.0f);
}

TEST_F(PitchDetectionTest, Convert_NoteToFrequency_A4) {
    float freq = noteToFrequency("A", 4, 440.0f);
    EXPECT_NEAR(freq, 440.0f, 0.1f);
}

TEST_F(PitchDetectionTest, Convert_NoteToFrequency_C4) {
    float freq = noteToFrequency("C", 4, 440.0f);
    EXPECT_NEAR(freq, 261.63f, 0.5f);
}

TEST_F(PitchDetectionTest, Convert_NoteToFrequency_FSharp4) {
    float freq = noteToFrequency("F#", 4, 440.0f);
    EXPECT_NEAR(freq, 369.99f, 0.5f);
}

TEST_F(PitchDetectionTest, Convert_NoteToFrequency_E2) {
    float freq = noteToFrequency("E", 2, 440.0f);
    EXPECT_NEAR(freq, 82.41f, 0.5f);
}

TEST_F(PitchDetectionTest, Convert_RoundTrip) {
    // Frequency -> Note -> Frequency should be consistent
    float originalFreq = 440.0f;
    auto note = frequencyToNote(originalFreq, 440.0f);
    float convertedFreq = noteToFrequency(note.name, note.octave, 440.0f);

    EXPECT_NEAR(convertedFreq, originalFreq, 1.0f);
}

TEST_F(PitchDetectionTest, Convert_MidiNoteName) {
    // MIDI 69 = A4
    std::string noteName = midiNoteToName(69);
    EXPECT_EQ(noteName, "A4");

    // MIDI 60 = C4 (middle C)
    noteName = midiNoteToName(60);
    EXPECT_EQ(noteName, "C4");

    // MIDI 40 = E2 (low E on guitar)
    noteName = midiNoteToName(40);
    EXPECT_EQ(noteName, "E2");
}

TEST_F(PitchDetectionTest, Convert_FrequencyToMIDI) {
    float midi = frequencyToMIDI(440.0f, 440.0f);
    EXPECT_NEAR(midi, 69.0f, 0.1f);

    midi = frequencyToMIDI(261.63f, 440.0f);
    EXPECT_NEAR(midi, 60.0f, 0.5f);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(PitchDetectionTest, EdgeCase_NullPointer) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto result = detector.detect(nullptr, 0, PitchMethod::YIN);

    EXPECT_FALSE(result.voiced);
    EXPECT_EQ(result.frequency, 0.0f);
}

TEST_F(PitchDetectionTest, EdgeCase_EmptyBuffer) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    std::vector<float> buffer;
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    EXPECT_FALSE(result.voiced);
}

TEST_F(PitchDetectionTest, EdgeCase_VeryLowAmplitude) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(440.0f, BUFFER_SIZE, 0.001f);  // Very quiet
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    // May or may not detect, but shouldn't crash
    EXPECT_GE(result.frequency, 0.0f);
}

TEST_F(PitchDetectionTest, EdgeCase_FrequencyBelowRange) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 100.0f, 2000.0f);  // Min = 100 Hz

    auto buffer = generateSineWave(50.0f, BUFFER_SIZE);  // Below range
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    // Should not detect frequency outside range
    EXPECT_FALSE(result.voiced);
}

TEST_F(PitchDetectionTest, EdgeCase_FrequencyAboveRange) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 1000.0f);  // Max = 1000 Hz

    auto buffer = generateSineWave(2000.0f, BUFFER_SIZE);  // Above range
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::YIN);

    // Should not detect frequency outside range
    EXPECT_FALSE(result.voiced);
}

// ============================================================================
// Auto Method Tests
// ============================================================================

TEST_F(PitchDetectionTest, Auto_SelectsBestMethod) {
    PitchDetector detector(SAMPLE_RATE, BUFFER_SIZE, 60.0f, 2000.0f);

    auto buffer = generateSineWave(440.0f, BUFFER_SIZE);
    auto result = detector.detect(buffer.data(), buffer.size(), PitchMethod::Auto);

    // Auto should detect successfully
    EXPECT_TRUE(result.voiced);
    EXPECT_NEAR(result.frequency, 440.0f, 10.0f);
    EXPECT_GT(result.confidence, 0.5f);
}
