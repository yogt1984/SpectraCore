# Filter Designer Window - Phase 5A Updates

## Overview

The Filter Designer Window has been enhanced to support **bandpass and bandstop filter design** with real-time visualization and interactive dual-frequency controls.

## New Features

### 1. **Dual-Frequency Controls**

For bandpass and bandstop filters, the UI now displays:
- **Low Frequency Slider**: Adjustable from 0.01 to 0.98 (normalized)
- **High Frequency Slider**: Adjustable from low+0.01 to 0.99 (normalized)
- **Center Frequency Display**: Automatically calculated: (low + high) / 2
- **Bandwidth Display**: Automatically calculated: high - low

#### UI Layout
```
Response Type: [Dropdown showing Bandpass or Bandstop]

Frequency Range:
  Low Frequency:     [----●---] 0.200
  High Frequency:    [-------●-] 0.400
  Center Frequency:  0.300
  Bandwidth:         0.200
```

### 2. **Bode Plot Enhancement for Bandpass/Bandstop**

#### Visual Indicators
- **Bandpass**: Green shaded region showing the passband
  - Yellow vertical lines at low and high frequency edges
  - Demonstrates where signal passes through

- **Bandstop**: Red shaded region showing the stopband
  - Yellow vertical lines at low and high frequency edges
  - Demonstrates where signal is attenuated

#### Before (Single-frequency only)
```
|H(jω)|
   |        ___
   |     /       \
   |  /             \___
   +--●--+--+--+--+--+--
   0  f_c  0.25 0.5 0.75 1.0 (normalized frequency)
```

#### After (Dual-frequency support)
```
|H(jω)|
   |
   |                ▓▓▓▓▓▓▓▓  ← Passband (green)
   |     \\\\\\\\|▓▓▓▓▓▓▓▓|\\\\\\\\
   |        |       |
   +--+--●--+--+--●--+--+--
   0  0.2  0.3  0.4 0.75 1.0
       ↑ low      ↑ high
```

### 3. **Smart Parameter Display**

The parameter panel automatically adjusts based on filter response type:

**For Lowpass/Highpass:**
- Shows single "Normalized Cutoff" slider
- Displays DC/Nyquist frequency labels

**For Bandpass/Bandstop:**
- Shows "Low Frequency" and "High Frequency" sliders
- Shows calculated center frequency
- Shows calculated bandwidth
- Prevents invalid frequency ranges (low < high)

### 4. **Enhanced Export Functions**

#### Copy Coefficients
Now includes frequency range information:
```csharp
// Filter Coefficients
// Type: Butterworth Bandpass
// Order: 4
// Frequency Range: 0.200 - 0.400
//
float[] b = new float[] { 0.015625, 0, -0.046875, ... };
float[] a = new float[] { 1, -3.456789, ... };
```

#### Export to C#
Generates class name with response type:
```csharp
/// <summary>
/// Butterworth Bandpass filter
/// Order: 4
/// Frequency Range: 0.200 - 0.400
/// </summary>
public class ButterworthBandpassFilter
{
    public static readonly float[] B = new float[] { ... };
    public static readonly float[] A = new float[] { ... };
}
```

## Implementation Details

### Code Changes

**File**: `unity/Editor/Scripts/FilterDesignerWindow.cs`

**Key Modifications:**

1. **New Fields**
   ```csharp
   private float lowFreq = 0.2f;      // For bandpass/bandstop
   private float highFreq = 0.4f;     // For bandpass/bandstop
   ```

2. **Conditional Parameter Display**
   - Checks `responseType == FilterType.Bandpass || FilterType.Bandstop`
   - Shows appropriate UI controls

