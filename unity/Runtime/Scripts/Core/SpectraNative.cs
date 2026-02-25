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
        // STFT
        // ====================================================================

        [StructLayout(LayoutKind.Sequential)]
        public struct STFTConfig
        {
            public int fftSize;
            public int hopSize;
            public int window;
            public int center;
        }

        [DllImport(LibraryName)]
        public static extern IntPtr spectra_stft_create(ref STFTConfig config);

        [DllImport(LibraryName)]
        public static extern void spectra_stft_destroy(IntPtr stft);

        [DllImport(LibraryName)]
        public static extern int spectra_stft_analyze(
            IntPtr stft,
            [In] float[] input,
            int inputSize,
            [Out] float[] outputReal,
            [Out] float[] outputImag,
            ref int numFrames);

        [DllImport(LibraryName)]
        public static extern int spectra_stft_num_frames(int inputSize, int fftSize, int hopSize);

        [DllImport(LibraryName)]
        public static extern int spectra_stft_push_samples(
            IntPtr stft,
            [In] float[] samples,
            int count);

        [DllImport(LibraryName)]
        public static extern int spectra_stft_pop_frame(
            IntPtr stft,
            [Out] float[] real,
            [Out] float[] imag);

        [DllImport(LibraryName)]
        public static extern int spectra_stft_frames_available(IntPtr stft);

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
        public static extern int spectra_cheby1(
            int order,
            float rippleDb,
            float normalizedFreq,
            int filterType,
            [Out] float[] b,
            ref int bLen,
            [Out] float[] a,
            ref int aLen);

        [DllImport(LibraryName)]
        public static extern int spectra_cheby2(
            int order,
            float stopbandDb,
            float normalizedFreq,
            int filterType,
            [Out] float[] b,
            ref int bLen,
            [Out] float[] a,
            ref int aLen);

        [DllImport(LibraryName)]
        public static extern int spectra_ellip(
            int order,
            float passbandRippleDb,
            float stopbandDb,
            float normalizedFreq,
            int filterType,
            [Out] float[] b,
            ref int bLen,
            [Out] float[] a,
            ref int aLen);

        // Dual-frequency overloads for bandpass/bandstop
        [DllImport(LibraryName)]
        public static extern int spectra_butter_bp(
            int order,
            float lowFreq,
            float highFreq,
            int filterType,
            [Out] float[] b,
            ref int bLen,
            [Out] float[] a,
            ref int aLen);

        [DllImport(LibraryName)]
        public static extern int spectra_cheby1_bp(
            int order,
            float rippleDb,
            float lowFreq,
            float highFreq,
            int filterType,
            [Out] float[] b,
            ref int bLen,
            [Out] float[] a,
            ref int aLen);

        [DllImport(LibraryName)]
        public static extern int spectra_cheby2_bp(
            int order,
            float stopbandDb,
            float lowFreq,
            float highFreq,
            int filterType,
            [Out] float[] b,
            ref int bLen,
            [Out] float[] a,
            ref int aLen);

        [DllImport(LibraryName)]
        public static extern int spectra_ellip_bp(
            int order,
            float passbandRippleDb,
            float stopbandDb,
            float lowFreq,
            float highFreq,
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
        // Filter Analysis
        // ====================================================================

        [DllImport(LibraryName)]
        public static extern int spectra_freqz(
            [In] float[] b, int bLen,
            [In] float[] a, int aLen,
            [Out] float[] magnitude,
            [Out] float[] phase,
            int numPoints);

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
        // Power Spectral Density
        // ====================================================================

        [StructLayout(LayoutKind.Sequential)]
        public struct PWelchConfig
        {
            public int segmentSize;
            public int overlap;
            public int window;
            public int onesided;
        }

        [DllImport(LibraryName)]
        public static extern int spectra_pwelch(
            [In] float[] input,
            int len,
            ref PWelchConfig config,
            float sampleRate,
            [Out] float[] psd,
            [Out] float[] frequencies,
            ref int outputLen);

        [DllImport(LibraryName)]
        public static extern int spectra_pwelch_output_size(ref PWelchConfig config);

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
