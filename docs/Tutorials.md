# SpectraCore Unity Tutorials

Step-by-step guides for learning SpectraCore DSP in Unity.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Tutorial 1: Your First Filter](#tutorial-1-your-first-filter)
3. [Tutorial 2: Real-Time Spectrum Analyzer](#tutorial-2-real-time-spectrum-analyzer)
4. [Tutorial 3: Beat Detection and Visualization](#tutorial-3-beat-detection-and-visualization)
5. [Tutorial 4: Custom Audio Effect Processor](#tutorial-4-custom-audio-effect-processor)
6. [Tutorial 5: Building a Spectrogram](#tutorial-5-building-a-spectrogram)
7. [Tutorial 6: Bandpass & Bandstop Filters](#tutorial-6-bandpass--bandstop-filters) ⭐ NEW
8. [Integration Guide](#integration-guide)
9. [Best Practices](#best-practices)
10. [Quick Start Recipes](#quick-start-recipes)
11. [Troubleshooting](#troubleshooting)

---

## Getting Started

### Prerequisites

**Required:**
- Unity 2020.3 or later
- Basic C# knowledge
- Understanding of Unity MonoBehaviours

**Helpful but not required:**
- Audio DSP concepts
- Signal processing background
- MATLAB experience

---

### Installation

#### Step 1: Add SpectraCore to Your Project

**Option A: Import Unity Package**
1. Download `SpectraCore.unitypackage`
2. In Unity: Assets → Import Package → Custom Package
3. Select the package file
4. Click "Import" to import all files

**Option B: Copy Files Manually**
1. Copy the `unity/` folder contents to your Unity project:
   ```
   YourProject/
   ├── Assets/
   │   ├── SpectraCore/
   │   │   ├── Runtime/
   │   │   │   ├── Scripts/
   │   │   │   └── Plugins/      # Native libraries
   │   │   └── Editor/
   │   │       └── Scripts/
   ```

2. Ensure native libraries are in the correct locations:
   - Windows: `Plugins/Windows/x86_64/spectra.dll`
   - macOS: `Plugins/macOS/libspectra.dylib`
   - Linux: `Plugins/Linux/x86_64/libspectra.so`

---

#### Step 2: Verify Installation

Create a test script:

```csharp
using UnityEngine;
using Spectra;

public class SpectraTest : MonoBehaviour
{
    void Start()
    {
        // Test DSP functions
        var (b, a) = DSP.Butter(4, 0.25f, FilterType.Lowpass);
        Debug.Log($"Filter created: {b.Length} numerator coefficients");

        // Test FFT
        using (var fft = new FFTAnalyzer(1024))
        {
            Debug.Log($"FFT created with size {fft.Size}");
        }

        Debug.Log("SpectraCore is working!");
    }
}
```

**Expected Output:**
```
Filter created: 5 numerator coefficients
FFT created with size 1024
SpectraCore is working!
```

If you see these messages, installation is successful!

---

#### Step 3: Explore the Examples

**Open Example Windows:**
- Window → Spectra → Filter Designer
- Window → Spectra → Signal Inspector

**Load Sample Audio:**
- Import an audio file to your project
- Drag it into the Signal Inspector window
- Explore the waveform, spectrum, and spectrogram

---

### Project Structure

After installation, your project should have:

```
Assets/
├── SpectraCore/
│   ├── Runtime/
│   │   ├── Scripts/
│   │   │   ├── Core/
│   │   │   │   ├── DSP.cs              # Main API
│   │   │   │   ├── SpectraNative.cs    # P/Invoke bindings
│   │   │   │   └── AudioPipeline.cs    # Audio input
│   │   │   ├── Analysis/
│   │   │   │   ├── FFTAnalyzer.cs
│   │   │   │   ├── STFTAnalyzer.cs
│   │   │   │   └── OnsetDetector.cs
│   │   │   └── Filters/
│   │   │       └── StreamingIIRFilter.cs
│   │   └── Plugins/                     # Native libraries
│   └── Editor/
│       └── Scripts/
│           ├── FilterDesignerWindow.cs
│           └── SignalInspectorWindow.cs
```

---

## Tutorial 1: Your First Filter

**Goal:** Learn basic filter design and application

**Time:** 15 minutes

**What You'll Build:** A simple lowpass filter to remove high-frequency noise from audio

---

### Step 1: Create the Script

Create `MyFirstFilter.cs`:

```csharp
using UnityEngine;
using Spectra;

public class MyFirstFilter : MonoBehaviour
{
    [Header("Audio Source")]
    public AudioClip inputAudio;

    [Header("Filter Settings")]
    [Range(100f, 10000f)]
    public float cutoffFrequency = 1000f;

    [Range(1, 8)]
    public int filterOrder = 4;

    private float[] originalAudio;
    private float[] filteredAudio;
    private int sampleRate;

    void Start()
    {
        if (inputAudio == null)
        {
            Debug.LogError("Please assign an audio clip!");
            return;
        }

        LoadAudio();
        ApplyFilter();
        PlayFiltered();
    }

    void LoadAudio()
    {
        // Get audio data
        originalAudio = new float[inputAudio.samples * inputAudio.channels];
        inputAudio.GetData(originalAudio, 0);
        sampleRate = inputAudio.frequency;

        Debug.Log($"Loaded audio: {inputAudio.samples} samples at {sampleRate} Hz");
    }

    void ApplyFilter()
    {
        // Design lowpass filter
        float normalizedCutoff = cutoffFrequency / (sampleRate / 2f);
        var (b, a) = DSP.Butter(filterOrder, normalizedCutoff, FilterType.Lowpass);

        Debug.Log($"Designed {filterOrder}-order Butterworth lowpass filter");
        Debug.Log($"Cutoff: {cutoffFrequency} Hz (normalized: {normalizedCutoff:F4})");

        // Apply filter
        filteredAudio = DSP.LFilter(b, a, originalAudio);

        Debug.Log("Filter applied successfully!");
    }

    void PlayFiltered()
    {
        // Create AudioClip from filtered data
        AudioClip filtered = AudioClip.Create(
            "Filtered Audio",
            filteredAudio.Length / inputAudio.channels,
            inputAudio.channels,
            sampleRate,
            false
        );

        filtered.SetData(filteredAudio, 0);

        // Play it
        AudioSource audioSource = GetComponent<AudioSource>();
        if (audioSource == null)
            audioSource = gameObject.AddComponent<AudioSource>();

        audioSource.clip = filtered;
        audioSource.Play();

        Debug.Log("Playing filtered audio!");
    }
}
```

---

### Step 2: Set Up in Unity

1. **Create GameObject:**
   - Create empty GameObject: GameObject → Create Empty
   - Rename to "AudioFilter"

2. **Attach Script:**
   - Add `MyFirstFilter` component
   - Add `AudioSource` component

3. **Assign Audio:**
   - Import an audio file (any format Unity supports)
   - Drag it to "Input Audio" field in Inspector

4. **Adjust Settings:**
   - Cutoff Frequency: 1000 Hz (start here)
   - Filter Order: 4 (standard)

5. **Run:**
   - Press Play
   - Listen to the filtered audio
   - Check Console for messages

---

### Step 3: Experiment

**Try Different Cutoffs:**
- 500 Hz - Very muffled (only bass)
- 1000 Hz - Moderate filtering
- 5000 Hz - Subtle filtering
- 10000 Hz - Almost no change

**Try Different Orders:**
- Order 2 - Gentle rolloff
- Order 4 - Standard (balanced)
- Order 8 - Very steep rolloff

**Try Different Filter Types:**

Change this line:
```csharp
var (b, a) = DSP.Butter(filterOrder, normalizedCutoff, FilterType.Lowpass);
```

To:
```csharp
// Highpass - removes bass, keeps treble
var (b, a) = DSP.Butter(filterOrder, normalizedCutoff, FilterType.Highpass);
```

---

### Step 4: Visualize the Difference

Add visualization:

```csharp
void OnGUI()
{
    if (originalAudio == null || filteredAudio == null)
        return;

    // Draw original
    GUI.Label(new Rect(10, 10, 200, 20), "Original (white):");
    DrawWaveform(originalAudio, new Rect(10, 30, 400, 100), Color.white);

    // Draw filtered
    GUI.Label(new Rect(10, 140, 200, 20), "Filtered (green):");
    DrawWaveform(filteredAudio, new Rect(10, 160, 400, 100), Color.green);
}

void DrawWaveform(float[] samples, Rect rect, Color color)
{
    int step = Mathf.Max(1, samples.Length / (int)rect.width);

    for (int i = 0; i < (int)rect.width - 1; i++)
    {
        int idx1 = i * step;
        int idx2 = (i + 1) * step;

        if (idx2 >= samples.Length) break;

        float y1 = rect.y + rect.height / 2f - samples[idx1] * rect.height / 2f;
        float y2 = rect.y + rect.height / 2f - samples[idx2] * rect.height / 2f;

        Debug.DrawLine(
            new Vector3(rect.x + i, y1, 0),
            new Vector3(rect.x + i + 1, y2, 0),
            color,
            0.016f // One frame at 60 FPS
        );
    }
}
```

Now you can see the visual difference!

---

### What You Learned

✅ How to design a filter with `DSP.Butter()`
✅ How to normalize cutoff frequency
✅ How to apply a filter with `DSP.LFilter()`
✅ How to work with AudioClip data
✅ The effect of filter order on rolloff steepness
✅ Difference between filter types (lowpass vs highpass)

---

### Next Challenge

**Make it interactive:**
- Use sliders to adjust cutoff in real-time
- Add A/B comparison (switch between original and filtered)
- Display frequency response using `DSP.FreqzDb()`

**Hint for real-time:**
```csharp
void Update()
{
    if (Input.GetKeyDown(KeyCode.Space))
    {
        ApplyFilter(); // Reapply with new settings
        PlayFiltered();
    }
}
```

---

## Tutorial 2: Real-Time Spectrum Analyzer

**Goal:** Build a live audio spectrum visualizer

**Time:** 30 minutes

**What You'll Build:** A bar-graph spectrum analyzer that responds to audio in real-time

---

### Step 1: Create the Analyzer Script

Create `SpectrumAnalyzer.cs`:

```csharp
using UnityEngine;
using Spectra;

public class SpectrumAnalyzer : MonoBehaviour
{
    [Header("FFT Settings")]
    [Range(6, 13)]
    public int fftSizeLog2 = 11; // 2^11 = 2048

    [Range(0f, 1f)]
    public float smoothing = 0.8f;

    [Header("Display")]
    public int numBars = 64;
    public float barWidth = 10f;
    public float barSpacing = 2f;
    public float barHeightScale = 500f;
    public Color barColor = Color.cyan;

    private FFTAnalyzer fft;
    private float[] spectrum;
    private float[] smoothedSpectrum;
    private float[] audioBuffer;
    private int fftSize;

    void Start()
    {
        fftSize = 1 << fftSizeLog2; // 2^n
        fft = new FFTAnalyzer(fftSize);
        spectrum = new float[fft.SpectrumSize];
        smoothedSpectrum = new float[numBars];
        audioBuffer = new float[fftSize];

        Debug.Log($"Spectrum Analyzer initialized: FFT size {fftSize}");
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        // Convert to mono and fill buffer
        int samplesNeeded = Mathf.Min(fftSize, data.Length / channels);

        for (int i = 0; i < samplesNeeded; i++)
        {
            if (channels == 2)
                audioBuffer[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                audioBuffer[i] = data[i];
        }

        // Pad with zeros if needed
        for (int i = samplesNeeded; i < fftSize; i++)
        {
            audioBuffer[i] = 0f;
        }
    }

    void Update()
    {
        // Compute FFT
        float[] rawSpectrum = fft.GetMagnitudeSpectrum(audioBuffer);

        // Downsample to numBars
        int binsPerBar = rawSpectrum.Length / numBars;

        for (int i = 0; i < numBars; i++)
        {
            float sum = 0f;
            int start = i * binsPerBar;
            int end = Mathf.Min(start + binsPerBar, rawSpectrum.Length);

            for (int j = start; j < end; j++)
            {
                sum += rawSpectrum[j];
            }

            float average = sum / binsPerBar;

            // Smooth
            smoothedSpectrum[i] = Mathf.Lerp(
                average,
                smoothedSpectrum[i],
                smoothing
            );
        }
    }

    void OnRenderObject()
    {
        // Draw spectrum bars
        GL.PushMatrix();
        GL.LoadPixelMatrix();

        Material mat = new Material(Shader.Find("Hidden/Internal-Colored"));
        mat.SetPass(0);

        GL.Begin(GL.QUADS);
        GL.Color(barColor);

        float xPos = 50f;

        for (int i = 0; i < numBars; i++)
        {
            float height = smoothedSpectrum[i] * barHeightScale;
            float yPos = Screen.height - 100f;

            // Draw bar
            GL.Vertex3(xPos, yPos, 0);
            GL.Vertex3(xPos + barWidth, yPos, 0);
            GL.Vertex3(xPos + barWidth, yPos - height, 0);
            GL.Vertex3(xPos, yPos - height, 0);

            xPos += barWidth + barSpacing;
        }

        GL.End();
        GL.PopMatrix();
    }

    void OnDestroy()
    {
        fft?.Dispose();
    }
}
```

---

### Step 2: Set Up in Unity

1. **Create GameObject:**
   - GameObject → Create Empty
   - Rename to "SpectrumAnalyzer"

2. **Add Components:**
   - Add `SpectrumAnalyzer` script
   - Add `AudioSource` component
   - Add `AudioListener` component (if not on Main Camera)

3. **Configure AudioSource:**
   - Assign an audio clip
   - Check "Play On Awake"
   - Check "Loop"

4. **Adjust Settings:**
   - FFT Size Log2: 11 (2048 samples)
   - Smoothing: 0.8 (smooth animation)
   - Num Bars: 64
   - Bar Height Scale: 500 (adjust to fit screen)

5. **Run:**
   - Press Play
   - You should see bars responding to audio

---

### Step 3: Improve the Visualization

**Add logarithmic scaling (better for music):**

```csharp
void Update()
{
    float[] rawSpectrum = fft.GetMagnitudeSpectrum(audioBuffer);

    for (int i = 0; i < numBars; i++)
    {
        // Logarithmic frequency distribution
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

        // Convert to dB
        float dB = 20f * Mathf.Log10(Mathf.Max(average, 1e-6f));
        float normalized = Mathf.InverseLerp(-60f, 0f, dB);

        // Smooth
        smoothedSpectrum[i] = Mathf.Lerp(
            normalized,
            smoothedSpectrum[i],
            smoothing
        );
    }
}
```

---

### Step 4: Add Color Gradient

**Make bars change color by frequency:**

```csharp
void OnRenderObject()
{
    GL.PushMatrix();
    GL.LoadPixelMatrix();

    Material mat = new Material(Shader.Find("Hidden/Internal-Colored"));
    mat.SetPass(0);

    GL.Begin(GL.QUADS);

    float xPos = 50f;

    for (int i = 0; i < numBars; i++)
    {
        float height = smoothedSpectrum[i] * barHeightScale;
        float yPos = Screen.height - 100f;

        // Color gradient: blue (bass) → cyan → green → yellow → red (treble)
        float t = (float)i / numBars;
        Color color = Color.Lerp(Color.blue, Color.red, t);

        // Brightness based on amplitude
        color *= Mathf.Lerp(0.3f, 1.5f, smoothedSpectrum[i]);

        GL.Color(color);

        // Draw bar
        GL.Vertex3(xPos, yPos, 0);
        GL.Vertex3(xPos + barWidth, yPos, 0);
        GL.Vertex3(xPos + barWidth, yPos - height, 0);
        GL.Vertex3(xPos, yPos - height, 0);

        xPos += barWidth + barSpacing;
    }

    GL.End();
    GL.PopMatrix();
}
```

---

### Step 5: Add Microphone Input

**Make it respond to live audio:**

```csharp
[Header("Input Source")]
public bool useMicrophone = false;
public string microphoneDevice = "";

private AudioClip micClip;
private int lastMicPos = 0;

void Start()
{
    // ... existing code ...

    if (useMicrophone)
    {
        StartMicrophone();
    }
}

void StartMicrophone()
{
    if (Microphone.devices.Length == 0)
    {
        Debug.LogError("No microphone found!");
        return;
    }

    string device = string.IsNullOrEmpty(microphoneDevice)
        ? Microphone.devices[0]
        : microphoneDevice;

    micClip = Microphone.Start(device, true, 1, AudioSettings.outputSampleRate);
    lastMicPos = 0;

    AudioSource audioSource = GetComponent<AudioSource>();
    audioSource.clip = micClip;
    audioSource.loop = true;

    // Wait for mic to start
    while (Microphone.GetPosition(device) <= 0) { }

    audioSource.Play();

    Debug.Log($"Microphone started: {device}");
}

void Update()
{
    if (useMicrophone && micClip != null)
    {
        int currentPos = Microphone.GetPosition(microphoneDevice);

        if (currentPos < lastMicPos)
            lastMicPos = 0; // Wrapped

        if (currentPos - lastMicPos >= fftSize)
        {
            micClip.GetData(audioBuffer, lastMicPos);
            lastMicPos += fftSize / 2; // 50% overlap
        }
    }

    // ... existing FFT code ...
}
```

Now toggle "Use Microphone" to visualize live audio input!

---

### What You Learned

✅ Real-time FFT computation with `FFTAnalyzer`
✅ Audio thread (`OnAudioFilterRead`) vs game thread
✅ Downsampling spectrum for visualization
✅ Logarithmic frequency scaling
✅ Magnitude to dB conversion
✅ Temporal smoothing for animation
✅ OpenGL immediate mode rendering
✅ Microphone input in Unity

---

### Challenges

1. **Add peak hold:** Show peak values that slowly decay
2. **Add frequency labels:** Display Hz values for bars
3. **Add waveform:** Show time-domain alongside spectrum
4. **Make it 3D:** Render bars as 3D cubes in world space

---

## Tutorial 3: Beat Detection and Visualization

**Goal:** Detect beats in music and create visual effects

**Time:** 25 minutes

**What You'll Build:** A beat detection system that triggers particle effects

---

### Step 1: Create Beat Detector Script

Create `BeatDetector.cs`:

```csharp
using UnityEngine;
using Spectra;

public class BeatDetector : MonoBehaviour
{
    [Header("Detection Settings")]
    [Range(0.1f, 2f)]
    public float sensitivity = 0.5f;

    [Range(10f, 200f)]
    public float minBeatInterval = 100f; // ms

    [Header("Visual Feedback")]
    public ParticleSystem beatParticles;
    public Light beatLight;
    public float lightIntensity = 5f;
    public float lightFadeSpeed = 10f;

    [Header("Audio Analysis")]
    public bool showDebugInfo = true;

    private OnsetDetector detector;
    private float currentLightIntensity = 0f;
    private int beatCount = 0;
    private float lastBeatTime = 0f;

    void Start()
    {
        // Configure onset detector
        var config = new OnsetConfig
        {
            FftSize = 2048,
            HopSize = 512,
            Threshold = sensitivity,
            MinIntervalMs = minBeatInterval
        };

        detector = new OnsetDetector(config, AudioSettings.outputSampleRate);
        detector.OnOnsetDetected += OnBeat;

        Debug.Log("Beat detector initialized!");
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        // Convert to mono
        float[] mono = new float[data.Length / channels];

        for (int i = 0; i < mono.Length; i++)
        {
            if (channels == 2)
                mono[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                mono[i] = data[i];
        }

        // Process for beat detection
        detector.Process(mono);
    }

    void OnBeat(float timestamp, float strength)
    {
        beatCount++;
        lastBeatTime = Time.time;

        if (showDebugInfo)
        {
            Debug.Log($"Beat #{beatCount} at {timestamp:F3}s, strength: {strength:F2}");
        }

        // Trigger visual effects
        TriggerParticles(strength);
        TriggerLight(strength);
    }

    void TriggerParticles(float strength)
    {
        if (beatParticles == null) return;

        // Scale particle emission by beat strength
        var emission = beatParticles.emission;
        beatParticles.Emit((int)(strength * 50f));
    }

    void TriggerLight(float strength)
    {
        if (beatLight == null) return;

        // Flash light intensity
        currentLightIntensity = lightIntensity * strength;
    }

    void Update()
    {
        // Fade light
        if (beatLight != null)
        {
            currentLightIntensity = Mathf.Lerp(
                currentLightIntensity,
                0f,
                lightFadeSpeed * Time.deltaTime
            );

            beatLight.intensity = currentLightIntensity;
        }
    }

    void OnGUI()
    {
        if (!showDebugInfo) return;

        GUI.Label(new Rect(10, 10, 300, 20), $"Total Beats: {beatCount}");

        float timeSinceBeat = Time.time - lastBeatTime;
        GUI.Label(new Rect(10, 30, 300, 20), $"Last Beat: {timeSinceBeat:F2}s ago");

        if (timeSinceBeat < 0.1f)
        {
            GUI.backgroundColor = Color.red;
            GUI.Box(new Rect(10, 50, 100, 30), "BEAT!");
        }
    }

    void OnDestroy()
    {
        detector?.Dispose();
    }
}
```

---

### Step 2: Set Up the Scene

1. **Create Beat Detector GameObject:**
   - GameObject → Create Empty
   - Rename to "BeatDetector"
   - Add `BeatDetector` script
   - Add `AudioSource` component

2. **Add Visual Elements:**

   **Particle System:**
   ```
   GameObject → Effects → Particle System
   Rename to "BeatParticles"
   Configure:
   - Emission: Rate over Time = 0 (we emit manually)
   - Start Lifetime: 1
   - Start Speed: 5
   - Start Size: 0.5
   - Start Color: Random between colors
   ```

   **Point Light:**
   ```
   GameObject → Light → Point Light
   Rename to "BeatLight"
   Configure:
   - Intensity: 0 (controlled by script)
   - Range: 10
   - Color: Orange/Red
   ```

3. **Connect References:**
   - Drag BeatParticles to "Beat Particles" field
   - Drag BeatLight to "Beat Light" field

4. **Add Music:**
   - Import music file with clear beats
   - Assign to AudioSource
   - Check "Play On Awake" and "Loop"

5. **Adjust Sensitivity:**
   - Start with 0.5
   - Lower = more sensitive (more beats detected)
   - Higher = less sensitive (only strong beats)

---

### Step 3: Add Beat-Synchronized Objects

**Make objects pulse on beat:**

```csharp
using UnityEngine;
using Spectra;

public class BeatPulser : MonoBehaviour
{
    [Header("References")]
    public BeatDetector beatDetector;

    [Header("Pulse Settings")]
    public float pulseScale = 1.5f;
    public float pulseSpeed = 10f;

    private Vector3 originalScale;
    private Vector3 targetScale;

    void Start()
    {
        originalScale = transform.localScale;
        targetScale = originalScale;

        if (beatDetector == null)
        {
            beatDetector = FindObjectOfType<BeatDetector>();
        }
    }

    void Update()
    {
        // Smoothly return to original scale
        transform.localScale = Vector3.Lerp(
            transform.localScale,
            targetScale,
            pulseSpeed * Time.deltaTime
        );

        targetScale = Vector3.Lerp(targetScale, originalScale, pulseSpeed * Time.deltaTime);
    }

    // Call this from BeatDetector or subscribe to an event
    public void OnBeat(float strength)
    {
        targetScale = originalScale * (1f + pulseScale * strength);
    }
}
```

**Modify BeatDetector to support subscribers:**

```csharp
// Add to BeatDetector class
public event System.Action<float> OnBeatEvent;

void OnBeat(float timestamp, float strength)
{
    // ... existing code ...

    // Notify subscribers
    OnBeatEvent?.Invoke(strength);
}
```

**Connect in Unity:**
```csharp
void Start()
{
    BeatDetector detector = FindObjectOfType<BeatDetector>();
    detector.OnBeatEvent += OnBeat;
}
```

---

### Step 4: Create Beat-Driven Gameplay

**Example: Rhythm game mechanic:**

```csharp
using UnityEngine;
using Spectra;

public class RhythmGameController : MonoBehaviour
{
    [Header("Gameplay")]
    public float perfectTimingWindow = 0.1f; // seconds
    public float goodTimingWindow = 0.2f;
    public int score = 0;

    [Header("UI")]
    public UnityEngine.UI.Text scoreText;
    public UnityEngine.UI.Text feedbackText;

    private BeatDetector beatDetector;
    private float lastBeatTime = float.MinValue;
    private bool beatAvailable = false;

    void Start()
    {
        beatDetector = FindObjectOfType<BeatDetector>();
        beatDetector.OnBeatEvent += OnBeat;
    }

    void OnBeat(float strength)
    {
        lastBeatTime = Time.time;
        beatAvailable = true;

        // Visual cue for player
        ShowBeatCue();
    }

    void Update()
    {
        // Player input
        if (Input.GetKeyDown(KeyCode.Space) && beatAvailable)
        {
            float timeSinceBeat = Time.time - lastBeatTime;

            if (timeSinceBeat < perfectTimingWindow)
            {
                Score(100, "PERFECT!");
            }
            else if (timeSinceBeat < goodTimingWindow)
            {
                Score(50, "Good");
            }
            else
            {
                Score(-10, "Miss");
                beatAvailable = false;
            }
        }

        // Beat expired
        if (beatAvailable && Time.time - lastBeatTime > goodTimingWindow)
        {
            beatAvailable = false;
            ShowFeedback("Missed", Color.red);
        }
    }

    void Score(int points, string feedback)
    {
        score += points;
        scoreText.text = $"Score: {score}";

        Color color = points > 0 ? Color.green : Color.red;
        ShowFeedback(feedback, color);

        beatAvailable = false;
    }

    void ShowBeatCue()
    {
        // Flash indicator
        // Play sound effect
        // Animate beat marker
    }

    void ShowFeedback(string text, Color color)
    {
        feedbackText.text = text;
        feedbackText.color = color;

        // Fade out feedback after delay
        CancelInvoke("ClearFeedback");
        Invoke("ClearFeedback", 0.5f);
    }

    void ClearFeedback()
    {
        feedbackText.text = "";
    }
}
```

---

### Step 5: Advanced Beat Tracking

**Track tempo (BPM):**

```csharp
public class BPMTracker : MonoBehaviour
{
    private const int historySize = 8;
    private float[] beatIntervals = new float[historySize];
    private int historyIndex = 0;
    private float lastBeatTime = 0f;

    public float CurrentBPM { get; private set; }

    void Start()
    {
        BeatDetector detector = FindObjectOfType<BeatDetector>();
        detector.OnBeatEvent += OnBeat;
    }

    void OnBeat(float strength)
    {
        float currentTime = Time.time;

        if (lastBeatTime > 0f)
        {
            float interval = currentTime - lastBeatTime;
            beatIntervals[historyIndex] = interval;
            historyIndex = (historyIndex + 1) % historySize;

            // Calculate average interval
            float sum = 0f;
            int count = 0;

            for (int i = 0; i < historySize; i++)
            {
                if (beatIntervals[i] > 0f)
                {
                    sum += beatIntervals[i];
                    count++;
                }
            }

            if (count > 0)
            {
                float avgInterval = sum / count;
                CurrentBPM = 60f / avgInterval;

                Debug.Log($"Current BPM: {CurrentBPM:F1}");
            }
        }

        lastBeatTime = currentTime;
    }
}
```

---

### What You Learned

✅ Real-time onset detection with `OnsetDetector`
✅ Event-driven architecture for beat synchronization
✅ Sensitivity tuning for different music styles
✅ Visual feedback with particles and lights
✅ Beat-synchronized object animation
✅ Rhythm game mechanics (timing windows)
✅ BPM tracking from beat intervals

---

### Challenges

1. **Multi-band detection:** Detect kicks, snares, hi-hats separately
2. **Beat prediction:** Predict next beat for smoother gameplay
3. **Difficulty scaling:** Adjust timing windows by player performance
4. **Combo system:** Reward consecutive perfect hits

---

## Tutorial 4: Custom Audio Effect Processor

**Goal:** Build a real-time audio effects chain

**Time:** 35 minutes

**What You'll Build:** An adjustable effects processor with EQ, distortion, and delay

---

### Step 1: Create Effect Chain Script

Create `AudioEffectChain.cs`:

```csharp
using UnityEngine;
using Spectra;

public class AudioEffectChain : MonoBehaviour
{
    [Header("EQ (3-Band Equalizer)")]
    [Range(-20f, 20f)] public float lowGain = 0f;    // 20-200 Hz
    [Range(-20f, 20f)] public float midGain = 0f;    // 200-2000 Hz
    [Range(-20f, 20f)] public float highGain = 0f;   // 2000+ Hz

    [Header("Distortion")]
    [Range(0f, 1f)] public float distortionAmount = 0f;
    [Range(0.1f, 10f)] public float distortionGain = 1f;

    [Header("Delay")]
    [Range(0f, 1f)] public float delayMix = 0f;
    [Range(0.01f, 1f)] public float delayTime = 0.5f; // seconds
    [Range(0f, 0.9f)] public float delayFeedback = 0.5f;

    [Header("Master")]
    [Range(0f, 2f)] public float masterVolume = 1f;

    // EQ filters
    private StreamingIIRFilter lowBandFilter;
    private StreamingIIRFilter midBandFilter;
    private StreamingIIRFilter highBandFilter;

    // Delay buffer
    private float[] delayBuffer;
    private int delayBufferSize;
    private int delayWritePos = 0;
    private float sampleRate;

    // Processing buffers
    private float[] lowBand;
    private float[] midBand;
    private float[] highBand;
    private float[] mixed;

    void Start()
    {
        sampleRate = AudioSettings.outputSampleRate;
        InitializeFilters();
        InitializeDelayBuffer();

        Debug.Log("Audio Effect Chain initialized!");
    }

    void InitializeFilters()
    {
        // Low: Lowpass at 200 Hz
        float lowCutoff = 200f / (sampleRate / 2f);
        var lpf = DSP.Butter(4, lowCutoff, FilterType.Lowpass);
        lowBandFilter = new StreamingIIRFilter(lpf);

        // Mid: Bandpass 200-2000 Hz (using highpass + lowpass)
        float midLow = 200f / (sampleRate / 2f);
        var hpf = DSP.Butter(4, midLow, FilterType.Highpass);
        midBandFilter = new StreamingIIRFilter(hpf);

        // High: Highpass at 2000 Hz
        float highCutoff = 2000f / (sampleRate / 2f);
        var hpf2 = DSP.Butter(4, highCutoff, FilterType.Highpass);
        highBandFilter = new StreamingIIRFilter(hpf2);
    }

    void InitializeDelayBuffer()
    {
        delayBufferSize = (int)(sampleRate * 2f); // 2 seconds max
        delayBuffer = new float[delayBufferSize];
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        int samplesPerChannel = data.Length / channels;

        // Ensure buffers are allocated
        if (lowBand == null || lowBand.Length != samplesPerChannel)
        {
            lowBand = new float[samplesPerChannel];
            midBand = new float[samplesPerChannel];
            highBand = new float[samplesPerChannel];
            mixed = new float[samplesPerChannel];
        }

        // Extract mono
        for (int i = 0; i < samplesPerChannel; i++)
        {
            if (channels == 2)
                mixed[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                mixed[i] = data[i];
        }

        // Apply EQ
        ApplyEQ(mixed, samplesPerChannel);

        // Apply distortion
        if (distortionAmount > 0.01f)
        {
            ApplyDistortion(mixed, samplesPerChannel);
        }

        // Apply delay
        if (delayMix > 0.01f)
        {
            ApplyDelay(mixed, samplesPerChannel);
        }

        // Master volume and write back
        for (int i = 0; i < samplesPerChannel; i++)
        {
            float sample = mixed[i] * masterVolume;

            // Soft clip to prevent harsh clipping
            sample = Mathf.Clamp(sample, -1f, 1f);

            data[i * channels] = sample;
            if (channels == 2)
                data[i * channels + 1] = sample;
        }
    }

    void ApplyEQ(float[] buffer, int length)
    {
        // Process each band
        lowBandFilter.Process(buffer, lowBand);
        midBandFilter.Process(buffer, midBand);
        highBandFilter.Process(buffer, highBand);

        // Mix with gains (dB to linear)
        float lowMult = Mathf.Pow(10f, lowGain / 20f);
        float midMult = Mathf.Pow(10f, midGain / 20f);
        float highMult = Mathf.Pow(10f, highGain / 20f);

        for (int i = 0; i < length; i++)
        {
            buffer[i] = lowBand[i] * lowMult +
                       midBand[i] * midMult +
                       highBand[i] * highMult;
        }
    }

    void ApplyDistortion(float[] buffer, int length)
    {
        for (int i = 0; i < length; i++)
        {
            // Soft clipping distortion
            float x = buffer[i] * distortionGain;
            float distorted = Mathf.Sign(x) * (1f - Mathf.Exp(-Mathf.Abs(x)));

            // Mix wet/dry
            buffer[i] = Mathf.Lerp(buffer[i], distorted, distortionAmount);
        }
    }

    void ApplyDelay(float[] buffer, int length)
    {
        int delaySamples = (int)(delayTime * sampleRate);
        delaySamples = Mathf.Clamp(delaySamples, 1, delayBufferSize - 1);

        for (int i = 0; i < length; i++)
        {
            // Read from delay buffer
            int readPos = (delayWritePos - delaySamples + delayBufferSize) % delayBufferSize;
            float delayed = delayBuffer[readPos];

            // Mix
            float output = buffer[i] + delayed * delayMix;

            // Write to delay buffer (with feedback)
            delayBuffer[delayWritePos] = buffer[i] + delayed * delayFeedback;
            delayWritePos = (delayWritePos + 1) % delayBufferSize;

            buffer[i] = output;
        }
    }

    void OnDestroy()
    {
        lowBandFilter?.Dispose();
        midBandFilter?.Dispose();
        highBandFilter?.Dispose();
    }
}
```

---

### Step 2: Set Up in Unity

1. **Create Effect Processor:**
   - GameObject → Create Empty → "Audio FX"
   - Add `AudioEffectChain` script
   - Add `AudioSource` component

2. **Add Music:**
   - Assign audio clip
   - Play On Awake: checked
   - Loop: checked

3. **Test Effects:**
   - Press Play
   - Adjust sliders in real-time
   - Hear the effects immediately

---

### Step 3: Add Visual Feedback

**Create an effect visualizer:**

```csharp
using UnityEngine;
using UnityEngine.UI;

public class EffectVisualizer : MonoBehaviour
{
    public AudioEffectChain effectChain;

    [Header("UI Elements")]
    public Slider lowGainSlider;
    public Slider midGainSlider;
    public Slider highGainSlider;
    public Slider distortionSlider;
    public Slider delayMixSlider;

    [Header("Visual Feedback")]
    public Image lowBandIndicator;
    public Image midBandIndicator;
    public Image highBandIndicator;

    void Start()
    {
        // Connect sliders
        lowGainSlider.onValueChanged.AddListener(v => effectChain.lowGain = v);
        midGainSlider.onValueChanged.AddListener(v => effectChain.midGain = v);
        highGainSlider.onValueChanged.AddListener(v => effectChain.highGain = v);
        distortionSlider.onValueChanged.AddListener(v => effectChain.distortionAmount = v);
        delayMixSlider.onValueChanged.AddListener(v => effectChain.delayMix = v);
    }

    void Update()
    {
        // Update visual indicators
        UpdateBandIndicator(lowBandIndicator, effectChain.lowGain);
        UpdateBandIndicator(midBandIndicator, effectChain.midGain);
        UpdateBandIndicator(highBandIndicator, effectChain.highGain);
    }

    void UpdateBandIndicator(Image indicator, float gain)
    {
        // Color based on boost/cut
        if (gain > 0f)
            indicator.color = Color.Lerp(Color.white, Color.green, gain / 20f);
        else
            indicator.color = Color.Lerp(Color.white, Color.red, -gain / 20f);

        // Scale
        indicator.transform.localScale = Vector3.one * (1f + Mathf.Abs(gain) / 20f);
    }
}
```

---

### Step 4: Add Presets

**Create effect presets:**

```csharp
[System.Serializable]
public class EffectPreset
{
    public string name;
    public float lowGain, midGain, highGain;
    public float distortionAmount, distortionGain;
    public float delayMix, delayTime, delayFeedback;
}

// Add to AudioEffectChain class
public EffectPreset[] presets = new EffectPreset[]
{
    new EffectPreset
    {
        name = "Telephone",
        lowGain = -12f,
        midGain = 6f,
        highGain = -12f,
        distortionAmount = 0.3f
    },
    new EffectPreset
    {
        name = "Bass Boost",
        lowGain = 12f,
        midGain = 0f,
        highGain = -6f
    },
    new EffectPreset
    {
        name = "Echo Chamber",
        lowGain = 0f,
        midGain = 0f,
        highGain = 0f,
        delayMix = 0.6f,
        delayTime = 0.4f,
        delayFeedback = 0.7f
    },
    new EffectPreset
    {
        name = "Distorted Rock",
        lowGain = 3f,
        midGain = 6f,
        highGain = 0f,
        distortionAmount = 0.7f,
        distortionGain = 3f
    }
};

public void LoadPreset(int index)
{
    if (index < 0 || index >= presets.Length) return;

    EffectPreset preset = presets[index];
    lowGain = preset.lowGain;
    midGain = preset.midGain;
    highGain = preset.highGain;
    distortionAmount = preset.distortionAmount;
    distortionGain = preset.distortionGain;
    delayMix = preset.delayMix;
    delayTime = preset.delayTime;
    delayFeedback = preset.delayFeedback;

    Debug.Log($"Loaded preset: {preset.name}");
}
```

---

### Step 5: Add More Effects

**Reverb (simple):**

```csharp
[Header("Reverb")]
[Range(0f, 1f)] public float reverbMix = 0f;
[Range(0.1f, 2f)] public float reverbDecay = 0.5f;

private StreamingIIRFilter[] reverbFilters;
private const int numReverbDelays = 4;

void InitializeReverb()
{
    reverbFilters = new StreamingIIRFilter[numReverbDelays];
    float[] delays = { 0.037f, 0.043f, 0.051f, 0.067f }; // Prime number delays

    for (int i = 0; i < numReverbDelays; i++)
    {
        // Simple comb filter
        int delaySamples = (int)(delays[i] * sampleRate);
        // Implementation left as exercise
    }
}

void ApplyReverb(float[] buffer, int length)
{
    // Multi-tap delay for simple reverb
    // Mix all delays with decay
}
```

---

### What You Learned

✅ Real-time audio effects processing
✅ Multi-band EQ implementation
✅ Distortion algorithms (soft clipping)
✅ Delay/echo with feedback
✅ Effect chaining and signal flow
✅ dB to linear gain conversion
✅ Soft clipping for preventing harsh distortion
✅ Circular buffer for delay
✅ Effect presets and parameter management

---

### Challenges

1. **Add compressor:** Dynamic range compression
2. **Add chorus:** Modulated delay for thickness
3. **Add flanger:** Short modulated delay
4. **Add parametric EQ:** Adjustable frequency bands
5. **Save/load presets:** JSON serialization

---

## Tutorial 5: Building a Spectrogram

**Goal:** Create a time-frequency visualization

**Time:** 30 minutes

**What You'll Build:** A scrolling spectrogram showing audio content over time

---

### Step 1: Create Spectrogram Script

Create `SpectrogramVisualizer.cs`:

```csharp
using UnityEngine;
using Spectra;

public class SpectrogramVisualizer : MonoBehaviour
{
    [Header("Spectrogram Settings")]
    [Range(6, 12)]
    public int fftSizeLog2 = 10; // 1024

    [Range(1, 8)]
    public int hopDivider = 4; // 75% overlap

    public int displayWidth = 512;
    public int displayHeight = 256;

    [Header("Display")]
    public bool useLogarithmicFrequency = true;
    public float minDb = -80f;
    public float maxDb = 0f;

    private STFTAnalyzer stft;
    private Texture2D spectrogram;
    private Color[] pixels;
    private int currentColumn = 0;
    private float[] audioBuffer;
    private int fftSize;
    private int hopSize;

    void Start()
    {
        fftSize = 1 << fftSizeLog2;
        hopSize = fftSize / hopDivider;

        stft = new STFTAnalyzer(fftSize, hopSize, WindowType.Hann, true);

        // Create texture
        spectrogram = new Texture2D(displayWidth, displayHeight);
        spectrogram.filterMode = FilterMode.Point;
        spectrogram.wrapMode = TextureWrapMode.Clamp;

        pixels = new Color[displayWidth * displayHeight];

        // Clear to black
        for (int i = 0; i < pixels.Length; i++)
        {
            pixels[i] = Color.black;
        }

        spectrogram.SetPixels(pixels);
        spectrogram.Apply();

        // Display on quad or UI
        GetComponent<Renderer>().material.mainTexture = spectrogram;

        Debug.Log($"Spectrogram initialized: {fftSize} FFT, {hopSize} hop");
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        // Convert to mono
        int samplesPerChannel = data.Length / channels;

        if (audioBuffer == null || audioBuffer.Length != samplesPerChannel)
        {
            audioBuffer = new float[samplesPerChannel];
        }

        for (int i = 0; i < samplesPerChannel; i++)
        {
            if (channels == 2)
                audioBuffer[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                audioBuffer[i] = data[i];
        }

        // Push to STFT
        stft.PushSamples(audioBuffer);
    }

    void Update()
    {
        // Process all available frames
        while (stft.FramesAvailable > 0)
        {
            if (stft.PopFrame(out float[] real, out float[] imag))
            {
                AddFrameToSpectrogram(real, imag);
            }
        }
    }

    void AddFrameToSpectrogram(float[] real, float[] imag)
    {
        // Compute power spectrum
        float[] power = STFTAnalyzer.ComputePower(real, imag);

        // Convert to dB
        float[] dB = STFTAnalyzer.PowerToDb(power);

        // Map to display height
        for (int y = 0; y < displayHeight; y++)
        {
            int binIndex;

            if (useLogarithmicFrequency)
            {
                // Logarithmic frequency mapping
                float logMin = Mathf.Log10(1f);
                float logMax = Mathf.Log10(power.Length);
                float logY = Mathf.Lerp(logMin, logMax, (float)y / displayHeight);
                binIndex = (int)Mathf.Pow(10f, logY);
                binIndex = Mathf.Clamp(binIndex, 0, power.Length - 1);
            }
            else
            {
                // Linear frequency mapping
                binIndex = (int)((float)y / displayHeight * power.Length);
            }

            // Normalize dB to 0-1
            float normalized = Mathf.InverseLerp(minDb, maxDb, dB[binIndex]);

            // Get color
            Color color = GetSpectrogramColor(normalized);

            // Set pixel
            int pixelIndex = currentColumn + y * displayWidth;
            pixels[pixelIndex] = color;
        }

        // Move to next column
        currentColumn = (currentColumn + 1) % displayWidth;

        // Clear next column (for scrolling effect)
        for (int y = 0; y < displayHeight; y++)
        {
            int nextCol = (currentColumn + 1) % displayWidth;
            pixels[nextCol + y * displayWidth] = Color.black;
        }

        // Update texture
        spectrogram.SetPixels(pixels);
        spectrogram.Apply();
    }

    Color GetSpectrogramColor(float value)
    {
        // Jet colormap: Blue -> Cyan -> Green -> Yellow -> Red
        value = Mathf.Clamp01(value);

        if (value < 0.125f)
            return Color.Lerp(new Color(0, 0, 0.5f), Color.blue, value * 8f);
        else if (value < 0.375f)
            return Color.Lerp(Color.blue, Color.cyan, (value - 0.125f) * 4f);
        else if (value < 0.625f)
            return Color.Lerp(Color.cyan, Color.green, (value - 0.375f) * 4f);
        else if (value < 0.875f)
            return Color.Lerp(Color.green, Color.yellow, (value - 0.625f) * 4f);
        else
            return Color.Lerp(Color.yellow, Color.red, (value - 0.875f) * 8f);
    }

    void OnDestroy()
    {
        stft?.Dispose();
    }
}
```

---

### Step 2: Set Up Display

1. **Create Display Quad:**
   ```
   GameObject → 3D Object → Quad
   Rename to "Spectrogram Display"
   Scale: X=10, Y=5
   Rotation: X=0 (facing camera)
   ```

2. **Add Script:**
   - Add `SpectrogramVisualizer` component
   - Add `AudioSource` component

3. **Configure:**
   - FFT Size Log2: 10 (1024 samples)
   - Hop Divider: 4 (75% overlap)
   - Display Width: 512
   - Display Height: 256
   - Min dB: -80
   - Max dB: 0

4. **Add Audio:**
   - Assign music clip
   - Play and loop

---

### Step 3: Add Frequency Labels

```csharp
void OnGUI()
{
    // Draw frequency labels
    float sampleRate = AudioSettings.outputSampleRate;
    int numLabels = 5;

    for (int i = 0; i <= numLabels; i++)
    {
        float t = (float)i / numLabels;
        float freq;

        if (useLogarithmicFrequency)
        {
            float logMin = Mathf.Log10(20f);
            float logMax = Mathf.Log10(sampleRate / 2f);
            freq = Mathf.Pow(10f, Mathf.Lerp(logMin, logMax, t));
        }
        else
        {
            freq = t * sampleRate / 2f;
        }

        // Convert screen position
        Vector3 worldPos = transform.position +
                          Vector3.up * (t - 0.5f) * transform.localScale.y;
        Vector3 screenPos = Camera.main.WorldToScreenPoint(worldPos);

        // Draw label
        string label = freq >= 1000f ? $"{freq / 1000f:F1}kHz" : $"{freq:F0}Hz";
        GUI.Label(new Rect(screenPos.x + 10, Screen.height - screenPos.y - 10, 100, 20), label);
    }
}
```

---

### Step 4: Add Time Axis

```csharp
private float totalTime = 0f;

void Update()
{
    totalTime += Time.deltaTime;

    // ... existing code ...
}

void OnGUI()
{
    // Draw time marker
    float timePerColumn = hopSize / (float)AudioSettings.outputSampleRate;
    float totalDisplayTime = displayWidth * timePerColumn;

    Vector3 worldPos = transform.position + Vector3.down * transform.localScale.y / 2f;
    Vector3 screenPos = Camera.main.WorldToScreenPoint(worldPos);

    GUI.Label(
        new Rect(screenPos.x - 50, Screen.height - screenPos.y + 10, 200, 20),
        $"Time: {totalTime:F1}s (showing {totalDisplayTime:F1}s)"
    );
}
```

---

### Step 5: Add Interactive Features

**Click to analyze specific time/frequency:**

```csharp
void Update()
{
    // ... existing code ...

    // Mouse click analysis
    if (Input.GetMouseButtonDown(0))
    {
        Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
        RaycastHit hit;

        if (Physics.Raycast(ray, out hit))
        {
            if (hit.collider.gameObject == gameObject)
            {
                // Get UV coordinates
                Vector2 uv = hit.textureCoord;

                // Convert to frequency
                float sampleRate = AudioSettings.outputSampleRate;
                float freq = uv.y * sampleRate / 2f;

                // Convert to time
                float timePerColumn = hopSize / sampleRate;
                float relativeTime = uv.x * displayWidth * timePerColumn;

                Debug.Log($"Clicked: {freq:F0} Hz at {relativeTime:F2}s ago");
            }
        }
    }
}
```

---

### What You Learned

✅ STFT (Short-Time Fourier Transform) usage
✅ Time-frequency analysis
✅ Real-time texture generation
✅ Circular buffer for scrolling effect
✅ Logarithmic vs linear frequency mapping
✅ Power spectral density visualization
✅ Colormap implementation (Jet colormap)
✅ Interactive visualization techniques

---

### Challenges

1. **Add playback cursor:** Show current time position
2. **Add zoom:** Mouse wheel to zoom time/frequency
3. **Add screenshot export:** Save spectrogram as PNG
4. **Add harmonic tracking:** Highlight harmonic series
5. **Add onset markers:** Show detected beats on spectrogram

---

## Tutorial 6: Bandpass & Bandstop Filters ⭐ NEW

**Goal:** Design and apply bandpass/bandstop filters for practical audio processing

**Time:** 25 minutes

**What You'll Build:** A speech enhancement filter and hum removal system

---

### Step 1: Understanding Bandpass Filters

A **bandpass filter** isolates a specific frequency band:
- Attenuates frequencies below `lowFreq`
- Passes frequencies between `lowFreq` and `highFreq`
- Attenuates frequencies above `highFreq`

**Use Cases:**
- Speech enhancement (60 Hz - 3.5 kHz)
- Acoustic analysis (80 Hz - 5 kHz)
- Music frequency band separation (bass/mid/treble)

**Bandstop (Notch) Filter** does the opposite:
- Passes low and high frequencies
- Attenuates a specific band (the notch)

**Use Cases:**
- Removing 50/60 Hz mains hum
- Eliminating quantization noise
- Removing ultrasonic artifacts

---

### Step 2: Design a Speech Enhancement Filter

Create `SpeechEnhancer.cs`:

```csharp
using UnityEngine;
using Spectra;

public class SpeechEnhancer : MonoBehaviour
{
    [Header("Filter Settings")]
    [SerializeField] private int filterOrder = 4;
    [SerializeField] private AudioSource audioSource;

    private StreamingIIRFilter speechFilter;
    private float sampleRate;

    void Start()
    {
        sampleRate = AudioSettings.outputSampleRate;

        // Speech bandwidth: 60 Hz - 3500 Hz at 44.1 kHz
        // Normalized: 60/22050 ≈ 0.003, 3500/22050 ≈ 0.159
        float lowFreq = 60f / (sampleRate / 2f);
        float highFreq = 3500f / (sampleRate / 2f);

        // Butterworth for smooth response
        var (b, a) = DSP.Butter(filterOrder, lowFreq, highFreq, FilterType.Bandpass);
        speechFilter = new StreamingIIRFilter(b, a);

        Debug.Log($"Speech filter: {lowFreq:F4} - {highFreq:F4} (normalized)");
        Debug.Log($"Coefficients: {b.Length} numerator, {a.Length} denominator");
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        // Convert to mono
        float[] mono = new float[data.Length / channels];
        for (int i = 0; i < mono.Length; i++)
        {
            if (channels == 2)
                mono[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                mono[i] = data[i];
        }

        // Apply bandpass filter
        float[] filtered = speechFilter.Process(mono);

        // Convert back to stereo
        for (int i = 0; i < mono.Length; i++)
        {
            data[i * 2] = filtered[i];
            data[i * 2 + 1] = filtered[i];
        }
    }

    void OnDestroy()
    {
        speechFilter?.Dispose();
    }
}
```

**Key Points:**
- Order 4 is typical for speech (8 total for bandpass)
- Normalized frequency = Hz / (sampleRate / 2)
- Streaming filter maintains state across audio blocks

---

### Step 3: Design a Hum Removal Filter

Create `HumRemover.cs`:

```csharp
using UnityEngine;
using Spectra;

public class HumRemover : MonoBehaviour
{
    [Header("Hum Removal")]
    [SerializeField] private float humFrequency = 60f; // Hz
    [SerializeField] private float notchWidth = 10f;   // ±Hz around hum frequency
    [SerializeField] private int filterOrder = 6;       // High order for narrow notch

    private StreamingIIRFilter humFilter;
    private float sampleRate;

    void Start()
    {
        sampleRate = AudioSettings.outputSampleRate;

        // Convert Hz to normalized frequency
        float centerHumNorm = humFrequency / (sampleRate / 2f);
        float widthNorm = notchWidth / (sampleRate / 2f);

        // Create bandstop (notch) filter
        float lowFreq = centerHumNorm - widthNorm / 2f;
        float highFreq = centerHumNorm + widthNorm / 2f;

        // Clamp to valid range
        lowFreq = Mathf.Clamp(lowFreq, 0.001f, 0.999f);
        highFreq = Mathf.Clamp(highFreq, lowFreq + 0.001f, 0.999f);

        // Use high order for sharp notch
        var (b, a) = DSP.Butter(filterOrder, lowFreq, highFreq, FilterType.Bandstop);
        humFilter = new StreamingIIRFilter(b, a);

        Debug.Log($"Hum notch: {humFrequency} Hz ± {notchWidth} Hz (order {filterOrder})");
        Debug.Log($"Normalized: {lowFreq:F4} - {highFreq:F4}");
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        // Convert to mono
        float[] mono = new float[data.Length / channels];
        for (int i = 0; i < mono.Length; i++)
        {
            if (channels == 2)
                mono[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
            else
                mono[i] = data[i];
        }

        // Apply notch filter (removes hum only)
        float[] filtered = humFilter.Process(mono);

        // Convert back to stereo
        for (int i = 0; i < mono.Length; i++)
        {
            data[i * 2] = filtered[i];
            data[i * 2 + 1] = filtered[i];
        }
    }

    void OnDestroy()
    {
        humFilter?.Dispose();
    }
}
```

**Key Points:**
- Use bandstop to remove unwanted frequency
- Higher order (6-8) for narrower, sharper notch
- Preserve audio outside the notch

---

### Step 4: Practical Example - Multi-Band Analysis

Create `MusicBandAnalyzer.cs` to separate bass/mid/treble:

```csharp
using UnityEngine;
using Spectra;

public class MusicBandAnalyzer : MonoBehaviour
{
    [Header("Music Band Analysis")]
    [SerializeField] private int filterOrder = 4;
    [SerializeField] private float bassGain = 0f;   // dB
    [SerializeField] private float midGain = 0f;    // dB
    [SerializeField] private float trebleGain = 0f; // dB

    private StreamingIIRFilter bassFilter, midFilter, trebleFilter;
    private float sampleRate;

    void Start()
    {
        sampleRate = AudioSettings.outputSampleRate;

        // Define frequency bands (at 44.1 kHz)
        // Bass: 20-250 Hz
        var (b_bass, a_bass) = DSP.Butter(filterOrder,
            20f / (sampleRate / 2f),
            250f / (sampleRate / 2f),
            FilterType.Bandpass);
        bassFilter = new StreamingIIRFilter(b_bass, a_bass);

        // Mid: 250-2000 Hz
        var (b_mid, a_mid) = DSP.Butter(filterOrder,
            250f / (sampleRate / 2f),
            2000f / (sampleRate / 2f),
            FilterType.Bandpass);
        midFilter = new StreamingIIRFilter(b_mid, a_mid);

        // Treble: 2000-20000 Hz
        var (b_treble, a_treble) = DSP.Butter(filterOrder,
            2000f / (sampleRate / 2f),
            20000f / (sampleRate / 2f),
            FilterType.Bandpass);
        trebleFilter = new StreamingIIRFilter(b_treble, a_treble);

        Debug.Log("Music band filters initialized");
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        // Convert to mono
        float[] mono = new float[data.Length / channels];
        for (int i = 0; i < mono.Length; i++)
        {
            mono[i] = channels == 2
                ? (data[i * 2] + data[i * 2 + 1]) * 0.5f
                : data[i];
        }

        // Extract bands
        float[] bass = bassFilter.Process(mono);
        float[] mid = midFilter.Process(mono);
        float[] treble = trebleFilter.Process(mono);

        // Apply gains (dB → linear)
        float bassMultiplier = Mathf.Pow(10f, bassGain / 20f);
        float midMultiplier = Mathf.Pow(10f, midGain / 20f);
        float trebleMultiplier = Mathf.Pow(10f, trebleGain / 20f);

        // Combine
        for (int i = 0; i < mono.Length; i++)
        {
            float mixed = bass[i] * bassMultiplier +
                         mid[i] * midMultiplier +
                         treble[i] * trebleMultiplier;

            // Convert back to stereo
            data[i * 2] = mixed;
            data[i * 2 + 1] = mixed;
        }
    }

    void OnDestroy()
    {
        bassFilter?.Dispose();
        midFilter?.Dispose();
        trebleFilter?.Dispose();
    }
}
```

---

### Step 5: Verify Your Filter (Editor Test)

In the Unity Editor:

1. **Create Audio Test Scene:**
   - Add an AudioSource with a music file
   - Add SpeechEnhancer or HumRemover script
   - Play and compare before/after

2. **Inspect Frequency Response:**
   ```csharp
   // In Editor script or console
   float lowFreq = 0.003f;    // 60 Hz at 44.1 kHz
   float highFreq = 0.159f;   // 3500 Hz
   var (b, a) = DSP.Butter(4, lowFreq, highFreq, FilterType.Bandpass);
   var (mag, phase, freqs) = DSP.Freqz(b, a, 512);

   // Check gains
   Debug.Log($"DC gain: {20 * Mathf.Log10(mag[0]):F1} dB");
   Debug.Log($"Mid gain: {20 * Mathf.Log10(mag[256]):F1} dB");
   Debug.Log($"Nyquist gain: {20 * Mathf.Log10(mag[511]):F1} dB");
   ```

---

### What You Learned

✅ Bandpass filter design and application
✅ Bandstop (notch) filter for removing specific frequencies
✅ Dual-frequency API (`Butter(order, lowFreq, highFreq, type)`)
✅ Order doubling effect (order N → 2N for bandpass)
✅ Practical audio processing (speech, hum removal, multi-band)
✅ Real-time streaming with state preservation
✅ Frequency normalization (Hz ↔ normalized)

---

### Challenges

1. **Add parametric EQ:** Adjustable bandpass filters
2. **Add graphic EQ:** 10-band fixed-frequency analyzer
3. **Add dynamic ranges:** Real-time frequency monitoring
4. **Add visualization:** Show which frequencies are being amplified/reduced
5. **Add adaptive filters:** Change bands based on incoming audio content

---

## Integration Guide

### Adding SpectraCore to Existing Projects

#### Method 1: Package Import

**Best for:** New projects or clean integration

```
1. Download SpectraCore.unitypackage
2. Assets → Import Package → Custom Package
3. Select all files
4. Click Import
```

**Result:** Complete integration with examples

---

#### Method 2: Selective Import

**Best for:** Existing projects with namespace concerns

```
1. Copy only needed components:
   - Runtime/Scripts/Core/ (required)
   - Runtime/Scripts/Analysis/ (if using FFT/STFT)
   - Runtime/Scripts/Filters/ (if using filters)
   - Runtime/Plugins/ (required - native libraries)

2. Skip:
   - Editor scripts (optional - tools only)
   - Examples (optional)
```

---

#### Method 3: Namespace Isolation

**Best for:** Avoiding conflicts

```csharp
// Create wrapper with different namespace
namespace MyProject.Audio
{
    public static class DSPWrapper
    {
        public static (float[] b, float[] a) DesignFilter(
            int order, float freq)
        {
            return Spectra.DSP.Butter(order, freq);
        }

        // Wrap other functions as needed
    }
}
```

---

### Integration Patterns

#### Pattern 1: Component-Based

**Use AudioPipeline component:**

```csharp
// In your existing audio manager
public class AudioManager : MonoBehaviour
{
    private Spectra.AudioPipeline pipeline;

    void Start()
    {
        pipeline = gameObject.AddComponent<Spectra.AudioPipeline>();
        pipeline.OnAudioSamples += ProcessAudio;
    }

    void ProcessAudio(float[] samples)
    {
        // Your existing audio processing
    }
}
```

---

#### Pattern 2: Service-Based

**Singleton audio service:**

```csharp
public class AudioService : MonoBehaviour
{
    public static AudioService Instance { get; private set; }

    private Spectra.FFTAnalyzer fft;
    private Spectra.OnsetDetector onset;

    void Awake()
    {
        if (Instance != null)
        {
            Destroy(gameObject);
            return;
        }

        Instance = this;
        DontDestroyOnLoad(gameObject);

        InitializeSpectra();
    }

    void InitializeSpectra()
    {
        fft = new Spectra.FFTAnalyzer(2048);
        // Initialize other components
    }

    public float[] GetSpectrum(float[] audio)
    {
        return fft.GetMagnitudeSpectrum(audio);
    }

    void OnDestroy()
    {
        fft?.Dispose();
        onset?.Dispose();
    }
}
```

---

#### Pattern 3: Event-Driven

**Decouple via events:**

```csharp
public class AudioEvents
{
    public static event System.Action<float[]> OnSpectrumUpdated;
    public static event System.Action<float, float> OnBeatDetected;

    public static void RaiseSpectrum(float[] spectrum)
    {
        OnSpectrumUpdated?.Invoke(spectrum);
    }

    public static void RaiseBeat(float timestamp, float strength)
    {
        OnBeatDetected?.Invoke(timestamp, strength);
    }
}

// In your beat detector
void OnBeat(float timestamp, float strength)
{
    AudioEvents.RaiseBeat(timestamp, strength);
}

// In your UI
void Start()
{
    AudioEvents.OnBeatDetected += OnBeat;
}

void OnBeat(float time, float strength)
{
    // Update UI
}
```

---

### Common Integration Scenarios

#### Scenario 1: Adding to Existing Audio Game

**You have:** Audio-based gameplay (rhythm game, music visualizer)

**Integration:**
1. Keep existing AudioSource setup
2. Add OnsetDetector for beat detection
3. Add FFTAnalyzer for visualization
4. Connect via events to decouple systems

```csharp
// Your existing game controller
public class RhythmGameController : MonoBehaviour
{
    void Start()
    {
        var beatDetector = gameObject.AddComponent<BeatDetector>();
        AudioEvents.OnBeatDetected += OnGameBeat;
    }

    void OnGameBeat(float time, float strength)
    {
        // Your existing beat handling code
        SpawnBeatMarker();
        CheckPlayerInput();
    }
}
```

---

#### Scenario 2: Adding DSP to Audio Middleware

**You have:** FMOD, Wwise, or custom audio system

**Integration:**
1. Extract audio from middleware
2. Process with SpectraCore
3. Return processed audio or use for analysis only

```csharp
// With FMOD example
void ProcessFMODAudio()
{
    // Get audio from FMOD
    float[] samples = GetFMODAudioBuffer();

    // Process with SpectraCore
    var (b, a) = Spectra.DSP.Butter(4, 0.25f);
    float[] filtered = Spectra.DSP.LFilter(b, a, samples);

    // Send back to FMOD or use for analysis
    SendToFMOD(filtered);
}
```

---

#### Scenario 3: VR/XR Audio Spatialization

**You have:** Spatial audio setup

**Integration:**
1. Use SpectraCore for analysis only
2. Keep existing spatialization
3. Add reactive visuals based on analysis

```csharp
public class SpatialAudioAnalyzer : MonoBehaviour
{
    private Spectra.FFTAnalyzer fft;
    public AudioSource spatialSource;
    public GameObject[] reactiveObjects;

    void Update()
    {
        // Analyze but don't modify audio
        float[] buffer = GetAudioBuffer(spatialSource);
        float[] spectrum = fft.GetMagnitudeSpectrum(buffer);

        // React visually
        for (int i = 0; i < reactiveObjects.Length; i++)
        {
            float scale = spectrum[i * spectrum.Length / reactiveObjects.Length];
            reactiveObjects[i].transform.localScale = Vector3.one * (1f + scale);
        }
    }
}
```

---

## Best Practices

### Performance Optimization

#### 1. Minimize Allocations in Audio Thread

**Bad:**
```csharp
void OnAudioFilterRead(float[] data, int channels)
{
    var fft = new FFTAnalyzer(1024); // Allocation!
    float[] spectrum = fft.GetMagnitudeSpectrum(data); // Allocation!
    fft.Dispose();
}
```

**Good:**
```csharp
private FFTAnalyzer fft; // Reuse
private float[] spectrum; // Pre-allocated

void Start()
{
    fft = new FFTAnalyzer(1024);
    spectrum = new float[fft.SpectrumSize];
}

void OnAudioFilterRead(float[] data, int channels)
{
    // No allocations
    fft.Forward(data, spectrum, null);
}
```

---

#### 2. Choose Appropriate FFT Size

**Guidelines:**

| Use Case | FFT Size | Rationale |
|----------|----------|-----------|
| Real-time beat | 1024-2048 | Fast, sufficient resolution |
| Music visualization | 2048-4096 | Balanced |
| Pitch detection | 4096-8192 | High frequency resolution |
| Spectrogram | 1024-2048 | Balance time/freq |

**Rule:** Larger FFT = better frequency resolution, worse time resolution

---

#### 3. Use Appropriate Overlap

**Guidelines:**
- **No overlap (hop = FFT size):** Fastest, but can miss transients
- **50% overlap (hop = FFT/2):** Standard, good balance
- **75% overlap (hop = FFT/4):** Smooth visualization, higher CPU
- **> 75%:** Diminishing returns

---

#### 4. Dispose Resources

**Always dispose IDisposable objects:**

```csharp
void OnDestroy()
{
    fft?.Dispose();
    stft?.Dispose();
    onset?.Dispose();
    filter?.Dispose();
}
```

**Use using statements when possible:**
```csharp
void AnalyzeClip(AudioClip clip)
{
    using (var fft = new FFTAnalyzer(2048))
    {
        // Use fft
    } // Automatically disposed
}
```

---

### Thread Safety

#### Audio Thread vs Game Thread

**OnAudioFilterRead runs on audio thread:**
```csharp
// This is on AUDIO THREAD - be careful!
void OnAudioFilterRead(float[] data, int channels)
{
    // OK: Read/write to pre-allocated arrays
    ProcessAudio(data);

    // NOT OK: Access Unity objects (Transform, GameObject)
    // NOT OK: Allocate memory (new, List.Add)
    // NOT OK: Call Unity API (Debug.Log is OK)
}
```

**Safe pattern:**
```csharp
private float[] audioData;
private object lockObj = new object();

void OnAudioFilterRead(float[] data, int channels)
{
    lock (lockObj)
    {
        Array.Copy(data, audioData, data.Length);
    }
}

void Update()
{
    // Game thread - safe to access Unity objects
    lock (lockObj)
    {
        ProcessInGameThread(audioData);
    }
}
```

---

### Memory Management

#### 1. Pre-allocate Buffers

```csharp
// Bad - allocates every frame
void Update()
{
    float[] buffer = new float[1024]; // Allocation!
    GetAudioData(buffer);
}

// Good - allocate once
private float[] buffer;

void Start()
{
    buffer = new float[1024];
}

void Update()
{
    GetAudioData(buffer); // No allocation
}
```

---

#### 2. Reuse Objects

```csharp
// Bad - creates new filter every time
public float[] ApplyLowpass(float[] audio)
{
    var (b, a) = DSP.Butter(4, 0.25f);
    return DSP.LFilter(b, a, audio);
}

// Good - reuse filter coefficients
private float[] filterB, filterA;

void Start()
{
    (filterB, filterA) = DSP.Butter(4, 0.25f);
}

public float[] ApplyLowpass(float[] audio)
{
    return DSP.LFilter(filterB, filterA, audio);
}
```

---

#### 3. Use Object Pooling for Frequent Operations

```csharp
public class FFTPool
{
    private Queue<FFTAnalyzer> pool = new Queue<FFTAnalyzer>();
    private int fftSize;

    public FFTPool(int fftSize, int poolSize)
    {
        this.fftSize = fftSize;

        for (int i = 0; i < poolSize; i++)
        {
            pool.Enqueue(new FFTAnalyzer(fftSize));
        }
    }

    public FFTAnalyzer Get()
    {
        if (pool.Count > 0)
            return pool.Dequeue();

        return new FFTAnalyzer(fftSize);
    }

    public void Return(FFTAnalyzer fft)
    {
        pool.Enqueue(fft);
    }

    public void Dispose()
    {
        while (pool.Count > 0)
        {
            pool.Dequeue().Dispose();
        }
    }
}
```

---

### Error Handling

#### 1. Validate Parameters

```csharp
public void DesignFilter(float cutoffHz, float sampleRate)
{
    // Validate inputs
    if (cutoffHz <= 0f || cutoffHz >= sampleRate / 2f)
    {
        Debug.LogError($"Invalid cutoff: {cutoffHz} Hz (must be 0 < f < {sampleRate / 2f})");
        return;
    }

    float normalized = cutoffHz / (sampleRate / 2f);

    try
    {
        var (b, a) = DSP.Butter(4, normalized);
        // Use filter
    }
    catch (SpectraException ex)
    {
        Debug.LogError($"Filter design failed: {ex.Message}");
    }
}
```

---

#### 2. Handle Missing Audio

```csharp
void ProcessAudio()
{
    AudioClip clip = GetComponent<AudioSource>().clip;

    if (clip == null)
    {
        Debug.LogWarning("No audio clip assigned");
        return;
    }

    if (clip.samples == 0)
    {
        Debug.LogWarning("Audio clip is empty");
        return;
    }

    // Safe to process
}
```

---

#### 3. Graceful Degradation

```csharp
void Update()
{
    try
    {
        ProcessAudioFrame();
    }
    catch (Exception ex)
    {
        Debug.LogWarning($"Audio processing error: {ex.Message}");
        // Continue without crashing
    }
}
```

---

## Quick Start Recipes

### Recipe 1: Simple Lowpass Filter (5 lines)

```csharp
float cutoffHz = 1000f;
float sampleRate = 44100f;
float normalized = cutoffHz / (sampleRate / 2f);
var (b, a) = Spectra.DSP.Butter(4, normalized, Spectra.FilterType.Lowpass);
float[] filtered = Spectra.DSP.LFilter(b, a, audioData);
```

---

### Recipe 2: Get Audio Spectrum (3 lines)

```csharp
var fft = new Spectra.FFTAnalyzer(1024);
float[] spectrum = fft.GetMagnitudeSpectrum(audioFrame);
fft.Dispose();
```

---

### Recipe 3: Detect Beats (6 lines)

```csharp
var config = new Spectra.OnsetConfig { Threshold = 0.5f };
var detector = new Spectra.OnsetDetector(config, 44100f);
detector.OnOnsetDetected += (time, strength) =>
    Debug.Log($"Beat at {time}s");
detector.Process(audioSamples);
detector.Dispose();
```

---

### Recipe 4: Resample Audio (2 lines)

```csharp
// 44.1 kHz → 48 kHz
float[] resampled = Spectra.DSP.Resample(audio44k, 160, 147);
```

---

### Recipe 5: Get Filter Frequency Response (2 lines)

```csharp
var (b, a) = Spectra.DSP.Butter(4, 0.25f);
var (magDb, phase, freqs) = Spectra.DSP.FreqzDb(b, a, 512);
```

---

### Recipe 6: Compute Envelope (1 line)

```csharp
float[] envelope = Spectra.DSP.Envelope(audioSignal);
```

---

### Recipe 7: Real-Time Filtering (Setup + Process)

```csharp
// Setup (once)
var (b, a) = Spectra.DSP.Butter(4, 0.25f);
var filter = new Spectra.StreamingIIRFilter(b, a);

// Process (each block)
void OnAudioFilterRead(float[] data, int channels)
{
    filter.Process(data, data); // In-place
}
```

---

## Troubleshooting

### Common Issues

#### Issue 1: "DllNotFoundException: spectra"

**Cause:** Native library not found

**Solutions:**
1. Check library is in correct Plugins folder:
   - Windows: `Plugins/Windows/x86_64/spectra.dll`
   - macOS: `Plugins/macOS/libspectra.dylib`
   - Linux: `Plugins/Linux/x86_64/libspectra.so`

2. Verify platform settings in Unity:
   - Select library file
   - Check Inspector
   - Ensure correct platform is selected
   - CPU: x86_64
   - OS: Correct platform

3. Restart Unity after adding library

---

#### Issue 2: Filter produces no output

**Cause:** Incorrect normalization or unstable filter

**Check:**
```csharp
float cutoffHz = 1000f;
float sampleRate = AudioSettings.outputSampleRate;
float normalized = cutoffHz / (sampleRate / 2f);

Debug.Log($"Cutoff: {cutoffHz} Hz");
Debug.Log($"Sample Rate: {sampleRate} Hz");
Debug.Log($"Normalized: {normalized}");

// normalized should be between 0 and 1
if (normalized <= 0f || normalized >= 1f)
{
    Debug.LogError("Normalization out of range!");
}
```

---

#### Issue 3: Audio crackling/glitches

**Causes:**
1. Buffer size too small
2. Too much processing in audio thread
3. Allocations in audio thread

**Solutions:**
1. Increase buffer size:
   - Edit → Project Settings → Audio
   - DSP Buffer Size: Best Performance → Good Latency

2. Optimize audio thread:
   - Pre-allocate all buffers
   - Use simpler algorithms
   - Move analysis to Update()

3. Profile:
   - Use Unity Profiler
   - Check Audio.Update time
   - Should be < 10ms for 512 buffer

---

#### Issue 4: FFT returns all zeros

**Cause:** Input too quiet or all zeros

**Check:**
```csharp
float max = audioFrame.Max();
float min = audioFrame.Min();
Debug.Log($"Input range: {min} to {max}");

if (max == 0f && min == 0f)
{
    Debug.LogError("Input is silent!");
}
```

---

#### Issue 5: Beat detection not working

**Causes:**
1. Threshold too high/low
2. Music doesn't have clear beats
3. Sample rate mismatch

**Solutions:**
1. Adjust threshold:
   - Start with 0.3 (sensitive)
   - Increase if too many false positives
   - Decrease if missing beats

2. Check sample rate:
```csharp
Debug.Log($"Detector SR: 44100");
Debug.Log($"Unity SR: {AudioSettings.outputSampleRate}");
// Should match!
```

3. Test with different music:
   - Use music with strong beats
   - Avoid ambient/classical initially

---

### Performance Issues

#### Issue: Low FPS

**Check:**
1. FFT size too large
2. Too many effects
3. Inefficient visualization

**Profile:**
```csharp
void Update()
{
    var sw = System.Diagnostics.Stopwatch.StartNew();

    // Your audio processing

    sw.Stop();
    if (sw.ElapsedMilliseconds > 16) // 60 FPS = 16ms
    {
        Debug.LogWarning($"Slow frame: {sw.ElapsedMilliseconds}ms");
    }
}
```

---

### Getting Help

**When reporting issues:**

1. **Provide context:**
   - Unity version
   - Platform (Windows/Mac/Linux)
   - SpectraCore version

2. **Minimal reproduction:**
   - Simplest code that shows the problem
   - Sample audio if relevant

3. **Error messages:**
   - Full exception stack trace
   - Console output

4. **What you tried:**
   - Steps to reproduce
   - What you expected vs what happened

**Resources:**
- GitHub Issues: https://github.com/yogt1984/SpectraCore/issues
- API Reference: [API_Reference.md](API_Reference.md)
- Examples: Filter Designer, Signal Inspector windows

---

## Next Steps

**After completing these tutorials:**

1. **Explore Editor Tools:**
   - Window → Spectra → Filter Designer
   - Window → Spectra → Signal Inspector
   - Experiment with different filters
   - Analyze your own audio

2. **Read API Reference:**
   - [API_Reference.md](API_Reference.md)
   - Detailed documentation for all functions
   - Performance guidelines
   - Advanced usage patterns

3. **Build Your Own:**
   - Combine techniques from tutorials
   - Create custom audio effects
   - Build unique visualizations
   - Share your creations!

4. **Advanced Topics:**
   - Implement custom DSP algorithms
   - Integrate with audio middleware
   - Optimize for mobile/VR
   - Create audio-driven gameplay

---

## Conclusion

You now have the knowledge to:

✅ Design and apply digital filters
✅ Analyze audio in real-time (FFT, STFT)
✅ Detect beats and rhythm
✅ Create custom audio effects
✅ Visualize audio (spectrum, spectrogram)
✅ Integrate SpectraCore into existing projects
✅ Optimize for performance
✅ Troubleshoot common issues

**Happy coding! 🎵🎧**
