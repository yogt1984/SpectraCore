# SpectraCore Implementation Tasks

This document contains a systematic list of implementation tasks for SpectraCore. Each task is designed to be completable and testable in a single development session.

## Task Format

Each task includes:
- **ID**: Unique identifier (e.g., `CORE-001`)
- **Description**: What needs to be done
- **Files**: Files to create or modify
- **Tests**: How to verify completion
- **Dependencies**: Tasks that must be completed first

---

## Phase 0: Foundation (Open Source Core)

### CORE-001: Download and integrate real KissFFT
**Description:** Replace placeholder KissFFT with actual library source from GitHub.

**Files:**
- `native/third_party/kissfft/kiss_fft.h`
- `native/third_party/kissfft/kiss_fft.c`
- `native/third_party/kissfft/kiss_fftr.h`
- `native/third_party/kissfft/kiss_fftr.c`
- `native/third_party/kissfft/_kiss_fft_guts.h`

**Steps:**
1. Download KissFFT from https://github.com/mborgerding/kissfft
2. Copy required source files to `native/third_party/kissfft/`
3. Update `CMakeLists.txt` if needed
4. Build and verify no compilation errors

**Tests:**
```bash
cd native && cmake -B build && cmake --build build
./build/bin/test_fft
```

**Dependencies:** None

---

### CORE-002: Implement and test window functions
**Description:** Complete window function implementations with numerical validation.

**Files:**
- `native/src/core/window_functions.cpp`
- `native/tests/unit/test_window_functions.cpp`

**Steps:**
1. Verify all window types produce correct values
2. Add test comparing against known reference values
3. Test edge cases (N=1, N=2, very large N)

**Tests:**
```bash
cd native && cmake -B build && cmake --build build
./build/bin/test_window_functions
```

**Acceptance Criteria:**
- Hann window at center ≈ 1.0
- Hamming window at edges ≈ 0.08
- All windows symmetric
- All tests pass

**Dependencies:** CORE-001

---

### CORE-003: Implement and test FFT forward/inverse
**Description:** Verify FFT implementation produces correct results.

**Files:**
- `native/src/fft/fft.cpp`
- `native/tests/unit/test_fft.cpp`

**Steps:**
1. Test DC signal produces energy only at bin 0
2. Test sine wave produces peak at correct bin
3. Test inverse FFT recovers original signal (error < 1e-5)
4. Test power-of-2 and non-power-of-2 sizes

**Tests:**
```bash
./build/bin/test_fft
```

**Acceptance Criteria:**
- Forward-inverse roundtrip error < 1e-5
- Parseval's theorem holds (energy conservation)
- Correct bin locations for known frequencies

**Dependencies:** CORE-001

---

### CORE-004: Implement and test SPSC ring buffer
**Description:** Verify lock-free ring buffer correctness.

**Files:**
- `native/include/spectra/core/ring_buffer.hpp`
- `native/tests/unit/test_ring_buffer.cpp`

**Steps:**
1. Test push/pop single elements
2. Test batch operations
3. Test wraparound behavior
4. Test full/empty edge cases
5. Add multi-threaded stress test

**Tests:**
```bash
./build/bin/test_ring_buffer
```

**Acceptance Criteria:**
- FIFO ordering preserved
- No data corruption under concurrent access
- Correct capacity (Size - 1)

**Dependencies:** None

---

### CORE-005: Implement STFT with streaming support
**Description:** Complete STFT implementation for both batch and streaming modes.

**Files:**
- `native/src/fft/stft.cpp`
- `native/tests/unit/test_stft.cpp` (new file)

**Steps:**
1. Implement `analyze()` for batch processing
2. Implement `push_samples()` / `pop_frame()` for streaming
3. Implement `synthesize()` for reconstruction
4. Test overlap-add reconstruction

**Tests:**
```bash
./build/bin/test_stft
```

**Acceptance Criteria:**
- Streaming produces same frames as batch mode
- Overlap-add reconstruction error < 1e-4
- Correct number of frames for given input

**Dependencies:** CORE-001, CORE-002, CORE-003

---

### CORE-006: Implement spectral flux onset detection
**Description:** Complete onset detection with adaptive thresholding.

