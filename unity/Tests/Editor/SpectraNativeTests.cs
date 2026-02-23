using NUnit.Framework;
using System;
using System.Runtime.InteropServices;
using Spectra;

namespace Spectra.Tests
{
    /// <summary>
    /// Unit tests for P/Invoke bindings to native SpectraCore library.
    /// These tests verify that all native functions are correctly bound and callable.
    /// </summary>
    public class SpectraNativeTests
    {
        // ====================================================================
        // Version Tests
        // ====================================================================

        [Test]
        public void VersionInfo_ReturnsValidVersion()
        {
            int major = SpectraNative.spectra_version_major();
            int minor = SpectraNative.spectra_version_minor();
            int patch = SpectraNative.spectra_version_patch();

            Assert.GreaterOrEqual(major, 0);
            Assert.GreaterOrEqual(minor, 0);
            Assert.GreaterOrEqual(patch, 0);

            IntPtr versionPtr = SpectraNative.spectra_version_string();
            Assert.AreNotEqual(IntPtr.Zero, versionPtr);

            string version = Marshal.PtrToStringAnsi(versionPtr);
            Assert.IsNotEmpty(version);
        }

        // ====================================================================
        // Error Handling Tests
        // ====================================================================

        [Test]
        public void ErrorHandling_ClearError_Works()
        {
            SpectraNative.spectra_clear_error();
            int error = SpectraNative.spectra_get_last_error();
            Assert.AreEqual(0, error); // SPECTRA_OK = 0
        }

        [Test]
        public void ErrorHandling_InvalidHandle_SetsError()
        {
            SpectraNative.spectra_clear_error();

            // Try to use null FFT handle
            float[] input = new float[256];
            float[] outputReal = new float[256];
            float[] outputImag = new float[256];

            int result = SpectraNative.spectra_fft_forward(
                IntPtr.Zero, input, outputReal, outputImag);

            Assert.AreNotEqual(0, result);
            int error = SpectraNative.spectra_get_last_error();
            Assert.AreNotEqual(0, error);
        }

        // ====================================================================
        // FFT Tests
        // ====================================================================

        [Test]
        public void FFT_CreateDestroy_Works()
        {
            IntPtr fft = SpectraNative.spectra_fft_create(256);
            Assert.AreNotEqual(IntPtr.Zero, fft);

            int size = SpectraNative.spectra_fft_size(fft);
            Assert.AreEqual(256, size);

            int spectrumSize = SpectraNative.spectra_fft_spectrum_size(fft);
            Assert.AreEqual(129, spectrumSize); // N/2 + 1

            SpectraNative.spectra_fft_destroy(fft);
        }

        [Test]
        public void FFT_ForwardTransform_ProducesDCComponent()
        {
            IntPtr fft = SpectraNative.spectra_fft_create(256);

            float[] input = new float[256];
            for (int i = 0; i < 256; i++)
                input[i] = 1.0f; // DC signal

            float[] outputReal = new float[256];
            float[] outputImag = new float[256];

            int result = SpectraNative.spectra_fft_forward(
                fft, input, outputReal, outputImag);

            Assert.AreEqual(0, result);
            Assert.Greater(outputReal[0], 0); // DC component should be large

            SpectraNative.spectra_fft_destroy(fft);
        }

        [Test]
        public void FFT_RoundTrip_RecreatesSignal()
        {
            IntPtr fft = SpectraNative.spectra_fft_create(256);

            float[] original = new float[256];
            for (int i = 0; i < 256; i++)
                original[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 256);

            float[] freqReal = new float[256];
            float[] freqImag = new float[256];
            float[] recovered = new float[256];

            SpectraNative.spectra_fft_forward(fft, original, freqReal, freqImag);
            SpectraNative.spectra_fft_inverse(fft, freqReal, freqImag, recovered);

            // Check a few samples
            for (int i = 50; i < 200; i++)
            {
                Assert.AreEqual(original[i], recovered[i], 1e-4f);
            }

            SpectraNative.spectra_fft_destroy(fft);
        }

        // ====================================================================
        // Window Function Tests
        // ====================================================================

        [Test]
        public void Window_GenerateHann_HasCorrectShape()
        {
            float[] window = new float[256];
            int result = SpectraNative.spectra_generate_window(window, 256, 1); // Hann

            Assert.AreEqual(0, result);
            Assert.AreEqual(0.0f, window[0], 0.01f); // Edges near zero
            Assert.AreEqual(1.0f, window[128], 0.01f); // Center near 1
        }

