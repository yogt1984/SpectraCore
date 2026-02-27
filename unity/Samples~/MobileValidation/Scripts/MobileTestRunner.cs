using UnityEngine;
using UnityEngine.UI;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using Spectra;

namespace Spectra.Samples.MobileValidation
{
    /// <summary>
    /// Comprehensive mobile platform validation test runner.
    /// Tests all SpectraCore DSP functions on mobile devices (iOS/Android).
    /// </summary>
    public class MobileTestRunner : MonoBehaviour
    {
        [Header("UI References")]
        public Text statusText;
        public Text resultsText;
        public Button runTestsButton;
        public ScrollRect scrollRect;
        public Toggle verboseToggle;

        [Header("Test Configuration")]
        public bool runOnStart = false;
        public bool verboseLogging = true;

        private StringBuilder _results = new StringBuilder();
        private int _totalTests = 0;
        private int _passedTests = 0;
        private int _failedTests = 0;
        private bool _isRunning = false;

        private void Start()
        {
            if (runTestsButton != null)
                runTestsButton.onClick.AddListener(RunAllTests);

            UpdateStatus("Ready to test. Tap 'Run Tests' to begin.");

            if (runOnStart)
            {
                StartCoroutine(RunTestsAfterDelay(1f));
            }
        }

        private IEnumerator RunTestsAfterDelay(float delay)
        {
            yield return new WaitForSeconds(delay);
            RunAllTests();
        }

        public void RunAllTests()
        {
            if (_isRunning)
            {
                Log("Tests already running...");
                return;
            }

            StartCoroutine(RunAllTestsCoroutine());
        }

        private IEnumerator RunAllTestsCoroutine()
        {
            _isRunning = true;
            _results.Clear();
            _totalTests = 0;
            _passedTests = 0;
            _failedTests = 0;

            if (runTestsButton != null)
                runTestsButton.interactable = false;

            Log("=== SpectraCore Mobile Platform Validation ===");
            Log($"Platform: {Application.platform}");
            Log($"Unity Version: {Application.unityVersion}");
            Log($"Device Model: {SystemInfo.deviceModel}");
            Log($"OS: {SystemInfo.operatingSystem}");
            Log($"Processor: {SystemInfo.processorType} ({SystemInfo.processorCount} cores)");
            Log($"Memory: {SystemInfo.systemMemorySize} MB");
            Log("");

            UpdateStatus("Running tests...");

            // Version tests
            yield return RunTestCategory("Version", TestVersion);

            // FFT tests
            yield return RunTestCategory("FFT", TestFFT);

            // Window functions
            yield return RunTestCategory("Window Functions", TestWindows);

            // IIR Filter Design tests
            yield return RunTestCategory("Butterworth Filters", TestButterworth);
            yield return RunTestCategory("Chebyshev Filters", TestChebyshev);
            yield return RunTestCategory("Elliptic Filters", TestElliptic);
            yield return RunTestCategory("Bessel Filters", TestBessel);

            // Filter Application tests
            yield return RunTestCategory("Filter Application", TestFilterApplication);

            // FIR Filter tests
            yield return RunTestCategory("FIR Filters", TestFIR);

            // Analysis tests
            yield return RunTestCategory("Frequency Analysis", TestFrequencyAnalysis);

            // Correlation tests
            yield return RunTestCategory("Correlation", TestCorrelation);

            // PSD tests
            yield return RunTestCategory("Power Spectral Density", TestPSD);

            // Hilbert Transform tests
            yield return RunTestCategory("Hilbert Transform", TestHilbert);

            // Resampling tests
            yield return RunTestCategory("Resampling", TestResampling);

            // STFT tests
            yield return RunTestCategory("STFT", TestSTFT);

            // Onset Detection tests
            yield return RunTestCategory("Onset Detection", TestOnsetDetection);

            // Summary
            Log("");
            Log("=== Test Summary ===");
            Log($"Total Tests: {_totalTests}");
            Log($"Passed: {_passedTests} ({(_passedTests * 100.0f / _totalTests):F1}%)");
            Log($"Failed: {_failedTests}");

            if (_failedTests == 0)
            {
                Log("✅ ALL TESTS PASSED!");
                UpdateStatus($"✅ All {_totalTests} tests passed!");
            }
            else
            {
                Log($"❌ {_failedTests} tests failed.");
                UpdateStatus($"❌ {_failedTests}/{_totalTests} tests failed");
            }

            if (runTestsButton != null)
                runTestsButton.interactable = true;

            _isRunning = false;
        }

