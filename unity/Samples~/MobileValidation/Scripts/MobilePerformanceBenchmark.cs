using UnityEngine;
using UnityEngine.UI;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Spectra;

namespace Spectra.Samples.MobileValidation
{
    /// <summary>
    /// Performance benchmarking for SpectraCore on mobile platforms.
    /// Measures execution time and throughput for all DSP operations.
    /// </summary>
    public class MobilePerformanceBenchmark : MonoBehaviour
    {
        [Header("UI References")]
        public Text statusText;
        public Text resultsText;
        public Button runBenchmarkButton;
        public ScrollRect scrollRect;

        [Header("Benchmark Configuration")]
        public bool runOnStart = false;
        public int iterations = 100;
        public int signalLength = 2048;

        private StringBuilder _results = new StringBuilder();
        private bool _isRunning = false;

        private void Start()
        {
            if (runBenchmarkButton != null)
                runBenchmarkButton.onClick.AddListener(RunBenchmarks);

            UpdateStatus("Ready to benchmark. Tap 'Run Benchmark' to begin.");

            if (runOnStart)
            {
                StartCoroutine(RunBenchmarksAfterDelay(1f));
            }
        }

        private IEnumerator RunBenchmarksAfterDelay(float delay)
        {
            yield return new WaitForSeconds(delay);
            RunBenchmarks();
        }

        public void RunBenchmarks()
        {
            if (_isRunning)
            {
                Log("Benchmark already running...");
                return;
            }

            StartCoroutine(RunBenchmarksCoroutine());
        }

        private IEnumerator RunBenchmarksCoroutine()
        {
            _isRunning = true;
            _results.Clear();

            if (runBenchmarkButton != null)
                runBenchmarkButton.interactable = false;

            Log("=== SpectraCore Mobile Performance Benchmark ===");
            Log($"Platform: {Application.platform}");
            Log($"Device: {SystemInfo.deviceModel}");
            Log($"Processor: {SystemInfo.processorType} ({SystemInfo.processorCount} cores)");
            Log($"Memory: {SystemInfo.systemMemorySize} MB");
            Log($"Iterations: {iterations}");
            Log($"Signal Length: {signalLength}");
            Log("");

            // FFT Benchmarks
            yield return BenchmarkCategory("FFT Operations", BenchmarkFFT);

            // IIR Filter Design Benchmarks
            yield return BenchmarkCategory("IIR Filter Design", BenchmarkIIRDesign);

            // Filter Application Benchmarks
            yield return BenchmarkCategory("Filter Application", BenchmarkFilterApplication);

            // FIR Filter Benchmarks
            yield return BenchmarkCategory("FIR Design", BenchmarkFIRDesign);

            // Analysis Benchmarks
            yield return BenchmarkCategory("Frequency Analysis", BenchmarkFrequencyAnalysis);

            // STFT Benchmarks
            yield return BenchmarkCategory("STFT", BenchmarkSTFT);

            // Summary
            Log("");
            Log("=== Benchmark Complete ===");
            UpdateStatus("Benchmark complete!");

            if (runBenchmarkButton != null)
                runBenchmarkButton.interactable = true;

            _isRunning = false;
        }

        private IEnumerator BenchmarkCategory(string category, Action benchmarkMethod)
        {
            Log($"\n--- {category} ---");
            UpdateStatus($"Benchmarking: {category}");

            try
            {
                benchmarkMethod?.Invoke();
            }
            catch (Exception ex)
            {
                Log($"ERROR: {ex.Message}");
            }

            yield return null;
        }

        #region FFT Benchmarks

        private void BenchmarkFFT()
        {
            float[] signal = GenerateSineWave(signalLength, 440, 44100);

            // FFT Forward
            Benchmark("FFT Forward", () =>
            {
                using (var fft = new FFT(signalLength))
                {
                    for (int i = 0; i < iterations; i++)
                    {
                        var (real, imag) = fft.Forward(signal);
                    }
                }
            });

            // FFT Inverse
            using (var fft = new FFT(signalLength))
            {
                var (real, imag) = fft.Forward(signal);

                Benchmark("FFT Inverse", () =>
                {
                    for (int i = 0; i < iterations; i++)
                    {
                        float[] output = fft.Inverse(real, imag);
                    }
                });
            }

            // FFT Round-trip
            Benchmark("FFT Round-trip", () =>
            {
                using (var fft = new FFT(signalLength))
                {
                    for (int i = 0; i < iterations; i++)
                    {
                        var (real, imag) = fft.Forward(signal);
                        float[] output = fft.Inverse(real, imag);
                    }
                }
            });
        }

        #endregion

        #region IIR Filter Design Benchmarks

        private void BenchmarkIIRDesign()
        {
            Benchmark("Butterworth Design", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
                }
            });