        [Test]
        public void Window_ApplyWindow_ModifiesData()
        {
            float[] data = new float[256];
            for (int i = 0; i < 256; i++)
                data[i] = 1.0f;

            int result = SpectraNative.spectra_apply_window(data, 256, 2); // Hamming

            Assert.AreEqual(0, result);
            Assert.Less(data[0], 0.2f); // Edges attenuated
            Assert.Greater(data[128], 0.8f); // Center preserved
        }

        // ====================================================================
        // STFT Tests
        // ====================================================================

        [Test]
        public void STFT_CreateDestroy_Works()
        {
            var config = new SpectraNative.STFTConfig
            {
                fftSize = 512,
                hopSize = 256,
                window = 1, // Hann
                center = 0
            };

            IntPtr stft = SpectraNative.spectra_stft_create(ref config);
            Assert.AreNotEqual(IntPtr.Zero, stft);

            SpectraNative.spectra_stft_destroy(stft);
        }

        [Test]
        public void STFT_NumFrames_CalculatesCorrectly()
        {
            int numFrames = SpectraNative.spectra_stft_num_frames(1024, 256, 128);
            Assert.Greater(numFrames, 0);
            Assert.AreEqual(7, numFrames); // (1024 - 256) / 128 + 1
        }

        [Test]
        public void STFT_Analyze_ProducesFrames()
        {
            var config = new SpectraNative.STFTConfig
            {
                fftSize = 256,
                hopSize = 128,
                window = 1,
                center = 0
            };

            IntPtr stft = SpectraNative.spectra_stft_create(ref config);

            float[] input = new float[1024];
            for (int i = 0; i < 1024; i++)
                input[i] = (float)Math.Sin(2.0 * Math.PI * 10 * i / 1024);

            int expectedFrames = SpectraNative.spectra_stft_num_frames(1024, 256, 128);
            int spectrumSize = 129; // 256/2 + 1

            float[] outputReal = new float[expectedFrames * spectrumSize];
            float[] outputImag = new float[expectedFrames * spectrumSize];
            int actualFrames = 0;

            int result = SpectraNative.spectra_stft_analyze(
                stft, input, 1024, outputReal, outputImag, ref actualFrames);

            Assert.AreEqual(0, result);
            Assert.Greater(actualFrames, 0);

            SpectraNative.spectra_stft_destroy(stft);
        }

        // ====================================================================
        // Filter Design Tests
        // ====================================================================

        [Test]
        public void FilterDesign_Butterworth_ProducesValidCoefficients()
        {
            int order = 4;
            int maxSize = SpectraNative.spectra_iir_coeff_size(order);

            float[] b = new float[maxSize];
            float[] a = new float[maxSize];
            int bLen = 0;
            int aLen = 0;

            int result = SpectraNative.spectra_butter(
                order, 0.3f, 0, // Lowpass
                b, ref bLen, a, ref aLen);

            Assert.AreEqual(0, result);
            Assert.Greater(bLen, 0);
            Assert.Greater(aLen, 0);

            // Check no NaN values
            for (int i = 0; i < bLen; i++)
                Assert.IsFalse(float.IsNaN(b[i]));
            for (int i = 0; i < aLen; i++)
                Assert.IsFalse(float.IsNaN(a[i]));
        }

        [Test]
        public void FilterDesign_Chebyshev1_ProducesValidCoefficients()
        {
            int order = 4;
            int maxSize = SpectraNative.spectra_iir_coeff_size(order);

            float[] b = new float[maxSize];
            float[] a = new float[maxSize];
            int bLen = 0;
            int aLen = 0;

            int result = SpectraNative.spectra_cheby1(
                order, 1.0f, 0.3f, 0, // Lowpass, 1dB ripple
                b, ref bLen, a, ref aLen);

            Assert.AreEqual(0, result);
            Assert.Greater(bLen, 0);
            Assert.Greater(aLen, 0);
        }