**Files:**
- `native/src/analysis/onset_detection.cpp`
- `native/tests/unit/test_onset_detection.cpp` (new file)

**Steps:**
1. Implement spectral flux calculation
2. Implement adaptive threshold
3. Implement peak picking with minimum interval
4. Test with synthetic signals (clicks, drums)

**Tests:**
```bash
./build/bin/test_onset_detection
```

**Acceptance Criteria:**
- Detects transients in test signals
- No false positives in silence
- Respects minimum interval constraint

**Dependencies:** CORE-005

---

## Phase 1: Filter Design

### FILT-001: Implement Butterworth filter design
**Description:** Complete Butterworth filter coefficient calculation using bilinear transform.

**Files:**
- `native/src/filters/filter_design.cpp`
- `native/tests/unit/test_butterworth.cpp` (new file)

**Steps:**
1. Implement analog prototype poles calculation
2. Implement bilinear transform (s-domain to z-domain)
3. Implement frequency prewarping
4. Support lowpass, highpass, bandpass, bandstop
5. Compare coefficients against MATLAB/SciPy reference

**Tests:**
```bash
./build/bin/test_butterworth
```

**Acceptance Criteria:**
- Order 1-8 filters match MATLAB butter() output
- DC gain = 1 for lowpass
- Stable filter (poles inside unit circle)

**Dependencies:** CORE-003

---

### FILT-002: Implement Chebyshev Type I filter design
**Description:** Add Chebyshev Type I filter with passband ripple.

**Files:**
- `native/src/filters/filter_design.cpp`
- `native/tests/unit/test_chebyshev.cpp` (new file)

**Steps:**
1. Implement Chebyshev polynomial calculation
2. Implement analog prototype with ripple parameter
3. Apply bilinear transform
4. Validate ripple in passband matches specification

**Tests:**
```bash
./build/bin/test_chebyshev
```

**Acceptance Criteria:**
- Passband ripple matches Rp parameter
- Steeper rolloff than Butterworth of same order
- Matches MATLAB cheby1() coefficients

**Dependencies:** FILT-001

---

### FILT-003: Implement Chebyshev Type II filter design
**Description:** Add Chebyshev Type II filter with stopband ripple.

**Files:**
- `native/src/filters/filter_design.cpp`
- `native/tests/unit/test_chebyshev.cpp`

**Steps:**
1. Implement inverse Chebyshev prototype
2. Apply bilinear transform
3. Validate stopband attenuation matches specification

**Tests:**
```bash
./build/bin/test_chebyshev
```

**Acceptance Criteria:**
- Stopband attenuation meets Rs parameter
- Monotonic passband
- Matches MATLAB cheby2() coefficients

**Dependencies:** FILT-002

---

### FILT-004: Implement elliptic filter design
**Description:** Add elliptic (Cauer) filter with steepest rolloff.

**Files:**
- `native/src/filters/filter_design.cpp`
- `native/tests/unit/test_elliptic.cpp` (new file)

**Steps:**
1. Implement elliptic integral calculations (or use approximation)
2. Implement pole-zero placement
3. Apply bilinear transform
4. Validate both passband and stopband specs

**Tests:**
```bash
./build/bin/test_elliptic
```

**Acceptance Criteria:**
- Passband ripple ≤ Rp
- Stopband attenuation ≥ Rs
- Steepest transition band of all IIR types

**Dependencies:** FILT-001

---

### FILT-005: Implement lfilter with initial conditions
**Description:** Complete IIR filter application with state handling.

**Files:**
- `native/src/filters/iir_filter.cpp`
- `native/tests/unit/test_lfilter.cpp` (new file)

**Steps:**
1. Implement Direct Form II Transposed structure
2. Add initial condition support (zi parameter)
3. Return final conditions (zf parameter)
4. Handle edge cases (empty input, single sample)

**Tests:**
```bash
./build/bin/test_lfilter
```

**Acceptance Criteria:**
- Matches MATLAB filter() output exactly
- State continuity across chunked processing
- Numerical stability for high-order filters

**Dependencies:** FILT-001

---