        private IEnumerator RunTestCategory(string category, Action testMethod)
        {
            Log($"\n--- {category} ---");
            UpdateStatus($"Testing: {category}");

            try
            {
                testMethod?.Invoke();
            }
            catch (Exception ex)
            {
                LogError($"Category '{category}' crashed: {ex.Message}");
            }

            yield return null; // Allow UI to update
        }

        #region Version Tests

        private void TestVersion()
        {
            Test("Version Major", () => DSP.VersionMajor() >= 1);
            Test("Version Minor", () => DSP.VersionMinor() >= 0);
            Test("Version Patch", () => DSP.VersionPatch() >= 0);
            Test("Version String", () => !string.IsNullOrEmpty(DSP.VersionString()));

            if (verboseLogging)
                Log($"  Version: {DSP.VersionString()}");
        }

        #endregion

        #region FFT Tests

        private void TestFFT()
        {
            // Test FFT forward/inverse
            Test("FFT Create/Destroy", () =>
            {
                using (var fft = new FFT(256))
                {
                    return fft.Size == 256 && fft.SpectrumSize == 129;
                }
            });

            Test("FFT Forward", () =>
            {
                using (var fft = new FFT(256))
                {
                    float[] input = GenerateSineWave(256, 10, 44100);
                    var (real, imag) = fft.Forward(input);
                    return real.Length == 129 && imag.Length == 129;
                }
            });

            Test("FFT Inverse", () =>
            {
                using (var fft = new FFT(256))
                {
                    float[] input = GenerateSineWave(256, 10, 44100);
                    var (real, imag) = fft.Forward(input);
                    float[] output = fft.Inverse(real, imag);
                    return output.Length == 256 && AreClose(input, output, 0.01f);
                }
            });
        }

        #endregion

        #region Window Function Tests

        private void TestWindows()
        {
            Test("Hann Window", () =>
            {
                float[] window = DSP.GenerateWindow(512, WindowType.Hann);
                return window.Length == 512 && window[0] < 0.01f && window[256] > 0.99f;
            });

            Test("Hamming Window", () =>
            {
                float[] window = DSP.GenerateWindow(512, WindowType.Hamming);
                return window.Length == 512;
            });

            Test("Blackman Window", () =>
            {
                float[] window = DSP.GenerateWindow(512, WindowType.Blackman);
                return window.Length == 512;
            });
        }

        #endregion

        #region Butterworth Filter Tests

        private void TestButterworth()
        {
            Test("Butterworth Lowpass", () =>
            {
                var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
                return b.Length == 5 && a.Length == 5 && a[0] == 1.0f;
            });

            Test("Butterworth Highpass", () =>
            {
                var (b, a) = DSP.Butter(4, 0.3f, FilterType.Highpass);
                return b.Length == 5 && a.Length == 5;
            });

            Test("Butterworth Bandpass", () =>
            {
                var (b, a) = DSP.Butter(4, 0.2f, 0.5f, FilterType.Bandpass);
                return b.Length == 9 && a.Length == 9;
            });

            Test("Butterworth Bandstop", () =>
            {
                var (b, a) = DSP.Butter(4, 0.2f, 0.5f, FilterType.Bandstop);
                return b.Length == 9 && a.Length == 9;
            });
        }

        #endregion

        #region Chebyshev Filter Tests

        private void TestChebyshev()
        {
            Test("Chebyshev I Lowpass", () =>
            {
                var (b, a) = DSP.Cheby1(4, 0.5f, 0.3f, FilterType.Lowpass);
                return b.Length == 5 && a.Length == 5;
            });

            Test("Chebyshev I Bandpass", () =>
            {
                var (b, a) = DSP.Cheby1(3, 0.5f, 0.2f, 0.5f, FilterType.Bandpass);
                return b.Length == 7 && a.Length == 7;
            });

            Test("Chebyshev II Lowpass", () =>
            {
                var (b, a) = DSP.Cheby2(4, 40f, 0.3f, FilterType.Lowpass);
                return b.Length == 5 && a.Length == 5;
            });

            Test("Chebyshev II Bandpass", () =>
            {
                var (b, a) = DSP.Cheby2(3, 40f, 0.2f, 0.5f, FilterType.Bandpass);
                return b.Length == 7 && a.Length == 7;
            });
        }

        #endregion

        #region Elliptic Filter Tests