        [Test]
        public void FilterDesign_Elliptic_ProducesValidCoefficients()
        {
            int order = 4;
            int maxSize = SpectraNative.spectra_iir_coeff_size(order);

            float[] b = new float[maxSize];
            float[] a = new float[maxSize];
            int bLen = 0;
            int aLen = 0;

            int result = SpectraNative.spectra_ellip(
                order, 1.0f, 40.0f, 0.3f, 0, // Lowpass
                b, ref bLen, a, ref aLen);

            Assert.AreEqual(0, result);
            Assert.Greater(bLen, 0);
        }

        // ====================================================================
        // Streaming Filter Tests
        // ====================================================================

        [Test]
        public void StreamingFilter_CreateDestroy_Works()
        {
            float[] b = new float[] { 0.25f, 0.25f, 0.25f, 0.25f };
            float[] a = new float[] { 1.0f };

            IntPtr filter = SpectraNative.spectra_streaming_filter_create(
                b, b.Length, a, a.Length);

            Assert.AreNotEqual(IntPtr.Zero, filter);
            SpectraNative.spectra_streaming_filter_destroy(filter);
        }

        [Test]
        public void StreamingFilter_Process_ProducesOutput()
        {
            float[] b = new float[] { 0.25f, 0.25f, 0.25f, 0.25f };
            float[] a = new float[] { 1.0f };

            IntPtr filter = SpectraNative.spectra_streaming_filter_create(
                b, b.Length, a, a.Length);

            float[] input = new float[64];
            float[] output = new float[64];

            for (int i = 0; i < 64; i++)
                input[i] = 1.0f;

            int result = SpectraNative.spectra_streaming_filter_process(
                filter, input, output, 64);

            Assert.AreEqual(0, result);

            // After transient, moving average of 1s should be 1
            Assert.AreEqual(1.0f, output[10], 0.01f);

            SpectraNative.spectra_streaming_filter_destroy(filter);
        }

        // ====================================================================
        // Correlation Tests
        // ====================================================================

        [Test]
        public void Xcorr_AutoCorrelation_HasPeakAtCenter()
        {
            float[] x = new float[] { 1, 2, 3, 4 };

            int outputSize = SpectraNative.spectra_xcorr_output_size(4, 4);
            float[] output = new float[outputSize];
            int actualLen = 0;

            int result = SpectraNative.spectra_xcorr(
                x, 4, x, 4, output, ref actualLen);

            Assert.AreEqual(0, result);
            Assert.Greater(actualLen, 0);

            // Peak should be at center for auto-correlation
            int center = actualLen / 2;
            float maxVal = output[center];

            for (int i = 0; i < actualLen; i++)
                Assert.LessOrEqual(output[i], maxVal + 0.01f);
        }

        // ====================================================================
        // PSD Tests
        // ====================================================================

        [Test]
        public void Pwelch_ProducesValidPSD()
        {
            float[] input = new float[1024];
            for (int i = 0; i < 1024; i++)
                input[i] = (float)(Math.Sin(i * 0.1) + Math.Cos(i * 0.3));

            var config = new SpectraNative.PWelchConfig
            {
                segmentSize = 256,
                overlap = 128,
                window = 1, // Hann
                onesided = 1
            };

            int outputSize = SpectraNative.spectra_pwelch_output_size(ref config);
            Assert.AreEqual(129, outputSize); // 256/2 + 1

            float[] psd = new float[outputSize];
            float[] freq = new float[outputSize];
            int actualLen = 0;

            int result = SpectraNative.spectra_pwelch(
                input, 1024, ref config, 44100.0f,
                psd, freq, ref actualLen);

            Assert.AreEqual(0, result);
            Assert.Greater(actualLen, 0);

            // PSD should be non-negative
            for (int i = 0; i < actualLen; i++)
            {
                Assert.GreaterOrEqual(psd[i], 0.0f);
                Assert.IsFalse(float.IsNaN(psd[i]));
            }

            // Frequencies should be monotonic
            for (int i = 1; i < actualLen; i++)
                Assert.Greater(freq[i], freq[i - 1]);
        }

        // ====================================================================
        // Hilbert Transform Tests
        // ====================================================================

        [Test]
        public void Hilbert_ProducesValidOutput()
        {
            float[] input = new float[256];
            for (int i = 0; i < 256; i++)
                input[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 256);

            float[] outputReal = new float[256];
            float[] outputImag = new float[256];

            int result = SpectraNative.spectra_hilbert(
                input, outputReal, outputImag, 256);

            Assert.AreEqual(0, result);

            for (int i = 0; i < 256; i++)
            {
                Assert.IsFalse(float.IsNaN(outputReal[i]));
                Assert.IsFalse(float.IsNaN(outputImag[i]));
            }
        }

