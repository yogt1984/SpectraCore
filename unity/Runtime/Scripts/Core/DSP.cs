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
        /// Design a Chebyshev Type I filter.
        /// Equivalent to MATLAB: [b,a] = cheby1(order, Rp, Wn, type)
        /// </summary>
        /// <param name="order">Filter order</param>
        /// <param name="rippleDb">Passband ripple in dB</param>
        /// <param name="normalizedFreq">Normalized cutoff frequency (0 to 1, where 1 = Nyquist)</param>
        /// <param name="type">Filter type</param>
        /// <returns>Tuple of (b, a) filter coefficients</returns>
        public static (float[] b, float[] a) Cheby1(int order, float rippleDb, float normalizedFreq,
            FilterType type = FilterType.Lowpass)
        {
            int coeffSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[coeffSize];
            float[] a = new float[coeffSize];
            int bLen = coeffSize, aLen = coeffSize;

            int result = SpectraNative.spectra_cheby1(
                order, rippleDb, normalizedFreq, (int)type, b, ref bLen, a, ref aLen);

            if (result != 0)
                throw new SpectraException("Failed to design Chebyshev Type I filter");

            Array.Resize(ref b, bLen);
            Array.Resize(ref a, aLen);
            return (b, a);
        }

        /// <summary>
        /// Design a Chebyshev Type II filter.
        /// Equivalent to MATLAB: [b,a] = cheby2(order, Rs, Wn, type)
        /// </summary>
        /// <param name="order">Filter order</param>
        /// <param name="stopbandDb">Stopband attenuation in dB</param>
        /// <param name="normalizedFreq">Normalized cutoff frequency (0 to 1, where 1 = Nyquist)</param>
        /// <param name="type">Filter type</param>
        /// <returns>Tuple of (b, a) filter coefficients</returns>
        public static (float[] b, float[] a) Cheby2(int order, float stopbandDb, float normalizedFreq,
            FilterType type = FilterType.Lowpass)
        {
            int coeffSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[coeffSize];
            float[] a = new float[coeffSize];
            int bLen = coeffSize, aLen = coeffSize;

            int result = SpectraNative.spectra_cheby2(
                order, stopbandDb, normalizedFreq, (int)type, b, ref bLen, a, ref aLen);

            if (result != 0)
                throw new SpectraException("Failed to design Chebyshev Type II filter");

            Array.Resize(ref b, bLen);
            Array.Resize(ref a, aLen);
            return (b, a);
        }

        /// <summary>
        /// Design an Elliptic (Cauer) filter.
        /// Equivalent to MATLAB: [b,a] = ellip(order, Rp, Rs, Wn, type)
        /// </summary>
        /// <param name="order">Filter order</param>
        /// <param name="passbandRippleDb">Passband ripple in dB</param>
        /// <param name="stopbandDb">Stopband attenuation in dB</param>
        /// <param name="normalizedFreq">Normalized cutoff frequency (0 to 1, where 1 = Nyquist)</param>
        /// <param name="type">Filter type</param>
        /// <returns>Tuple of (b, a) filter coefficients</returns>
        public static (float[] b, float[] a) Ellip(int order, float passbandRippleDb,
            float stopbandDb, float normalizedFreq, FilterType type = FilterType.Lowpass)
        {
            int coeffSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[coeffSize];
            float[] a = new float[coeffSize];
            int bLen = coeffSize, aLen = coeffSize;

            int result = SpectraNative.spectra_ellip(
                order, passbandRippleDb, stopbandDb, normalizedFreq, (int)type,
                b, ref bLen, a, ref aLen);

            if (result != 0)
                throw new SpectraException("Failed to design Elliptic filter");

            Array.Resize(ref b, bLen);
            Array.Resize(ref a, aLen);
            return (b, a);
        }

        /// <summary>
        /// Design a Butterworth bandpass or bandstop filter.
        /// </summary>
        /// <param name="order">Filter order</param>
        /// <param name="lowFreq">Lower normalized frequency</param>
        /// <param name="highFreq">Upper normalized frequency</param>
        /// <param name="type">Filter type (Bandpass or Bandstop)</param>
        /// <returns>Tuple of (b, a) filter coefficients</returns>
        public static (float[] b, float[] a) Butter(int order, float lowFreq, float highFreq,
            FilterType type)
        {
            // Maximum possible size for bandpass/bandstop (will be 2*order + 1)
            int coeffSize = SpectraNative.spectra_iir_coeff_size(order * 2);
            float[] b = new float[coeffSize];
            float[] a = new float[coeffSize];
            int bLen = coeffSize, aLen = coeffSize;

            int result = SpectraNative.spectra_butter_bp(
                order, lowFreq, highFreq, (int)type, b, ref bLen, a, ref aLen);

            if (result != 0)
                throw new SpectraException("Failed to design Butterworth filter");

            Array.Resize(ref b, bLen);
            Array.Resize(ref a, aLen);
            return (b, a);
        }

        /// <summary>
        /// Design a Chebyshev Type I bandpass or bandstop filter.
        /// </summary>
        /// <param name="order">Filter order</param>
        /// <param name="rippleDb">Passband ripple in dB</param>
        /// <param name="lowFreq">Lower normalized frequency</param>
        /// <param name="highFreq">Upper normalized frequency</param>
        /// <param name="type">Filter type (Bandpass or Bandstop)</param>
        /// <returns>Tuple of (b, a) filter coefficients</returns>
        public static (float[] b, float[] a) Cheby1(int order, float rippleDb, float lowFreq,
            float highFreq, FilterType type)
        {
            int coeffSize = SpectraNative.spectra_iir_coeff_size(order * 2);
            float[] b = new float[coeffSize];
            float[] a = new float[coeffSize];
            int bLen = coeffSize, aLen = coeffSize;

            int result = SpectraNative.spectra_cheby1_bp(
                order, rippleDb, lowFreq, highFreq, (int)type, b, ref bLen, a, ref aLen);

            if (result != 0)
                throw new SpectraException("Failed to design Chebyshev Type I filter");

            Array.Resize(ref b, bLen);
            Array.Resize(ref a, aLen);
            return (b, a);
        }

        /// <summary>
        /// Design a Chebyshev Type II bandpass or bandstop filter.
        /// </summary>
        /// <param name="order">Filter order</param>
        /// <param name="stopbandDb">Stopband attenuation in dB</param>
        /// <param name="lowFreq">Lower normalized frequency</param>
        /// <param name="highFreq">Upper normalized frequency</param>
        /// <param name="type">Filter type (Bandpass or Bandstop)</param>
        /// <returns>Tuple of (b, a) filter coefficients</returns>
        public static (float[] b, float[] a) Cheby2(int order, float stopbandDb, float lowFreq,
            float highFreq, FilterType type)
        {
            int coeffSize = SpectraNative.spectra_iir_coeff_size(order * 2);
            float[] b = new float[coeffSize];
            float[] a = new float[coeffSize];
            int bLen = coeffSize, aLen = coeffSize;

            int result = SpectraNative.spectra_cheby2_bp(
                order, stopbandDb, lowFreq, highFreq, (int)type, b, ref bLen, a, ref aLen);

            if (result != 0)
                throw new SpectraException("Failed to design Chebyshev Type II filter");

            Array.Resize(ref b, bLen);
            Array.Resize(ref a, aLen);
            return (b, a);
        }

        /// <summary>
        /// Design an Elliptic (Cauer) bandpass or bandstop filter.
        /// </summary>
        /// <param name="order">Filter order</param>
        /// <param name="passbandRippleDb">Passband ripple in dB</param>
        /// <param name="stopbandDb">Stopband attenuation in dB</param>
        /// <param name="lowFreq">Lower normalized frequency</param>
        /// <param name="highFreq">Upper normalized frequency</param>
        /// <param name="type">Filter type (Bandpass or Bandstop)</param>
        /// <returns>Tuple of (b, a) filter coefficients</returns>
        public static (float[] b, float[] a) Ellip(int order, float passbandRippleDb,
            float stopbandDb, float lowFreq, float highFreq, FilterType type)
        {
            int coeffSize = SpectraNative.spectra_iir_coeff_size(order * 2);
            float[] b = new float[coeffSize];
            float[] a = new float[coeffSize];
            int bLen = coeffSize, aLen = coeffSize;

            int result = SpectraNative.spectra_ellip_bp(
                order, passbandRippleDb, stopbandDb, lowFreq, highFreq, (int)type,
                b, ref bLen, a, ref aLen);

            if (result != 0)
                throw new SpectraException("Failed to design Elliptic filter");

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
        /// Compute Hilbert transform.
        /// Equivalent to MATLAB: h = hilbert(x)
        /// Returns the analytic signal as (real, imaginary) components.
        /// </summary>
        public static (float[] real, float[] imag) Hilbert(float[] input)
        {
            float[] real = new float[input.Length];
            float[] imag = new float[input.Length];

            int result = SpectraNative.spectra_hilbert(
                input, real, imag, input.Length);

            if (result != 0)
                throw new SpectraException("Failed to compute Hilbert transform");

            return (real, imag);
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
        /// Compute Power Spectral Density using Welch's method.
        /// Equivalent to MATLAB: [psd, f] = pwelch(x, window, overlap, nfft, fs)
        /// </summary>
        /// <param name="input">Input signal</param>
        /// <param name="segmentSize">Segment size for FFT</param>
        /// <param name="overlap">Number of overlapping samples</param>
        /// <param name="window">Window type</param>
        /// <param name="sampleRate">Sample rate in Hz</param>
        /// <param name="onesided">If true, return one-sided spectrum for real signals</param>
        /// <returns>Tuple of (PSD values, frequency values in Hz)</returns>
        public static (float[] psd, float[] frequencies) Pwelch(
            float[] input, int segmentSize, int overlap,
            WindowType window, float sampleRate, bool onesided = true)
        {
            var config = new SpectraNative.PWelchConfig
            {
                segmentSize = segmentSize,
                overlap = overlap,
                window = (int)window,
                onesided = onesided ? 1 : 0
            };

            int outLen = SpectraNative.spectra_pwelch_output_size(ref config);
            float[] psd = new float[outLen];
            float[] frequencies = new float[outLen];

            int result = SpectraNative.spectra_pwelch(
                input, input.Length, ref config, sampleRate,
                psd, frequencies, ref outLen);

            if (result != 0)
                throw new SpectraException("Failed to compute PSD");

            Array.Resize(ref psd, outLen);
            Array.Resize(ref frequencies, outLen);
            return (psd, frequencies);
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

        /// <summary>
        /// Compute frequency response of a filter.
        /// Equivalent to MATLAB: [H, W] = freqz(b, a, nfft)
        /// </summary>
        /// <param name="b">Numerator coefficients</param>
        /// <param name="a">Denominator coefficients</param>
        /// <param name="numPoints">Number of frequency points (default: 512)</param>
        /// <returns>Tuple of (magnitude, phase in radians, normalized frequencies 0 to π)</returns>
        public static (float[] magnitude, float[] phase, float[] frequencies) Freqz(
            float[] b, float[] a, int numPoints = 512)
        {
            float[] magnitude = new float[numPoints];
            float[] phase = new float[numPoints];
            float[] frequencies = new float[numPoints];

            int result = SpectraNative.spectra_freqz(
                b, b.Length, a, a.Length, magnitude, phase, numPoints);

            if (result != 0)
                throw new SpectraException("Failed to compute frequency response");

            // Generate normalized frequency vector (0 to π)
            for (int i = 0; i < numPoints; i++)
            {
                frequencies[i] = (float)Math.PI * i / (numPoints - 1);
            }

            return (magnitude, phase, frequencies);
        }

        /// <summary>
        /// Compute frequency response magnitude in dB.
        /// </summary>
        /// <param name="b">Numerator coefficients</param>
        /// <param name="a">Denominator coefficients</param>
        /// <param name="numPoints">Number of frequency points</param>
        /// <returns>Tuple of (magnitude in dB, phase in radians, normalized frequencies)</returns>
        public static (float[] magnitudeDb, float[] phase, float[] frequencies) FreqzDb(
            float[] b, float[] a, int numPoints = 512)
        {
            var (mag, phase, freqs) = Freqz(b, a, numPoints);

            float[] magDb = new float[numPoints];
            for (int i = 0; i < numPoints; i++)
            {
                magDb[i] = 20.0f * (float)Math.Log10(Math.Max(mag[i], 1e-10f));
            }

            return (magDb, phase, freqs);
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