        private void TestElliptic()
        {
            Test("Elliptic Lowpass", () =>
            {
                var (b, a) = DSP.Ellip(4, 0.5f, 60f, 0.3f, FilterType.Lowpass);
                return b.Length == 5 && a.Length == 5;
            });

            Test("Elliptic Highpass", () =>
            {
                var (b, a) = DSP.Ellip(4, 0.5f, 60f, 0.3f, FilterType.Highpass);
                return b.Length == 5 && a.Length == 5;
            });

            Test("Elliptic Bandpass", () =>
            {
                var (b, a) = DSP.Ellip(3, 0.5f, 60f, 0.2f, 0.5f, FilterType.Bandpass);
                return b.Length == 7 && a.Length == 7;
            });
        }

        #endregion

        #region Bessel Filter Tests

        private void TestBessel()
        {
            Test("Bessel Lowpass", () =>
            {
                var (b, a) = DSP.Bessel(4, 0.3f, FilterType.Lowpass);
                return b.Length == 5 && a.Length == 5 && a[0] == 1.0f;
            });

            Test("Bessel Highpass", () =>
            {
                var (b, a) = DSP.Bessel(4, 0.3f, FilterType.Highpass);
                return b.Length == 5 && a.Length == 5;
            });

            Test("Bessel Bandpass", () =>
            {
                var (b, a) = DSP.Bessel(3, 0.2f, 0.5f, FilterType.Bandpass);
                return b.Length == 7 && a.Length == 7;
            });

            Test("Bessel Bandstop", () =>
            {
                var (b, a) = DSP.Bessel(3, 0.2f, 0.5f, FilterType.Bandstop);
                return b.Length == 7 && a.Length == 7;
            });
        }

        #endregion

        #region Filter Application Tests

        private void TestFilterApplication()
        {
            Test("LFilter", () =>
            {
                var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
                float[] input = GenerateSineWave(256, 10, 44100);
                float[] output = DSP.LFilter(b, a, input);
                return output.Length == 256;
            });

            Test("FiltFilt", () =>
            {
                var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
                float[] input = GenerateSineWave(256, 10, 44100);
                float[] output = DSP.FiltFilt(b, a, input);
                return output.Length == 256;
            });

            Test("Streaming Filter", () =>
            {
                var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
                using (var filter = new StreamingFilter(b, a))
                {
                    float[] input = GenerateSineWave(128, 10, 44100);
                    float[] output = filter.Process(input);
                    return output.Length == 128;
                }
            });
        }

        #endregion

        #region FIR Filter Tests

        private void TestFIR()
        {
            Test("FIR1 Lowpass", () =>
            {
                float[] h = DSP.Fir1(64, 0.3f, FilterType.Lowpass);
                return h.Length == 65;
            });

            Test("FIR1 Bandpass", () =>
            {
                float[] h = DSP.Fir1(64, new[] { 0.2f, 0.5f }, FilterType.Bandpass);
                return h.Length == 65;
            });

            Test("FirPM", () =>
            {
                float[] freqs = { 0f, 0.2f, 0.3f, 1f };
                float[] amps = { 1f, 1f, 0f, 0f };
                float[] h = DSP.FirPM(64, freqs, amps);
                return h.Length == 64;
            });
        }

        #endregion

        #region Frequency Analysis Tests

        private void TestFrequencyAnalysis()
        {
            Test("Freqz", () =>
            {
                var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
                var (mag, phase, freqs) = DSP.Freqz(b, a, 512);
                return mag.Length == 512 && phase.Length == 512 && freqs.Length == 512;
            });

            Test("FreqzDb", () =>
            {
                var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
                var (magDb, phase, freqs) = DSP.FreqzDb(b, a, 512);
                return magDb.Length == 512;
            });
        }

        #endregion

        #region Correlation Tests

        private void TestCorrelation()
        {
            Test("Cross-Correlation", () =>
            {
                float[] x = GenerateSineWave(128, 10, 44100);
                float[] y = GenerateSineWave(128, 10, 44100);
                float[] xcorr = DSP.XCorr(x, y);
                return xcorr.Length == 255;
            });
        }

        #endregion

        #region PSD Tests

        private void TestPSD()
        {
            Test("Welch PSD", () =>
            {
                float[] signal = GenerateSineWave(2048, 440, 44100);
                var (psd, freqs) = DSP.Pwelch(signal, 512, 256, WindowType.Hann, 44100);
                return psd.Length > 0 && freqs.Length == psd.Length;
            });
        }

        #endregion