        [Test]
        public void InstantaneousAmplitude_ProducesEnvelope()
        {
            float[] input = new float[256];
            for (int i = 0; i < 256; i++)
                input[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 256);

            float[] envelope = new float[256];

            int result = SpectraNative.spectra_instantaneous_amplitude(
                input, envelope, 256);

            Assert.AreEqual(0, result);

            for (int i = 0; i < 256; i++)
            {
                Assert.GreaterOrEqual(envelope[i], 0.0f);
                Assert.IsFalse(float.IsNaN(envelope[i]));
            }
        }

        // ====================================================================
        // Resampling Tests
        // ====================================================================

        [Test]
        public void Resample_OutputSize_CalculatesCorrectly()
        {
            int size = SpectraNative.spectra_resample_output_size(100, 3, 2);
            Assert.AreEqual(150, size); // Upsample by 3/2

            size = SpectraNative.spectra_resample_output_size(100, 2, 3);
            Assert.AreEqual(67, size); // Downsample by 2/3
        }

        [Test]
        public void Resample_Upsample_ProducesOutput()
        {
            float[] input = new float[] { 0, 1, 0, -1, 0 };
            int outputSize = SpectraNative.spectra_resample_output_size(5, 2, 1);

            float[] output = new float[outputSize];
            int actualLen = 0;

            int result = SpectraNative.spectra_resample(
                input, 5, 2, 1, output, ref actualLen);

            Assert.AreEqual(0, result);
            Assert.AreEqual(10, actualLen);

            for (int i = 0; i < actualLen; i++)
                Assert.IsFalse(float.IsNaN(output[i]));
        }

        // ====================================================================
        // Onset Detection Tests
        // ====================================================================

        [Test]
        public void OnsetDetector_CreateDestroy_Works()
        {
            var config = new SpectraNative.OnsetConfig
            {
                fftSize = 512,
                hopSize = 128,
                threshold = 1.5f,
                minIntervalMs = 50.0f
            };

            IntPtr detector = SpectraNative.spectra_onset_create(
                ref config, 44100.0f);

            Assert.AreNotEqual(IntPtr.Zero, detector);
            SpectraNative.spectra_onset_destroy(detector);
        }

        [Test]
        public void OnsetDetector_Process_DoesNotCrash()
        {
            var config = new SpectraNative.OnsetConfig
            {
                fftSize = 512,
                hopSize = 128,
                threshold = 1.5f,
                minIntervalMs = 50.0f
            };

            IntPtr detector = SpectraNative.spectra_onset_create(
                ref config, 44100.0f);

            float[] samples = new float[4096];
            for (int i = 1000; i < 1010; i++)
                samples[i] = 1.0f; // Transient

            int result = SpectraNative.spectra_onset_process(
                detector, samples, 4096, null, IntPtr.Zero);

            Assert.AreEqual(0, result);

            SpectraNative.spectra_onset_destroy(detector);
        }

        // ====================================================================
        // Advanced Filter Design Tests
        // ====================================================================

        [Test]
        public void Cheby1_DesignsFilter_ValidCoefficients()
        {
            int order = 4;
            int maxSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[maxSize];
            float[] a = new float[maxSize];
            int bLen = 0, aLen = 0;

            int result = SpectraNative.spectra_cheby1(
                order, 0.5f, 0.3f, 0, b, ref bLen, a, ref aLen);

            Assert.AreEqual(0, result);
            Assert.Greater(bLen, 0);
            Assert.Greater(aLen, 0);
            Assert.AreNotEqual(0.0f, b[0]);
            Assert.AreNotEqual(0.0f, a[0]);
        }

        [Test]
        public void Cheby2_DesignsFilter_ValidCoefficients()
        {
            int order = 4;
            int maxSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[maxSize];
            float[] a = new float[maxSize];
            int bLen = 0, aLen = 0;

            int result = SpectraNative.spectra_cheby2(
                order, 40.0f, 0.3f, 0, b, ref bLen, a, ref aLen);

            Assert.AreEqual(0, result);
            Assert.Greater(bLen, 0);
            Assert.Greater(aLen, 0);
            Assert.AreNotEqual(0.0f, b[0]);
            Assert.AreNotEqual(0.0f, a[0]);
        }