### FILT-006: Implement filtfilt zero-phase filtering
**Description:** Complete forward-backward filtering with edge handling.

**Files:**
- `native/src/filters/iir_filter.cpp`
- `native/tests/unit/test_filtfilt.cpp` (new file)

**Steps:**
1. Implement signal padding (reflected edges)
2. Compute initial conditions for steady-state startup
3. Apply filter forward then backward
4. Remove padding from output

**Tests:**
```bash
./build/bin/test_filtfilt
```

**Acceptance Criteria:**
- Zero phase delay (peaks align with input)
- No edge transients
- Matches MATLAB filtfilt() output

**Dependencies:** FILT-005

---

### FILT-007: Implement streaming IIR filter
**Description:** Complete stateful streaming filter for real-time use.

**Files:**
- `native/src/filters/streaming_filter.cpp`
- `native/tests/unit/test_streaming_filter.cpp`

**Steps:**
1. Verify chunked processing matches single-block
2. Add steady-state initialization
3. Test reset functionality
4. Benchmark processing throughput

**Tests:**
```bash
./build/bin/test_streaming_filter
```

**Acceptance Criteria:**
- Chunked output == single-block output
- Process 48kHz audio in < 10% CPU
- No memory allocations in process()

**Dependencies:** FILT-005

---

### FILT-008: Implement biquad cascade (SOS)
**Description:** Add second-order sections for numerical stability.

**Files:**
- `native/src/filters/streaming_filter.cpp`
- `native/tests/unit/test_biquad.cpp` (new file)

**Steps:**
1. Implement tf2sos conversion (pole-zero pairing)
2. Implement cascaded biquad processing
3. Compare high-order filter output vs direct form
4. Verify improved numerical stability

**Tests:**
```bash
./build/bin/test_biquad
```

**Acceptance Criteria:**
- SOS cascade matches direct form output
- Better stability for order > 4
- Proper pole-zero pairing (closest pairs)

**Dependencies:** FILT-007

---

### FILT-009: Implement FIR windowed-sinc design
**Description:** Complete FIR filter design using windowed sinc method.

**Files:**
- `native/src/filters/filter_design.cpp`
- `native/tests/unit/test_fir_design.cpp` (new file)

**Steps:**
1. Implement ideal sinc function
2. Apply window function
3. Implement spectral inversion for highpass
4. Implement spectral reversal for bandpass

**Tests:**
```bash
./build/bin/test_fir_design
```

**Acceptance Criteria:**
- Symmetric coefficients (linear phase)
- Correct cutoff frequency (-3dB point)
- Lowpass/highpass/bandpass all working

**Dependencies:** CORE-002

---

### FILT-010: Implement Parks-McClellan (firpm) design
**Description:** Add equiripple FIR design using Remez exchange.

**Files:**
- `native/src/filters/remez.cpp` (new file)
- `native/include/spectra/filters/remez.hpp` (new file)
- `native/tests/unit/test_firpm.cpp` (new file)

**Steps:**
1. Implement Remez exchange algorithm
2. Implement Chebyshev polynomial interpolation
3. Support arbitrary frequency/amplitude specs
4. Support weighted bands

**Tests:**
```bash
./build/bin/test_firpm
```

**Acceptance Criteria:**
- Equiripple behavior in passband/stopband
- Matches MATLAB firpm() for standard cases
- Convergence for order up to 100

**Dependencies:** FILT-009

---

## Phase 1: Signal Analysis

### ANAL-001: Implement cross-correlation (xcorr)
**Description:** Complete correlation with all normalization modes.

**Files:**
- `native/src/analysis/correlation.cpp`
- `native/tests/unit/test_correlation.cpp`

**Steps:**
1. Implement FFT-based fast correlation
2. Add 'biased', 'unbiased', 'normalized' modes
3. Add maxlag parameter support
4. Optimize for len(x) != len(y)

**Tests:**
```bash
./build/bin/test_correlation
```

**Acceptance Criteria:**
- O(N log N) complexity for large inputs
- Matches MATLAB xcorr() output
- Correct lag indexing

**Dependencies:** CORE-003

---

### ANAL-002: Implement power spectral density (pwelch)
**Description:** Complete Welch's method PSD estimation.

