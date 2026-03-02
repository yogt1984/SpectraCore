using System;
using Microsoft.Win32.SafeHandles;

namespace Spectra
{
    /// <summary>
    /// Real-time pitch detection using YIN and autocorrelation algorithms.
    /// </summary>
    /// <remarks>
    /// Detects fundamental frequency (pitch) from audio signals.
    /// Supports multiple detection methods including YIN (most accurate),
    /// autocorrelation (faster), and HPS (good for harmonic signals).
    ///
    /// <example>
    /// Basic usage:
    /// <code>
    /// var detector = new PitchDetector(44100, 2048);
    /// var result = detector.Detect(audioBuffer);
    /// if (result.Voiced)
    /// {
    ///     Debug.Log($"Pitch: {result.Frequency} Hz, Confidence: {result.Confidence}");
    /// }
    /// </code>
    ///
    /// Musical note detection:
    /// <code>
    /// var note = detector.DetectNote(audioBuffer);
    /// if (!string.IsNullOrEmpty(note.Name))
    /// {
    ///     Debug.Log($"Note: {note.Name}{note.Octave}, Cents: {note.Cents:F1}");
    /// }
    /// </code>
    /// </example>
    /// </remarks>
    public class PitchDetector : IDisposable
    {
        private PitchHandle handle;
        private bool disposed;
        private readonly float sampleRate;
        private readonly int bufferSize;

        /// <summary>
        /// Sample rate in Hz.
        /// </summary>
        public float SampleRate => sampleRate;

        /// <summary>
        /// Buffer size in samples.
        /// </summary>
        public int BufferSize => bufferSize;

        /// <summary>
        /// Create a pitch detector.
        /// </summary>
        /// <param name="sampleRate">Sample rate in Hz (e.g., 44100)</param>
        /// <param name="bufferSize">Buffer size in samples (must be power of 2, default: 2048)</param>
        /// <param name="minFreq">Minimum detectable frequency in Hz (default: 60)</param>
        /// <param name="maxFreq">Maximum detectable frequency in Hz (default: 2000)</param>
        /// <exception cref="SpectraException">Thrown if detector creation fails</exception>
        public PitchDetector(float sampleRate, int bufferSize = 2048, float minFreq = 60f, float maxFreq = 2000f)
        {
            this.sampleRate = sampleRate;
            this.bufferSize = bufferSize;

            IntPtr ptr = SpectraNative.spectra_pitch_create(sampleRate, bufferSize, minFreq, maxFreq);
            if (ptr == IntPtr.Zero)
            {
                throw new SpectraException("Failed to create pitch detector");
            }

            handle = new PitchHandle(ptr);
        }

        /// <summary>
        /// Detect pitch from audio buffer.
        /// </summary>
        /// <param name="buffer">Audio samples</param>
        /// <param name="method">Detection method (default: YIN)</param>
        /// <returns>Pitch result with frequency, confidence, and voicing info</returns>
        /// <exception cref="ArgumentNullException">Thrown if buffer is null</exception>
        /// <exception cref="SpectraException">Thrown if detection fails</exception>
        public PitchResult Detect(float[] buffer, PitchMethod method = PitchMethod.YIN)
        {
            ThrowIfDisposed();

            if (buffer == null)
                throw new ArgumentNullException(nameof(buffer));

            int result = SpectraNative.spectra_pitch_detect(
                handle.DangerousGetHandle(),
                buffer,
                buffer.Length,
                (SpectraNative.PitchMethod)method,
                out SpectraNative.PitchResult nativeResult);

            if (result != 0)
            {
                throw new SpectraException("Pitch detection failed");
            }

            return new PitchResult
            {
                Frequency = nativeResult.frequency,
                Confidence = nativeResult.confidence,
                Voiced = nativeResult.voiced != 0,
                Clarity = nativeResult.clarity
            };
        }

        /// <summary>
        /// Detect pitch and convert to musical note.
        /// </summary>
        /// <param name="buffer">Audio samples</param>
        /// <param name="a4Freq">Reference frequency for A4 (default: 440 Hz)</param>
        /// <param name="method">Detection method (default: YIN)</param>
        /// <returns>Musical note with name, octave, and cents deviation</returns>
        /// <exception cref="ArgumentNullException">Thrown if buffer is null</exception>
        /// <exception cref="SpectraException">Thrown if detection fails</exception>
        public MusicalNote DetectNote(float[] buffer, float a4Freq = 440f, PitchMethod method = PitchMethod.YIN)
        {
            ThrowIfDisposed();

            if (buffer == null)
                throw new ArgumentNullException(nameof(buffer));

            int result = SpectraNative.spectra_pitch_detect_note(
                handle.DangerousGetHandle(),
                buffer,
                buffer.Length,
                a4Freq,
                (SpectraNative.PitchMethod)method,
                out SpectraNative.MusicalNote nativeNote);

            if (result != 0)
            {
                throw new SpectraException("Pitch note detection failed");
            }

            return new MusicalNote
            {
                Name = nativeNote.name,
                Octave = nativeNote.octave,
                Cents = nativeNote.cents,
                Frequency = nativeNote.frequency
            };
        }

