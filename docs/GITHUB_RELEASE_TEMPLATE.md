# SpectraCore v1.1.0 - Bandpass & Bandstop Filters

## 🎉 Release Summary

SpectraCore v1.1.0 brings **complete bandpass and bandstop filter support** for all IIR filter types (Butterworth, Chebyshev I/II, Elliptic), an enhanced Filter Designer window with dual-frequency controls, an enhanced FilterPlayground sample with 7+ practical examples, and comprehensive documentation with 41 new tests.

**Release Date:** February 25, 2026
**Status:** Production Ready ✅

---

## ✨ What's New

### Bandpass & Bandstop Filters ⭐ NEW

Design bandpass and bandstop filters for all 4 IIR types:

```csharp
// Speech enhancement (60 Hz - 3500 Hz)
var (b, a) = DSP.Butter(4, 0.003f, 0.159f, FilterType.Bandpass);

// 60 Hz hum removal (narrow notch)
var (b_hum, a_hum) = DSP.Butter(6, 0.0025f, 0.003f, FilterType.Bandstop);

// Chebyshev with steep rolloff
var (b_sharp, a_sharp) = DSP.Cheby1(4, 0.5f, 0.004f, 0.227f, FilterType.Bandpass);
```

**Key Features:**
- Order doubles for bandpass/bandstop: order N → 2N coefficients
- All frequency domains normalized (0-1, where 0.5 = Nyquist)
- Complementary property: Bandpass + Bandstop ≈ Unity
- Real-time capable with StreamingIIRFilter

### Enhanced Filter Designer Window 🎛️

Improved editor tool with dual-frequency support

### Enhanced FilterPlayground Sample 📦

Complete bandpass/bandstop demonstration with 7+ practical examples

### Comprehensive Documentation 📚

- New Tutorial 6: Bandpass & Bandstop Filters (25 min)
- API Reference expansion with 576+ new lines
- 4 new scripts for FilterPlayground sample
- Enhanced CHANGELOG documentation

---

## 📊 By The Numbers

- **Implementation:** ~1,200 lines of C++ code
- **Testing:** 41 new tests, 100% pass rate (144+ total)
- **Documentation:** 576+ lines added (13,500+ total)
- **Sample Code:** 4 new scripts, 7+ practical examples
- **Platforms:** Linux, Windows, macOS
- **Backward Compatibility:** 100% - No breaking changes

---

## ✅ Test Results

✅ All 68 native C++ tests passing
✅ All 76 Unity C# tests passing  
✅ 144+ total tests (100% pass rate)
✅ Cross-platform validation complete
✅ MATLAB equivalence verified
✅ Real-time performance verified

---

## 🚀 Getting Started

### Install v1.1.0

```
Window → Package Manager → + → Add package from git URL
https://github.com/yogt1984/SpectraCore.git#v1.1.0
```

### Try the Examples

Open FilterPlayground sample to explore:
1. Speech enhancement (60 Hz - 3.5 kHz)
2. Hum removal (60 Hz notch)
3. Acoustic analysis (80 Hz - 5 kHz)
4. Music frequency bands

### Learn More

- [Tutorial 6: Bandpass & Bandstop Filters](docs/Tutorials.md#tutorial-6-bandpass--bandstop-filters)
- [API Reference](docs/API_Reference.md#bandpass--bandstop-filter-design)
- [FilterPlayground Guide](Samples~/FilterPlayground/IMPLEMENTATION_GUIDE.md)

---

## 🔄 API Changes

### New Dual-Frequency Overloads

```csharp
// All filter functions now support bandpass/bandstop
var (b, a) = DSP.Butter(order, lowFreq, highFreq, FilterType.Bandpass);
var (b, a) = DSP.Cheby1(order, rippleDb, lowFreq, highFreq, FilterType.Bandpass);
var (b, a) = DSP.Cheby2(order, stopbandDb, lowFreq, highFreq, FilterType.Bandpass);
var (b, a) = DSP.Ellip(order, passRipple, stopAtten, lowFreq, highFreq, FilterType.Bandpass);
```

**Backward Compatible:** All v1.0 code works unchanged.

---

## 📝 Full Release Notes

See [RELEASE_NOTES_v1.1.0.md](RELEASE_NOTES_v1.1.0.md) for comprehensive details.

---

## 📞 Support

- **Issues:** [GitHub Issues](https://github.com/yogt1984/SpectraCore/issues)
- **Discussions:** [GitHub Discussions](https://github.com/yogt1984/SpectraCore/discussions)
- **API Reference:** [docs/API_Reference.md](https://github.com/yogt1984/SpectraCore/blob/main/docs/API_Reference.md)

---

**Happy Filtering! 🎵🎧**
