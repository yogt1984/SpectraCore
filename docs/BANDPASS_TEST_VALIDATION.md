# Bandpass/Bandstop Filter Testing & MATLAB Validation

## Test Coverage Summary

### Test Categories

1. **Filter Design Tests**
   - All 4 filter types (Butterworth, Chebyshev I/II, Elliptic)
   - Both bandpass and bandstop
   - Correct coefficient array sizes

2. **Frequency Response Tests**
   - Bandpass attenuates DC and Nyquist ✓
   - Bandpass passes center frequency ✓
   - Bandstop passes DC and Nyquist ✓
   - Bandstop attenuates center frequency ✓
   - Complementary property (BP + BS ≈ 1) ✓

3. **Filter Application Tests**
   - LFilter (single-pass) ✓
   - FiltFilt (zero-phase) ✓
   - Multi-frequency signal separation ✓

4. **Edge Cases**
   - Invalid frequency ranges ✓
   - Out-of-range frequencies ✓
   - Order doubling behavior ✓

5. **Comparison Tests**
   - Cascaded HP+LP vs. bandpass ✓
   - Parallel LP+HP vs. bandstop ✓

## MATLAB Reference Implementation

### Test Parameters

For validation against MATLAB, use these standard test cases:

#### Test Case 1: Butterworth Bandpass Order 2
```matlab
% MATLAB
[b, a] = butter(2, [0.2, 0.4], 'bandpass');
% Expected:
% - b length: 5 (order 4)
% - a length: 5
% - DC gain: < 0.1
% - Nyquist gain: < 0.1
% - Center frequency (0.3) gain: > 0.5
```

**C# Equivalent:**
```csharp
var (b, a) = DSP.Butter(2, 0.2f, 0.4f, FilterType.Bandpass);
Assert.AreEqual(5, b.Length);
Assert.AreEqual(5, a.Length);
```

#### Test Case 2: Butterworth Bandstop Order 2
```matlab
% MATLAB
[b, a] = butter(2, [0.2, 0.4], 'bandstop');
% Expected:
% - DC gain: > 0.7
% - Nyquist gain: > 0.7
% - Center frequency (0.3) gain: < 0.3
```

#### Test Case 3: Chebyshev Type I Bandpass Order 2
```matlab
% MATLAB
[b, a] = cheby1(2, 0.5, [0.2, 0.5], 'bandpass');
% Expected:
% - Filter order: 4
% - Passband ripple: 0.5 dB
% - Steeper rolloff than Butterworth
```

#### Test Case 4: Chebyshev Type II Bandpass Order 3
```matlab
% MATLAB
[b, a] = cheby2(3, 40, [0.2, 0.4], 'bandpass');
% Expected:
% - Filter order: 6
% - Stopband attenuation: 40 dB
% - Monotonic passband
```

#### Test Case 5: Elliptic Bandpass Order 2
```matlab
% MATLAB
[b, a] = ellip(2, 0.5, 40, [0.2, 0.5], 'bandpass');
% Expected:
% - Filter order: 4
% - Steepest rolloff of all types
% - Ripple in both bands
```

## Validation Approach

### 1. Coefficient Comparison
Generate filter coefficients and compare against MATLAB reference:
- Use same (order, ripple, frequencies) parameters
- Check coefficient values match within 1% tolerance
- Verify array lengths are correct

### 2. Frequency Response Comparison
```matlab
% MATLAB
[H, w] = freqz(b, a);
mag = abs(H);

% Generate frequency response at 512 points
plot(w/pi, 20*log10(mag));
grid on;
```

Compare magnitude response:
- At DC (w=0): expected attenuation for bandpass, pass for bandstop
- At Nyquist (w=pi): expected attenuation for bandpass, pass for bandstop
- At center frequency: expected pass for bandpass, attenuation for bandstop

### 3. Time-Domain Verification
```matlab
% MATLAB - Multi-frequency test signal
fs = 1; % Normalized sample rate
t = 0:1/512:1-1/512;
x = sin(2*pi*0.1*t) + sin(2*pi*0.3*t) + sin(2*pi*0.8*t);

% Apply bandpass (0.2-0.4)
[b, a] = butter(2, [0.2, 0.4], 'bandpass');
y = filter(b, a, x);

% y should be dominated by 0.3 component
% 0.1 and 0.8 should be attenuated
```

