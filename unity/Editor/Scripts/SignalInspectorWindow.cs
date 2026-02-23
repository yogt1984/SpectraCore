using UnityEngine;
using UnityEditor;
using System;
using System.IO;
using System.Text;

namespace Spectra.Editor
{
    /// <summary>
    /// Editor window for visualizing signals, spectra, and filter responses.
    /// Enhanced version with filter overlay, CSV export, and spectrogram.
    /// </summary>
    public class SignalInspectorWindow : EditorWindow
    {
        // Audio data
        private AudioClip selectedClip;
        private float[] signalData;
        private float[] spectrum;
        private float sampleRate;

        // Filter overlay
        private bool showFilterOverlay = false;
        private float[] filterMagnitude;
        private float[] filterPhase;
        private float[] filterB;
        private float[] filterA;
        private string filterName = "No filter loaded";

        // Spectrogram
        private bool showSpectrogram = false;
        private float[][] spectrogramData;
        private int spectrogramFrames;

        // Display options
        private bool showWaveform = true;
        private bool showSpectrum = true;
        private int fftSize = 2048;
        private bool useLogScale = false;
        private bool useDbScale = true;
        private Vector2 scrollPosition;

        // Export
        private bool showExportOptions = false;

        [MenuItem("Window/Spectra/Signal Inspector")]
        public static void ShowWindow()
        {
            var window = GetWindow<SignalInspectorWindow>();
            window.titleContent = new GUIContent("Signal Inspector");
            window.minSize = new Vector2(600, 400);
            window.Show();
        }

        private void OnGUI()
        {
            scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);
            EditorGUILayout.BeginVertical();

            DrawControlPanel();
            EditorGUILayout.Space(10);

            if (signalData != null && signalData.Length > 0)
            {
                DrawVisualizationPanel();
            }
            else
            {
                EditorGUILayout.HelpBox("Select an AudioClip to visualize.", MessageType.Info);
            }

            EditorGUILayout.EndVertical();
            EditorGUILayout.EndScrollView();
        }