**Files:**
- `native/src/analysis/psd.cpp`
- `native/tests/unit/test_psd.cpp` (new file)

**Steps:**
1. Implement segment extraction with overlap
2. Apply window and compute periodogram
3. Average periodograms
4. Apply correct scaling (power/Hz)

**Tests:**
```bash
./build/bin/test_psd
```

**Acceptance Criteria:**
- White noise has flat PSD
- Sine wave has peak at correct frequency
- Matches MATLAB pwelch() output

**Dependencies:** CORE-002, CORE-003

---

### ANAL-003: Implement Hilbert transform
**Description:** Complete analytic signal computation.

**Files:**
- `native/src/analysis/hilbert.cpp`
- `native/tests/unit/test_hilbert.cpp` (new file)

**Steps:**
1. Implement frequency-domain Hilbert transform
2. Extract instantaneous amplitude (envelope)
3. Extract instantaneous phase
4. Extract instantaneous frequency

**Tests:**
```bash
./build/bin/test_hilbert
```

**Acceptance Criteria:**
- AM signal envelope matches modulation
- Phase is continuous (unwrapped)
- Instantaneous frequency of chirp is correct

**Dependencies:** CORE-003

---

### ANAL-004: Implement phase unwrapping
**Description:** Add phase unwrapping for continuous phase signals.

**Files:**
- `native/src/analysis/hilbert.cpp`
- `native/tests/unit/test_hilbert.cpp`

**Steps:**
1. Detect 2π discontinuities
2. Add cumulative correction
3. Handle edge cases (noise, rapid phase changes)

**Tests:**
```bash
./build/bin/test_hilbert
```

**Acceptance Criteria:**
- Monotonic phase for increasing frequency
- No false unwraps from noise
- Matches MATLAB unwrap()

**Dependencies:** ANAL-003

---

### ANAL-005: Implement polyphase resampling
**Description:** Complete rational resampling with anti-aliasing.

**Files:**
- `native/src/analysis/resample.cpp`
- `native/tests/unit/test_resample.cpp` (new file)

**Steps:**
1. Design anti-aliasing filter
2. Implement polyphase filter structure
3. Handle p/q reduction (GCD)
4. Verify no aliasing with spectral analysis

**Tests:**
```bash
./build/bin/test_resample
```

**Acceptance Criteria:**
- 48kHz→44.1kHz and back recovers signal
- No aliasing artifacts
- Correct output length

**Dependencies:** FILT-009

---

### ANAL-006: Implement decimation and interpolation
**Description:** Add integer-factor resampling functions.

**Files:**
- `native/src/analysis/resample.cpp`
- `native/tests/unit/test_resample.cpp`

**Steps:**
1. Implement decimate() with anti-alias filter
2. Implement interp() with interpolation filter
3. Verify energy preservation

**Tests:**
```bash
./build/bin/test_resample
```

**Acceptance Criteria:**
- decimate() prevents aliasing
- interp() smooth interpolation
- Round-trip preserves signal shape

**Dependencies:** ANAL-005

---

### ANAL-007: Implement frequency response (freqz)
**Description:** Add frequency response computation for filters.

**Files:**
- `native/src/filters/filter_design.cpp`
- `native/tests/unit/test_freqz.cpp` (new file)

**Steps:**
1. Evaluate transfer function H(e^jω)
2. Return magnitude and phase
3. Support arbitrary frequency points
4. Add group delay computation (grpdelay)

**Tests:**
```bash
./build/bin/test_freqz
```

**Acceptance Criteria:**
- DC gain correct for lowpass
- -3dB point at cutoff frequency
- Group delay flat for linear phase FIR

**Dependencies:** FILT-001

---

## Phase 1: C ABI Completion

### ABI-001: Complete FFT ABI functions
**Description:** Ensure all FFT functions work through C ABI.

**Files:**
- `native/src/abi/spectra_abi.cpp`
- `native/tests/unit/test_abi_fft.cpp` (new file)

**Steps:**
1. Test spectra_fft_create/destroy
2. Test spectra_fft_forward/inverse
3. Verify no memory leaks (valgrind)
4. Test error handling