### 4. Complementary Property Test
```matlab
% MATLAB
[b_bp, a_bp] = butter(2, [0.2, 0.4], 'bandpass');
[b_bs, a_bs] = butter(2, [0.2, 0.4], 'bandstop');

[H_bp, w] = freqz(b_bp, a_bp, 512);
[H_bs, ~] = freqz(b_bs, a_bs, 512);

mag_bp = abs(H_bp);
mag_bs = abs(H_bs);

% At each frequency: mag_bp + mag_bs ≈ 1
sum_mag = mag_bp + mag_bs;
assert(all(abs(sum_mag - 1) < 0.15), 'Complementary property failed');
```

## Test Execution Instructions

### In Unity Editor
1. Window → General → Test Runner
2. Select "SpectraComponentTests"
3. Filter for "Bandpass" or "Bandstop"
4. Run selected tests

### Command Line (if available)
```bash
Unity -runTests -testCategory "Bandpass" -batchmode
```

### Manual MATLAB Validation
1. Run corresponding MATLAB code from Test Case sections above
2. Compare:
   - Filter coefficients (b, a arrays)
   - Frequency response at key points
   - Time-domain filtering results

## Expected Test Results

### All Tests Should Pass ✓
- 22 bandpass/bandstop design tests
- 4 frequency response characteristic tests
- 3 complementarity tests
- 2 comparison tests
- 2 filter application tests
- 5 edge case tests
- 1 order doubling test
- 2 comprehensive filter type tests

### Total: 41 tests for bandpass/bandstop functionality

## Known Tolerances

Due to numerical precision differences between C++ and MATLAB:
- Coefficient values: ±0.1% tolerance
- Magnitude response: ±0.15 (linear scale)
- DC/Nyquist gain: ±0.1
- Center frequency gain: ±0.15

## MATLAB Script for Batch Validation

Save as `validate_bandpass.m`:

```matlab
% Validate SpectraCore bandpass/bandstop implementation
clear; clc;

filters = {
    {'butter', [2, 0.2, 0.4]},
    {'cheby1', [2, 0.5, 0.2, 0.5]},
    {'cheby2', [2, 40, 0.2, 0.4]},
    {'ellip', [2, 0.5, 40, 0.2, 0.5]}
};

types = {'bandpass', 'bandstop'};

for f = 1:length(filters)
    filter_type = filters{f}{1};
    params = filters{f}{2};

    for t = 1:length(types)
        type = types{t};

        % Design filter
        if strcmp(filter_type, 'butter')
            [b, a] = butter(params{1}, [params{2}, params{3}], type);
        elseif strcmp(filter_type, 'cheby1')
            [b, a] = cheby1(params{1}, params{2}, [params{3}, params{4}], type);
        elseif strcmp(filter_type, 'cheby2')
            [b, a] = cheby2(params{1}, params{2}, [params{3}, params{4}], type);
        elseif strcmp(filter_type, 'ellip')
            [b, a] = ellip(params{1}, params{2}, params{3}, [params{4}, params{5}], type);
        end

        % Get frequency response
        [H, w] = freqz(b, a, 512);
        mag = abs(H);

        % Check properties
        dc_gain = mag(1);
        nyq_gain = mag(end);
        center_gain = mag(round(length(mag)/2));

        fprintf('%s %s: |H(DC)|=%f, |H(Nyq)|=%f, |H(center)|=%f\n', ...
            filter_type, type, dc_gain, nyq_gain, center_gain);

        if strcmp(type, 'bandpass')
            assert(dc_gain < 0.1, sprintf('%s bandpass DC not attenuated', filter_type));
            assert(nyq_gain < 0.1, sprintf('%s bandpass Nyquist not attenuated', filter_type));
            assert(center_gain > 0.5, sprintf('%s bandpass center not passing', filter_type));
        else % bandstop
            assert(dc_gain > 0.7, sprintf('%s bandstop DC not passing', filter_type));
            assert(nyq_gain > 0.7, sprintf('%s bandstop Nyquist not passing', filter_type));
            assert(center_gain < 0.3, sprintf('%s bandstop center not attenuated', filter_type));
        end
    end
end

fprintf('\nAll validations passed!\n');
```

## Next Steps

1. ✅ Implement comprehensive C# unit tests (DONE)
2. ⏳ Run Unity tests and verify all pass
3. ⏳ Compare with MATLAB reference implementation
4. ⏳ Document any discrepancies and tolerances
5. ⏳ Proceed to Phase 5 (Filter Designer UI + Documentation)

