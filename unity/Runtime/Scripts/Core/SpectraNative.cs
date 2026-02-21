using System;
using System.Runtime.InteropServices;

namespace Spectra
{
    /// <summary>
    /// P/Invoke bindings for the native SpectraCore library.
    /// </summary>
    internal static class SpectraNative
    {
        private const string LibraryName = "spectra";

        // ====================================================================
        // Version
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern int spectra_version_major();

        [DllImport(LibraryName)]
        public static extern int spectra_version_minor();

        [DllImport(LibraryName)]
        public static extern int spectra_version_patch();

        [DllImport(LibraryName)]
        public static extern IntPtr spectra_version_string();

        // ====================================================================
        // Error Handling
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern int spectra_get_last_error();

        [DllImport(LibraryName)]
        public static extern IntPtr spectra_get_error_message();

        [DllImport(LibraryName)]
        public static extern void spectra_clear_error();

        // ====================================================================
        // FFT
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern IntPtr spectra_fft_create(int size);

        [DllImport(LibraryName)]
        public static extern void spectra_fft_destroy(IntPtr fft);

        [DllImport(LibraryName)]
        public static extern int spectra_fft_forward(
            IntPtr fft,
            [In] float[] input,
            [Out] float[] outputReal,
            [Out] float[] outputImag);

        [DllImport(LibraryName)]
        public static extern int spectra_fft_inverse(
            IntPtr fft,
            [In] float[] inputReal,
            [In] float[] inputImag,
            [Out] float[] output);

        [DllImport(LibraryName)]
        public static extern int spectra_fft_size(IntPtr fft);

        [DllImport(LibraryName)]
        public static extern int spectra_fft_spectrum_size(IntPtr fft);

        // ====================================================================
        // Window Functions
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern int spectra_generate_window(
            [Out] float[] output,
            int size,
            int windowType);

        [DllImport(LibraryName)]
        public static extern int spectra_apply_window(
            [In, Out] float[] data,
            int size,
            int windowType);

        // ====================================================================
        // Streaming Filter
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern IntPtr spectra_streaming_filter_create(
            [In] float[] b, int bLen,
            [In] float[] a, int aLen);

        [DllImport(LibraryName)]
        public static extern void spectra_streaming_filter_destroy(IntPtr filter);

        [DllImport(LibraryName)]
        public static extern int spectra_streaming_filter_process(
            IntPtr filter,
            [In] float[] input,
            [Out] float[] output,
            int count);

        [DllImport(LibraryName)]
        public static extern int spectra_streaming_filter_reset(IntPtr filter);

        // ====================================================================
        // Filter Design
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern int spectra_butter(
            int order,
            float normalizedFreq,
            int filterType,
            [Out] float[] b,
            ref int bLen,
            [Out] float[] a,
            ref int aLen);

        [DllImport(LibraryName)]
        public static extern int spectra_iir_coeff_size(int order);

        // ====================================================================
        // Filter Application
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern int spectra_lfilter(
            [In] float[] b, int bLen,
            [In] float[] a, int aLen,
            [In] float[] input,
            [Out] float[] output,
            int len);

        [DllImport(LibraryName)]
        public static extern int spectra_filtfilt(
            [In] float[] b, int bLen,
            [In] float[] a, int aLen,
            [In] float[] input,
            [Out] float[] output,
            int len);

        // ====================================================================
        // Correlation
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern int spectra_xcorr(
            [In] float[] x, int xLen,
            [In] float[] y, int yLen,
            [Out] float[] output,
            ref int outputLen);

        [DllImport(LibraryName)]
        public static extern int spectra_xcorr_output_size(int xLen, int yLen);

        // ====================================================================
        // Hilbert
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern int spectra_hilbert(
            [In] float[] input,
            [Out] float[] outputReal,
            [Out] float[] outputImag,
            int len);

        [DllImport(LibraryName)]
        public static extern int spectra_instantaneous_amplitude(
            [In] float[] input,
            [Out] float[] output,
            int len);

        // ====================================================================
        // Resampling
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern int spectra_resample(
            [In] float[] input, int inputLen,
            int p, int q,
            [Out] float[] output,
            ref int outputLen);

        [DllImport(LibraryName)]
        public static extern int spectra_resample_output_size(int inputLen, int p, int q);

        // ====================================================================
        // Onset Detection
        // ====================================================================

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnsetCallback(float timestamp, float strength, IntPtr userData);

        [DllImport(LibraryName)]
        public static extern IntPtr spectra_onset_create(
            ref OnsetConfig config,
            float sampleRate);

        [DllImport(LibraryName)]
        public static extern void spectra_onset_destroy(IntPtr detector);

        [DllImport(LibraryName)]
        public static extern int spectra_onset_process(
            IntPtr detector,
            [In] float[] samples,
            int count,
            OnsetCallback callback,
            IntPtr userData);

        [DllImport(LibraryName)]
        public static extern int spectra_onset_reset(IntPtr detector);

        [StructLayout(LayoutKind.Sequential)]
        public struct OnsetConfig
        {
            public int fftSize;
            public int hopSize;
            public float threshold;
            public float minIntervalMs;
        }
    }
}
