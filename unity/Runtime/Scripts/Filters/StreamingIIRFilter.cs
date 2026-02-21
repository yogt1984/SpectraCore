using System;
using Microsoft.Win32.SafeHandles;
using System.Runtime.InteropServices;

namespace Spectra
{
    /// <summary>
    /// Streaming IIR filter for real-time audio processing.
    /// </summary>
    public class StreamingIIRFilter : IDisposable
    {
        private StreamingFilterHandle handle;
        private bool disposed;

        /// <summary>
        /// Create a streaming IIR filter from coefficients.
        /// </summary>
        public StreamingIIRFilter(float[] b, float[] a)
        {
            IntPtr ptr = SpectraNative.spectra_streaming_filter_create(b, b.Length, a, a.Length);
            if (ptr == IntPtr.Zero)
            {
                throw new SpectraException("Failed to create streaming filter");
            }
            handle = new StreamingFilterHandle(ptr);
        }

        /// <summary>
        /// Create from filter coefficients tuple.
        /// </summary>
        public StreamingIIRFilter((float[] b, float[] a) coeffs)
            : this(coeffs.b, coeffs.a)
        {
        }

        /// <summary>
        /// Process a block of samples.
        /// </summary>
        public void Process(float[] input, float[] output)
        {
            if (input.Length != output.Length)
                throw new ArgumentException("Input and output must have same length");

            ThrowIfDisposed();

            int result = SpectraNative.spectra_streaming_filter_process(
                handle.DangerousGetHandle(), input, output, input.Length);

            if (result != 0)
                throw new SpectraException("Failed to process samples");
        }

        /// <summary>
        /// Process and return new array.
        /// </summary>
        public float[] Process(float[] input)
        {
            float[] output = new float[input.Length];
            Process(input, output);
            return output;
        }

        /// <summary>
        /// Reset filter state.
        /// </summary>
        public void Reset()
        {
            ThrowIfDisposed();
            SpectraNative.spectra_streaming_filter_reset(handle.DangerousGetHandle());
        }

        private void ThrowIfDisposed()
        {
            if (disposed || handle.IsInvalid)
                throw new ObjectDisposedException(nameof(StreamingIIRFilter));
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

        ~StreamingIIRFilter()
        {
            Dispose(false);
        }
    }

    internal class StreamingFilterHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        public StreamingFilterHandle(IntPtr handle) : base(true)
        {
            SetHandle(handle);
        }

        protected override bool ReleaseHandle()
        {
            SpectraNative.spectra_streaming_filter_destroy(handle);
            return true;
        }
    }
}