3. **Updated UpdateFilter() Method**
   - Uses dual-frequency overloads for bandpass/bandstop:
     - `DSP.Butter(order, lowFreq, highFreq, FilterType.Bandpass)`
     - `DSP.Cheby1(order, rippleDb, lowFreq, highFreq, FilterType.Bandpass)`
     - `DSP.Cheby2(order, stopbandDb, lowFreq, highFreq, FilterType.Bandpass)`
     - `DSP.Ellip(order, rippleDb, stopbandDb, lowFreq, highFreq, FilterType.Bandpass)`
   - Falls back to single-frequency for lowpass/highpass

4. **Enhanced Bode Plot Visualization**
   - `DrawMagnitudePlot()`: Draws shaded passband/stopband region
   - `DrawPhasePlot()`: Shows frequency markers at both edges
   - Conditional logic for bandpass vs. single-frequency display

5. **Improved Export Functions**
   - Frequency range included in comments
   - Class names updated with response type
   - Better documentation in generated files

## Usage Guide

### Design a Bandpass Filter

1. **Open Filter Designer**
   - Menu: `Window → Spectra → Filter Designer`

2. **Configure Filter**
   ```
   Filter Type:      [Butterworth ▼]
   Response Type:    [Bandpass ▼]
   Order:            [4 ▬●▬▬▬]

   Frequency Range:
     Low Frequency:  [▬●▬▬▬] 0.200
     High Frequency: [▬▬▬●▬] 0.400
   ```

3. **Observe Bode Plot**
   - Green shaded region shows passband (0.2 - 0.4)
   - Yellow lines at edges show exact frequencies
   - Magnitude plot shows ~0.5 gain in passband

4. **Export Filter**
   - Click "Copy Coefficients" to copy to clipboard
   - Click "Export to C#" to save as class file

### Design a Bandstop Filter

Same process, but:
1. Select `Response Type: [Bandstop ▼]`
2. Red shaded region shows stopband
3. Signal passes outside the band (0.0-0.2 and 0.4-1.0)

## Behavior Specifications

### Frequency Range Validation
- Low frequency: Must be > 0.01
- High frequency: Must be < 0.99
- High - Low: Minimum gap of 0.01
- Invalid ranges automatically constrained by UI sliders

### Filter Order Doubling
- Order 1 bandpass → 3 coefficients
- Order 2 bandpass → 5 coefficients
- Order 3 bandpass → 7 coefficients
- Order 4 bandpass → 9 coefficients
(Correctly shown in coefficient display)

### Response Type Switching
- Switching from Bandpass to Lowpass: Retains current lowFreq as cutoffFreq
- Switching from Lowpass to Bandpass: Sets lowFreq=0.2, highFreq=0.4 (defaults)
- No loss of other parameters (order, ripple, etc.)

## Visual Design

### Color Coding

| Element | Color | Meaning |
|---------|-------|---------|
| Passband (Bandpass) | Green | Signal passes |
| Stopband (Bandstop) | Red | Signal attenuated |
| Frequency Markers | Yellow | Exact cutoff/edge frequencies |
| Magnitude Line | Green | Filter response |
| Phase Line | Blue | Filter phase response |
| Grid | Dark Gray | Reference lines |
| Background | Very Dark Gray | Plot area |

### Layout