        [Test]
        public void Ellip_DesignsFilter_ValidCoefficients()
        {
            int order = 4;
            int maxSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[maxSize];
            float[] a = new float[maxSize];
            int bLen = 0, aLen = 0;

            int result = SpectraNative.spectra_ellip(
                order, 0.5f, 40.0f, 0.3f, 0, b, ref bLen, a, ref aLen);

            Assert.AreEqual(0, result);
            Assert.Greater(bLen, 0);
            Assert.Greater(aLen, 0);
            Assert.AreNotEqual(0.0f, b[0]);
            Assert.AreNotEqual(0.0f, a[0]);
        }

        // ====================================================================
        // Power Spectral Density Tests
        // ====================================================================

        [Test]
        public void Pwelch_ComputesPSD_ValidOutput()
        {
            float[] signal = new float[4096];
            for (int i = 0; i < signal.Length; i++)
                signal[i] = (float)Math.Sin(2.0 * Math.PI * 440 * i / 44100);

            var config = new SpectraNative.PWelchConfig
            {
                segmentSize = 256,
                overlap = 128,
                window = 1, // Hann
                onesided = 1
            };

            int outLen = SpectraNative.spectra_pwelch_output_size(ref config);
            float[] psd = new float[outLen];
            float[] freqs = new float[outLen];

            int result = SpectraNative.spectra_pwelch(
                signal, signal.Length, ref config, 44100.0f,
                psd, freqs, ref outLen);

            Assert.AreEqual(0, result);
            Assert.Greater(outLen, 0);
            Assert.AreEqual(129, outLen); // 256/2 + 1

            // Check that PSD has energy
            float maxPsd = 0;
            for (int i = 0; i < outLen; i++)
                maxPsd = Math.Max(maxPsd, psd[i]);
            Assert.Greater(maxPsd, 0);
        }

        // ====================================================================
        // Hilbert Transform Tests
        // ====================================================================

        [Test]
        public void Hilbert_ComputesAnalyticSignal_ValidOutput()
        {
            float[] input = new float[256];
            for (int i = 0; i < input.Length; i++)
                input[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 256);

            float[] real = new float[256];
            float[] imag = new float[256];

            int result = SpectraNative.spectra_hilbert(input, real, imag, 256);

            Assert.AreEqual(0, result);

            // Real part should approximately match input
            for (int i = 50; i < 200; i++)
                Assert.AreEqual(input[i], real[i], 0.1f);

            // Imaginary part should be non-zero
            float maxImag = 0;
            for (int i = 0; i < 256; i++)
                maxImag = Math.Max(maxImag, Math.Abs(imag[i]));
            Assert.Greater(maxImag, 0.1f);
        }

        [Test]
        public void InstantaneousAmplitude_ComputesEnvelope_ValidOutput()
        {
            float[] input = new float[256];
            for (int i = 0; i < input.Length; i++)
                input[i] = (float)(Math.Sin(2.0 * Math.PI * 5 * i / 256) *
                                   (0.5 + 0.5 * Math.Cos(2.0 * Math.PI * i / 256)));

            float[] envelope = new float[256];

            int result = SpectraNative.spectra_instantaneous_amplitude(
                input, envelope, 256);

            Assert.AreEqual(0, result);

            // Envelope should be non-negative
            for (int i = 0; i < 256; i++)
                Assert.GreaterOrEqual(envelope[i], 0);
        }

        // ====================================================================
        // STFT Streaming Tests
        // ====================================================================

        [Test]
        public void STFT_CreateDestroy_Works()
        {
            var config = new SpectraNative.STFTConfig
            {
                fftSize = 256,
                hopSize = 128,
                window = 1, // Hann
                center = 1
            };

            IntPtr stft = SpectraNative.spectra_stft_create(ref config);
            Assert.AreNotEqual(IntPtr.Zero, stft);
            SpectraNative.spectra_stft_destroy(stft);
        }

