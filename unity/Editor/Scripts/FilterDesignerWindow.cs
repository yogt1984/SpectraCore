using UnityEngine;
using UnityEditor;
using System;
using System.Text;
using Spectra;

namespace Spectra.Editor
{
    /// <summary>
    /// Interactive filter design tool with real-time Bode plot visualization.
    /// </summary>
    public class FilterDesignerWindow : EditorWindow
    {
        // Filter parameters
        private enum FilterDesignType
        {
            Butterworth,
            ChebyshevI,
            ChebyshevII,
            Elliptic
        }

        private FilterDesignType filterType = FilterDesignType.Butterworth;
        private FilterType responseType = FilterType.Lowpass;
        private int order = 4;
        private float cutoffFreq = 0.25f;
        private float passbandRipple = 0.5f;
        private float stopbandAtten = 40.0f;

        // Frequency response data
        private float[] magnitude;
        private float[] phase;
        private float[] frequencies;
        private float[] b;
        private float[] a;

        // UI state
        private Vector2 scrollPosition;
        private bool showCoefficients = false;
        private bool showMagnitude = true;
        private bool showPhase = true;
        private int numPoints = 512;
        private bool autoUpdate = true;
        private bool needsUpdate = true;

        // Display settings
        private float magnitudeMin = -80f;
        private float magnitudeMax = 10f;
        private float phaseMin = -180f;
        private float phaseMax = 180f;

        [MenuItem("Window/Spectra/Filter Designer")]
        public static void ShowWindow()
        {
            var window = GetWindow<FilterDesignerWindow>();
            window.titleContent = new GUIContent("Filter Designer");
            window.minSize = new Vector2(600, 500);
            window.Show();
        }

        private void OnEnable()
        {
            UpdateFilter();
        }

        private void OnGUI()
        {
            scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);

            DrawParameterPanel();
            EditorGUILayout.Space(10);
            DrawBodePlot();
            EditorGUILayout.Space(10);
            DrawCoefficientsPanel();

            EditorGUILayout.EndScrollView();

            if (needsUpdate && autoUpdate)
            {
                UpdateFilter();
                needsUpdate = false;
            }
        }

        private void DrawParameterPanel()
        {
            EditorGUILayout.LabelField("Filter Parameters", EditorStyles.boldLabel);

            EditorGUI.BeginChangeCheck();

            // Filter type selection
            filterType = (FilterDesignType)EditorGUILayout.EnumPopup("Filter Type", filterType);
            responseType = (FilterType)EditorGUILayout.EnumPopup("Response Type", responseType);

            // Order
            order = EditorGUILayout.IntSlider("Order", order, 1, 10);

            // Cutoff frequency
            cutoffFreq = EditorGUILayout.Slider("Normalized Cutoff", cutoffFreq, 0.01f, 0.99f);
            EditorGUILayout.LabelField("", $"(0 = DC, 1 = Nyquist frequency)", EditorStyles.miniLabel);

            // Type-specific parameters
            switch (filterType)
            {
                case FilterDesignType.ChebyshevI:
                    passbandRipple = EditorGUILayout.Slider("Passband Ripple (dB)", passbandRipple, 0.1f, 5.0f);
                    break;

                case FilterDesignType.ChebyshevII:
                    stopbandAtten = EditorGUILayout.Slider("Stopband Attenuation (dB)", stopbandAtten, 20f, 100f);
                    break;

                case FilterDesignType.Elliptic:
                    passbandRipple = EditorGUILayout.Slider("Passband Ripple (dB)", passbandRipple, 0.1f, 5.0f);
                    stopbandAtten = EditorGUILayout.Slider("Stopband Attenuation (dB)", stopbandAtten, 20f, 100f);
                    break;
            }

            // Display settings
            EditorGUILayout.Space(5);
            autoUpdate = EditorGUILayout.Toggle("Auto Update", autoUpdate);

            if (EditorGUI.EndChangeCheck())
            {
                needsUpdate = true;
            }

            if (!autoUpdate && GUILayout.Button("Update Filter"))
            {
                UpdateFilter();
            }

            EditorGUILayout.Space(5);

            // Export buttons
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Copy Coefficients"))
            {
                CopyCoefficientsToClipboard();
            }
            if (GUILayout.Button("Export to C#"))
            {
                ExportToCSharp();
            }
            EditorGUILayout.EndHorizontal();
        }

