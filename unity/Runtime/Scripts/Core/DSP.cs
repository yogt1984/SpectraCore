using System;

namespace Spectra
{
    /// <summary>
    /// Static API for DSP operations. Function names mirror MATLAB equivalents.
    /// </summary>
    public static class DSP
    {
        /// <summary>
        /// Design a Butterworth filter.
        /// Equivalent to MATLAB: [b,a] = butter(order, Wn, type)
        /// </summary>
        /// <param name="order">Filter order</param>
        /// <param name="normalizedFreq">Normalized cutoff frequency (0 to 1, where 1 = Nyquist)</param>
        /// <param name="type">Filter type</param>
        /// <returns>Tuple of (b, a) filter coefficients</returns>
        public static (float[] b, float[] a) Butter(int order, float normalizedFreq,
            FilterType type = FilterType.Lowpass)
        {
            int coeffSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[coeffSize];
            float[] a = new float[coeffSize];
            int bLen = coeffSize, aLen = coeffSize;

            int result = SpectraNative.spectra_butter(
                order, normalizedFreq, (int)type, b, ref bLen, a, ref aLen);

            if (result != 0)
                throw new SpectraException("Failed to design Butterworth filter");

            Array.Resize(ref b, bLen);
            Array.Resize(ref a, aLen);
            return (b, a);
        }

        /// <summary>
        /// Apply a digital filter to a signal.
        /// Equivalent to MATLAB: y = filter(b, a, x)
        /// </summary>
        public static float[] LFilter(float[] b, float[] a, float[] input)
        {
            float[] output = new float[input.Length];

            int result = SpectraNative.spectra_lfilter(
                b, b.Length, a, a.Length, input, output, input.Length);

            if (result != 0)
                throw new SpectraException("Failed to apply filter");

            return output;
        }

        /// <summary>
        /// Zero-phase digital filtering.
        /// Equivalent to MATLAB: y = filtfilt(b, a, x)
        /// </summary>
        public static float[] FiltFilt(float[] b, float[] a, float[] input)
        {
            float[] output = new float[input.Length];

            int result = SpectraNative.spectra_filtfilt(
                b, b.Length, a, a.Length, input, output, input.Length);

            if (result != 0)
                throw new SpectraException("Failed to apply zero-phase filter");

            return output;
        }

        /// <summary>
        /// Cross-correlation of two signals.
        /// Equivalent to MATLAB: r = xcorr(x, y)
        /// </summary>
        public static float[] Xcorr(float[] x, float[] y)
        {
            int outLen = SpectraNative.spectra_xcorr_output_size(x.Length, y.Length);
            float[] output = new float[outLen];

            int result = SpectraNative.spectra_xcorr(
                x, x.Length, y, y.Length, output, ref outLen);

            if (result != 0)
                throw new SpectraException("Failed to compute cross-correlation");

            return output;
        }

        /// <summary>
        /// Autocorrelation of a signal.
        /// Equivalent to MATLAB: r = xcorr(x)
        /// </summary>
        public static float[] Xcorr(float[] x)
        {
            return Xcorr(x, x);
        }

        /// <summary>
        /// Compute instantaneous amplitude (envelope) using Hilbert transform.
        /// Equivalent to MATLAB: amp = abs(hilbert(x))
        /// </summary>
        public static float[] Envelope(float[] input)
        {
            float[] output = new float[input.Length];

            int result = SpectraNative.spectra_instantaneous_amplitude(
                input, output, input.Length);

            if (result != 0)
                throw new SpectraException("Failed to compute envelope");

            return output;
        }

        /// <summary>
        /// Resample signal by rational factor p/q.
        /// Equivalent to MATLAB: y = resample(x, p, q)
        /// </summary>
        public static float[] Resample(float[] input, int p, int q)
        {
            int outLen = SpectraNative.spectra_resample_output_size(input.Length, p, q);
            float[] output = new float[outLen];

            int result = SpectraNative.spectra_resample(
                input, input.Length, p, q, output, ref outLen);

            if (result != 0)
                throw new SpectraException("Failed to resample signal");

            Array.Resize(ref output, outLen);
            return output;
        }

        /// <summary>
        /// Generate a window function.
        /// </summary>
        public static float[] Window(int size, WindowType type)
        {
            float[] window = new float[size];
            SpectraNative.spectra_generate_window(window, size, (int)type);
            return window;
        }

        /// <summary>
        /// Apply a window function to data in-place.
        /// </summary>
        public static void ApplyWindow(float[] data, WindowType type)
        {
            SpectraNative.spectra_apply_window(data, data.Length, (int)type);
        }
    }

    /// <summary>
    /// Filter type enumeration
    /// </summary>
    public enum FilterType
    {
        Lowpass = 0,
        Highpass = 1,
        Bandpass = 2,
        Bandstop = 3
    }

    /// <summary>
    /// Window function type
    /// </summary>
    public enum WindowType
    {
        Rectangular = 0,
        Hann = 1,
        Hamming = 2,
        Blackman = 3,
        BlackmanHarris = 4,
        FlatTop = 5
    }

    /// <summary>
    /// Exception thrown by SpectraCore operations
    /// </summary>
    public class SpectraException : Exception
    {
        public SpectraException(string message) : base(message) { }
    }
}