        [Test]
        public void STFT_Analyze_ComputesFrames()
        {
            var config = new SpectraNative.STFTConfig
            {
                fftSize = 256,
                hopSize = 128,
                window = 1,
                center = 0
            };

            IntPtr stft = SpectraNative.spectra_stft_create(ref config);

            float[] input = new float[1024];
            for (int i = 0; i < input.Length; i++)
                input[i] = (float)Math.Sin(2.0 * Math.PI * 10 * i / 256);

            int numFrames = 0;
            int spectrumSize = 256 / 2 + 1;
            int maxFrames = SpectraNative.spectra_stft_num_frames(1024, 256, 128);

            float[] real = new float[maxFrames * spectrumSize];
            float[] imag = new float[maxFrames * spectrumSize];

            int result = SpectraNative.spectra_stft_analyze(
                stft, input, 1024, real, imag, ref numFrames);

            Assert.AreEqual(0, result);
            Assert.Greater(numFrames, 0);
            Assert.LessOrEqual(numFrames, maxFrames);

            SpectraNative.spectra_stft_destroy(stft);
        }

        [Test]
        public void STFT_PushPop_StreamingWorks()
        {
            var config = new SpectraNative.STFTConfig
            {
                fftSize = 256,
                hopSize = 128,
                window = 1,
                center = 0
            };

            IntPtr stft = SpectraNative.spectra_stft_create(ref config);

            float[] samples = new float[512];
            for (int i = 0; i < samples.Length; i++)
                samples[i] = (float)Math.Sin(2.0 * Math.PI * 10 * i / 256);

            // Push samples
            int result = SpectraNative.spectra_stft_push_samples(stft, samples, 512);
            Assert.AreEqual(0, result);

            // Check frames available
            int available = SpectraNative.spectra_stft_frames_available(stft);
            Assert.Greater(available, 0);

            // Pop a frame
            int spectrumSize = 256 / 2 + 1;
            float[] real = new float[spectrumSize];
            float[] imag = new float[spectrumSize];

            result = SpectraNative.spectra_stft_pop_frame(stft, real, imag);
            Assert.AreEqual(0, result);

            SpectraNative.spectra_stft_destroy(stft);
        }

        [Test]
        public void STFT_NumFrames_CalculatesCorrectly()
        {
            int numFrames = SpectraNative.spectra_stft_num_frames(1024, 256, 128);

            // Formula: 1 + (input_size - fft_size) / hop_size
            int expected = 1 + (1024 - 256) / 128;
            Assert.AreEqual(expected, numFrames);
        }

        // ====================================================================
        // Filter Analysis Tests
        // ====================================================================

        [Test]
        public void Freqz_ComputesFrequencyResponse_ValidOutput()
        {
            // Design a simple lowpass filter
            int order = 4;
            int maxSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[maxSize];
            float[] a = new float[maxSize];
            int bLen = 0, aLen = 0;

            int result = SpectraNative.spectra_butter(
                order, 0.3f, 0, b, ref bLen, a, ref aLen);
            Assert.AreEqual(0, result);

            // Compute frequency response
            int numPoints = 256;
            float[] magnitude = new float[numPoints];
            float[] phase = new float[numPoints];

            result = SpectraNative.spectra_freqz(
                b, bLen, a, aLen, magnitude, phase, numPoints);

            Assert.AreEqual(0, result);

            // DC gain should be close to 1 for lowpass
            Assert.Greater(magnitude[0], 0.9f);
            Assert.Less(magnitude[0], 1.1f);

            // High frequencies should be attenuated
            Assert.Less(magnitude[numPoints - 1], 0.1f);
        }

        [Test]
        public void Freqz_HighpassFilter_CorrectResponse()
        {
            // Design highpass filter
            int order = 4;
            int maxSize = SpectraNative.spectra_iir_coeff_size(order);
            float[] b = new float[maxSize];
            float[] a = new float[maxSize];
            int bLen = 0, aLen = 0;

            int result = SpectraNative.spectra_butter(
                order, 0.3f, 1, b, ref bLen, a, ref aLen);
            Assert.AreEqual(0, result);

            // Compute frequency response
            int numPoints = 256;
            float[] magnitude = new float[numPoints];
            float[] phase = new float[numPoints];

            result = SpectraNative.spectra_freqz(
                b, bLen, a, aLen, magnitude, phase, numPoints);

            Assert.AreEqual(0, result);

            // DC should be attenuated for highpass
            Assert.Less(magnitude[0], 0.1f);

            // High frequencies should pass
            Assert.Greater(magnitude[numPoints - 1], 0.5f);
        }
    }
}