        private void DrawBodePlot()
        {
            if (magnitude == null || frequencies == null)
                return;

            EditorGUILayout.LabelField("Bode Plot", EditorStyles.boldLabel);

            // Display options
            EditorGUILayout.BeginHorizontal();
            showMagnitude = EditorGUILayout.Toggle("Magnitude", showMagnitude);
            showPhase = EditorGUILayout.Toggle("Phase", showPhase);
            EditorGUILayout.EndHorizontal();

            if (showMagnitude)
            {
                EditorGUILayout.LabelField("Magnitude Response (dB)", EditorStyles.boldLabel);
                Rect magRect = GUILayoutUtility.GetRect(position.width - 40, 200);
                DrawMagnitudePlot(magRect);
            }

            if (showPhase)
            {
                EditorGUILayout.Space(5);
                EditorGUILayout.LabelField("Phase Response (degrees)", EditorStyles.boldLabel);
                Rect phaseRect = GUILayoutUtility.GetRect(position.width - 40, 150);
                DrawPhasePlot(phaseRect);
            }
        }

        private void DrawMagnitudePlot(Rect rect)
        {
            // Background
            EditorGUI.DrawRect(rect, new Color(0.15f, 0.15f, 0.15f));

            // Convert magnitude to dB
            float[] magDb = new float[magnitude.Length];
            for (int i = 0; i < magnitude.Length; i++)
            {
                magDb[i] = 20f * Mathf.Log10(Mathf.Max(magnitude[i], 1e-10f));
            }

            // Draw grid
            DrawGrid(rect, magnitudeMin, magnitudeMax, "dB");

            // Draw magnitude response
            Handles.color = new Color(0.3f, 0.8f, 0.3f);
            Vector3 prevPoint = new Vector3(rect.x, 0, 0);

            for (int i = 0; i < magDb.Length; i++)
            {
                float x = rect.x + (float)i / (magDb.Length - 1) * rect.width;
                float normalized = Mathf.InverseLerp(magnitudeMin, magnitudeMax, magDb[i]);
                float y = rect.yMax - normalized * rect.height;

                Vector3 point = new Vector3(x, y, 0);

                if (i > 0)
                {
                    Handles.DrawLine(prevPoint, point);
                }

                prevPoint = point;
            }

            // Draw cutoff frequency marker
            float cutoffX = rect.x + cutoffFreq * rect.width;
            Handles.color = Color.yellow;
            Handles.DrawLine(new Vector3(cutoffX, rect.y, 0), new Vector3(cutoffX, rect.yMax, 0));
        }

        private void DrawPhasePlot(Rect rect)
        {
            // Background
            EditorGUI.DrawRect(rect, new Color(0.15f, 0.15f, 0.15f));

            // Convert phase to degrees
            float[] phaseDeg = new float[phase.Length];
            for (int i = 0; i < phase.Length; i++)
            {
                phaseDeg[i] = phase[i] * Mathf.Rad2Deg;
            }

            // Draw grid
            DrawGrid(rect, phaseMin, phaseMax, "deg");

            // Draw phase response
            Handles.color = new Color(0.3f, 0.6f, 0.9f);
            Vector3 prevPoint = new Vector3(rect.x, 0, 0);

            for (int i = 0; i < phaseDeg.Length; i++)
            {
                float x = rect.x + (float)i / (phaseDeg.Length - 1) * rect.width;
                float normalized = Mathf.InverseLerp(phaseMin, phaseMax, phaseDeg[i]);
                float y = rect.yMax - normalized * rect.height;

                Vector3 point = new Vector3(x, y, 0);

                if (i > 0)
                {
                    Handles.DrawLine(prevPoint, point);
                }

                prevPoint = point;
            }

            // Draw cutoff frequency marker
            float cutoffX = rect.x + cutoffFreq * rect.width;
            Handles.color = Color.yellow;
            Handles.DrawLine(new Vector3(cutoffX, rect.y, 0), new Vector3(cutoffX, rect.yMax, 0));
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
                Handles.Label(new Vector3(rect.x - 35, y - 8, 0),
                    $"{value:F0}{unit}", EditorStyles.miniLabel);
            }