```
┌─────────────────────────────────────────┐
│ Filter Parameters                       │
│ ─────────────────────────────────────── │
│                                         │
│ Filter Type:    [Butterworth ▼]        │
│ Response Type:  [Bandpass ▼]           │
│ Order:          [4]                     │
│                                         │
│ Frequency Range                         │
│   Low Frequency:  [0.20] ◄─────────►   │
│   High Frequency: [0.40] ◄─────────►   │
│   Center Freq: 0.30                     │
│   Bandwidth: 0.20                       │
│                                         │
│ ☑ Auto Update                           │
│                                         │
│ [Copy Coefficients] [Export to C#]      │
│                                         │
├─────────────────────────────────────────┤
│ Bode Plot                               │
│ ─────────────────────────────────────── │
│                                         │
│ ☑ Magnitude  ☑ Phase                    │
│                                         │
│ Magnitude Response (dB)                 │
│ ┌───────────────────────────────────┐  │
│ │    0 dB        ▂▂▂▂▂             │  │
│ │           ▂▂▂▂▌ G ▌▂▂▂▂           │  │
│ │      ▂▂▂▌        ▌▂▂▂              │  │
│ │     ▂▌ ↑          ↑ ▌▂             │  │
│ │  -20│  0.2       0.4  │             │  │
│ │  -40│              │             │  │
│ │  -60└───────────────────────────────┘  │
│ │     0  0.25  0.5 0.75 1.0 (norm freq) │
│                                         │
│ Phase Response (degrees)                │
│ ┌───────────────────────────────────┐  │
│ │ 0 deg     ╱─────────╲             │  │
│ │        ╱               ╲          │  │
│ │ -45 ╱   ↑           ↑   ╲         │  │
│ │     0.2           0.4               │  │
│ │-90└───────────────────────────────┘   │
│                                         │
├─────────────────────────────────────────┤
│ Filter Coefficients   ◀──────────────◀  │
│                                         │
│ Numerator (b) - 5 coefficients:         │
│ [0.015625, 0, -0.046875, 0, 0.015625] │
│                                         │
│ Denominator (a) - 5 coefficients:       │
│ [1, -3.456789, 4.234567, -2.1, 0.5]   │
└─────────────────────────────────────────┘
```

## Testing the Updates

### Manual Test Cases

**Test 1: Butterworth Bandpass**
1. Set: Filter=Butterworth, Response=Bandpass, Order=2
2. Set: Low=0.2, High=0.4
3. Observe: Green shaded region, passband gain > 0.5
4. Expected: Magnitude plot shows attenuation at 0.0 and 1.0, peak at 0.3

**Test 2: Chebyshev I Bandpass**
1. Set: Filter=ChebyshevI, Ripple=0.5, Low=0.2, High=0.5
2. Observe: Steeper rolloff than Butterworth
3. Expected: Faster transition to stopband

**Test 3: Elliptic Bandstop**
1. Set: Filter=Elliptic, Response=Bandstop, Low=0.25, High=0.45
2. Observe: Red shaded region, notch at center
3. Expected: Deep attenuation in center, passes at edges

**Test 4: UI Responsiveness**
1. Drag Low Frequency slider to 0.3
2. Observe: High Frequency auto-constrained to stay > 0.31
3. Observe: Center Frequency and Bandwidth update in real-time

**Test 5: Export with Bandpass**
1. Design Butterworth Bandpass Order 4, Low=0.2, High=0.4
2. Click "Copy Coefficients"
3. Expected: Comments include "Frequency Range: 0.200 - 0.400"
4. Click "Export to C#"
5. Expected: Class name is "ButterworthBandpassFilter"

## Files Modified

- `unity/Editor/Scripts/FilterDesignerWindow.cs` (+167 lines, -32 lines)

## Compatibility

- ✅ All existing lowpass/highpass functionality preserved
- ✅ No breaking changes to API
- ✅ Automatic fallback to single-frequency for LP/HP
- ✅ Works with all 4 filter types (Butterworth, Cheby1, Cheby2, Elliptic)

## Known Limitations

- Bandpass/Bandstop not available in real-time Bode plot for invalid frequency ranges
- Phase plot scaling may compress for very wide or very narrow bandpasses
- Export filename uses response type (e.g., "ButterworthBandpassFilter"), not custom names

## Future Enhancements

- [ ] Frequency response comparison mode (BP vs BS)
- [ ] Zoom/Pan on Bode plot for detailed inspection
- [ ] Preset filter configurations (audio, RF, etc.)
- [ ] Import filter specifications from CSV
- [ ] 3D surface plot of frequency response vs. order

---

**Status**: Phase 5A Complete - Filter Designer Window updated with bandpass/bandstop support
**Date**: 2026-02-25
**Lines Changed**: +167, -32 in FilterDesignerWindow.cs
