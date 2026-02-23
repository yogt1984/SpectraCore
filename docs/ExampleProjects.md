# SpectraCore Example Unity Projects

Complete example projects demonstrating SpectraCore features with ready-to-use scenes and prefabs.

## Table of Contents

1. [Overview](#overview)
2. [Project Structure](#project-structure)
3. [Example 1: Audio Visualizer](#example-1-audio-visualizer)
4. [Example 2: Filter Playground](#example-2-filter-playground)
5. [Example 3: Beat-Synced Game](#example-3-beat-synced-game)
6. [Example 4: Audio Effects Studio](#example-4-audio-effects-studio)
7. [Example 5: Spectrogram Analyzer](#example-5-spectrogram-analyzer)
8. [Prefabs Reference](#prefabs-reference)
9. [Templates](#templates)
10. [Customization Guide](#customization-guide)

---

## Overview

This document describes the example Unity projects included with SpectraCore. Each example demonstrates key features and provides reusable code you can integrate into your own projects.

### What's Included

**5 Complete Example Scenes:**
1. Audio Visualizer - Real-time spectrum and waveform display
2. Filter Playground - Interactive filter design and testing
3. Beat-Synced Game - Rhythm game with onset detection
4. Audio Effects Studio - Multi-effect audio processor with UI
5. Spectrogram Analyzer - Time-frequency visualization

**10+ Reusable Prefabs:**
- SpectrumDisplay (3D and UI versions)
- WaveformDisplay
- BeatDetector
- AudioEffectChain
- SpectrogramDisplay
- And more...

**Templates:**
- Audio analysis setup
- Real-time processing setup
- Visualization templates

---

## Project Structure

```
Assets/
├── SpectraCore/
│   ├── Examples/
│   │   ├── Scenes/
│   │   │   ├── 1_AudioVisualizer.unity
│   │   │   ├── 2_FilterPlayground.unity
│   │   │   ├── 3_BeatGame.unity
│   │   │   ├── 4_EffectsStudio.unity
│   │   │   └── 5_SpectrogramAnalyzer.unity
│   │   ├── Scripts/
│   │   │   ├── Visualizers/
│   │   │   │   ├── SpectrumDisplayController.cs
│   │   │   │   ├── WaveformRenderer.cs
│   │   │   │   ├── SpectrogramRenderer.cs
│   │   │   │   └── FrequencyBars3D.cs
│   │   │   ├── Effects/
│   │   │   │   ├── EffectsChainUI.cs
│   │   │   │   ├── EQController.cs
│   │   │   │   └── EffectPresetManager.cs
│   │   │   ├── Game/
│   │   │   │   ├── BeatGameController.cs
│   │   │   │   ├── NoteSpawner.cs
│   │   │   │   └── ScoreManager.cs
│   │   │   └── Utility/
│   │   │       ├── AudioClipLoader.cs
│   │   │       └── FPSDisplay.cs
│   │   ├── Prefabs/
│   │   │   ├── Visualizers/
│   │   │   │   ├── SpectrumDisplay3D.prefab
│   │   │   │   ├── SpectrumDisplayUI.prefab
│   │   │   │   ├── WaveformDisplay.prefab
│   │   │   │   └── SpectrogramDisplay.prefab
│   │   │   ├── Controllers/
│   │   │   │   ├── BeatDetector.prefab
│   │   │   │   ├── AudioEffectChain.prefab
│   │   │   │   └── AudioAnalyzer.prefab
│   │   │   └── UI/
│   │   │       ├── EffectsPanel.prefab
│   │   │       ├── FilterControls.prefab
│   │   │       └── AnalysisPanel.prefab
│   │   ├── Materials/
│   │   │   ├── SpectrumBar.mat
│   │   │   ├── Waveform.mat
│   │   │   └── Spectrogram.mat
│   │   └── Audio/
│   │       ├── DemoTrack.wav
│   │       └── TestTones/
│   │           ├── Sweep.wav
│   │           └── WhiteNoise.wav
```

---

## Example 1: Audio Visualizer

**Scene:** `1_AudioVisualizer.unity`

**Description:** Real-time audio visualization with spectrum analyzer, waveform display, and 3D frequency bars.

### Features

- **3D Spectrum Bars** - Color-coded frequency visualization
- **Waveform Display** - Oscilloscope-style waveform
- **Spectrogram** - Time-frequency scrolling display
- **Audio Input** - Supports audio file or microphone
- **Adjustable Settings** - FFT size, colors, scaling

### Scene Setup

#### Step 1: Create New Scene

```
File → New Scene
Save as: 1_AudioVisualizer.unity
```

#### Step 2: Add Main Camera

```
Position: (0, 5, -10)
Rotation: (20, 0, 0)
Background: Black
```

#### Step 3: Add Directional Light

```
Rotation: (50, -30, 0)
Intensity: 1
Color: White
```

#### Step 4: Add Audio Source

```
GameObject → Create Empty → "AudioManager"
Add Component: AudioSource
  - Audio Clip: [Your music file]
  - Play On Awake: true
  - Loop: true
```

#### Step 5: Add Spectrum Display (3D)

```
GameObject → Create Empty → "SpectrumDisplay3D"
Position: (0, 0, 0)
Add Component: SpectrumDisplayController
Add Component: FrequencyBars3D

Configure FrequencyBars3D:
  - Num Bars: 64
  - Bar Width: 0.3
  - Bar Spacing: 0.1
  - Max Height: 10
  - Color Gradient: Blue → Red
```

#### Step 6: Add Waveform Display

```
GameObject → Create Empty → "WaveformDisplay"
Position: (0, -2, 0)
Add Component: WaveformRenderer

Configure:
  - Display Width: 20
  - Display Height: 2
  - Line Color: Green
  - Background Color: Black
```

#### Step 7: Add UI Canvas

```
GameObject → UI → Canvas
Render Mode: Screen Space - Overlay

Add UI elements:
  - Text: "Audio Visualizer Demo"
  - Slider: FFT Size
  - Toggle: Microphone Input
  - Button: Load Audio File
```

### Scripts

#### FrequencyBars3D.cs

```csharp
using UnityEngine;
using Spectra;

public class FrequencyBars3D : MonoBehaviour
{
    [Header("Bar Settings")]
    public int numBars = 64;
    public float barWidth = 0.3f;
    public float barSpacing = 0.1f;
    public float maxHeight = 10f;

    [Header("Visual")]
    public Gradient colorGradient;
    public float smoothing = 0.8f;

    [Header("Audio")]
    [Range(6, 13)]
    public int fftSizeLog2 = 11;

    private FFTAnalyzer fft;
    private GameObject[] bars;
    private float[] spectrum;
    private float[] smoothedSpectrum;
    private float[] audioBuffer;
    private Material barMaterial;

    void Start()
    {
        CreateBars();
        InitializeAudio();
    }

    void CreateBars()
    {
        bars = new GameObject[numBars];
        smoothedSpectrum = new float[numBars];

        // Create material
        barMaterial = new Material(Shader.Find("Standard"));
        barMaterial.SetFloat("_Metallic", 0.5f);
        barMaterial.SetFloat("_Glossiness", 0.8f);

        float totalWidth = numBars * (barWidth + barSpacing) - barSpacing;
        float startX = -totalWidth / 2f;

        for (int i = 0; i < numBars; i++)
        {
            // Create bar
            GameObject bar = GameObject.CreatePrimitive(PrimitiveType.Cube);
            bar.transform.parent = transform;

            float xPos = startX + i * (barWidth + barSpacing);
            bar.transform.localPosition = new Vector3(xPos, 0, 0);
            bar.transform.localScale = new Vector3(barWidth, 0.1f, barWidth);

            // Set material with gradient color
            float t = (float)i / numBars;
            Color color = colorGradient.Evaluate(t);

            Material mat = new Material(barMaterial);
            mat.color = color;
            bar.GetComponent<Renderer>().material = mat;

            bars[i] = bar;
        }
    }

    void InitializeAudio()
    {
        int fftSize = 1 << fftSizeLog2;
        fft = new FFTAnalyzer(fftSize);
        spectrum = new float[fft.SpectrumSize];
        audioBuffer = new float[fftSize];
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        int samplesNeeded = Mathf.Min(audioBuffer.Length, data.Length / channels);

        for (int i = 0; i < samplesNeeded; i++)
        {
            if (channels == 2)
                audioBuffer[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                audioBuffer[i] = data[i];
        }
    }

    void Update()
    {
        UpdateSpectrum();
        UpdateBars();
    }

    void UpdateSpectrum()
    {
        float[] rawSpectrum = fft.GetMagnitudeSpectrum(audioBuffer);

        // Downsample to numBars with logarithmic distribution
        for (int i = 0; i < numBars; i++)
        {
            float logStart = Mathf.Log10(i + 1);
            float logEnd = Mathf.Log10(i + 2);
            float logScale = Mathf.Log10(numBars + 1);

            int binStart = (int)(rawSpectrum.Length * logStart / logScale);
            int binEnd = (int)(rawSpectrum.Length * logEnd / logScale);

            float sum = 0f;
            int count = 0;

            for (int j = binStart; j < binEnd && j < rawSpectrum.Length; j++)
            {
                sum += rawSpectrum[j];
                count++;
            }

            float average = count > 0 ? sum / count : 0f;

            // Convert to dB and normalize
            float dB = 20f * Mathf.Log10(Mathf.Max(average, 1e-6f));
            float normalized = Mathf.InverseLerp(-60f, 0f, dB);

            // Smooth
            smoothedSpectrum[i] = Mathf.Lerp(normalized, smoothedSpectrum[i], smoothing);
        }
    }

    void UpdateBars()
    {
        for (int i = 0; i < numBars; i++)
        {
            float targetHeight = smoothedSpectrum[i] * maxHeight;
            Vector3 scale = bars[i].transform.localScale;
            scale.y = Mathf.Lerp(scale.y, Mathf.Max(0.1f, targetHeight), 10f * Time.deltaTime);

            bars[i].transform.localScale = scale;
            bars[i].transform.localPosition = new Vector3(
                bars[i].transform.localPosition.x,
                scale.y / 2f,
                bars[i].transform.localPosition.z
            );

            // Brightness based on amplitude
            Material mat = bars[i].GetComponent<Renderer>().material;
            float t = (float)i / numBars;
            Color baseColor = colorGradient.Evaluate(t);
            mat.color = baseColor * Mathf.Lerp(0.5f, 2f, smoothedSpectrum[i]);
        }
    }

    void OnDestroy()
    {
        fft?.Dispose();
    }
}
```

#### WaveformRenderer.cs

```csharp
using UnityEngine;
using Spectra;

public class WaveformRenderer : MonoBehaviour
{
    [Header("Display Settings")]
    public float displayWidth = 20f;
    public float displayHeight = 2f;
    public Color lineColor = Color.green;
    public int resolution = 512;

    [Header("Audio")]
    public int bufferSize = 2048;

    private LineRenderer lineRenderer;
    private float[] audioBuffer;
    private int writePos = 0;

    void Start()
    {
        SetupLineRenderer();
        audioBuffer = new float[bufferSize];
    }

    void SetupLineRenderer()
    {
        lineRenderer = gameObject.AddComponent<LineRenderer>();
        lineRenderer.positionCount = resolution;
        lineRenderer.startWidth = 0.05f;
        lineRenderer.endWidth = 0.05f;
        lineRenderer.material = new Material(Shader.Find("Sprites/Default"));
        lineRenderer.startColor = lineColor;
        lineRenderer.endColor = lineColor;
        lineRenderer.useWorldSpace = false;
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        for (int i = 0; i < data.Length / channels; i++)
        {
            if (channels == 2)
                audioBuffer[writePos] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                audioBuffer[writePos] = data[i];

            writePos = (writePos + 1) % bufferSize;
        }
    }

    void Update()
    {
        DrawWaveform();
    }

    void DrawWaveform()
    {
        int step = audioBuffer.Length / resolution;
        float xStep = displayWidth / resolution;
        float startX = -displayWidth / 2f;

        for (int i = 0; i < resolution; i++)
        {
            int bufferIdx = (writePos + i * step) % audioBuffer.Length;
            float sample = audioBuffer[bufferIdx];

            float x = startX + i * xStep;
            float y = sample * displayHeight;

            lineRenderer.SetPosition(i, new Vector3(x, y, 0));
        }
    }
}
```

#### SpectrumDisplayController.cs

```csharp
using UnityEngine;
using UnityEngine.UI;
using Spectra;

public class SpectrumDisplayController : MonoBehaviour
{
    [Header("UI References")]
    public Slider fftSizeSlider;
    public Toggle micToggle;
    public Text fftSizeText;

    [Header("Display References")]
    public FrequencyBars3D frequencyBars;

    private AudioSource audioSource;

    void Start()
    {
        audioSource = GetComponent<AudioSource>();

        if (fftSizeSlider != null)
        {
            fftSizeSlider.onValueChanged.AddListener(OnFFTSizeChanged);
            fftSizeSlider.minValue = 8;
            fftSizeSlider.maxValue = 13;
            fftSizeSlider.value = 11;
        }

        if (micToggle != null)
        {
            micToggle.onValueChanged.AddListener(OnMicToggle);
        }
    }

    void OnFFTSizeChanged(float value)
    {
        int fftSize = 1 << (int)value;

        if (fftSizeText != null)
        {
            fftSizeText.text = $"FFT Size: {fftSize}";
        }

        if (frequencyBars != null)
        {
            frequencyBars.fftSizeLog2 = (int)value;
        }
    }

    void OnMicToggle(bool useMic)
    {
        if (useMic)
        {
            StartMicrophone();
        }
        else
        {
            StopMicrophone();
        }
    }

    void StartMicrophone()
    {
        if (Microphone.devices.Length == 0)
        {
            Debug.LogError("No microphone found");
            return;
        }

        audioSource.clip = Microphone.Start(null, true, 1, AudioSettings.outputSampleRate);
        audioSource.loop = true;

        while (Microphone.GetPosition(null) <= 0) { }

        audioSource.Play();
        Debug.Log("Microphone started");
    }

    void StopMicrophone()
    {
        Microphone.End(null);
        audioSource.Stop();
        Debug.Log("Microphone stopped");
    }
}
```

### Testing the Scene

1. **Open Scene:**
   - Navigate to Examples/Scenes/
   - Double-click 1_AudioVisualizer.unity

2. **Assign Audio:**
   - Select AudioManager object
   - Drag an audio file to Audio Clip field

3. **Run:**
   - Press Play
   - Watch the visualization respond to audio

4. **Experiment:**
   - Adjust FFT Size slider
   - Toggle microphone input
   - Try different audio files

### Expected Results

- 64 colored bars rising and falling with music
- Green waveform showing time-domain signal
- Smooth animations
- Responsive to different frequencies

---

## Example 2: Filter Playground

**Scene:** `2_FilterPlayground.unity`

**Description:** Interactive filter design and testing environment with real-time frequency response visualization.

### Features

- **4 Filter Types** - Butterworth, Chebyshev I/II, Elliptic
- **Real-Time Preview** - Hear filtered audio immediately
- **Frequency Response Plot** - Bode magnitude and phase plots
- **Before/After Comparison** - A/B testing
- **Parameter Sliders** - Interactive adjustment

### Scene Setup

#### GameObject Hierarchy

```
FilterPlayground
├── AudioManager
│   └── Components: AudioSource, FilterPlaygroundController
├── UI Canvas
│   ├── FilterControls Panel
│   │   ├── Filter Type Dropdown
│   │   ├── Order Slider
│   │   ├── Cutoff Slider
│   │   ├── Ripple Slider (for Chebyshev)
│   │   └── Apply Button
│   ├── FrequencyResponse Panel
│   │   ├── Magnitude Plot
│   │   └── Phase Plot
│   └── Comparison Panel
│       ├── Original Button
│       ├── Filtered Button
│       └── Volume Slider
└── Main Camera
```

### Scripts

#### FilterPlaygroundController.cs

```csharp
using UnityEngine;
using UnityEngine.UI;
using Spectra;

public class FilterPlaygroundController : MonoBehaviour
{
    [Header("UI References")]
    public Dropdown filterTypeDropdown;
    public Slider orderSlider;
    public Slider cutoffSlider;
    public Slider rippleSlider;
    public Text orderText;
    public Text cutoffText;
    public Text rippleText;
    public Toggle enableFilterToggle;

    [Header("Frequency Response")]
    public FrequencyResponsePlot responsePlot;

    [Header("Audio")]
    public AudioClip sourceAudio;

    private AudioSource audioSource;
    private float[] originalAudio;
    private float[] filteredAudio;
    private StreamingIIRFilter streamingFilter;
    private float[] filterB, filterA;
    private bool filterEnabled = false;
    private int sampleRate;

    void Start()
    {
        audioSource = GetComponent<AudioSource>();
        sampleRate = AudioSettings.outputSampleRate;

        LoadAudio();
        SetupUI();
        DesignFilter();
    }

    void LoadAudio()
    {
        if (sourceAudio != null)
        {
            originalAudio = new float[sourceAudio.samples * sourceAudio.channels];
            sourceAudio.GetData(originalAudio, 0);
        }
    }

    void SetupUI()
    {
        if (filterTypeDropdown != null)
        {
            filterTypeDropdown.ClearOptions();
            filterTypeDropdown.AddOptions(new System.Collections.Generic.List<string>
            {
                "Butterworth",
                "Chebyshev Type I",
                "Chebyshev Type II",
                "Elliptic"
            });
            filterTypeDropdown.onValueChanged.AddListener(_ => DesignFilter());
        }

        if (orderSlider != null)
        {
            orderSlider.minValue = 1;
            orderSlider.maxValue = 8;
            orderSlider.value = 4;
            orderSlider.onValueChanged.AddListener(_ => DesignFilter());
        }

        if (cutoffSlider != null)
        {
            cutoffSlider.minValue = 0.01f;
            cutoffSlider.maxValue = 0.99f;
            cutoffSlider.value = 0.25f;
            cutoffSlider.onValueChanged.AddListener(_ => DesignFilter());
        }

        if (rippleSlider != null)
        {
            rippleSlider.minValue = 0.1f;
            rippleSlider.maxValue = 5f;
            rippleSlider.value = 0.5f;
            rippleSlider.onValueChanged.AddListener(_ => DesignFilter());
        }

        if (enableFilterToggle != null)
        {
            enableFilterToggle.onValueChanged.AddListener(OnFilterToggle);
        }
    }

    void DesignFilter()
    {
        int order = (int)orderSlider.value;
        float cutoff = cutoffSlider.value;
        float ripple = rippleSlider.value;

        try
        {
            switch (filterTypeDropdown.value)
            {
                case 0: // Butterworth
                    (filterB, filterA) = DSP.Butter(order, cutoff, FilterType.Lowpass);
                    break;

                case 1: // Chebyshev I
                    (filterB, filterA) = DSP.Cheby1(order, ripple, cutoff, FilterType.Lowpass);
                    break;

                case 2: // Chebyshev II
                    (filterB, filterA) = DSP.Cheby2(order, ripple * 10f, cutoff, FilterType.Lowpass);
                    break;

                case 3: // Elliptic
                    (filterB, filterA) = DSP.Ellip(order, ripple, ripple * 10f, cutoff, FilterType.Lowpass);
                    break;
            }

            // Update streaming filter
            streamingFilter?.Dispose();
            streamingFilter = new StreamingIIRFilter(filterB, filterA);

            // Update frequency response plot
            if (responsePlot != null)
            {
                responsePlot.UpdateFilter(filterB, filterA);
            }

            // Update UI text
            UpdateUIText(order, cutoff, ripple);

            Debug.Log("Filter designed successfully");
        }
        catch (SpectraException ex)
        {
            Debug.LogError($"Filter design failed: {ex.Message}");
        }
    }

    void UpdateUIText(int order, float cutoff, float ripple)
    {
        if (orderText != null)
            orderText.text = $"Order: {order}";

        if (cutoffText != null)
        {
            float cutoffHz = cutoff * (sampleRate / 2f);
            cutoffText.text = $"Cutoff: {cutoffHz:F0} Hz";
        }

        if (rippleText != null)
            rippleText.text = $"Ripple: {ripple:F1} dB";
    }

    void OnFilterToggle(bool enabled)
    {
        filterEnabled = enabled;
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        if (!filterEnabled || streamingFilter == null)
            return;

        // Convert to mono
        float[] mono = new float[data.Length / channels];
        for (int i = 0; i < mono.Length; i++)
        {
            if (channels == 2)
                mono[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                mono[i] = data[i];
        }

        // Apply filter
        streamingFilter.Process(mono, mono);

        // Convert back to stereo
        for (int i = 0; i < mono.Length; i++)
        {
            data[i * channels] = mono[i];
            if (channels == 2)
                data[i * channels + 1] = mono[i];
        }
    }

    void OnDestroy()
    {
        streamingFilter?.Dispose();
    }
}
```

#### FrequencyResponsePlot.cs

```csharp
using UnityEngine;
using UnityEngine.UI;
using Spectra;

public class FrequencyResponsePlot : MonoBehaviour
{
    [Header("Plot Settings")]
    public RawImage magnitudePlot;
    public RawImage phasePlot;
    public int plotWidth = 512;
    public int plotHeight = 256;

    [Header("Colors")]
    public Color backgroundColor = Color.black;
    public Color gridColor = new Color(0.3f, 0.3f, 0.3f);
    public Color curveColor = Color.cyan;

    private Texture2D magTexture;
    private Texture2D phaseTexture;

    void Start()
    {
        InitializeTextures();
    }

    void InitializeTextures()
    {
        magTexture = new Texture2D(plotWidth, plotHeight);
        phaseTexture = new Texture2D(plotWidth, plotHeight);

        if (magnitudePlot != null)
            magnitudePlot.texture = magTexture;

        if (phasePlot != null)
            phasePlot.texture = phaseTexture;

        ClearPlots();
    }

    void ClearPlots()
    {
        Color[] clearPixels = new Color[plotWidth * plotHeight];
        for (int i = 0; i < clearPixels.Length; i++)
            clearPixels[i] = backgroundColor;

        magTexture.SetPixels(clearPixels);
        magTexture.Apply();

        phaseTexture.SetPixels(clearPixels);
        phaseTexture.Apply();
    }

    public void UpdateFilter(float[] b, float[] a)
    {
        // Compute frequency response
        var (mag, phase, freqs) = DSP.FreqzDb(b, a, plotWidth);

        DrawMagnitudePlot(mag);
        DrawPhasePlot(phase);
    }

    void DrawMagnitudePlot(float[] magDb)
    {
        ClearTexture(magTexture);
        DrawGrid(magTexture, -80f, 20f, "dB");

        // Draw curve
        for (int x = 0; x < plotWidth - 1; x++)
        {
            float mag1 = magDb[x];
            float mag2 = magDb[x + 1];

            int y1 = Mathf.RoundToInt(Mathf.InverseLerp(-80f, 20f, mag1) * plotHeight);
            int y2 = Mathf.RoundToInt(Mathf.InverseLerp(-80f, 20f, mag2) * plotHeight);

            DrawLine(magTexture, x, y1, x + 1, y2, curveColor);
        }

        magTexture.Apply();
    }

    void DrawPhasePlot(float[] phase)
    {
        ClearTexture(phaseTexture);
        DrawGrid(phaseTexture, -Mathf.PI, Mathf.PI, "rad");

        // Draw curve
        for (int x = 0; x < plotWidth - 1; x++)
        {
            float p1 = phase[x];
            float p2 = phase[x + 1];

            int y1 = Mathf.RoundToInt(Mathf.InverseLerp(-Mathf.PI, Mathf.PI, p1) * plotHeight);
            int y2 = Mathf.RoundToInt(Mathf.InverseLerp(-Mathf.PI, Mathf.PI, p2) * plotHeight);

            DrawLine(phaseTexture, x, y1, x + 1, y2, Color.yellow);
        }

        phaseTexture.Apply();
    }

    void ClearTexture(Texture2D tex)
    {
        Color[] pixels = new Color[plotWidth * plotHeight];
        for (int i = 0; i < pixels.Length; i++)
            pixels[i] = backgroundColor;
        tex.SetPixels(pixels);
    }

    void DrawGrid(Texture2D tex, float minVal, float maxVal, string unit)
    {
        // Horizontal lines
        for (int i = 0; i <= 4; i++)
        {
            int y = (plotHeight * i) / 4;
            for (int x = 0; x < plotWidth; x++)
            {
                tex.SetPixel(x, y, gridColor);
            }
        }

        // Vertical lines
        for (int i = 0; i <= 4; i++)
        {
            int x = (plotWidth * i) / 4;
            for (int y = 0; y < plotHeight; y++)
            {
                tex.SetPixel(x, y, gridColor);
            }
        }
    }

    void DrawLine(Texture2D tex, int x0, int y0, int x1, int y1, Color color)
    {
        // Bresenham's line algorithm
        int dx = Mathf.Abs(x1 - x0);
        int dy = Mathf.Abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;

        while (true)
        {
            if (x0 >= 0 && x0 < plotWidth && y0 >= 0 && y0 < plotHeight)
            {
                tex.SetPixel(x0, y0, color);
            }

            if (x0 == x1 && y0 == y1) break;

            int e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }
}
```

### Using the Filter Playground

1. **Select Filter Type:**
   - Choose from dropdown (Butterworth, Chebyshev, Elliptic)

2. **Adjust Parameters:**
   - Order: 1-8 (higher = steeper rolloff)
   - Cutoff: 0.01-0.99 (normalized frequency)
   - Ripple: 0.1-5 dB (for Chebyshev/Elliptic)

3. **View Response:**
   - Magnitude plot shows frequency response
   - Phase plot shows phase shift

4. **Apply Filter:**
   - Toggle "Enable Filter" checkbox
   - Hear filtered audio in real-time

5. **Compare:**
   - Toggle filter on/off to hear difference
   - Adjust parameters while audio plays

---

## Example 3: Beat-Synced Game

**Scene:** `3_BeatGame.unity`

**Description:** Simple rhythm game where players press spacebar in sync with detected beats.

### Features

- **Beat Detection** - Real-time onset detection
- **Score System** - Perfect/Good/Miss timing windows
- **Visual Feedback** - Beat markers, combo display
- **Difficulty Levels** - Adjustable sensitivity
- **Leaderboard** - High score tracking

### Scene Setup

```
BeatGame
├── GameManager
│   └── Components: BeatGameController, AudioSource
├── BeatDetector
│   └── Components: OnsetDetector wrapper
├── UI Canvas
│   ├── Score Text
│   ├── Combo Text
│   ├── Feedback Text
│   ├── Beat Indicator (animated)
│   └── Instructions
├── Particle Systems
│   ├── Perfect Effect (green)
│   ├── Good Effect (yellow)
│   └── Miss Effect (red)
└── Main Camera
```

### Scripts

#### BeatGameController.cs

```csharp
using UnityEngine;
using UnityEngine.UI;
using Spectra;
using System.Collections.Generic;

public class BeatGameController : MonoBehaviour
{
    [Header("UI References")]
    public Text scoreText;
    public Text comboText;
    public Text feedbackText;
    public Image beatIndicator;

    [Header("Timing Windows (seconds)")]
    public float perfectWindow = 0.08f;
    public float goodWindow = 0.15f;
    public float missWindow = 0.25f;

    [Header("Scoring")]
    public int perfectPoints = 100;
    public int goodPoints = 50;
    public int missPoints = -10;
    public float comboMultiplier = 0.1f;

    [Header("Particle Effects")]
    public ParticleSystem perfectEffect;
    public ParticleSystem goodEffect;
    public ParticleSystem missEffect;

    private OnsetDetector detector;
    private float lastBeatTime = float.MinValue;
    private bool beatAvailable = false;
    private int score = 0;
    private int combo = 0;
    private int maxCombo = 0;
    private Queue<float> recentBeats = new Queue<float>();

    void Start()
    {
        SetupBeatDetector();
        UpdateUI();
    }

    void SetupBeatDetector()
    {
        var config = new OnsetConfig
        {
            FftSize = 2048,
            HopSize = 512,
            Threshold = 0.5f,
            MinIntervalMs = 100f
        };

        detector = new OnsetDetector(config, AudioSettings.outputSampleRate);
        detector.OnOnsetDetected += OnBeat;
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        float[] mono = new float[data.Length / channels];

        for (int i = 0; i < mono.Length; i++)
        {
            if (channels == 2)
                mono[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                mono[i] = data[i];
        }

        detector.Process(mono);
    }

    void OnBeat(float timestamp, float strength)
    {
        lastBeatTime = Time.time;
        beatAvailable = true;
        recentBeats.Enqueue(lastBeatTime);

        // Clean up old beats
        while (recentBeats.Count > 0 && Time.time - recentBeats.Peek() > missWindow)
        {
            recentBeats.Dequeue();
        }

        // Animate beat indicator
        if (beatIndicator != null)
        {
            StopAllCoroutines();
            StartCoroutine(AnimateBeatIndicator());
        }
    }

    System.Collections.IEnumerator AnimateBeatIndicator()
    {
        float elapsed = 0f;
        float duration = 0.2f;

        while (elapsed < duration)
        {
            elapsed += Time.deltaTime;
            float t = elapsed / duration;

            beatIndicator.transform.localScale = Vector3.one * Mathf.Lerp(1.5f, 1f, t);
            beatIndicator.color = Color.Lerp(Color.white, new Color(1f, 1f, 1f, 0.5f), t);

            yield return null;
        }
    }

    void Update()
    {
        // Player input
        if (Input.GetKeyDown(KeyCode.Space))
        {
            HandleInput();
        }

        // Auto-miss beats
        if (beatAvailable && Time.time - lastBeatTime > missWindow)
        {
            Miss();
            beatAvailable = false;
        }
    }

    void HandleInput()
    {
        if (!beatAvailable)
        {
            ShowFeedback("Too Early!", Color.gray);
            return;
        }

        float timeSinceBeat = Time.time - lastBeatTime;

        if (timeSinceBeat <= perfectWindow)
        {
            Perfect();
        }
        else if (timeSinceBeat <= goodWindow)
        {
            Good();
        }
        else if (timeSinceBeat <= missWindow)
        {
            Miss();
        }

        beatAvailable = false;
    }

    void Perfect()
    {
        combo++;
        int points = Mathf.RoundToInt(perfectPoints * (1f + combo * comboMultiplier));
        score += points;

        ShowFeedback($"PERFECT! +{points}", Color.green);
        TriggerEffect(perfectEffect);
        UpdateUI();
    }

    void Good()
    {
        combo++;
        int points = Mathf.RoundToInt(goodPoints * (1f + combo * comboMultiplier));
        score += points;

        ShowFeedback($"Good +{points}", Color.yellow);
        TriggerEffect(goodEffect);
        UpdateUI();
    }

    void Miss()
    {
        if (combo > maxCombo)
            maxCombo = combo;

        combo = 0;
        score += missPoints;

        ShowFeedback("MISS", Color.red);
        TriggerEffect(missEffect);
        UpdateUI();
    }

    void TriggerEffect(ParticleSystem effect)
    {
        if (effect != null)
        {
            effect.Play();
        }
    }

    void ShowFeedback(string text, Color color)
    {
        if (feedbackText != null)
        {
            feedbackText.text = text;
            feedbackText.color = color;

            CancelInvoke("ClearFeedback");
            Invoke("ClearFeedback", 0.5f);
        }
    }

    void ClearFeedback()
    {
        if (feedbackText != null)
            feedbackText.text = "";
    }

    void UpdateUI()
    {
        if (scoreText != null)
            scoreText.text = $"Score: {score}";

        if (comboText != null)
        {
            if (combo > 0)
                comboText.text = $"Combo: {combo}x";
            else
                comboText.text = "";
        }
    }

    void OnDestroy()
    {
        detector?.Dispose();
    }

    void OnGUI()
    {
        GUI.Label(new Rect(10, 10, 300, 20), $"Instructions: Press SPACE on the beat!");
        GUI.Label(new Rect(10, 30, 300, 20), $"Max Combo: {maxCombo}");
        GUI.Label(new Rect(10, 50, 300, 20), $"Recent Beats: {recentBeats.Count}");
    }
}
```

### Playing the Game

1. **Start:**
   - Press Play
   - Music starts automatically

2. **Gameplay:**
   - Watch for beat indicator flash
   - Press Spacebar in time with music
   - Build combo for higher scores

3. **Scoring:**
   - **Perfect** (within 80ms): 100 points + combo bonus
   - **Good** (within 150ms): 50 points + combo bonus
   - **Miss** (> 150ms): -10 points, combo resets

4. **Tips:**
   - Watch the beat indicator animation
   - Listen for strong beats (kicks, snares)
   - Build combo for maximum score

---

## Example 4: Audio Effects Studio

**Scene:** `4_EffectsStudio.unity`

**Description:** Interactive audio effects processor with EQ, distortion, delay, and preset management.

### Features

- **3-Band EQ** - Low/Mid/High adjustable
- **Distortion** - Soft-clip with gain control
- **Delay/Echo** - Time, feedback, mix controls
- **Reverb** - Simple reverb effect
- **Presets** - Save/load effect chains
- **A/B Comparison** - Toggle effects on/off
- **Visual Feedback** - Real-time waveform and spectrum

### Scene Setup

```
EffectsStudio
├── AudioProcessor
│   └── Components: AudioEffectChain, AudioSource
├── UI Canvas
│   ├── EQ Panel
│   │   ├── Low Gain Slider (-20 to +20 dB)
│   │   ├── Mid Gain Slider
│   │   └── High Gain Slider
│   ├── Distortion Panel
│   │   ├── Amount Slider (0-1)
│   │   └── Gain Slider (0.1-10)
│   ├── Delay Panel
│   │   ├── Mix Slider (0-1)
│   │   ├── Time Slider (0.01-1s)
│   │   └── Feedback Slider (0-0.9)
│   ├── Master Panel
│   │   ├── Volume Slider
│   │   ├── Enable Effects Toggle
│   │   └── Preset Dropdown
│   └── Visualization
│       ├── Waveform Display
│       └── Spectrum Display
└── Main Camera
```

(The AudioEffectChain.cs script was provided in Tutorial 4, so it's ready to use)

### EffectsStudioUI.cs

```csharp
using UnityEngine;
using UnityEngine.UI;

public class EffectsStudioUI : MonoBehaviour
{
    [Header("Effect Chain Reference")]
    public AudioEffectChain effectChain;

    [Header("EQ Sliders")]
    public Slider lowGainSlider;
    public Slider midGainSlider;
    public Slider highGainSlider;

    [Header("Distortion Sliders")]
    public Slider distortionAmountSlider;
    public Slider distortionGainSlider;

    [Header("Delay Sliders")]
    public Slider delayMixSlider;
    public Slider delayTimeSlider;
    public Slider delayFeedbackSlider;

    [Header("Master Controls")]
    public Slider masterVolumeSlider;
    public Toggle enableEffectsToggle;
    public Dropdown presetDropdown;

    [Header("Text Labels")]
    public Text lowGainText;
    public Text midGainText;
    public Text highGainText;

    void Start()
    {
        SetupSliders();
        LoadPresetList();
    }

    void SetupSliders()
    {
        // EQ
        if (lowGainSlider != null)
        {
            lowGainSlider.onValueChanged.AddListener(v =>
            {
                if (effectChain != null) effectChain.lowGain = v;
                if (lowGainText != null) lowGainText.text = $"{v:F1} dB";
            });
        }

        if (midGainSlider != null)
        {
            midGainSlider.onValueChanged.AddListener(v =>
            {
                if (effectChain != null) effectChain.midGain = v;
                if (midGainText != null) midGainText.text = $"{v:F1} dB";
            });
        }

        if (highGainSlider != null)
        {
            highGainSlider.onValueChanged.AddListener(v =>
            {
                if (effectChain != null) effectChain.highGain = v;
                if (highGainText != null) highGainText.text = $"{v:F1} dB";
            });
        }

        // Distortion
        if (distortionAmountSlider != null)
        {
            distortionAmountSlider.onValueChanged.AddListener(v =>
            {
                if (effectChain != null) effectChain.distortionAmount = v;
            });
        }

        if (distortionGainSlider != null)
        {
            distortionGainSlider.onValueChanged.AddListener(v =>
            {
                if (effectChain != null) effectChain.distortionGain = v;
            });
        }

        // Delay
        if (delayMixSlider != null)
        {
            delayMixSlider.onValueChanged.AddListener(v =>
            {
                if (effectChain != null) effectChain.delayMix = v;
            });
        }

        if (delayTimeSlider != null)
        {
            delayTimeSlider.onValueChanged.AddListener(v =>
            {
                if (effectChain != null) effectChain.delayTime = v;
            });
        }

        if (delayFeedbackSlider != null)
        {
            delayFeedbackSlider.onValueChanged.AddListener(v =>
            {
                if (effectChain != null) effectChain.delayFeedback = v;
            });
        }

        // Master
        if (masterVolumeSlider != null)
        {
            masterVolumeSlider.onValueChanged.AddListener(v =>
            {
                if (effectChain != null) effectChain.masterVolume = v;
            });
        }
    }

    void LoadPresetList()
    {
        if (presetDropdown != null && effectChain != null)
        {
            presetDropdown.ClearOptions();

            var presetNames = new System.Collections.Generic.List<string>();
            foreach (var preset in effectChain.presets)
            {
                presetNames.Add(preset.name);
            }

            presetDropdown.AddOptions(presetNames);
            presetDropdown.onValueChanged.AddListener(LoadPreset);
        }
    }

    void LoadPreset(int index)
    {
        if (effectChain != null)
        {
            effectChain.LoadPreset(index);
            UpdateSlidersFromChain();
        }
    }

    void UpdateSlidersFromChain()
    {
        if (effectChain == null) return;

        if (lowGainSlider != null) lowGainSlider.value = effectChain.lowGain;
        if (midGainSlider != null) midGainSlider.value = effectChain.midGain;
        if (highGainSlider != null) highGainSlider.value = effectChain.highGain;
        if (distortionAmountSlider != null) distortionAmountSlider.value = effectChain.distortionAmount;
        if (distortionGainSlider != null) distortionGainSlider.value = effectChain.distortionGain;
        if (delayMixSlider != null) delayMixSlider.value = effectChain.delayMix;
        if (delayTimeSlider != null) delayTimeSlider.value = effectChain.delayTime;
        if (delayFeedbackSlider != null) delayFeedbackSlider.value = effectChain.delayFeedback;
        if (masterVolumeSlider != null) masterVolumeSlider.value = effectChain.masterVolume;
    }
}
```

### Using the Effects Studio

1. **Load Audio:**
   - Assign audio clip to AudioSource
   - Press Play

2. **Adjust EQ:**
   - Boost/cut Low (20-200 Hz)
   - Boost/cut Mid (200-2000 Hz)
   - Boost/cut High (2000+ Hz)

3. **Add Distortion:**
   - Increase Amount slider
   - Adjust Gain for intensity

4. **Add Delay:**
   - Increase Mix for wet/dry balance
   - Adjust Time for delay interval
   - Adjust Feedback for repeats

5. **Try Presets:**
   - Select from dropdown
   - Presets instantly apply all parameters

6. **Compare:**
   - Toggle "Enable Effects" to A/B test
   - Adjust master volume

---

## Example 5: Spectrogram Analyzer

**Scene:** `5_SpectrogramAnalyzer.unity`

**Description:** Scientific-grade spectrogram visualization with analysis tools.

### Features

- **Scrolling Spectrogram** - Time-frequency display
- **Colormap Options** - Jet, Hot, Grayscale
- **Frequency Scale** - Linear or logarithmic
- **Time Cursor** - Click to analyze specific time
- **Export** - Save spectrogram as PNG
- **Measurements** - Frequency and time readouts

(The SpectrogramVisualizer.cs script was provided in Tutorial 5)

### Additional Script - SpectrogramAnalyzer.cs

```csharp
using UnityEngine;
using UnityEngine.UI;
using Spectra;

public class SpectrogramAnalyzer : MonoBehaviour
{
    [Header("References")]
    public SpectrogramVisualizer visualizer;
    public Text cursorInfoText;
    public Toggle logFreqToggle;
    public Dropdown colormapDropdown;
    public Button exportButton;

    [Header("Cursor")]
    public RectTransform cursorLine;

    private float sampleRate;

    void Start()
    {
        sampleRate = AudioSettings.outputSampleRate;

        if (logFreqToggle != null)
        {
            logFreqToggle.onValueChanged.AddListener(OnLogFreqToggle);
        }

        if (exportButton != null)
        {
            exportButton.onClick.AddListener(ExportSpectrogram);
        }
    }

    void Update()
    {
        HandleMouseInput();
    }

    void HandleMouseInput()
    {
        if (Input.GetMouseButtonDown(0))
        {
            // Check if mouse is over spectrogram display
            RaycastHit hit;
            Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);

            if (Physics.Raycast(ray, out hit))
            {
                if (hit.collider.gameObject == visualizer.gameObject)
                {
                    AnalyzePoint(hit.textureCoord);
                }
            }
        }
    }

    void AnalyzePoint(Vector2 uv)
    {
        // Convert UV to frequency and time
        float freq = uv.y * (sampleRate / 2f);
        float timePerColumn = visualizer.hopSize / sampleRate;
        float relativeTime = uv.x * visualizer.displayWidth * timePerColumn;

        if (cursorInfoText != null)
        {
            cursorInfoText.text = $"Frequency: {freq:F0} Hz\nTime: {relativeTime:F2}s ago";
        }

        // Position cursor line
        if (cursorLine != null)
        {
            cursorLine.anchoredPosition = new Vector2(uv.x * visualizer.displayWidth, cursorLine.anchoredPosition.y);
        }

        Debug.Log($"Analyzed point: {freq:F0} Hz at {relativeTime:F2}s");
    }

    void OnLogFreqToggle(bool useLog)
    {
        if (visualizer != null)
        {
            visualizer.useLogarithmicFrequency = useLog;
        }
    }

    void ExportSpectrogram()
    {
        string timestamp = System.DateTime.Now.ToString("yyyyMMdd_HHmmss");
        string filename = $"Spectrogram_{timestamp}.png";
        string path = System.IO.Path.Combine(Application.persistentDataPath, filename);

        Texture2D texture = visualizer.GetComponent<Renderer>().material.mainTexture as Texture2D;

        if (texture != null)
        {
            byte[] bytes = texture.EncodeToPNG();
            System.IO.File.WriteAllBytes(path, bytes);

            Debug.Log($"Spectrogram exported to: {path}");
        }
    }
}
```

---

## Prefabs Reference

### Visualizer Prefabs

#### 1. SpectrumDisplay3D.prefab

**Components:**
- FrequencyBars3D script
- 64 bar meshes (children)
- Materials with gradient colors

**Configuration:**
- Num Bars: 64 (adjustable)
- Bar Width: 0.3
- Max Height: 10
- Color Gradient: Blue → Red

**Usage:**
```
Drag into scene
Assign AudioSource reference
Press Play
```

**Customization:**
- Adjust num bars for more/less detail
- Change color gradient
- Modify bar dimensions

---

#### 2. SpectrumDisplayUI.prefab

**Components:**
- Canvas Group
- Raw Image (spectrum texture)
- UI sliders for controls

**Configuration:**
- Resolution: 512x256
- Update Rate: 60 FPS
- Color Scheme: Customizable

**Usage:**
```
Drag into Canvas
Automatically connects to AudioListener
```

---

#### 3. WaveformDisplay.prefab

**Components:**
- Line Renderer
- WaveformRenderer script

**Configuration:**
- Display Width: 20 units
- Display Height: 2 units
- Line Color: Green
- Resolution: 512 points

**Usage:**
```
Drag into scene
Automatically visualizes audio
```

---

#### 4. SpectrogramDisplay.prefab

**Components:**
- Quad mesh
- Spectrogram material
- SpectrogramVisualizer script

**Configuration:**
- FFT Size: 1024
- Display Size: 512x256 pixels
- Colormap: Jet (default)

---

### Controller Prefabs

#### 5. BeatDetector.prefab

**Components:**
- OnsetDetector wrapper
- Event system

**Configuration:**
- Threshold: 0.5 (default)
- Min Interval: 100ms
- FFT Size: 2048

**Events:**
- OnBeatDetected(float time, float strength)

**Usage:**
```csharp
BeatDetector detector = Instantiate(beatDetectorPrefab);
detector.OnBeatDetected += (time, strength) =>
{
    Debug.Log($"Beat at {time}");
};
```

---

#### 6. AudioEffectChain.prefab

**Components:**
- AudioEffectChain script
- Pre-configured effects

**Includes:**
- 3-band EQ
- Distortion
- Delay
- Master volume

**Usage:**
```
Drag onto AudioSource GameObject
Adjust parameters in Inspector
```

---

#### 7. AudioAnalyzer.prefab

**Components:**
- FFTAnalyzer wrapper
- Automatic buffer management

**Provides:**
- Real-time spectrum
- Automatic memory management
- Configurable FFT size

**Usage:**
```csharp
AudioAnalyzer analyzer = GetComponent<AudioAnalyzer>();
float[] spectrum = analyzer.GetSpectrum();
```

---

### UI Prefabs

#### 8. EffectsPanel.prefab

**Components:**
- Complete effects UI
- Sliders for all parameters
- Preset dropdown
- Enable/disable toggle

**Connects to:** AudioEffectChain

---

#### 9. FilterControls.prefab

**Components:**
- Filter type dropdown
- Parameter sliders
- Frequency response plot

**Connects to:** FilterPlaygroundController

---

#### 10. AnalysisPanel.prefab

**Components:**
- Spectrum display
- Waveform display
- Analysis readouts

**Usage:**
Drag into Canvas for instant visualization

---

## Templates

### Template 1: Audio Analysis Setup

**Quick setup for audio analysis:**

```
1. Create empty GameObject: "AudioAnalysis"

2. Add Components:
   - AudioSource (with clip assigned)
   - AudioAnalyzer (from prefabs)

3. Connect visualization:
   - Drag SpectrumDisplay3D into scene
   - Assign AudioAnalysis reference

4. Press Play - instant visualization!
```

**Time:** 2 minutes

---

### Template 2: Real-Time Processing Setup

**For real-time audio effects:**

```
1. Create GameObject: "AudioProcessor"

2. Add Components:
   - AudioSource
   - AudioEffectChain (from prefabs)

3. Add UI:
   - Drag EffectsPanel into Canvas
   - Connect to AudioEffectChain

4. Configure audio:
   - Assign audio clip or enable microphone
   - Press Play
   - Adjust effects in real-time
```

**Time:** 3 minutes

---

### Template 3: Beat-Reactive Setup

**For beat-synchronized effects:**

```
1. Create GameObject: "BeatSystem"

2. Add Components:
   - AudioSource (with music)
   - BeatDetector (from prefabs)

3. Add reactive objects:
   - Create cubes/spheres
   - Add script to scale on beat

4. Connect:
   BeatDetector.OnBeatDetected += OnBeat;

5. Press Play - objects react to beats!
```

**Time:** 5 minutes

---

## Customization Guide

### Customizing Spectrum Display

**Change Colors:**
```csharp
// In FrequencyBars3D.cs
public Gradient colorGradient;

// In Inspector:
// Create custom gradient
// Assign to Color Gradient field
```

**Change Bar Count:**
```csharp
// In Inspector
Num Bars: 32  // Less detail, better performance
Num Bars: 128 // More detail, higher CPU cost
```

**Change Scaling:**
```csharp
// Linear to dB
float dB = 20f * Mathf.Log10(Mathf.Max(magnitude, 1e-6f));
float normalized = Mathf.InverseLerp(-60f, 0f, dB);

// dB to Linear
float linear = Mathf.Pow(10f, dB / 20f);
```

---

### Customizing Effects

**Add New Effect:**

```csharp
// In AudioEffectChain.cs

[Header("New Effect")]
public float effectParameter = 0f;

void ApplyNewEffect(float[] buffer, int length)
{
    for (int i = 0; i < length; i++)
    {
        // Your effect processing
        buffer[i] = ProcessSample(buffer[i]);
    }
}

// Call in OnAudioFilterRead:
if (effectParameter > 0.01f)
{
    ApplyNewEffect(mixed, samplesPerChannel);
}
```

---

### Customizing Beat Detection

**Adjust Sensitivity:**
```csharp
// Lower = more sensitive (more beats detected)
config.Threshold = 0.3f;

// Higher = less sensitive (only strong beats)
config.Threshold = 0.8f;
```

**Multi-Band Detection:**
```csharp
// Detect kicks (low freq)
var kickDetector = new OnsetDetector(config, sampleRate);
// Apply highpass filter to input before processing

// Detect snares (mid freq)
var snareDetector = new OnsetDetector(config, sampleRate);
// Apply bandpass filter to input

// Detect hi-hats (high freq)
var hihatDetector = new OnsetDetector(config, sampleRate);
// Apply lowpass filter to input
```

---

## Testing the Examples

### System Requirements

**Minimum:**
- Unity 2020.3+
- Windows/Mac/Linux
- Audio output device

**Recommended:**
- Unity 2021.3+
- Dedicated GPU for 3D visualizations
- Audio interface for low latency

---

### Performance Tips

1. **Reduce FFT Size for better FPS:**
   - 1024 instead of 2048
   - 512 for mobile

2. **Limit Visualizations:**
   - Don't run all 3 at once
   - Use simpler shaders

3. **Optimize Update Rate:**
   ```csharp
   private float updateInterval = 0.033f; // 30 Hz instead of 60 Hz
   ```

4. **Profile:**
   - Use Unity Profiler
   - Check Audio.Update time
   - Monitor GC allocations

---

## Troubleshooting Examples

### Issue: Spectrum bars don't move

**Check:**
1. AudioSource is playing
2. Audio clip assigned
3. OnAudioFilterRead is being called
4. FFTAnalyzer is not null

**Debug:**
```csharp
void OnAudioFilterRead(float[] data, int channels)
{
    Debug.Log($"Audio callback: {data.Length} samples");
}
```

---

### Issue: Beat detection not working

**Check:**
1. Music has clear beats
2. Threshold not too high
3. OnsetDetector initialized
4. Audio being processed

**Test with:**
- Simple 4/4 dance music
- Lower threshold (0.3)
- Check console for beat logs

---

### Issue: Effects sound distorted

**Check:**
1. Master volume not too high
2. No clipping in effect chain
3. Filter coefficients valid

**Fix:**
```csharp
// Add soft clipping
sample = Mathf.Clamp(sample, -1f, 1f);

// Or use tanh
sample = Mathf.Tanh(sample);
```

---

## Next Steps

**After exploring examples:**

1. **Modify Examples:**
   - Change colors, parameters
   - Add new features
   - Combine examples

2. **Create Your Own:**
   - Use prefabs as building blocks
   - Reference example scripts
   - Build custom applications

3. **Share:**
   - Post screenshots/videos
   - Share presets
   - Contribute improvements

---

## Additional Resources

- [API Reference](API_Reference.md) - Detailed function documentation
- [Tutorials](Tutorials.md) - Step-by-step learning
- [Filter Designer Guide](FilterDesignerGuide.md) - Filter design tool
- [Signal Inspector Guide](SignalInspectorGuide.md) - Audio analysis tool

---

## Conclusion

These example projects demonstrate:

✅ Real-time audio visualization
✅ Interactive filter design
✅ Beat-synchronized gameplay
✅ Audio effects processing
✅ Scientific analysis tools

All examples include:
- Complete, working code
- Reusable prefabs
- Customization options
- Performance optimization
- Documentation

**Start exploring and building!** 🎵