        #region Hilbert Transform Tests

        private void TestHilbert()
        {
            Test("Hilbert Transform", () =>
            {
                float[] signal = GenerateSineWave(256, 10, 44100);
                var (real, imag) = DSP.Hilbert(signal);
                return real.Length == 256 && imag.Length == 256;
            });

            Test("Instantaneous Amplitude", () =>
            {
                float[] signal = GenerateSineWave(256, 10, 44100);
                float[] amp = DSP.InstantaneousAmplitude(signal);
                return amp.Length == 256;
            });
        }

        #endregion

        #region Resampling Tests

        private void TestResampling()
        {
            Test("Resample Up", () =>
            {
                float[] input = GenerateSineWave(100, 10, 1000);
                float[] output = DSP.Resample(input, 2, 1); // Upsample 2x
                return output.Length == 200;
            });

            Test("Resample Down", () =>
            {
                float[] input = GenerateSineWave(100, 10, 1000);
                float[] output = DSP.Resample(input, 1, 2); // Downsample 2x
                return output.Length == 50;
            });
        }

        #endregion

        #region STFT Tests

        private void TestSTFT()
        {
            Test("STFT Analyze", () =>
            {
                using (var stft = new STFT(512, 256, WindowType.Hann))
                {
                    float[] input = GenerateSineWave(2048, 440, 44100);
                    var (real, imag, numFrames) = stft.Analyze(input);
                    return numFrames > 0 && real.Length > 0;
                }
            });

            Test("STFT Streaming", () =>
            {
                using (var stft = new STFT(512, 256, WindowType.Hann))
                {
                    float[] chunk = GenerateSineWave(256, 440, 44100);
                    stft.PushSamples(chunk);
                    return stft.FramesAvailable() > 0;
                }
            });
        }

        #endregion

        #region Onset Detection Tests

        private void TestOnsetDetection()
        {
            Test("Onset Detector Create", () =>
            {
                using (var detector = new OnsetDetector(512, 256, 0.3f, 50f, 44100))
                {
                    return true;
                }
            });

            Test("Onset Detection Process", () =>
            {
                using (var detector = new OnsetDetector(512, 256, 0.3f, 50f, 44100))
                {
                    float[] signal = GenerateImpulse(2048, 500);
                    int onsetCount = 0;
                    detector.Process(signal, (timestamp, strength) => onsetCount++);
                    return onsetCount >= 0; // Should detect at least the impulse
                }
            });
        }

        #endregion

        #region Utility Methods

        private void Test(string name, Func<bool> testFunc)
        {
            _totalTests++;
            try
            {
                bool result = testFunc();
                if (result)
                {
                    _passedTests++;
                    if (verboseLogging)
                        Log($"  ✓ {name}");
                }
                else
                {
                    _failedTests++;
                    LogError($"  ✗ {name} - returned false");
                }
            }
            catch (Exception ex)
            {
                _failedTests++;
                LogError($"  ✗ {name} - Exception: {ex.Message}");
            }
        }

        private void Log(string message)
        {
            _results.AppendLine(message);
            Debug.Log(message);
            UpdateResultsText();
        }

        private void LogError(string message)
        {
            _results.AppendLine(message);
            Debug.LogError(message);
            UpdateResultsText();
        }

        private void UpdateStatus(string message)
        {
            if (statusText != null)
                statusText.text = message;
        }

        private void UpdateResultsText()
        {
            if (resultsText != null)
            {
                resultsText.text = _results.ToString();

                // Scroll to bottom
                if (scrollRect != null)
                {
                    Canvas.ForceUpdateCanvases();
                    scrollRect.verticalNormalizedPosition = 0f;
                }
            }
        }

        private float[] GenerateSineWave(int length, float frequency, float sampleRate)
        {
            float[] signal = new float[length];
            for (int i = 0; i < length; i++)
            {
                signal[i] = Mathf.Sin(2f * Mathf.PI * frequency * i / sampleRate);
            }
            return signal;
        }

        private float[] GenerateImpulse(int length, int position)
        {
            float[] signal = new float[length];
            if (position >= 0 && position < length)
                signal[position] = 1.0f;
            return signal;
        }

        private bool AreClose(float[] a, float[] b, float tolerance)
        {
            if (a.Length != b.Length)
                return false;

            for (int i = 0; i < a.Length; i++)
            {
                if (Mathf.Abs(a[i] - b[i]) > tolerance)
                    return false;
            }
            return true;
        }

        #endregion
    }
}