**Tests:**
```bash
./build/bin/test_abi_fft
valgrind ./build/bin/test_abi_fft
```

**Acceptance Criteria:**
- All FFT operations work via ABI
- Proper error codes on invalid input
- No memory leaks

**Dependencies:** CORE-003

---

### ABI-002: Complete filter ABI functions
**Description:** Ensure all filter functions work through C ABI.

**Files:**
- `native/src/abi/spectra_abi.cpp`
- `native/tests/unit/test_abi_filter.cpp` (new file)

**Steps:**
1. Test spectra_butter, spectra_cheby1, etc.
2. Test spectra_streaming_filter_*
3. Test spectra_lfilter, spectra_filtfilt
4. Verify coefficient arrays are correctly populated

**Tests:**
```bash
./build/bin/test_abi_filter
```

**Acceptance Criteria:**
- All filter design functions accessible
- Streaming filter works via ABI
- Correct coefficient sizes returned

**Dependencies:** FILT-001 through FILT-007

---

### ABI-003: Complete analysis ABI functions
**Description:** Ensure all analysis functions work through C ABI.

**Files:**
- `native/src/abi/spectra_abi.cpp`
- `native/tests/unit/test_abi_analysis.cpp` (new file)

**Steps:**
1. Test spectra_xcorr
2. Test spectra_pwelch
3. Test spectra_hilbert and derivatives
4. Test spectra_resample

**Tests:**
```bash
./build/bin/test_abi_analysis
```

**Acceptance Criteria:**
- All analysis functions accessible via ABI
- Correct output sizes
- Proper error handling

**Dependencies:** ANAL-001 through ANAL-006

---

### ABI-004: Implement thread-safe error handling
**Description:** Complete thread-local error storage and retrieval.

**Files:**
- `native/src/abi/spectra_abi.cpp`
- `native/tests/unit/test_abi_errors.cpp` (new file)

**Steps:**
1. Verify thread-local error storage
2. Test error codes for all failure modes
3. Test error message strings
4. Test spectra_clear_error()

**Tests:**
```bash
./build/bin/test_abi_errors
```

**Acceptance Criteria:**
- Errors don't cross threads
- Meaningful error messages
- Error cleared after retrieval

**Dependencies:** ABI-001

---

## Phase 1: Unity Integration

### UNITY-001: Test P/Invoke bindings on Windows
**Description:** Verify all native bindings work in Unity on Windows.

**Files:**
- `unity/Runtime/Scripts/Core/SpectraNative.cs`
- `unity/Tests/Editor/SpectraNativeTests.cs` (new file)

**Steps:**
1. Build native library for Windows x64
2. Copy to unity/Runtime/Plugins/Windows/x86_64/
3. Create Unity test project
4. Run P/Invoke tests

**Tests:**
```
Unity Test Runner → Edit Mode Tests
```

**Acceptance Criteria:**
- All P/Invoke calls succeed
- No marshalling errors
- Correct data returned

**Dependencies:** ABI-001 through ABI-004

---

### UNITY-002: Test P/Invoke bindings on macOS
**Description:** Verify all native bindings work in Unity on macOS.

**Files:**
- Same as UNITY-001

**Steps:**
1. Build native library for macOS (universal)
2. Copy to unity/Runtime/Plugins/macOS/
3. Run P/Invoke tests

**Tests:**
```
Unity Test Runner → Edit Mode Tests
```

**Acceptance Criteria:**
- Works on both Intel and Apple Silicon
- All tests pass

**Dependencies:** UNITY-001

---

### UNITY-003: Implement FFTAnalyzer component
**Description:** Complete Unity-ready FFT analyzer with visualization support.

**Files:**
- `unity/Runtime/Scripts/Analysis/FFTAnalyzer.cs`
- `unity/Tests/Runtime/FFTAnalyzerTests.cs` (new file)

**Steps:**
1. Add spectrum caching for visualization
2. Add frequency bin to Hz conversion
3. Add dB conversion helpers
4. Test with AudioSource input

**Tests:**
```
Unity Test Runner → Play Mode Tests
```

