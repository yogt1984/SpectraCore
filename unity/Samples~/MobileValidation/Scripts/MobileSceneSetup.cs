using UnityEngine;
using UnityEngine.UI;

#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Spectra.Samples.MobileValidation
{
    /// <summary>
    /// Editor utility to automatically set up the Mobile Validation test scene.
    /// Run this once to create the complete UI hierarchy.
    /// </summary>
    public class MobileSceneSetup : MonoBehaviour
    {
#if UNITY_EDITOR
        [MenuItem("Spectra/Setup Mobile Test Scene")]
        public static void SetupScene()
        {
            // Find or create Canvas
            Canvas canvas = FindObjectOfType<Canvas>();
            if (canvas == null)
            {
                GameObject canvasObj = new GameObject("Canvas");
                canvas = canvasObj.AddComponent<Canvas>();
                canvas.renderMode = RenderMode.ScreenSpaceOverlay;

                CanvasScaler scaler = canvasObj.AddComponent<CanvasScaler>();
                scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
                scaler.referenceResolution = new Vector2(1080, 1920); // Portrait mobile
                scaler.matchWidthOrHeight = 0.5f;

                canvasObj.AddComponent<GraphicRaycaster>();

                Debug.Log("Created Canvas");
            }

            // Ensure EventSystem exists
            UnityEngine.EventSystems.EventSystem eventSystem = FindObjectOfType<UnityEngine.EventSystems.EventSystem>();
            if (eventSystem == null)
            {
                GameObject eventSystemObj = new GameObject("EventSystem");
                eventSystemObj.AddComponent<UnityEngine.EventSystems.EventSystem>();
                eventSystemObj.AddComponent<UnityEngine.EventSystems.StandaloneInputModule>();
                Debug.Log("Created EventSystem");
            }

            // Create main panel
            GameObject panel = CreatePanel(canvas.transform, "MainPanel");

            // Create header
            GameObject header = CreatePanel(panel.transform, "Header");
            RectTransform headerRect = header.GetComponent<RectTransform>();
            headerRect.anchorMin = new Vector2(0, 1);
            headerRect.anchorMax = new Vector2(1, 1);
            headerRect.pivot = new Vector2(0.5f, 1);
            headerRect.anchoredPosition = Vector2.zero;
            headerRect.sizeDelta = new Vector2(0, 150);

            Text titleText = CreateText(header.transform, "TitleText", "SpectraCore Mobile Validation", 36);
            titleText.alignment = TextAnchor.MiddleCenter;
            titleText.fontStyle = FontStyle.Bold;

            // Create status text
            Text statusText = CreateText(panel.transform, "StatusText", "Ready to test...", 24);
            RectTransform statusRect = statusText.GetComponent<RectTransform>();
            statusRect.anchorMin = new Vector2(0, 1);
            statusRect.anchorMax = new Vector2(1, 1);
            statusRect.pivot = new Vector2(0.5f, 1);
            statusRect.anchoredPosition = new Vector2(0, -160);
            statusRect.sizeDelta = new Vector2(-40, 60);

            // Create buttons container
            GameObject buttonsPanel = CreatePanel(panel.transform, "ButtonsPanel");
            RectTransform buttonsRect = buttonsPanel.GetComponent<RectTransform>();
            buttonsRect.anchorMin = new Vector2(0, 1);
            buttonsRect.anchorMax = new Vector2(1, 1);
            buttonsRect.pivot = new Vector2(0.5f, 1);
            buttonsRect.anchoredPosition = new Vector2(0, -230);
            buttonsRect.sizeDelta = new Vector2(-40, 140);

            // Create Run Tests button
            GameObject runTestsBtn = CreateButton(buttonsPanel.transform, "RunTestsButton", "Run Tests");
            RectTransform runBtnRect = runTestsBtn.GetComponent<RectTransform>();
            runBtnRect.anchorMin = new Vector2(0, 0.5f);
            runBtnRect.anchorMax = new Vector2(0.48f, 0.5f);
            runBtnRect.pivot = new Vector2(0, 0.5f);
            runBtnRect.anchoredPosition = Vector2.zero;
            runBtnRect.sizeDelta = new Vector2(0, 100);

            // Create Run Benchmark button
            GameObject runBenchBtn = CreateButton(buttonsPanel.transform, "RunBenchmarkButton", "Run Benchmark");
            RectTransform benchBtnRect = runBenchBtn.GetComponent<RectTransform>();
            benchBtnRect.anchorMin = new Vector2(0.52f, 0.5f);
            benchBtnRect.anchorMax = new Vector2(1, 0.5f);
            benchBtnRect.pivot = new Vector2(1, 0.5f);
            benchBtnRect.anchoredPosition = Vector2.zero;
            benchBtnRect.sizeDelta = new Vector2(0, 100);

            // Create ScrollView for results
            GameObject scrollView = CreateScrollView(panel.transform, "ResultsScrollView");
            RectTransform scrollRect = scrollView.GetComponent<RectTransform>();
            scrollRect.anchorMin = new Vector2(0, 0);
            scrollRect.anchorMax = new Vector2(1, 1);
            scrollRect.pivot = new Vector2(0.5f, 1);
            scrollRect.anchoredPosition = new Vector2(0, -380);
            scrollRect.sizeDelta = new Vector2(-40, -400);

            Text resultsText = scrollView.transform.Find("Viewport/Content/ResultsText").GetComponent<Text>();

            // Create TestRunner GameObject
            GameObject testRunner = new GameObject("TestRunner");
            MobileTestRunner testRunnerComponent = testRunner.AddComponent<MobileTestRunner>();
            testRunnerComponent.statusText = statusText;
            testRunnerComponent.resultsText = resultsText;
            testRunnerComponent.runTestsButton = runTestsBtn.GetComponent<Button>();
            testRunnerComponent.scrollRect = scrollView.GetComponent<ScrollRect>();
            testRunnerComponent.runOnStart = false;
            testRunnerComponent.verboseLogging = true;

            // Create BenchmarkRunner GameObject
            GameObject benchmarkRunner = new GameObject("BenchmarkRunner");
            MobilePerformanceBenchmark benchmarkComponent = benchmarkRunner.AddComponent<MobilePerformanceBenchmark>();
            benchmarkComponent.statusText = statusText;
            benchmarkComponent.resultsText = resultsText;
            benchmarkComponent.runBenchmarkButton = runBenchBtn.GetComponent<Button>();
            benchmarkComponent.scrollRect = scrollView.GetComponent<ScrollRect>();
            benchmarkComponent.runOnStart = false;
            benchmarkComponent.iterations = 100;
            benchmarkComponent.signalLength = 2048;

            Debug.Log("✅ Mobile Test Scene setup complete!");
            Debug.Log("Scene components created:");
            Debug.Log("  - Canvas with UI elements");
            Debug.Log("  - TestRunner with MobileTestRunner component");
            Debug.Log("  - BenchmarkRunner with MobilePerformanceBenchmark component");
            Debug.Log("");
            Debug.Log("Next steps:");
            Debug.Log("  1. Save scene: File → Save Scene As → MobileTestScene");
            Debug.Log("  2. Add scene to Build Settings");
            Debug.Log("  3. Build for mobile platform");
            Debug.Log("  4. Deploy and run tests on device");

            EditorUtility.DisplayDialog(
                "Scene Setup Complete",
                "Mobile Test Scene has been set up successfully!\n\n" +
                "Next steps:\n" +
                "1. Save the scene (File → Save Scene As)\n" +
                "2. Add to Build Settings\n" +
                "3. Build for mobile platform\n" +
                "4. Deploy and test on device",
                "OK"
            );

            // Select the TestRunner for easy inspection
            Selection.activeGameObject = testRunner;
        }

        private static GameObject CreatePanel(Transform parent, string name)
        {
            GameObject panel = new GameObject(name, typeof(RectTransform));
            panel.transform.SetParent(parent, false);

            Image image = panel.AddComponent<Image>();
            image.color = new Color(0.1f, 0.1f, 0.1f, 0.9f);

            RectTransform rect = panel.GetComponent<RectTransform>();
            rect.anchorMin = Vector2.zero;
            rect.anchorMax = Vector2.one;
            rect.sizeDelta = Vector2.zero;

            return panel;
        }

        private static Text CreateText(Transform parent, string name, string content, int fontSize)
        {
            GameObject textObj = new GameObject(name, typeof(RectTransform));
            textObj.transform.SetParent(parent, false);

            Text text = textObj.AddComponent<Text>();
            text.text = content;
            text.font = Resources.GetBuiltinResource<Font>("Arial.ttf");
            text.fontSize = fontSize;
            text.color = Color.white;
            text.alignment = TextAnchor.UpperLeft;

            RectTransform rect = textObj.GetComponent<RectTransform>();
            rect.anchorMin = Vector2.zero;
            rect.anchorMax = Vector2.one;
            rect.sizeDelta = Vector2.zero;

            return text;
        }

        private static GameObject CreateButton(Transform parent, string name, string label)
        {
            GameObject buttonObj = new GameObject(name, typeof(RectTransform));
            buttonObj.transform.SetParent(parent, false);

            Image image = buttonObj.AddComponent<Image>();
            image.color = new Color(0.2f, 0.6f, 0.2f, 1f);

            Button button = buttonObj.AddComponent<Button>();
            ColorBlock colors = button.colors;
            colors.normalColor = new Color(0.2f, 0.6f, 0.2f, 1f);
            colors.highlightedColor = new Color(0.3f, 0.7f, 0.3f, 1f);
            colors.pressedColor = new Color(0.15f, 0.5f, 0.15f, 1f);
            button.colors = colors;

            GameObject textObj = new GameObject("Text", typeof(RectTransform));
            textObj.transform.SetParent(buttonObj.transform, false);

            Text text = textObj.AddComponent<Text>();
            text.text = label;
            text.font = Resources.GetBuiltinResource<Font>("Arial.ttf");
            text.fontSize = 32;
            text.color = Color.white;
            text.alignment = TextAnchor.MiddleCenter;
            text.fontStyle = FontStyle.Bold;

            RectTransform textRect = textObj.GetComponent<RectTransform>();
            textRect.anchorMin = Vector2.zero;
            textRect.anchorMax = Vector2.one;
            textRect.sizeDelta = Vector2.zero;

            return buttonObj;
        }

        private static GameObject CreateScrollView(Transform parent, string name)
        {
            GameObject scrollViewObj = new GameObject(name, typeof(RectTransform));
            scrollViewObj.transform.SetParent(parent, false);

            Image scrollImage = scrollViewObj.AddComponent<Image>();
            scrollImage.color = new Color(0.05f, 0.05f, 0.05f, 1f);

            ScrollRect scrollRect = scrollViewObj.AddComponent<ScrollRect>();
            scrollRect.horizontal = false;
            scrollRect.vertical = true;

            // Create Viewport
            GameObject viewport = new GameObject("Viewport", typeof(RectTransform));
            viewport.transform.SetParent(scrollViewObj.transform, false);

            RectTransform viewportRect = viewport.GetComponent<RectTransform>();
            viewportRect.anchorMin = Vector2.zero;
            viewportRect.anchorMax = Vector2.one;
            viewportRect.sizeDelta = Vector2.zero;

            Image viewportImage = viewport.AddComponent<Image>();
            viewportImage.color = Color.clear;

            Mask mask = viewport.AddComponent<Mask>();
            mask.showMaskGraphic = false;

            scrollRect.viewport = viewportRect;

            // Create Content
            GameObject content = new GameObject("Content", typeof(RectTransform));
            content.transform.SetParent(viewport.transform, false);

            RectTransform contentRect = content.GetComponent<RectTransform>();
            contentRect.anchorMin = new Vector2(0, 1);
            contentRect.anchorMax = new Vector2(1, 1);
            contentRect.pivot = new Vector2(0.5f, 1);
            contentRect.sizeDelta = new Vector2(0, 2000); // Large initial height

            ContentSizeFitter fitter = content.AddComponent<ContentSizeFitter>();
            fitter.verticalFit = ContentSizeFitter.FitMode.PreferredSize;

            VerticalLayoutGroup layout = content.AddComponent<VerticalLayoutGroup>();
            layout.childControlHeight = true;
            layout.childControlWidth = true;
            layout.childForceExpandHeight = false;
            layout.childForceExpandWidth = true;

            scrollRect.content = contentRect;

            // Create Results Text
            Text resultsText = CreateText(content.transform, "ResultsText", "Results will appear here...", 20);
            resultsText.alignment = TextAnchor.UpperLeft;

            return scrollViewObj;
        }
#endif
    }
}