            // Vertical grid lines (frequency markers)
            float[] freqMarkers = { 0f, 0.25f, 0.5f, 0.75f, 1.0f };
            foreach (float freq in freqMarkers)
            {
                float x = rect.x + freq * rect.width;
                Handles.DrawLine(new Vector3(x, rect.y, 0), new Vector3(x, rect.yMax, 0));

                // Label
                Handles.Label(new Vector3(x - 15, rect.yMax + 2, 0),
                    $"{freq:F2}π", EditorStyles.miniLabel);
            }
        }

        private void DrawCoefficientsPanel()
        {
            showCoefficients = EditorGUILayout.Foldout(showCoefficients, "Filter Coefficients", true);

            if (showCoefficients && b != null && a != null)
            {
                EditorGUI.indentLevel++;

                EditorGUILayout.LabelField($"Numerator (b) - {b.Length} coefficients:");
                EditorGUILayout.SelectableLabel(FormatCoefficients(b),
                    EditorStyles.textArea, GUILayout.Height(60));

                EditorGUILayout.Space(5);

                EditorGUILayout.LabelField($"Denominator (a) - {a.Length} coefficients:");
                EditorGUILayout.SelectableLabel(FormatCoefficients(a),
                    EditorStyles.textArea, GUILayout.Height(60));

                EditorGUI.indentLevel--;
            }
        }

        private void UpdateFilter()
        {
            try
            {
                // Design filter based on type
                switch (filterType)
                {
                    case FilterDesignType.Butterworth:
                        (b, a) = DSP.Butter(order, cutoffFreq, responseType);
                        break;

                    case FilterDesignType.ChebyshevI:
                        (b, a) = DSP.Cheby1(order, passbandRipple, cutoffFreq, responseType);
                        break;

                    case FilterDesignType.ChebyshevII:
                        (b, a) = DSP.Cheby2(order, stopbandAtten, cutoffFreq, responseType);
                        break;

                    case FilterDesignType.Elliptic:
                        (b, a) = DSP.Ellip(order, passbandRipple, stopbandAtten, cutoffFreq, responseType);
                        break;
                }

                // Compute frequency response
                (magnitude, phase, frequencies) = DSP.Freqz(b, a, numPoints);

                Repaint();
            }
            catch (Exception e)
            {
                Debug.LogError($"Filter design failed: {e.Message}");
            }
        }

        private string FormatCoefficients(float[] coeffs)
        {
            var sb = new StringBuilder();
            sb.Append("[ ");
            for (int i = 0; i < coeffs.Length; i++)
            {
                sb.AppendFormat("{0:F6}", coeffs[i]);
                if (i < coeffs.Length - 1)
                    sb.Append(", ");
            }
            sb.Append(" ]");
            return sb.ToString();
        }

        private void CopyCoefficientsToClipboard()
        {
            if (b == null || a == null)
                return;

            var sb = new StringBuilder();
            sb.AppendLine("// Filter Coefficients");
            sb.AppendLine($"// Type: {filterType}, Order: {order}, Cutoff: {cutoffFreq:F3}");
            sb.AppendLine();
            sb.AppendLine($"float[] b = new float[] {{ {FormatCoefficients(b)} }};");
            sb.AppendLine($"float[] a = new float[] {{ {FormatCoefficients(a)} }};");

            GUIUtility.systemCopyBuffer = sb.ToString();
            Debug.Log("Coefficients copied to clipboard");
        }

        private void ExportToCSharp()
        {
            if (b == null || a == null)
                return;

            string path = EditorUtility.SaveFilePanel(
                "Export Filter Coefficients",
                Application.dataPath,
                $"{filterType}Filter_{order}order",
                "cs");

            if (string.IsNullOrEmpty(path))
                return;

            var sb = new StringBuilder();
            sb.AppendLine("using Spectra;");
            sb.AppendLine();
            sb.AppendLine("/// <summary>");
            sb.AppendLine($"/// {filterType} {responseType} filter");
            sb.AppendLine($"/// Order: {order}, Cutoff: {cutoffFreq:F3}");
            if (filterType == FilterDesignType.ChebyshevI || filterType == FilterDesignType.Elliptic)
                sb.AppendLine($"/// Passband Ripple: {passbandRipple:F2} dB");
            if (filterType == FilterDesignType.ChebyshevII || filterType == FilterDesignType.Elliptic)
                sb.AppendLine($"/// Stopband Attenuation: {stopbandAtten:F2} dB");
            sb.AppendLine("/// </summary>");
            sb.AppendLine($"public class {filterType}Filter");
            sb.AppendLine("{");
            sb.AppendLine($"    public static readonly float[] B = new float[]");
            sb.AppendLine("    {");
            sb.Append("        ");
            for (int i = 0; i < b.Length; i++)
            {
                sb.AppendFormat("{0:F8}f", b[i]);
                if (i < b.Length - 1)
                    sb.Append(", ");
                if ((i + 1) % 4 == 0 && i < b.Length - 1)
                    sb.Append("\n        ");
            }
            sb.AppendLine();
            sb.AppendLine("    };");
            sb.AppendLine();
            sb.AppendLine($"    public static readonly float[] A = new float[]");
            sb.AppendLine("    {");
            sb.Append("        ");
            for (int i = 0; i < a.Length; i++)
            {
                sb.AppendFormat("{0:F8}f", a[i]);
                if (i < a.Length - 1)
                    sb.Append(", ");
                if ((i + 1) % 4 == 0 && i < a.Length - 1)
                    sb.Append("\n        ");
            }
            sb.AppendLine();
            sb.AppendLine("    };");
            sb.AppendLine("}");

            System.IO.File.WriteAllText(path, sb.ToString());
            AssetDatabase.Refresh();
            Debug.Log($"Filter exported to: {path}");
        }
    }
}