**Acceptance Criteria:**
- Real-time spectrum analysis works
- Memory-efficient (no allocations per frame)
- Thread-safe

**Dependencies:** UNITY-001

---

### UNITY-004: Implement StreamingIIRFilter component
**Description:** Complete real-time filter for Unity audio.

**Files:**
- `unity/Runtime/Scripts/Filters/StreamingIIRFilter.cs`
- `unity/Tests/Runtime/StreamingFilterTests.cs` (new file)

**Steps:**
1. Add factory methods (CreateLowpass, CreateHighpass, etc.)
2. Test in OnAudioFilterRead
3. Verify latency < 10ms
4. Add bypass functionality

**Tests:**
```
Unity Test Runner → Play Mode Tests
```

**Acceptance Criteria:**
- Works in OnAudioFilterRead
- No audio glitches
- Proper disposal

**Dependencies:** UNITY-001

---

### UNITY-005: Implement OnsetDetector component
**Description:** Complete real-time onset detection for Unity.

**Files:**
- `unity/Runtime/Scripts/Analysis/OnsetDetector.cs`
- `unity/Tests/Runtime/OnsetDetectorTests.cs` (new file)

**Steps:**
1. Add UnityEvent for onset callbacks
2. Add threshold adjustment UI
3. Test with AudioSource and Microphone
4. Add beat visualization demo

**Tests:**
```
Unity Test Runner → Play Mode Tests
```

**Acceptance Criteria:**
- Real-time onset detection
- Callback invoked on main thread
- Configurable sensitivity

**Dependencies:** UNITY-001, CORE-006

---

### UNITY-006: Implement Signal Inspector window
**Description:** Complete editor window for signal visualization.

**Files:**
- `unity/Editor/Scripts/SignalInspectorWindow.cs`

**Steps:**
1. Add AudioClip waveform display
2. Add spectrum display with frequency axis
3. Add filter response overlay
4. Add data export (CSV)

**Tests:**
```
Manual: Window → Spectra → Signal Inspector
```

**Acceptance Criteria:**
- Displays waveform correctly
- Spectrum updates on FFT size change
- No editor freezing for large files

**Dependencies:** UNITY-003

---

### UNITY-007: Implement Filter Designer window
**Description:** Add interactive filter design editor tool.

**Files:**
- `unity/Editor/Scripts/FilterDesignerWindow.cs` (new file)

**Steps:**
1. Add filter type selection
2. Add order/cutoff/ripple parameters
3. Add real-time Bode plot (magnitude + phase)
4. Add coefficient export

**Tests:**
```
Manual: Window → Spectra → Filter Designer
```

**Acceptance Criteria:**
- Real-time Bode plot updates
- Shows pole-zero diagram
- Can copy coefficients to clipboard

**Dependencies:** ANAL-007, UNITY-001

---

## Phase 1: Documentation

### DOC-001: Add API reference for core functions
**Description:** Document all public C++ APIs with examples.

**Files:**
- `docs/api/core.md` (new file)
- `docs/api/fft.md` (new file)

**Steps:**
1. Document all public classes/functions
2. Add code examples
3. Add parameter descriptions
4. Add return value descriptions

**Acceptance Criteria:**
- All public APIs documented
- At least one example per function
- Consistent format

**Dependencies:** CORE-001 through CORE-006

---

### DOC-002: Add API reference for filter functions
**Description:** Document all filter design and application functions.

**Files:**
- `docs/api/filters.md` (new file)

**Steps:**
1. Document filter design functions
2. Add MATLAB comparison examples
3. Document streaming filter usage
4. Add filter selection guide

**Acceptance Criteria:**
- Side-by-side MATLAB comparison
- Explains when to use each filter type
- Includes frequency response plots

**Dependencies:** FILT-001 through FILT-010

---

### DOC-003: Add Unity integration guide
**Description:** Document Unity package usage.

**Files:**
- `docs/unity-guide.md` (new file)

**Steps:**
1. Installation instructions
2. Quick start example
3. Real-time audio processing guide
4. Performance tips

**Acceptance Criteria:**
- Step-by-step setup
- Working code samples
- Platform-specific notes

**Dependencies:** UNITY-001 through UNITY-005

---