        /// <summary>
        /// Set detection threshold for YIN algorithm.
        /// </summary>
        /// <param name="threshold">Threshold value (0.0 to 1.0, default: 0.1).
        /// Lower = more sensitive but more false positives.
        /// Higher = less sensitive but fewer false positives.</param>
        public void SetThreshold(float threshold)
        {
            ThrowIfDisposed();
            SpectraNative.spectra_pitch_set_threshold(handle.DangerousGetHandle(), threshold);
        }

        /// <summary>
        /// Set minimum confidence for voiced detection.
        /// </summary>
        /// <param name="minConfidence">Minimum confidence (0.0 to 1.0, default: 0.5)</param>
        public void SetMinConfidence(float minConfidence)
        {
            ThrowIfDisposed();
            SpectraNative.spectra_pitch_set_min_confidence(handle.DangerousGetHandle(), minConfidence);
        }

        /// <summary>
        /// Convert frequency to musical note.
        /// </summary>
        /// <param name="frequency">Frequency in Hz</param>
        /// <param name="a4Freq">Reference frequency for A4 (default: 440 Hz)</param>
        /// <returns>Musical note</returns>
        public static MusicalNote FrequencyToNote(float frequency, float a4Freq = 440f)
        {
            int result = SpectraNative.spectra_frequency_to_note(
                frequency,
                a4Freq,
                out SpectraNative.MusicalNote nativeNote);

            if (result != 0)
            {
                return new MusicalNote();
            }

            return new MusicalNote
            {
                Name = nativeNote.name,
                Octave = nativeNote.octave,
                Cents = nativeNote.cents,
                Frequency = nativeNote.frequency
            };
        }

        /// <summary>
        /// Convert musical note to frequency.
        /// </summary>
        /// <param name="noteName">Note name (e.g., "A", "C#", "Bb")</param>
        /// <param name="octave">Octave number</param>
        /// <param name="a4Freq">Reference frequency for A4 (default: 440 Hz)</param>
        /// <returns>Frequency in Hz</returns>
        public static float NoteToFrequency(string noteName, int octave, float a4Freq = 440f)
        {
            return SpectraNative.spectra_note_to_frequency(noteName, octave, a4Freq);
        }

        private void ThrowIfDisposed()
        {
            if (disposed || handle.IsInvalid)
                throw new ObjectDisposedException(nameof(PitchDetector));
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (disposing)
                {
                    handle?.Dispose();
                }
                disposed = true;
            }
        }

        ~PitchDetector()
        {
            Dispose(false);
        }
    }

    /// <summary>
    /// Result of pitch detection.
    /// </summary>
    public struct PitchResult
    {
        /// <summary>
        /// Detected frequency in Hz (0 if no pitch detected).
        /// </summary>
        public float Frequency { get; set; }

        /// <summary>
        /// Confidence score (0.0 to 1.0).
        /// </summary>
        public float Confidence { get; set; }

        /// <summary>
        /// True if signal is voiced (has clear pitch).
        /// </summary>
        public bool Voiced { get; set; }

        /// <summary>
        /// Pitch clarity metric (0.0 to 1.0).
        /// </summary>
        public float Clarity { get; set; }
    }

    /// <summary>
    /// Musical note information.
    /// </summary>
    public struct MusicalNote
    {
        /// <summary>
        /// Note name (e.g., "A", "C#", "Bb").
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Octave number (e.g., 4 for middle C = C4).
        /// </summary>
        public int Octave { get; set; }

        /// <summary>
        /// Deviation in cents (-50 to +50, 0 = perfectly in tune).
        /// </summary>
        public float Cents { get; set; }

        /// <summary>
        /// Exact frequency in Hz.
        /// </summary>
        public float Frequency { get; set; }
    }

    /// <summary>
    /// Pitch detection method.
    /// </summary>
    public enum PitchMethod
    {
        /// <summary>
        /// YIN algorithm (most accurate, default).
        /// </summary>
        YIN = 0,

        /// <summary>
        /// Autocorrelation method (faster, less accurate).
        /// </summary>
        Autocorrelation = 1,

        /// <summary>
        /// Harmonic Product Spectrum (good for harmonic signals).
        /// </summary>
        HPS = 2,

        /// <summary>
        /// Automatically choose best method.
        /// </summary>
        Auto = 3
    }

    internal class PitchHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        public PitchHandle(IntPtr handle) : base(true)
        {
            SetHandle(handle);
        }

        protected override bool ReleaseHandle()
        {
            SpectraNative.spectra_pitch_destroy(handle);
            return true;
        }
    }
}
