using UnityEngine;
using UnityEditor;

namespace Spectra.Editor
{
    /// <summary>
    /// Editor window for visualizing signals, spectra, and filter responses.
    /// </summary>
    public class SignalInspectorWindow : EditorWindow
    {
        private AudioClip selectedClip;
        private float[] signalData;
        private float[] spectrum;
        private Vector2 scrollPosition;

        private bool showWaveform = true;
        private bool showSpectrum = true;
        private int fftSize = 2048;

        [MenuItem("Window/Spectra/Signal Inspector")]
        public static void ShowWindow()
        {
            var window = GetWindow<SignalInspectorWindow>();
            window.titleContent = new GUIContent("Signal Inspector");
            window.Show();
        }

        private void OnGUI()
        {
            EditorGUILayout.BeginVertical();

            // Audio clip selector
            EditorGUI.BeginChangeCheck();
            selectedClip = EditorGUILayout.ObjectField("Audio Clip", selectedClip, typeof(AudioClip), false) as AudioClip;
            if (EditorGUI.EndChangeCheck() && selectedClip != null)
            {
                LoadAudioData();
            }

            EditorGUILayout.Space();

            // Display options
            showWaveform = EditorGUILayout.Toggle("Show Waveform", showWaveform);
            showSpectrum = EditorGUILayout.Toggle("Show Spectrum", showSpectrum);
            fftSize = EditorGUILayout.IntPopup("FFT Size", fftSize,
                new[] { "256", "512", "1024", "2048", "4096", "8192" },
                new[] { 256, 512, 1024, 2048, 4096, 8192 });

            EditorGUILayout.Space();

            if (signalData != null && signalData.Length > 0)
            {
                scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);

                if (showWaveform)
                {
                    EditorGUILayout.LabelField("Waveform", EditorStyles.boldLabel);
                    Rect waveformRect = GUILayoutUtility.GetRect(position.width - 20, 150);
                    DrawWaveform(waveformRect);
                    EditorGUILayout.Space();
                }

                if (showSpectrum)
                {
                    EditorGUILayout.LabelField("Spectrum", EditorStyles.boldLabel);
                    Rect spectrumRect = GUILayoutUtility.GetRect(position.width - 20, 150);
                    DrawSpectrum(spectrumRect);
                }

                EditorGUILayout.EndScrollView();
            }
            else
            {
                EditorGUILayout.HelpBox("Select an AudioClip to visualize.", MessageType.Info);
            }

            EditorGUILayout.EndVertical();
        }

        private void LoadAudioData()
        {
            if (selectedClip == null) return;

            signalData = new float[selectedClip.samples * selectedClip.channels];
            selectedClip.GetData(signalData, 0);

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
            Repaint();
        }

        private void ComputeSpectrum()
        {
            if (signalData == null || signalData.Length < fftSize) return;

            // Take a segment from the middle
            int start = (signalData.Length - fftSize) / 2;
            float[] segment = new float[fftSize];
            System.Array.Copy(signalData, start, segment, 0, fftSize);

            // Apply window
            DSP.ApplyWindow(segment, WindowType.Hann);

            // Compute spectrum using native FFT
            using (var fft = new FFTAnalyzer(fftSize))
            {
                spectrum = fft.GetMagnitudeSpectrum(segment);
            }
        }

        private void DrawWaveform(Rect rect)
        {
            if (signalData == null) return;

            EditorGUI.DrawRect(rect, new Color(0.1f, 0.1f, 0.1f));

            // Draw waveform
            Handles.color = new Color(0.2f, 0.8f, 0.2f);
            float centerY = rect.y + rect.height / 2;
            int step = Mathf.Max(1, signalData.Length / (int)rect.width);

            Vector3 prevPoint = new Vector3(rect.x, centerY, 0);
            for (int i = 0; i < rect.width; i++)
            {
                int sampleIndex = (int)(i * signalData.Length / rect.width);
                float sample = signalData[Mathf.Min(sampleIndex, signalData.Length - 1)];
                float y = centerY - sample * rect.height / 2;
                Vector3 point = new Vector3(rect.x + i, y, 0);
                Handles.DrawLine(prevPoint, point);
                prevPoint = point;
            }

            // Center line
            Handles.color = Color.gray;
            Handles.DrawLine(new Vector3(rect.x, centerY, 0), new Vector3(rect.xMax, centerY, 0));
        }

        private void DrawSpectrum(Rect rect)
        {
            if (spectrum == null) return;

            EditorGUI.DrawRect(rect, new Color(0.1f, 0.1f, 0.1f));

            // Find max for normalization
            float maxVal = 0.001f;
            foreach (float v in spectrum)
            {
                if (v > maxVal) maxVal = v;
            }

            // Draw bars
            float barWidth = rect.width / spectrum.Length;
            for (int i = 0; i < spectrum.Length; i++)
            {
                float normalized = spectrum[i] / maxVal;
                float height = normalized * rect.height;
                Rect barRect = new Rect(
                    rect.x + i * barWidth,
                    rect.yMax - height,
                    Mathf.Max(1, barWidth - 1),
                    height);

                float hue = (float)i / spectrum.Length * 0.7f;
                EditorGUI.DrawRect(barRect, Color.HSVToRGB(hue, 0.8f, 0.8f));
            }
        }
    }
}
