using NUnit.Framework;
using System;
using Spectra;

namespace Spectra.Tests
{
    /// <summary>
    /// Tests for high-level Unity wrapper components.
    /// </summary>
    public class SpectraComponentTests
    {
        // ====================================================================
        // DSP Filter Design Tests
        // ====================================================================

        [Test]
        public void DSP_Butter_DesignsValidFilter()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);

            Assert.Greater(b.Length, 0);
            Assert.Greater(a.Length, 0);
            Assert.AreNotEqual(0.0f, b[0]);
            Assert.AreNotEqual(0.0f, a[0]);
        }

        [Test]
        public void DSP_Cheby1_DesignsValidFilter()
        {
            var (b, a) = DSP.Cheby1(4, 0.5f, 0.3f, FilterType.Lowpass);

            Assert.Greater(b.Length, 0);
            Assert.Greater(a.Length, 0);
            Assert.AreNotEqual(0.0f, b[0]);
            Assert.AreNotEqual(0.0f, a[0]);
        }

        [Test]
        public void DSP_Cheby2_DesignsValidFilter()
        {
            var (b, a) = DSP.Cheby2(4, 40.0f, 0.3f, FilterType.Lowpass);

            Assert.Greater(b.Length, 0);
            Assert.Greater(a.Length, 0);
            Assert.AreNotEqual(0.0f, b[0]);
            Assert.AreNotEqual(0.0f, a[0]);
        }

        [Test]
        public void DSP_Ellip_DesignsValidFilter()
        {
            var (b, a) = DSP.Ellip(4, 0.5f, 40.0f, 0.3f, FilterType.Lowpass);

            Assert.Greater(b.Length, 0);
            Assert.Greater(a.Length, 0);
            Assert.AreNotEqual(0.0f, b[0]);
            Assert.AreNotEqual(0.0f, a[0]);
        }

        // ====================================================================
        // DSP Filter Application Tests
        // ====================================================================

        [Test]
        public void DSP_LFilter_AppliesFilter()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);

            float[] input = new float[256];
            for (int i = 0; i < 256; i++)
                input[i] = (float)Math.Sin(2.0 * Math.PI * 10 * i / 256);

            float[] output = DSP.LFilter(b, a, input);

            Assert.AreEqual(input.Length, output.Length);
            Assert.AreNotEqual(input[100], output[100], 0.0001f);
        }

        [Test]
        public void DSP_FiltFilt_AppliesZeroPhaseFilter()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);

            float[] input = new float[256];
            for (int i = 0; i < 256; i++)
                input[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 256);

            float[] output = DSP.FiltFilt(b, a, input);

            Assert.AreEqual(input.Length, output.Length);
        }

        // ====================================================================
        // DSP Analysis Tests
        // ====================================================================

        [Test]
        public void DSP_Xcorr_ComputesCorrelation()
        {
            float[] x = new float[100];
            float[] y = new float[100];

            for (int i = 0; i < 100; i++)
            {
                x[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 100);
                y[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 100);
            }

            float[] corr = DSP.Xcorr(x, y);

            Assert.Greater(corr.Length, 0);
        }

        [Test]
        public void DSP_Envelope_ComputesAmplitude()
        {
            float[] input = new float[256];
            for (int i = 0; i < 256; i++)
                input[i] = (float)(Math.Sin(2.0 * Math.PI * 5 * i / 256) *
                                   (0.5 + 0.5 * Math.Cos(2.0 * Math.PI * i / 256)));

            float[] envelope = DSP.Envelope(input);

            Assert.AreEqual(input.Length, envelope.Length);
            for (int i = 0; i < envelope.Length; i++)
                Assert.GreaterOrEqual(envelope[i], 0);
        }

        [Test]
        public void DSP_Hilbert_ComputesAnalyticSignal()
        {
            float[] input = new float[256];
            for (int i = 0; i < 256; i++)
                input[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 256);

            var (real, imag) = DSP.Hilbert(input);

            Assert.AreEqual(input.Length, real.Length);
            Assert.AreEqual(input.Length, imag.Length);

            // Real part should approximately match input
            for (int i = 50; i < 200; i++)
                Assert.AreEqual(input[i], real[i], 0.1f);
        }

        [Test]
        public void DSP_Pwelch_ComputesPSD()
        {
            float[] signal = new float[4096];
            for (int i = 0; i < signal.Length; i++)
                signal[i] = (float)Math.Sin(2.0 * Math.PI * 440 * i / 44100);

            var (psd, freqs) = DSP.Pwelch(signal, 256, 128, WindowType.Hann, 44100.0f);

            Assert.Greater(psd.Length, 0);
            Assert.AreEqual(psd.Length, freqs.Length);
            Assert.AreEqual(129, psd.Length); // 256/2 + 1

            // Check that PSD has energy
            float maxPsd = 0;
            for (int i = 0; i < psd.Length; i++)
                maxPsd = Math.Max(maxPsd, psd[i]);
            Assert.Greater(maxPsd, 0);
        }

        [Test]
        public void DSP_Resample_ChangesLength()
        {
            float[] input = new float[100];
            for (int i = 0; i < 100; i++)
                input[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 100);

            // Upsample by 2
            float[] output = DSP.Resample(input, 2, 1);

            Assert.Greater(output.Length, input.Length);
        }

        [Test]
        public void DSP_Window_GeneratesWindow()
        {
            float[] window = DSP.Window(256, WindowType.Hann);

            Assert.AreEqual(256, window.Length);
            Assert.Less(window[0], 0.1f); // Should be near zero at edges
            Assert.Less(window[255], 0.1f);
            Assert.Greater(window[128], 0.9f); // Should be near 1.0 at center
        }

        [Test]
        public void DSP_ApplyWindow_ModifiesData()
        {
            float[] data = new float[256];
            for (int i = 0; i < 256; i++)
                data[i] = 1.0f;

            float[] original = (float[])data.Clone();
            DSP.ApplyWindow(data, WindowType.Hann);

            Assert.AreNotEqual(original[0], data[0]);
            Assert.Less(data[0], 0.1f);
            Assert.Less(data[255], 0.1f);
        }

        // ====================================================================
        // STFTAnalyzer Tests
        // ====================================================================

        [Test]
        public void STFTAnalyzer_CreateDispose_Works()
        {
            using (var stft = new STFTAnalyzer(256, 128, WindowType.Hann, true))
            {
                Assert.AreEqual(256, stft.FftSize);
                Assert.AreEqual(128, stft.HopSize);
                Assert.AreEqual(129, stft.SpectrumSize); // 256/2 + 1
            }
        }

        [Test]
        public void STFTAnalyzer_Analyze_ComputesFrames()
        {
            using (var stft = new STFTAnalyzer(256, 128, WindowType.Hann, false))
            {
                float[] input = new float[1024];
                for (int i = 0; i < input.Length; i++)
                    input[i] = (float)Math.Sin(2.0 * Math.PI * 10 * i / 256);

                var (real, imag, numFrames) = stft.Analyze(input);

                Assert.Greater(numFrames, 0);
                Assert.AreEqual(real.Length, numFrames * stft.SpectrumSize);
                Assert.AreEqual(imag.Length, numFrames * stft.SpectrumSize);
            }
        }

        [Test]
        public void STFTAnalyzer_StreamingMode_Works()
        {
            using (var stft = new STFTAnalyzer(256, 128, WindowType.Hann, false))
            {
                float[] samples = new float[512];
                for (int i = 0; i < samples.Length; i++)
                    samples[i] = (float)Math.Sin(2.0 * Math.PI * 10 * i / 256);

                stft.PushSamples(samples);

                Assert.Greater(stft.FramesAvailable, 0);

                bool gotFrame = stft.PopFrame(out float[] real, out float[] imag);

                Assert.IsTrue(gotFrame);
                Assert.AreEqual(stft.SpectrumSize, real.Length);
                Assert.AreEqual(stft.SpectrumSize, imag.Length);
            }
        }

        [Test]
        public void STFTAnalyzer_ComputeMagnitude_Works()
        {
            float[] real = new float[] { 3.0f, 4.0f, 0.0f };
            float[] imag = new float[] { 4.0f, 3.0f, 1.0f };

            float[] mag = STFTAnalyzer.ComputeMagnitude(real, imag);

            Assert.AreEqual(3, mag.Length);
            Assert.AreEqual(5.0f, mag[0], 0.0001f); // sqrt(3^2 + 4^2) = 5
            Assert.AreEqual(5.0f, mag[1], 0.0001f); // sqrt(4^2 + 3^2) = 5
            Assert.AreEqual(1.0f, mag[2], 0.0001f); // sqrt(0^2 + 1^2) = 1
        }

        [Test]
        public void STFTAnalyzer_ComputePower_Works()
        {
            float[] real = new float[] { 3.0f, 4.0f, 0.0f };
            float[] imag = new float[] { 4.0f, 3.0f, 1.0f };

            float[] power = STFTAnalyzer.ComputePower(real, imag);

            Assert.AreEqual(3, power.Length);
            Assert.AreEqual(25.0f, power[0], 0.0001f); // 3^2 + 4^2 = 25
            Assert.AreEqual(25.0f, power[1], 0.0001f); // 4^2 + 3^2 = 25
            Assert.AreEqual(1.0f, power[2], 0.0001f);  // 0^2 + 1^2 = 1
        }

        [Test]
        public void STFTAnalyzer_PowerToDb_Works()
        {
            float[] power = new float[] { 1.0f, 10.0f, 100.0f };

            float[] db = STFTAnalyzer.PowerToDb(power, 1.0f);

            Assert.AreEqual(3, db.Length);
            Assert.AreEqual(0.0f, db[0], 0.01f);     // 10*log10(1) = 0
            Assert.AreEqual(10.0f, db[1], 0.01f);    // 10*log10(10) = 10
            Assert.AreEqual(20.0f, db[2], 0.01f);    // 10*log10(100) = 20
        }

        // ====================================================================
        // FFTAnalyzer Tests
        // ====================================================================

        [Test]
        public void FFTAnalyzer_CreateDispose_Works()
        {
            using (var fft = new FFTAnalyzer(256))
            {
                Assert.AreEqual(256, fft.Size);
                Assert.AreEqual(129, fft.SpectrumSize); // 256/2 + 1
            }
        }

        [Test]
        public void FFTAnalyzer_Forward_ComputesSpectrum()
        {
            using (var fft = new FFTAnalyzer(256))
            {
                float[] input = new float[256];
                for (int i = 0; i < 256; i++)
                    input[i] = (float)Math.Sin(2.0 * Math.PI * 5 * i / 256);

                var (real, imag) = fft.Forward(input);

                Assert.AreEqual(256, real.Length);
                Assert.AreEqual(256, imag.Length);

                // Check DC component
                Assert.Less(Math.Abs(real[0]), 0.1f);
            }
        }

        [Test]
        public void FFTAnalyzer_GetMagnitudeSpectrum_Works()
        {
            using (var fft = new FFTAnalyzer(256))
            {
                float[] input = new float[256];
                for (int i = 0; i < 256; i++)
                    input[i] = 1.0f; // DC signal

                float[] magnitude = fft.GetMagnitudeSpectrum(input);

                Assert.AreEqual(129, magnitude.Length);
                Assert.Greater(magnitude[0], 100.0f); // Strong DC component
            }
        }

        // ====================================================================
        // StreamingIIRFilter Tests
        // ====================================================================

        [Test]
        public void StreamingIIRFilter_CreateDispose_Works()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
            using (var filter = new StreamingIIRFilter(b, a))
            {
                // Should not throw
            }
        }

        [Test]
        public void StreamingIIRFilter_Process_FiltersSignal()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
            using (var filter = new StreamingIIRFilter(b, a))
            {
                float[] input = new float[256];
                for (int i = 0; i < 256; i++)
                    input[i] = (float)Math.Sin(2.0 * Math.PI * 10 * i / 256);

                float[] output = filter.Process(input);

                Assert.AreEqual(input.Length, output.Length);
            }
        }

        [Test]
        public void StreamingIIRFilter_Reset_ClearsState()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
            using (var filter = new StreamingIIRFilter(b, a))
            {
                float[] input = new float[256];
                for (int i = 0; i < 256; i++)
                    input[i] = 1.0f;

                filter.Process(input);
                filter.Reset();

                // After reset, should behave like new filter
                float[] output = filter.Process(input);
                Assert.AreEqual(input.Length, output.Length);
            }
        }

        // ====================================================================
        // OnsetDetector Tests
        // ====================================================================

        [Test]
        public void OnsetDetector_CreateDispose_Works()
        {
            var config = new OnsetConfig
            {
                FftSize = 512,
                HopSize = 128,
                Threshold = 1.5f,
                MinIntervalMs = 50.0f
            };

            using (var detector = new OnsetDetector(config, 44100.0f))
            {
                // Should not throw
            }
        }

        [Test]
        public void OnsetDetector_Process_Works()
        {
            var config = new OnsetConfig
            {
                FftSize = 512,
                HopSize = 128,
                Threshold = 1.5f,
                MinIntervalMs = 50.0f
            };

            using (var detector = new OnsetDetector(config, 44100.0f))
            {
                bool onsetDetected = false;
                detector.OnOnsetDetected += (time, strength) =>
                {
                    onsetDetected = true;
                };

                float[] samples = new float[4096];
                // Create a transient
                for (int i = 1000; i < 1010; i++)
                    samples[i] = 1.0f;

                detector.Process(samples);

                // Onset may or may not be detected depending on threshold
                // Just verify no crash
            }
        }

        [Test]
        public void OnsetDetector_Reset_Works()
        {
            var config = new OnsetConfig
            {
                FftSize = 512,
                HopSize = 128,
                Threshold = 1.5f,
                MinIntervalMs = 50.0f
            };

            using (var detector = new OnsetDetector(config, 44100.0f))
            {
                float[] samples = new float[2048];
                detector.Process(samples);
                detector.Reset();

                // Should not throw after reset
                detector.Process(samples);
            }
        }

        // ====================================================================
        // Frequency Response Tests
        // ====================================================================

        [Test]
        public void DSP_Freqz_ComputesFrequencyResponse()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);

            var (mag, phase, freqs) = DSP.Freqz(b, a, 256);

            Assert.AreEqual(256, mag.Length);
            Assert.AreEqual(256, phase.Length);
            Assert.AreEqual(256, freqs.Length);

            // DC gain should be close to 1 for lowpass
            Assert.Greater(mag[0], 0.9f);
            Assert.Less(mag[0], 1.1f);

            // High frequencies should be attenuated
            Assert.Less(mag[255], 0.1f);

            // Frequencies should range from 0 to π
            Assert.AreEqual(0f, freqs[0], 0.001f);
            Assert.AreEqual(Mathf.PI, freqs[255], 0.001f);
        }

        [Test]
        public void DSP_FreqzDb_ReturnsDecibelScale()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);

            var (magDb, phase, freqs) = DSP.FreqzDb(b, a, 256);

            Assert.AreEqual(256, magDb.Length);

            // DC gain should be close to 0 dB for lowpass
            Assert.Greater(magDb[0], -1f);
            Assert.Less(magDb[0], 1f);

            // High frequencies should be heavily attenuated (large negative dB)
            Assert.Less(magDb[255], -20f);
        }

        [Test]
        public void DSP_Freqz_DifferentFilterTypes()
        {
            // Test different filter types produce expected responses
            var filterTypes = new[]
            {
                (FilterType.Lowpass, "Lowpass"),
                (FilterType.Highpass, "Highpass")
            };

            foreach (var (type, name) in filterTypes)
            {
                var (b, a) = DSP.Butter(4, 0.3f, type);
                var (mag, _, _) = DSP.Freqz(b, a, 256);

                if (type == FilterType.Lowpass)
                {
                    // DC should pass, high freq attenuated
                    Assert.Greater(mag[0], 0.9f, $"{name} DC gain");
                    Assert.Less(mag[255], 0.1f, $"{name} high freq");
                }
                else if (type == FilterType.Highpass)
                {
                    // DC should be attenuated, high freq pass
                    Assert.Less(mag[0], 0.1f, $"{name} DC gain");
                    Assert.Greater(mag[255], 0.5f, $"{name} high freq");
                }
            }
        }

        [Test]
        public void DSP_Freqz_Cheby1Filter()
        {
            var (b, a) = DSP.Cheby1(4, 0.5f, 0.3f, FilterType.Lowpass);
            var (mag, phase, freqs) = DSP.Freqz(b, a);

            Assert.Greater(mag.Length, 0);
            Assert.AreEqual(mag.Length, phase.Length);
            Assert.AreEqual(mag.Length, freqs.Length);

            // Should have DC gain close to 1
            Assert.Greater(mag[0], 0.8f);
        }

        [Test]
        public void DSP_Freqz_EllipticFilter()
        {
            var (b, a) = DSP.Ellip(4, 0.5f, 40f, 0.3f, FilterType.Lowpass);
            var (mag, phase, freqs) = DSP.Freqz(b, a);

            Assert.Greater(mag.Length, 0);

            // Elliptic should have steeper rolloff than Butterworth
            // Check that high frequencies are well attenuated
            Assert.Less(mag[mag.Length - 1], 0.01f);
        }
    }
}
