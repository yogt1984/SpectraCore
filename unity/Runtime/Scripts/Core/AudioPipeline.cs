using System;
using UnityEngine;

namespace Spectra
{
    /// <summary>
    /// Audio input pipeline for real-time DSP processing.
    /// </summary>
    public class AudioPipeline : MonoBehaviour
    {
        [SerializeField] private int bufferSize = 1024;
        [SerializeField] private bool useMicrophone = false;
        [SerializeField] private string microphoneDevice = "";

        private AudioClip microphoneClip;
        private int lastMicPosition;
        private float[] audioBuffer;

        /// <summary>
        /// Event fired when new audio samples are available.
        /// </summary>
        public event Action<float[]> OnAudioSamples;

        /// <summary>
        /// Current sample rate.
        /// </summary>
        public int SampleRate { get; private set; }

        private void Awake()
        {
            audioBuffer = new float[bufferSize];
            SampleRate = AudioSettings.outputSampleRate;
        }

        private void OnEnable()
        {
            if (useMicrophone)
            {
                StartMicrophone();
            }
        }

        private void OnDisable()
        {
            StopMicrophone();
        }

        private void Update()
        {
            if (useMicrophone && microphoneClip != null)
            {
                ProcessMicrophoneInput();
            }
        }

        private void StartMicrophone()
        {
            if (Microphone.devices.Length == 0)
            {
                Debug.LogWarning("No microphone devices found");
                return;
            }

            string device = string.IsNullOrEmpty(microphoneDevice)
                ? Microphone.devices[0]
                : microphoneDevice;

            microphoneClip = Microphone.Start(device, true, 1, SampleRate);
            lastMicPosition = 0;
        }

        private void StopMicrophone()
        {
            if (microphoneClip != null)
            {
                Microphone.End(microphoneDevice);
                microphoneClip = null;
            }
        }

        private void ProcessMicrophoneInput()
        {
            int currentPosition = Microphone.GetPosition(microphoneDevice);
            if (currentPosition < lastMicPosition)
            {
                // Wrapped around
                lastMicPosition = 0;
            }

            int samplesToRead = currentPosition - lastMicPosition;
            if (samplesToRead >= bufferSize)
            {
                microphoneClip.GetData(audioBuffer, lastMicPosition);
                lastMicPosition += bufferSize;
                OnAudioSamples?.Invoke(audioBuffer);
            }
        }

        /// <summary>
        /// Called by Unity's audio thread for AudioSource-based input.
        /// </summary>
        private void OnAudioFilterRead(float[] data, int channels)
        {
            if (useMicrophone) return;

            // Convert to mono if stereo
            if (channels == 2)
            {
                for (int i = 0; i < data.Length / 2 && i < audioBuffer.Length; i++)
                {
                    audioBuffer[i] = (data[i * 2] + data[i * 2 + 1]) * 0.5f;
                }
            }
            else
            {
                Array.Copy(data, audioBuffer, Math.Min(data.Length, audioBuffer.Length));
            }

            // Note: OnAudioFilterRead runs on audio thread
            // For thread-safe processing, use a lock-free queue
        }

        /// <summary>
        /// Get the current audio buffer (copy).
        /// </summary>
        public float[] GetAudioBuffer()
        {
            float[] copy = new float[audioBuffer.Length];
            Array.Copy(audioBuffer, copy, audioBuffer.Length);
            return copy;
        }
    }
}
