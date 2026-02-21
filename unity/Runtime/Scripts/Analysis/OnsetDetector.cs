using System;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace Spectra
{
    /// <summary>
    /// Real-time onset detector using spectral flux.
    /// </summary>
    public class OnsetDetector : IDisposable
    {
        private OnsetHandle handle;
        private bool disposed;
        private SpectraNative.OnsetCallback nativeCallback;
        private GCHandle callbackHandle;

        /// <summary>
        /// Event fired when an onset is detected.
        /// </summary>
        public event Action<float, float> OnOnsetDetected;

        /// <summary>
        /// Create an onset detector.
        /// </summary>
        public OnsetDetector(OnsetConfig config, float sampleRate)
        {
            var nativeConfig = new SpectraNative.OnsetConfig
            {
                fftSize = config.FftSize,
                hopSize = config.HopSize,
                threshold = config.Threshold,
                minIntervalMs = config.MinIntervalMs
            };

            IntPtr ptr = SpectraNative.spectra_onset_create(ref nativeConfig, sampleRate);
            if (ptr == IntPtr.Zero)
            {
                throw new SpectraException("Failed to create onset detector");
            }

            handle = new OnsetHandle(ptr);

            // Pin the callback
            nativeCallback = NativeOnsetCallback;
            callbackHandle = GCHandle.Alloc(nativeCallback);
        }

        /// <summary>
        /// Process audio samples.
        /// </summary>
        public void Process(float[] samples)
        {
            ThrowIfDisposed();

            SpectraNative.spectra_onset_process(
                handle.DangerousGetHandle(),
                samples,
                samples.Length,
                nativeCallback,
                IntPtr.Zero);
        }

        /// <summary>
        /// Reset detector state.
        /// </summary>
        public void Reset()
        {
            ThrowIfDisposed();
            SpectraNative.spectra_onset_reset(handle.DangerousGetHandle());
        }

        private void NativeOnsetCallback(float timestamp, float strength, IntPtr userData)
        {
            OnOnsetDetected?.Invoke(timestamp, strength);
        }

        private void ThrowIfDisposed()
        {
            if (disposed || handle.IsInvalid)
                throw new ObjectDisposedException(nameof(OnsetDetector));
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
                    if (callbackHandle.IsAllocated)
                    {
                        callbackHandle.Free();
                    }
                }
                disposed = true;
            }
        }

        ~OnsetDetector()
        {
            Dispose(false);
        }
    }

    /// <summary>
    /// Onset detection configuration.
    /// </summary>
    public class OnsetConfig
    {
        public int FftSize { get; set; } = 2048;
        public int HopSize { get; set; } = 512;
        public float Threshold { get; set; } = 0.5f;
        public float MinIntervalMs { get; set; } = 50f;
    }

    internal class OnsetHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        public OnsetHandle(IntPtr handle) : base(true)
        {
            SetHandle(handle);
        }

        protected override bool ReleaseHandle()
        {
            SpectraNative.spectra_onset_destroy(handle);
            return true;
        }
    }
}