        private void DrawControlPanel()
        {
            EditorGUILayout.LabelField("Signal Inspector", EditorStyles.boldLabel);

            // Audio clip selector
            EditorGUI.BeginChangeCheck();
            selectedClip = EditorGUILayout.ObjectField("Audio Clip", selectedClip, typeof(AudioClip), false) as AudioClip;
            if (EditorGUI.EndChangeCheck() && selectedClip != null)
            {
                LoadAudioData();
            }

            if (selectedClip != null)
            {
                EditorGUILayout.LabelField("Info", $"{selectedClip.length:F2}s, {selectedClip.frequency}Hz, {selectedClip.channels}ch", EditorStyles.miniLabel);
            }

            EditorGUILayout.Space(5);

            // Display options
            EditorGUILayout.BeginHorizontal();
            showWaveform = EditorGUILayout.Toggle("Waveform", showWaveform);
            showSpectrum = EditorGUILayout.Toggle("Spectrum", showSpectrum);
            showSpectrogram = EditorGUILayout.Toggle("Spectrogram", showSpectrogram);
            EditorGUILayout.EndHorizontal();

            // FFT options
            EditorGUI.BeginChangeCheck();
            fftSize = EditorGUILayout.IntPopup("FFT Size", fftSize,
                new[] { "256", "512", "1024", "2048", "4096", "8192" },
                new[] { 256, 512, 1024, 2048, 4096, 8192 });

            if (EditorGUI.EndChangeCheck() && signalData != null)
            {
                ComputeSpectrum();
                if (showSpectrogram)
                    ComputeSpectrogram();
            }

            // Scale options
            EditorGUILayout.BeginHorizontal();
            useDbScale = EditorGUILayout.Toggle("dB Scale", useDbScale);
            useLogScale = EditorGUILayout.Toggle("Log Frequency", useLogScale);
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.Space(5);

            // Filter overlay section
            showFilterOverlay = EditorGUILayout.Foldout(showFilterOverlay, "Filter Overlay", true);
            if (showFilterOverlay)
            {
                EditorGUI.indentLevel++;

                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Current Filter:", filterName, EditorStyles.miniLabel);
                if (GUILayout.Button("Load from Designer", GUILayout.Width(150)))
                {
                    LoadFilterFromDesigner();
                }
                if (filterB != null && GUILayout.Button("Clear", GUILayout.Width(60)))
                {
                    ClearFilter();
                }
                EditorGUILayout.EndHorizontal();

                if (filterB != null)
                {
                    EditorGUILayout.HelpBox("Filter loaded. Overlay will be shown on spectrum.", MessageType.Info);
                }

                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space(5);

            // Export section
            showExportOptions = EditorGUILayout.Foldout(showExportOptions, "Export", true);
            if (showExportOptions)
            {
                EditorGUI.indentLevel++;

                EditorGUILayout.BeginHorizontal();
                if (GUILayout.Button("Export Waveform (CSV)") && signalData != null)
                {
                    ExportWaveformCSV();
                }
                if (GUILayout.Button("Export Spectrum (CSV)") && spectrum != null)
                {
                    ExportSpectrumCSV();
                }
                EditorGUILayout.EndHorizontal();

                if (showSpectrogram && spectrogramData != null)
                {
                    if (GUILayout.Button("Export Spectrogram (CSV)"))
                    {
                        ExportSpectrogramCSV();
                    }
                }

                EditorGUI.indentLevel--;
            }
        }

        private void DrawVisualizationPanel()
        {
            if (showWaveform)
            {
                EditorGUILayout.LabelField("Waveform", EditorStyles.boldLabel);
                Rect waveformRect = GUILayoutUtility.GetRect(position.width - 40, 150);
                DrawWaveform(waveformRect);
                EditorGUILayout.Space(10);
            }

            if (showSpectrum)
            {
                EditorGUILayout.LabelField("Spectrum", EditorStyles.boldLabel);
                Rect spectrumRect = GUILayoutUtility.GetRect(position.width - 40, 200);
                DrawSpectrum(spectrumRect);
                EditorGUILayout.Space(10);
            }

            if (showSpectrogram)
            {
                EditorGUILayout.LabelField("Spectrogram", EditorStyles.boldLabel);
                Rect spectrogramRect = GUILayoutUtility.GetRect(position.width - 40, 250);
                DrawSpectrogram(spectrogramRect);
            }
        }

        private void LoadAudioData()
        {
            if (selectedClip == null) return;

            signalData = new float[selectedClip.samples * selectedClip.channels];
            selectedClip.GetData(signalData, 0);
            sampleRate = selectedClip.frequency;

            // Convert to mono if stereo
            if (selectedClip.channels == 2)
            {
                float[] mono = new float[signalData.Length / 2];
                for (int i = 0; i < mono.Length; i++)
                {
                    mono[i] = (signalData[i * 2] + signalData[i * 2 + 1]) * 0.5f;
                }
                signalData = mono;
            }

            ComputeSpectrum();

            if (showSpectrogram)
            {
                ComputeSpectrogram();
            }

            Repaint();
        }

        private void ComputeSpectrum()
        {
            if (signalData == null || signalData.Length < fftSize) return;

            // Take a segment from the middle
            int start = (signalData.Length - fftSize) / 2;
            float[] segment = new float[fftSize];
            Array.Copy(signalData, start, segment, 0, fftSize);

            // Apply window
            DSP.ApplyWindow(segment, WindowType.Hann);

            // Compute spectrum using native FFT
            using (var fft = new FFTAnalyzer(fftSize))
            {
                spectrum = fft.GetMagnitudeSpectrum(segment);
            }
        }

        private void ComputeSpectrogram()
        {
            if (signalData == null || signalData.Length < fftSize) return;

            int hopSize = fftSize / 4; // 75% overlap
            int maxFrames = (signalData.Length - fftSize) / hopSize + 1;

            // Limit frames for performance
            maxFrames = Mathf.Min(maxFrames, 500);

            spectrogramData = new float[maxFrames][];

            using (var fft = new FFTAnalyzer(fftSize))
            {
                for (int frame = 0; frame < maxFrames; frame++)
                {
                    int start = frame * hopSize;
                    if (start + fftSize > signalData.Length)
                        break;

                    float[] segment = new float[fftSize];
                    Array.Copy(signalData, start, segment, 0, fftSize);

                    // Apply window
                    DSP.ApplyWindow(segment, WindowType.Hann);

                    // Compute magnitude spectrum
                    spectrogramData[frame] = fft.GetMagnitudeSpectrum(segment);
                }
            }

            spectrogramFrames = maxFrames;
        }

        private void LoadFilterFromDesigner()
        {
            // This is a simplified version - in a real implementation,
            // you could use EditorPrefs or a shared data structure
            EditorGUILayout.HelpBox(
                "Filter Designer integration: Design a filter in the Filter Designer window, " +
                "then use this button to load it for overlay visualization.\n\n" +
                "For now, you can manually create a filter in code.",
                MessageType.Info);

            // Placeholder: Create a simple Butterworth filter for demonstration
            try
            {
                var (b, a) = DSP.Butter(4, 0.25f, FilterType.Lowpass);
                filterB = b;
                filterA = a;
                filterName = "Butterworth LP (0.25)";

                // Compute filter frequency response
                var (mag, phase, _) = DSP.Freqz(b, a, spectrum.Length);
                filterMagnitude = mag;
                filterPhase = phase;

                Debug.Log("Filter loaded successfully for overlay");
                Repaint();
            }
            catch (Exception e)
            {
                Debug.LogError($"Failed to load filter: {e.Message}");
            }
        }

        private void ClearFilter()
        {
            filterB = null;
            filterA = null;
            filterMagnitude = null;
            filterPhase = null;
            filterName = "No filter loaded";
            Repaint();
        }

        private void DrawWaveform(Rect rect)
        {
            if (signalData == null) return;

            // Background
            EditorGUI.DrawRect(rect, new Color(0.1f, 0.1f, 0.1f));

            // Grid
            DrawGrid(rect, -1f, 1f, "Amplitude");

            // Draw waveform
            Handles.color = new Color(0.2f, 0.8f, 0.2f);
            float centerY = rect.y + rect.height / 2;

            Vector3 prevPoint = new Vector3(rect.x, centerY, 0);
            for (int i = 0; i < rect.width; i++)
            {
                int sampleIndex = (int)(i * signalData.Length / rect.width);
                float sample = signalData[Mathf.Min(sampleIndex, signalData.Length - 1)];
                float y = centerY - sample * rect.height / 2;
                Vector3 point = new Vector3(rect.x + i, y, 0);

                if (i > 0)
                {
                    Handles.DrawLine(prevPoint, point);
                }

                prevPoint = point;
            }

            // Time axis labels
            DrawTimeAxis(rect, signalData.Length / sampleRate);
        }

        private void DrawSpectrum(Rect rect)
        {
            if (spectrum == null) return;

            // Background
            EditorGUI.DrawRect(rect, new Color(0.1f, 0.1f, 0.1f));

            // Convert to dB if needed
            float[] displaySpectrum = new float[spectrum.Length];
            float minDb = -80f;
            float maxDb = 0f;

            if (useDbScale)
            {
                for (int i = 0; i < spectrum.Length; i++)
                {
                    displaySpectrum[i] = 20f * Mathf.Log10(Mathf.Max(spectrum[i], 1e-10f));
                    displaySpectrum[i] = Mathf.Clamp(displaySpectrum[i], minDb, maxDb);
                }
            }
            else
            {
                Array.Copy(spectrum, displaySpectrum, spectrum.Length);
            }

            // Find range for normalization
            float minVal = useDbScale ? minDb : 0f;
            float maxVal = useDbScale ? maxDb : 0f;

            if (!useDbScale)
            {
                foreach (float v in displaySpectrum)
                {
                    if (v > maxVal) maxVal = v;
                }
                maxVal = Mathf.Max(maxVal, 0.001f);
            }

            // Draw grid
            DrawGrid(rect, minVal, maxVal, useDbScale ? "dB" : "Magnitude");

            // Draw spectrum
            Handles.color = new Color(0.3f, 0.6f, 0.9f);
            Vector3 prevPoint = Vector3.zero;

            for (int i = 0; i < spectrum.Length; i++)
            {
                float x = rect.x + (float)i / (spectrum.Length - 1) * rect.width;
                float normalized = Mathf.InverseLerp(minVal, maxVal, displaySpectrum[i]);
                float y = rect.yMax - normalized * rect.height;

                Vector3 point = new Vector3(x, y, 0);

                if (i > 0)
                {
                    Handles.DrawLine(prevPoint, point);
                }

                prevPoint = point;
            }

            // Draw filter overlay if loaded
            if (filterMagnitude != null && filterB != null)
            {
                DrawFilterOverlay(rect, minVal, maxVal);
            }

            // Frequency axis
            DrawFrequencyAxis(rect, sampleRate / 2);
        }

        private void DrawFilterOverlay(Rect rect, float minVal, float maxVal)
        {
            if (filterMagnitude == null) return;

            // Convert filter response to dB if needed
            float[] displayFilter = new float[filterMagnitude.Length];

            if (useDbScale)
            {
                for (int i = 0; i < filterMagnitude.Length; i++)
                {
                    displayFilter[i] = 20f * Mathf.Log10(Mathf.Max(filterMagnitude[i], 1e-10f));
                }
            }
            else
            {
                Array.Copy(filterMagnitude, displayFilter, filterMagnitude.Length);
            }

            // Draw filter response
            Handles.color = new Color(1.0f, 0.5f, 0.0f, 0.8f); // Orange, semi-transparent
            Vector3 prevPoint = Vector3.zero;

            for (int i = 0; i < filterMagnitude.Length; i++)
            {
                float x = rect.x + (float)i / (filterMagnitude.Length - 1) * rect.width;
                float normalized = Mathf.InverseLerp(minVal, maxVal, displayFilter[i]);
                float y = rect.yMax - normalized * rect.height;

                Vector3 point = new Vector3(x, y, 0);

                if (i > 0)
                {
                    Handles.DrawLine(prevPoint, point);
                }

                prevPoint = point;
            }

            // Legend
            Handles.color = new Color(1.0f, 0.5f, 0.0f);
            Handles.Label(new Vector3(rect.x + 10, rect.y + 10, 0),
                $"Filter: {filterName}", EditorStyles.whiteBoldLabel);
        }

        private void DrawSpectrogram(Rect rect)
        {
            if (spectrogramData == null || spectrogramFrames == 0) return;

            EditorGUI.DrawRect(rect, new Color(0.1f, 0.1f, 0.1f));

            int numBins = spectrogramData[0].Length;
            float binHeight = rect.height / numBins;
            float frameWidth = rect.width / spectrogramFrames;

            // Find global max for normalization
            float maxVal = 0.001f;
            for (int frame = 0; frame < spectrogramFrames; frame++)
            {
                if (spectrogramData[frame] == null) continue;
                foreach (float v in spectrogramData[frame])
                {
                    if (v > maxVal) maxVal = v;
                }
            }

            // Draw spectrogram
            for (int frame = 0; frame < spectrogramFrames; frame++)
            {
                if (spectrogramData[frame] == null) continue;

                for (int bin = 0; bin < numBins; bin++)
                {
                    float magnitude = spectrogramData[frame][bin];
                    float normalized = magnitude / maxVal;

                    // Color mapping: blue (low) -> red (high)
                    Color color = Color.Lerp(Color.blue, Color.red, normalized);

                    Rect pixelRect = new Rect(
                        rect.x + frame * frameWidth,
                        rect.yMax - (bin + 1) * binHeight,
                        Mathf.Max(1, frameWidth),
                        binHeight);

                    EditorGUI.DrawRect(pixelRect, color);
                }
            }

            // Labels
            Handles.color = Color.white;
            Handles.Label(new Vector3(rect.x + 5, rect.y + 5, 0), "Frequency →", EditorStyles.miniLabel);
            Handles.Label(new Vector3(rect.x + 5, rect.yMax - 20, 0), "Time →", EditorStyles.miniLabel);
        }

        private void DrawGrid(Rect rect, float minVal, float maxVal, string unit)
        {
            Handles.color = new Color(0.3f, 0.3f, 0.3f);

            // Horizontal grid lines
            int numHLines = 5;
            for (int i = 0; i <= numHLines; i++)
            {
                float y = rect.y + (float)i / numHLines * rect.height;
                Handles.DrawLine(new Vector3(rect.x, y, 0), new Vector3(rect.xMax, y, 0));

                // Label
                float value = Mathf.Lerp(maxVal, minVal, (float)i / numHLines);
                Handles.Label(new Vector3(rect.x - 50, y - 8, 0),
                    $"{value:F1}{unit}", EditorStyles.miniLabel);
            }
        }

        private void DrawFrequencyAxis(Rect rect, float nyquist)
        {
            Handles.color = Color.gray;

            // Draw frequency markers
            float[] freqMarkers = { 0f, 0.25f, 0.5f, 0.75f, 1.0f };
            foreach (float norm in freqMarkers)
            {
                float x = rect.x + norm * rect.width;
                float freq = norm * nyquist;

                Handles.DrawLine(new Vector3(x, rect.yMax, 0), new Vector3(x, rect.yMax + 5, 0));

                string label = freq >= 1000 ? $"{freq / 1000:F1}k" : $"{freq:F0}";
                Handles.Label(new Vector3(x - 15, rect.yMax + 5, 0), label, EditorStyles.miniLabel);
            }

            Handles.Label(new Vector3(rect.xMax + 10, rect.yMax, 0), "Hz", EditorStyles.miniLabel);
        }

        private void DrawTimeAxis(Rect rect, float duration)
        {
            Handles.color = Color.gray;

            // Draw time markers
            float[] timeMarkers = { 0f, 0.25f, 0.5f, 0.75f, 1.0f };
            foreach (float norm in timeMarkers)
            {
                float x = rect.x + norm * rect.width;
                float time = norm * duration;

                Handles.DrawLine(new Vector3(x, rect.yMax, 0), new Vector3(x, rect.yMax + 5, 0));
                Handles.Label(new Vector3(x - 15, rect.yMax + 5, 0),
                    $"{time:F2}s", EditorStyles.miniLabel);
            }
        }

        private void ExportWaveformCSV()
        {
            string path = EditorUtility.SaveFilePanel(
                "Export Waveform Data",
                Application.dataPath,
                $"{selectedClip.name}_waveform",
                "csv");

            if (string.IsNullOrEmpty(path)) return;

            try
            {
                var sb = new StringBuilder();
                sb.AppendLine("Sample,Time(s),Amplitude");

                for (int i = 0; i < signalData.Length; i++)
                {
                    float time = i / sampleRate;
                    sb.AppendLine($"{i},{time:F6},{signalData[i]:F6}");
                }

                File.WriteAllText(path, sb.ToString());
                Debug.Log($"Waveform exported to: {path}");
                EditorUtility.DisplayDialog("Export Complete",
                    $"Waveform data exported successfully.\n\n{signalData.Length} samples written.",
                    "OK");
            }
            catch (Exception e)
            {
                Debug.LogError($"Export failed: {e.Message}");
                EditorUtility.DisplayDialog("Export Failed", e.Message, "OK");
            }
        }

        private void ExportSpectrumCSV()
        {
            string path = EditorUtility.SaveFilePanel(
                "Export Spectrum Data",
                Application.dataPath,
                $"{selectedClip.name}_spectrum",
                "csv");

            if (string.IsNullOrEmpty(path)) return;

            try
            {
                var sb = new StringBuilder();
                sb.AppendLine("Bin,Frequency(Hz),Magnitude,Magnitude(dB)");

                float nyquist = sampleRate / 2;
                for (int i = 0; i < spectrum.Length; i++)
                {
                    float freq = (float)i / (spectrum.Length - 1) * nyquist;
                    float magDb = 20f * Mathf.Log10(Mathf.Max(spectrum[i], 1e-10f));
                    sb.AppendLine($"{i},{freq:F2},{spectrum[i]:F6},{magDb:F2}");
                }

                File.WriteAllText(path, sb.ToString());
                Debug.Log($"Spectrum exported to: {path}");
                EditorUtility.DisplayDialog("Export Complete",
                    $"Spectrum data exported successfully.\n\n{spectrum.Length} frequency bins written.",
                    "OK");
            }
            catch (Exception e)
            {
                Debug.LogError($"Export failed: {e.Message}");
                EditorUtility.DisplayDialog("Export Failed", e.Message, "OK");
            }
        }

        private void ExportSpectrogramCSV()
        {
            string path = EditorUtility.SaveFilePanel(
                "Export Spectrogram Data",
                Application.dataPath,
                $"{selectedClip.name}_spectrogram",
                "csv");

            if (string.IsNullOrEmpty(path)) return;

            try
            {
                var sb = new StringBuilder();

                // Header: Frame, Bin0, Bin1, Bin2, ...
                sb.Append("Frame,Time(s)");
                for (int bin = 0; bin < spectrogramData[0].Length; bin++)
                {
                    float freq = (float)bin / spectrogramData[0].Length * (sampleRate / 2);
                    sb.Append($",Bin{bin}({freq:F0}Hz)");
                }
                sb.AppendLine();

                // Data rows
                int hopSize = fftSize / 4;
                for (int frame = 0; frame < spectrogramFrames; frame++)
                {
                    if (spectrogramData[frame] == null) continue;

                    float time = frame * hopSize / sampleRate;
                    sb.Append($"{frame},{time:F3}");

                    foreach (float magnitude in spectrogramData[frame])
                    {
                        sb.Append($",{magnitude:F6}");
                    }
                    sb.AppendLine();
                }

                File.WriteAllText(path, sb.ToString());
                Debug.Log($"Spectrogram exported to: {path}");
                EditorUtility.DisplayDialog("Export Complete",
                    $"Spectrogram data exported successfully.\n\n{spectrogramFrames} frames × {spectrogramData[0].Length} bins written.",
                    "OK");
            }
            catch (Exception e)
            {
                Debug.LogError($"Export failed: {e.Message}");
                EditorUtility.DisplayDialog("Export Failed", e.Message, "OK");
            }
        }
    }
}