## Phase 1: CI/CD

### CI-001: Fix CI pipeline for all platforms
**Description:** Ensure CI builds and tests pass on all platforms.

**Files:**
- `.github/workflows/ci.yml`

**Steps:**
1. Fix any build errors on Windows/macOS/Linux
2. Ensure tests run and pass
3. Add test result artifacts
4. Configure branch protection

**Tests:**
```
GitHub Actions → CI workflow
```

**Acceptance Criteria:**
- Green builds on all platforms
- Test results visible in PR
- Coverage report generated

**Dependencies:** All CORE and FILT tasks

---

### CI-002: Implement MATLAB reference comparison
**Description:** Add automated comparison against MATLAB output.

**Files:**
- `tools/matlab/generate_test_vectors.m`
- `native/tests/unit/test_matlab_reference.cpp` (new file)
- `.github/workflows/ci.yml`

**Steps:**
1. Generate MATLAB test vectors (pre-commit)
2. Add test that loads and compares vectors
3. Report max absolute error for each function
4. Fail if error exceeds threshold

**Tests:**
```bash
./build/bin/test_matlab_reference
```

**Acceptance Criteria:**
- All functions < 1e-5 error vs MATLAB
- Test vectors checked into repo
- CI runs comparison automatically

**Dependencies:** All FILT and ANAL tasks

---

## Task Status Legend

- ⬜ Not started
- 🟡 In progress
- ✅ Complete
- ❌ Blocked

## Progress Tracking

| Phase | Total | Complete | Progress |
|-------|-------|----------|----------|
| Phase 0: Foundation | 6 | 0 | 0% |
| Phase 1: Filters | 10 | 0 | 0% |
| Phase 1: Analysis | 7 | 0 | 0% |
| Phase 1: ABI | 4 | 0 | 0% |
| Phase 1: Unity | 7 | 0 | 0% |
| Phase 1: Documentation | 3 | 0 | 0% |
| Phase 1: CI/CD | 2 | 0 | 0% |
| **Total** | **39** | **0** | **0%** |

## Execution Order (Recommended)

For systematic implementation, follow this order:

### Week 1-2: Core Foundation
1. CORE-001 (KissFFT integration)
2. CORE-002 (Window functions)
3. CORE-003 (FFT)
4. CORE-004 (Ring buffer)

### Week 3-4: STFT and Basic Filters
5. CORE-005 (STFT)
6. FILT-001 (Butterworth)
7. FILT-005 (lfilter)
8. FILT-006 (filtfilt)
9. FILT-007 (Streaming filter)

### Week 5-6: Advanced Filters
10. FILT-002 (Chebyshev I)
11. FILT-003 (Chebyshev II)
12. FILT-004 (Elliptic)
13. FILT-008 (Biquad cascade)
14. FILT-009 (FIR windowed-sinc)

### Week 7-8: Analysis Functions
15. ANAL-001 (Correlation)
16. ANAL-002 (PSD)
17. ANAL-003 (Hilbert)
18. ANAL-004 (Phase unwrap)
19. ANAL-005 (Resample)
20. ANAL-007 (Frequency response)

### Week 9: Onset Detection
21. CORE-006 (Onset detection)

### Week 10: ABI Completion
22. ABI-001 (FFT ABI)
23. ABI-002 (Filter ABI)
24. ABI-003 (Analysis ABI)
25. ABI-004 (Error handling)

### Week 11-12: Unity Integration
26. UNITY-001 (Windows P/Invoke)
27. UNITY-002 (macOS P/Invoke)
28. UNITY-003 (FFTAnalyzer)
29. UNITY-004 (StreamingIIRFilter)
30. UNITY-005 (OnsetDetector)
31. UNITY-006 (Signal Inspector)
32. UNITY-007 (Filter Designer)

### Week 13: Documentation and CI
33. DOC-001 (Core API docs)
34. DOC-002 (Filter API docs)
35. DOC-003 (Unity guide)
36. CI-001 (Fix CI)
37. CI-002 (MATLAB comparison)

### Week 14: Advanced FIR
38. FILT-010 (Parks-McClellan)
39. ANAL-006 (Decimation/Interpolation)