            Benchmark("Chebyshev I Design", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    var (b, a) = DSP.Cheby1(4, 0.5f, 0.3f, FilterType.Lowpass);
                }
            });

            Benchmark("Chebyshev II Design", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    var (b, a) = DSP.Cheby2(4, 40f, 0.3f, FilterType.Lowpass);
                }
            });

            Benchmark("Elliptic Design", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    var (b, a) = DSP.Ellip(4, 0.5f, 60f, 0.3f, FilterType.Lowpass);
                }
            });

            Benchmark("Bessel Design", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    var (b, a) = DSP.Bessel(4, 0.3f, FilterType.Lowpass);
                }
            });

            Benchmark("Bandpass Design", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    var (b, a) = DSP.Butter(4, 0.2f, 0.5f, FilterType.Bandpass);
                }
            });
        }

        #endregion

        #region Filter Application Benchmarks

        private void BenchmarkFilterApplication()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
            float[] signal = GenerateSineWave(signalLength, 440, 44100);

            Benchmark("LFilter", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    float[] output = DSP.LFilter(b, a, signal);
                }
            });

            Benchmark("FiltFilt", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    float[] output = DSP.FiltFilt(b, a, signal);
                }
            });

            Benchmark("Streaming Filter", () =>
            {
                using (var filter = new StreamingFilter(b, a))
                {
                    int chunkSize = 256;
                    float[] chunk = new float[chunkSize];

                    for (int i = 0; i < iterations; i++)
                    {
                        Array.Copy(signal, 0, chunk, 0, chunkSize);
                        float[] output = filter.Process(chunk);
                    }
                }
            });
        }

        #endregion

        #region FIR Design Benchmarks

        private void BenchmarkFIRDesign()
        {
            Benchmark("FIR1 Lowpass", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    float[] h = DSP.Fir1(64, 0.3f, FilterType.Lowpass);
                }
            });

            Benchmark("FIR1 Bandpass", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    float[] h = DSP.Fir1(64, new[] { 0.2f, 0.5f }, FilterType.Bandpass);
                }
            });

            Benchmark("FirPM", () =>
            {
                float[] freqs = { 0f, 0.2f, 0.3f, 1f };
                float[] amps = { 1f, 1f, 0f, 0f };

                for (int i = 0; i < iterations / 10; i++) // FirPM is slower
                {
                    float[] h = DSP.FirPM(64, freqs, amps);
                }
            });
        }

        #endregion

        #region Frequency Analysis Benchmarks

        private void BenchmarkFrequencyAnalysis()
        {
            var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);

            Benchmark("Freqz", () =>
            {
                for (int i = 0; i < iterations; i++)
                {
                    var (mag, phase, freqs) = DSP.Freqz(b, a, 512);
                }
            });

            float[] signal = GenerateSineWave(signalLength, 440, 44100);

            Benchmark("Cross-Correlation", () =>
            {
                for (int i = 0; i < iterations / 10; i++)
                {
                    float[] xcorr = DSP.XCorr(signal, signal);
                }
            });

            Benchmark("Pwelch", () =>
            {
                for (int i = 0; i < iterations / 10; i++)
                {
                    var (psd, freqs) = DSP.Pwelch(signal, 512, 256, WindowType.Hann, 44100);
                }
            });
        }

        #endregion

        #region STFT Benchmarks

        private void BenchmarkSTFT()
        {
            float[] signal = GenerateSineWave(signalLength, 440, 44100);

            Benchmark("STFT Analyze", () =>
            {
                using (var stft = new STFT(512, 256, WindowType.Hann))
                {
                    for (int i = 0; i < iterations / 10; i++)
                    {
                        var (real, imag, numFrames) = stft.Analyze(signal);
                    }
                }
            });

            Benchmark("STFT Streaming", () =>
            {
                using (var stft = new STFT(512, 256, WindowType.Hann))
                {
                    float[] chunk = new float[256];

                    for (int i = 0; i < iterations; i++)
                    {
                        Array.Copy(signal, 0, chunk, 0, 256);
                        stft.PushSamples(chunk);

                        while (stft.FramesAvailable() > 0)
                        {
                            var (real, imag) = stft.PopFrame();
                        }
                    }
                }
            });
        }

        #endregion

        #region Utility Methods

        private void Benchmark(string name, Action action)
        {
            // Warm-up
            try
            {
                action();
            }
            catch (Exception ex)
            {
                Log($"  {name}: ERROR - {ex.Message}");
                return;
            }

            // Force garbage collection before benchmark
            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();

            // Measure
            Stopwatch sw = Stopwatch.StartNew();
            try
            {
                action();
            }
            catch (Exception ex)
            {
                Log($"  {name}: ERROR - {ex.Message}");
                return;
            }
            sw.Stop();

            double totalMs = sw.Elapsed.TotalMilliseconds;
            double avgMs = totalMs / iterations;
            double opsPerSec = 1000.0 / avgMs;

            Log($"  {name}:");
            Log($"    Total: {totalMs:F2} ms");
            Log($"    Average: {avgMs:F3} ms");
            Log($"    Throughput: {opsPerSec:F1} ops/sec");

            // Check if real-time capable (for audio processing)
            if (name.Contains("Filter") || name.Contains("STFT") || name.Contains("FFT"))
            {
                // For 2048 samples at 44.1kHz, we have ~46ms available
                double availableTime = (signalLength / 44100.0) * 1000.0;
                double cpuLoad = (avgMs / availableTime) * 100.0;

                if (cpuLoad < 50)
                {
                    Log($"    CPU Load: {cpuLoad:F1}% ✓ Real-time capable");
                }
                else if (cpuLoad < 100)
                {
                    Log($"    CPU Load: {cpuLoad:F1}% ⚠ Marginal");
                }
                else
                {
                    Log($"    CPU Load: {cpuLoad:F1}% ✗ Not real-time");
                }
            }
        }

        private void Log(string message)
        {
            _results.AppendLine(message);
            UnityEngine.Debug.Log(message);
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

        #endregion
    }
}
