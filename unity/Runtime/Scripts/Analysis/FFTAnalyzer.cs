using System;
using Microsoft.Win32.SafeHandles;

namespace Spectra
{
    /// <summary>
    /// FFT analyzer for spectral analysis.
    /// </summary>
    public class FFTAnalyzer : IDisposable
    {
        private FFTHandle handle;
        private bool disposed;
        private int size;
        private int spectrumSize;
        private float[] spectrumReal;
        private float[] spectrumImag;

        /// <summary>
        /// FFT size.
        /// </summary>
        public int Size => size;

        /// <summary>
        /// Spectrum size (N/2 + 1).
        /// </summary>
        public int SpectrumSize => spectrumSize;

        /// <summary>
        /// Create an FFT analyzer.
        /// </summary>
        public FFTAnalyzer(int fftSize)
        {
            IntPtr ptr = SpectraNative.spectra_fft_create(fftSize);
            if (ptr == IntPtr.Zero)
            {
                throw new SpectraException("Failed to create FFT");
            }

            handle = new FFTHandle(ptr);
            size = SpectraNative.spectra_fft_size(ptr);
            spectrumSize = SpectraNative.spectra_fft_spectrum_size(ptr);
            spectrumReal = new float[spectrumSize];
            spectrumImag = new float[spectrumSize];
        }

        /// <summary>
        /// Compute forward FFT and return magnitude spectrum.
        /// </summary>
        public float[] GetMagnitudeSpectrum(float[] input)
        {
            ThrowIfDisposed();

            if (input.Length != size)
                throw new ArgumentException($"Input length must be {size}");

            SpectraNative.spectra_fft_forward(
                handle.DangerousGetHandle(), input, spectrumReal, spectrumImag);

            float[] magnitude = new float[spectrumSize];
            for (int i = 0; i < spectrumSize; i++)
            {
                float r = spectrumReal[i];
                float im = spectrumImag[i];
                magnitude[i] = (float)Math.Sqrt(r * r + im * im);
            }

            return magnitude;
        }

        /// <summary>
        /// Compute forward FFT and return power spectrum.
        /// </summary>
        public float[] GetPowerSpectrum(float[] input)
        {
            ThrowIfDisposed();

            if (input.Length != size)
                throw new ArgumentException($"Input length must be {size}");

            SpectraNative.spectra_fft_forward(
                handle.DangerousGetHandle(), input, spectrumReal, spectrumImag);

            float[] power = new float[spectrumSize];
            for (int i = 0; i < spectrumSize; i++)
            {
                float r = spectrumReal[i];
                float im = spectrumImag[i];
                power[i] = r * r + im * im;
            }

            return power;
        }

        /// <summary>
        /// Compute forward FFT and return complex spectrum.
        /// </summary>
        public (float[] real, float[] imag) Forward(float[] input)
        {
            ThrowIfDisposed();

            if (input.Length != size)
                throw new ArgumentException($"Input length must be {size}");

            float[] real = new float[size];
            float[] imag = new float[size];

            SpectraNative.spectra_fft_forward(
                handle.DangerousGetHandle(), input, real, imag);

            return (real, imag);
        }

        /// <summary>
        /// Compute forward FFT.
        /// </summary>
        public void Forward(float[] input, float[] outputReal, float[] outputImag)
        {
            ThrowIfDisposed();
            SpectraNative.spectra_fft_forward(
                handle.DangerousGetHandle(), input, outputReal, outputImag);
        }

        /// <summary>
        /// Compute inverse FFT.
        /// </summary>
        public void Inverse(float[] inputReal, float[] inputImag, float[] output)
        {
            ThrowIfDisposed();
            SpectraNative.spectra_fft_inverse(
                handle.DangerousGetHandle(), inputReal, inputImag, output);
        }

        private void ThrowIfDisposed()
        {
            if (disposed || handle.IsInvalid)
                throw new ObjectDisposedException(nameof(FFTAnalyzer));
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

        ~FFTAnalyzer()
        {
            Dispose(false);
        }
    }

    internal class FFTHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        public FFTHandle(IntPtr handle) : base(true)
        {
            SetHandle(handle);
        }

        protected override bool ReleaseHandle()
        {
            SpectraNative.spectra_fft_destroy(handle);
            return true;
        }
    }
}
