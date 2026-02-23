using System;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace Spectra
{
    /// <summary>
    /// Short-Time Fourier Transform analyzer for time-frequency analysis.
    /// </summary>
    public class STFTAnalyzer : IDisposable
    {
        private STFTHandle handle;
        private bool disposed;
        private int spectrumSize;

        /// <summary>
        /// FFT size used for analysis.
        /// </summary>
        public int FftSize { get; }

        /// <summary>
        /// Hop size (number of samples between frames).
        /// </summary>
        public int HopSize { get; }

        /// <summary>
        /// Size of the spectrum output (FFT size / 2 + 1 for real signals).
        /// </summary>
        public int SpectrumSize => spectrumSize;

        /// <summary>
        /// Create an STFT analyzer.
        /// </summary>
        /// <param name="fftSize">FFT size (must be power of 2)</param>
        /// <param name="hopSize">Hop size in samples</param>
        /// <param name="window">Window function type</param>
        /// <param name="center">If true, center the windows</param>
        public STFTAnalyzer(int fftSize, int hopSize, WindowType window = WindowType.Hann, bool center = true)
        {
            var config = new SpectraNative.STFTConfig
            {
                fftSize = fftSize,
                hopSize = hopSize,
                window = (int)window,
                center = center ? 1 : 0
            };

            IntPtr ptr = SpectraNative.spectra_stft_create(ref config);
            if (ptr == IntPtr.Zero)
            {
                throw new SpectraException("Failed to create STFT analyzer");
            }

            handle = new STFTHandle(ptr);
            FftSize = fftSize;
            HopSize = hopSize;
            spectrumSize = fftSize / 2 + 1;
        }

        /// <summary>
        /// Analyze an entire signal and return all frames.
        /// </summary>
        /// <param name="input">Input signal</param>
        /// <returns>Tuple of (real components, imaginary components, number of frames)</returns>
        public (float[] real, float[] imag, int numFrames) Analyze(float[] input)
        {
            ThrowIfDisposed();

            int numFrames = SpectraNative.spectra_stft_num_frames(input.Length, FftSize, HopSize);
            float[] real = new float[numFrames * spectrumSize];
            float[] imag = new float[numFrames * spectrumSize];

            int result = SpectraNative.spectra_stft_analyze(
                handle.DangerousGetHandle(),
                input, input.Length,
                real, imag,
                ref numFrames);

            if (result != 0)
            {
                throw new SpectraException("Failed to analyze signal");
            }

            return (real, imag, numFrames);
        }

        /// <summary>
        /// Push samples for streaming analysis.
        /// </summary>
        /// <param name="samples">Input samples</param>
        public void PushSamples(float[] samples)
        {
            ThrowIfDisposed();

            int result = SpectraNative.spectra_stft_push_samples(
                handle.DangerousGetHandle(),
                samples,
                samples.Length);

            if (result != 0)
            {
                throw new SpectraException("Failed to push samples");
            }
        }

        /// <summary>
        /// Pop a frame from the streaming buffer.
        /// </summary>
        /// <param name="real">Real components output</param>
        /// <param name="imag">Imaginary components output</param>
        /// <returns>True if a frame was available</returns>
        public bool PopFrame(out float[] real, out float[] imag)
        {
            ThrowIfDisposed();

            real = new float[spectrumSize];
            imag = new float[spectrumSize];

            int result = SpectraNative.spectra_stft_pop_frame(
                handle.DangerousGetHandle(),
                real, imag);

            return result == 0;
        }

        /// <summary>
        /// Get the number of frames available in the streaming buffer.
        /// </summary>
        public int FramesAvailable
        {
            get
            {
                ThrowIfDisposed();
                return SpectraNative.spectra_stft_frames_available(handle.DangerousGetHandle());
            }
        }

        /// <summary>
        /// Compute magnitude spectrum from real and imaginary components.
        /// </summary>
        public static float[] ComputeMagnitude(float[] real, float[] imag)
        {
            if (real.Length != imag.Length)
                throw new ArgumentException("Real and imaginary arrays must have the same length");

            float[] magnitude = new float[real.Length];
            for (int i = 0; i < real.Length; i++)
            {
                magnitude[i] = (float)Math.Sqrt(real[i] * real[i] + imag[i] * imag[i]);
            }
            return magnitude;
        }

        /// <summary>
        /// Compute power spectrum from real and imaginary components.
        /// </summary>
        public static float[] ComputePower(float[] real, float[] imag)
        {
            if (real.Length != imag.Length)
                throw new ArgumentException("Real and imaginary arrays must have the same length");

            float[] power = new float[real.Length];
            for (int i = 0; i < real.Length; i++)
            {
                power[i] = real[i] * real[i] + imag[i] * imag[i];
            }
            return power;
        }

        /// <summary>
        /// Convert power to decibels.
        /// </summary>
        public static float[] PowerToDb(float[] power, float referenceLevel = 1.0f)
        {
            float[] db = new float[power.Length];
            for (int i = 0; i < power.Length; i++)
            {
                db[i] = 10.0f * (float)Math.Log10(Math.Max(power[i], 1e-10f) / referenceLevel);
            }
            return db;
        }

        private void ThrowIfDisposed()
        {
            if (disposed || handle.IsInvalid)
                throw new ObjectDisposedException(nameof(STFTAnalyzer));
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

        ~STFTAnalyzer()
        {
            Dispose(false);
        }
    }

    internal class STFTHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        public STFTHandle(IntPtr handle) : base(true)
        {
            SetHandle(handle);
        }

        protected override bool ReleaseHandle()
        {
            SpectraNative.spectra_stft_destroy(handle);
            return true;
        }
    }
}
