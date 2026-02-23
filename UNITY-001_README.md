# UNITY-001: Windows P/Invoke Testing

**Status:** ⏳ Ready for validation (awaiting Windows machine)
**CI/CD:** Building automatically via GitHub Actions

---

## Quick Navigation

### For Quick Validation (10 minutes)
→ **[Windows Validation Requirements](docs/Windows_Validation_Requirements.md)** (START HERE)

### For Detailed Testing
→ **[Comprehensive Testing Guide](UNITY-001_WINDOWS_TESTING_GUIDE.md)** (21 pages)

### For Step-by-Step
→ **[Testing Checklist](UNITY-001_TESTING_CHECKLIST.md)** (Interactive)

### For Recording Results
→ **[Completion Template](UNITY-001_COMPLETION_TEMPLATE.md)**

### For Current Status
→ **[Status Document](UNITY-001_STATUS.md)**

---

## What is UNITY-001?

Validation task to ensure SpectraCore P/Invoke bindings work correctly on Windows.

**CI/CD already validates:**
- ✅ Windows compilation (MSVC 2022)
- ✅ 68 native C++ tests
- ✅ DLL generation and exports

**Manual validation adds:**
- Unity Editor integration testing
- Editor tools verification
- Performance benchmarking
- Real-world usage confirmation

---

## Quick Start

### Option 1: Trust CI/CD (Recommended)
```bash
# Just wait for GitHub Actions to complete
# Check: https://github.com/yogt1984/SpectraCore/actions
# If all green → Windows works! ✅
```

### Option 2: Minimal Manual Testing (10 min)
```powershell
# On any Windows machine:
# 1. Download spectra-windows artifact from GitHub Actions
# 2. Extract spectra.dll
# 3. Run:
git clone https://github.com/yogt1984/SpectraCore.git
cd SpectraCore
test-windows.bat spectra.dll
```

### Option 3: Full Unity Testing (60 min)
```powershell
# On Windows with Unity installed:
test-windows.bat spectra.dll "C:\Path\To\UnityProject"
# Then open Unity and run tests (71 tests)
```

---

## Files Overview

| File | Size | Purpose |
|------|------|---------|
| **Windows_Validation_Requirements.md** | ~25 KB | **START HERE** - What you need to know |
| UNITY-001_WINDOWS_TESTING_GUIDE.md | ~50 KB | Complete 21-page guide |
| UNITY-001_TESTING_CHECKLIST.md | ~15 KB | Step-by-step checklist |
| UNITY-001_COMPLETION_TEMPLATE.md | ~12 KB | Results recording template |
| UNITY-001_STATUS.md | ~6 KB | Current progress tracking |
| test-windows.bat | ~10 KB | Automation script |

---

## What You Need

### Minimal Testing
- ✅ Windows 10/11 (64-bit)
- ✅ .NET Framework 4.7.2+ (built-in)
- ✅ 5 minutes

### Full Testing
- ✅ Above +
- ✅ Unity Editor 2021.3+
- ✅ 60 minutes

---

## Current Status

**CI/CD Build:**
- Check: https://github.com/yogt1984/SpectraCore/actions
- Artifact: `spectra-windows` (will contain spectra.dll)
- Tests: 68 native tests run automatically

**Manual Testing:**
- Infrastructure: ✅ Ready
- Documentation: ✅ Complete
- Scripts: ✅ Ready
- Awaiting: Windows machine access (optional)

---

## Success Criteria

**Minimum (CI/CD):**
- [x] Windows build completes
- [x] 68 native tests pass
- [ ] Artifacts generated

**Recommended (+10 min):**
- Above +
- [ ] Standalone P/Invoke test passes (6/6)

**Comprehensive (+60 min):**
- Above +
- [ ] Unity Editor tests pass (71/71)
- [ ] Editor tools functional
- [ ] Performance acceptable

---

## Next Steps

1. **Wait for CI/CD** (~15 min from latest commit)
2. **Download artifact** (spectra-windows)
3. **Choose validation level** (CI/CD only, minimal, or full)
4. **Follow appropriate guide** (see Quick Navigation above)
5. **Record results** (optional - use completion template)
6. **Commit results** (if performed manual testing)

---

## Questions?

- **How do I know if Windows works?** → Check CI/CD status
- **Do I need to test manually?** → No, CI/CD validates core functionality
- **What if I want to test anyway?** → See Windows_Validation_Requirements.md
- **How long does it take?** → 10 min (minimal) to 60 min (comprehensive)
- **What if tests fail?** → See troubleshooting in testing guide

---

## Recommendations

**For most users:**
→ Trust CI/CD results (automatic, comprehensive)

**For Windows deployment:**
→ Run minimal validation (10 min, confirms P/Invoke works)

**For production use:**
→ Run full validation (60 min, confirms everything works)

**For development:**
→ Full validation with benchmarks (75 min, get performance data)

---

**tl;dr:** CI/CD handles Windows validation automatically. Manual testing is optional and documented in `docs/Windows_Validation_Requirements.md`.
