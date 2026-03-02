# SpectraCore Release Guide

Complete instructions for creating v1.2.0 and v1.3.0 releases.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Release v1.2.0](#release-v120)
3. [Release v1.3.0](#release-v130)
4. [Post-Release Tasks](#post-release-tasks)

---

## Prerequisites

### Required Tools

- Git (configured with your GitHub account)
- CMake 3.20+
- C++ compiler (GCC/Clang/MSVC)
- Unity 2021.3+ (optional, for Unity package)
- GitHub CLI `gh` (optional, but recommended)

### Verify Repository State

```bash
cd /home/onat/SpectraCore

# Check current branch
git branch

# Ensure you're on main
git checkout main

# Pull latest changes
git pull origin main

# Check status (should be clean)
git status
```

---

## Release v1.2.0

### Step 1: Verify v1.2.0 Code

Check that all v1.2.0 features are present:

```bash
# Verify Bessel filter implementation exists
ls -la native/src/filters/filter_design.cpp | grep -q "." && echo "✅ Filter design exists"

# Verify Bessel tests exist
ls -la native/tests/unit/test_bessel.cpp | grep -q "." && echo "✅ Bessel tests exist"

# Verify WebGL support
ls -la native/cmake/wasm.toolchain.cmake | grep -q "." && echo "✅ WebGL support exists"

# Verify mobile support
ls -la native/cmake/platforms/ | grep -q "." && echo "✅ Mobile platform files exist"
```

### Step 2: Build and Test v1.2.0

```bash
# Build native library
cd native/build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run all tests
ctest --output-on-failure

# Verify test count (should be 110+ tests passing)
ctest --output-on-failure | grep "tests passed"
```

Expected output: `100% tests passed, 0 tests failed out of 110+`

### Step 3: Update Version Numbers for v1.2.0

Check that version is set to 1.2.0 in all files:

```bash
# Check CMakeLists.txt
grep "VERSION 1.2.0" native/CMakeLists.txt

# Check package.json
grep '"version": "1.2.0"' unity/package.json

# Check README.md
grep "v1.2.0" README.md

# Check CHANGELOG.md
grep "\[1.2.0\]" CHANGELOG.md
```

If any version is wrong, update it manually.

### Step 4: Commit All v1.2.0 Changes

```bash
# Return to root directory
cd /home/onat/SpectraCore

# Check what's uncommitted
git status

# Add all changes
git add .

# Commit with descriptive message
git commit -m "Release v1.2.0: Bessel filters, WebGL support, mobile platforms

- Add complete Bessel (Thomson) filter implementation
- Add WebGL/WebAssembly support with Emscripten
- Add iOS and Android platform support
- Add comprehensive mobile testing infrastructure
- Update documentation and examples
- 110+ tests passing across all platforms"

# Push to main branch
git push origin main
```

### Step 5: Create v1.2.0 Git Tag

```bash
# Create annotated tag
git tag -a v1.2.0 -m "Release v1.2.0: Bessel Filters & Mobile Support

Major Features:
- Bessel (Thomson) filters with maximally flat group delay
- WebGL/WebAssembly support via Emscripten
- iOS and Android platform support
- Mobile testing infrastructure with on-device validation
- Cross-platform build system

Technical Details:
- 110+ unit tests passing
- 6 supported platforms (Linux, Windows, macOS, iOS, Android, WebGL)
- ~4,700 lines of new code
- Comprehensive documentation

See CHANGELOG.md for complete details."

# Verify tag was created
git tag -l -n9 v1.2.0

# Push tag to GitHub
git push origin v1.2.0
```

### Step 6: Build Release Artifacts for v1.2.0

```bash
# Create release directory
mkdir -p /tmp/spectracore-v1.2.0
cd /tmp/spectracore-v1.2.0

# Build Linux (x86_64)
mkdir linux-x64 && cd linux-x64
cmake /home/onat/SpectraCore/native -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cp lib/libspectra.so libspectra-v1.2.0-linux-x64.so
cd ..

# Build for other platforms (optional)
# Windows: Use Visual Studio or MinGW
# macOS: Use Xcode or clang
# WebGL: Use Emscripten (see docs/EmscriptenSetupGuide.md)

# Create source archive
cd /home/onat/SpectraCore
git archive --format=zip --prefix=SpectraCore-v1.2.0/ v1.2.0 -o /tmp/spectracore-v1.2.0/SpectraCore-v1.2.0-source.zip
git archive --format=tar.gz --prefix=SpectraCore-v1.2.0/ v1.2.0 -o /tmp/spectracore-v1.2.0/SpectraCore-v1.2.0-source.tar.gz

# Create Unity package
cd unity
zip -r /tmp/spectracore-v1.2.0/SpectraCore-v1.2.0-unity.zip . \
  -x "*.meta" "Temp/*" "Library/*" "Logs/*" "obj/*" "Builds/*"

# List artifacts
ls -lh /tmp/spectracore-v1.2.0/
```

### Step 7: Create GitHub Release for v1.2.0

**Option A: Using GitHub CLI (recommended)**

```bash
cd /home/onat/SpectraCore

# Create release with artifacts
gh release create v1.2.0 \
  --title "v1.2.0 - Bessel Filters & Mobile Support" \
  --notes-file <(cat <<'EOF'
# SpectraCore v1.2.0

**Release Date:** 2026-02-27

## 🎯 Highlights

- ⭐ **Bessel (Thomson) Filters** - Maximally flat group delay for optimal transient response
- 🌐 **WebGL Support** - Run DSP in the browser via WebAssembly
- 📱 **Mobile Platforms** - Full iOS and Android support with on-device testing
- 🧪 **110+ Tests** - Comprehensive test coverage across all platforms

## ✨ New Features

### Bessel Filters
- Complete implementation for all 4 filter types (Lowpass, Highpass, Bandpass, Bandstop)
- Pre-computed Bessel polynomial poles for orders 1-10
- Maximally flat group delay (linear phase response)
- Minimal overshoot and ringing (<1% in step response)
- Ideal for pulse signals, transients, and phase-critical applications

### Platform Support
- **WebGL:** Emscripten build system with optimized WASM output
- **iOS:** CMake toolchain with arm64/armv7 support
- **Android:** Multi-ABI builds (armeabi-v7a, arm64-v8a, x86, x86_64)
- **Mobile Testing:** On-device validation with 50+ automated tests

### Documentation
- `docs/EmscriptenSetupGuide.md` - WebGL build instructions
- `docs/CrossPlatformBuildGuide.md` - Platform-specific build guide
- `docs/MobilePlatformTesting.md` - Mobile deployment guide

## 📦 Downloads

- **Source Code:** SpectraCore-v1.2.0-source.zip / .tar.gz
- **Unity Package:** SpectraCore-v1.2.0-unity.zip
- **Linux Library:** libspectra-v1.2.0-linux-x64.so

## 🧪 Testing

- 110+ unit tests passing
- All platforms validated
- Mobile on-device testing infrastructure included

## 📊 Statistics

- **Lines Added:** ~4,700
- **Platforms Supported:** 6 (Linux, Windows, macOS, iOS, Android, WebGL)
- **Filter Types:** 4 IIR types × 4 filter modes = 16 configurations
- **Test Coverage:** Comprehensive (110+ tests)

## 🔧 Breaking Changes

None - fully backward compatible with v1.1.0

## 📚 Documentation

See [CHANGELOG.md](CHANGELOG.md) for complete details.

## 🙏 Acknowledgments

Thanks to all contributors and testers!

---

**Full Changelog:** https://github.com/yourusername/SpectraCore/compare/v1.1.0...v1.2.0
EOF
) \
  /tmp/spectracore-v1.2.0/SpectraCore-v1.2.0-source.zip \
  /tmp/spectracore-v1.2.0/SpectraCore-v1.2.0-source.tar.gz \
  /tmp/spectracore-v1.2.0/SpectraCore-v1.2.0-unity.zip \
  /tmp/spectracore-v1.2.0/libspectra-v1.2.0-linux-x64.so

echo "✅ v1.2.0 release created successfully!"
```

**Option B: Using GitHub Web Interface**

1. Go to: https://github.com/yourusername/SpectraCore/releases/new
2. Select tag: `v1.2.0`
3. Release title: `v1.2.0 - Bessel Filters & Mobile Support`
4. Copy release notes from the file above
5. Upload artifacts from `/tmp/spectracore-v1.2.0/`
6. Click "Publish release"

---

## Release v1.3.0

### Step 1: Verify v1.3.0 Code

Check that all v1.3.0 features are present:

```bash
cd /home/onat/SpectraCore

# Verify pitch detection implementation
ls -la native/src/analysis/pitch_detection.cpp | grep -q "." && echo "✅ Pitch detection exists"
ls -la native/include/spectra/analysis/pitch_detection.hpp | grep -q "." && echo "✅ Pitch detection header exists"

# Verify C# wrapper
ls -la unity/Runtime/Scripts/Analysis/PitchDetector.cs | grep -q "." && echo "✅ C# wrapper exists"

# Verify tests
ls -la native/tests/unit/test_pitch_detection.cpp | grep -q "." && echo "✅ Native tests exist"
grep -q "PitchDetector" unity/Tests/Editor/SpectraComponentTests.cs && echo "✅ Unity tests exist"

# Verify documentation
ls -la docs/PitchDetection.md | grep -q "." && echo "✅ Documentation exists"

# Verify example
ls -la unity/Samples~/PitchDetectionExample/ | grep -q "." && echo "✅ Example exists"
```

### Step 2: Build and Test v1.3.0

```bash
# Build native library
cd native/build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run ALL tests (including new pitch detection tests)
ctest --output-on-failure

# Run pitch detection tests specifically
./bin/test_pitch_detection --gtest_color=yes

# Verify test count (should be 142+ tests now: 110 from v1.2.0 + 32 new)
ctest --output-on-failure | grep "tests passed"
```

Expected output: `100% tests passed, 0 tests failed out of 142+`

### Step 3: Verify Version Numbers for v1.3.0

```bash
cd /home/onat/SpectraCore

# Check version in files
grep "v1.3.0" README.md
grep "\[1.3.0\]" CHANGELOG.md
```

If version needs updating:

```bash
# Update CMakeLists.txt
sed -i 's/VERSION 1.2.0/VERSION 1.3.0/' native/CMakeLists.txt

# Update package.json
sed -i 's/"version": "1.2.0"/"version": "1.3.0"/' unity/package.json

# Update spectra_abi.cpp version
sed -i 's/return "0.2.0"/return "1.3.0"/' native/src/abi/spectra_abi.cpp
```

### Step 4: Commit All v1.3.0 Changes

```bash
# Check status
git status

# Add all changes
git add .

# Commit
git commit -m "Release v1.3.0: Real-time pitch detection with YIN algorithm

- Add complete pitch detection implementation (YIN, Autocorrelation, HPS)
- Add musical note mapping with cents deviation
- Add Unity C# wrapper with high-level API
- Add 32 native unit tests (all passing)
- Add 23 Unity integration tests
- Add comprehensive documentation and examples
- Add SimplePitchTuner example with microphone input
- Performance: < 0.5ms @ 44.1 kHz, ±5 Hz accuracy

Total: 142+ tests passing across all platforms

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"

# Push to main
git push origin main
```

### Step 5: Create v1.3.0 Git Tag

```bash
# Create annotated tag
git tag -a v1.3.0 -m "Release v1.3.0: Real-Time Pitch Detection

Major Features:
- Real-time pitch detection using YIN algorithm
- Musical note mapping with cents deviation
- Multiple detection methods (YIN, Autocorrelation, HPS, Auto)
- Alternative tuning support (A4 = 432 Hz, 415 Hz, etc.)
- Unity example: SimplePitchTuner with microphone input

Performance:
- Processing time: < 0.5ms per frame @ 44.1 kHz
- Accuracy: ±5 Hz for pure tones
- Memory: ~50 KB per detector instance
- Zero allocations per frame

Testing:
- 32 new native tests (all passing)
- 23 Unity integration tests
- Total: 142+ tests passing

Use Cases:
- Musical instrument tuners
- Real-time pitch correction
- Music transcription
- Voice analysis
- Pitch-based game mechanics

See CHANGELOG.md for complete details."

# Verify tag
git tag -l -n15 v1.3.0

# Push tag
git push origin v1.3.0
```

### Step 6: Build Release Artifacts for v1.3.0

```bash
# Create release directory
mkdir -p /tmp/spectracore-v1.3.0
cd /tmp/spectracore-v1.3.0

# Build Linux (x86_64)
mkdir linux-x64 && cd linux-x64
cmake /home/onat/SpectraCore/native -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cp lib/libspectra.so libspectra-v1.3.0-linux-x64.so
cd ..

# Verify pitch detection is included in library
strings linux-x64/libspectra.so | grep -q "spectra_pitch_create" && echo "✅ Pitch detection included"

# Create source archives
cd /home/onat/SpectraCore
git archive --format=zip --prefix=SpectraCore-v1.3.0/ v1.3.0 -o /tmp/spectracore-v1.3.0/SpectraCore-v1.3.0-source.zip
git archive --format=tar.gz --prefix=SpectraCore-v1.3.0/ v1.3.0 -o /tmp/spectracore-v1.3.0/SpectraCore-v1.3.0-source.tar.gz

# Create Unity package
cd unity
zip -r /tmp/spectracore-v1.3.0/SpectraCore-v1.3.0-unity.zip . \
  -x "*.meta" "Temp/*" "Library/*" "Logs/*" "obj/*" "Builds/*"

# Create pitch detection example package
cd Samples~/PitchDetectionExample
zip -r /tmp/spectracore-v1.3.0/PitchDetectionExample-v1.3.0.zip . \
  -x "*.meta"

# List artifacts
ls -lh /tmp/spectracore-v1.3.0/
```

### Step 7: Create GitHub Release for v1.3.0

**Option A: Using GitHub CLI (recommended)**

```bash
cd /home/onat/SpectraCore

# Create release
gh release create v1.3.0 \
  --title "v1.3.0 - Real-Time Pitch Detection" \
  --notes-file <(cat <<'EOF'
# SpectraCore v1.3.0

**Release Date:** 2026-03-01

## 🎯 Highlights

- ⭐ **Real-Time Pitch Detection** - Industry-standard YIN algorithm with < 0.5ms processing
- 🎵 **Musical Note Mapping** - Automatic frequency-to-note conversion with cents
- 🎸 **Tuner-Grade Accuracy** - ±5 Hz precision for musical applications
- 🎤 **Multiple Methods** - YIN, Autocorrelation, HPS, and Auto selection

## ✨ New Features

### Pitch Detection
- **YIN Algorithm:** Most accurate method (de Cheveigné & Kawahara, 2002)
  - Sub-sample accuracy via parabolic interpolation
  - Confidence scoring for reliability
  - Voiced/unvoiced detection
- **Alternative Methods:**
  - Autocorrelation (faster, less accurate)
  - Harmonic Product Spectrum (HPS)
  - Auto mode (selects best method)
- **Performance:**
  - Processing: < 0.5ms @ 44.1 kHz
  - Accuracy: ±5 Hz for pure tones
  - Memory: ~50 KB per instance
  - Zero per-frame allocations

### Musical Note Features
- Automatic frequency-to-note conversion
- Octave detection (e.g., A4, C4)
- Cents deviation for tuning (±50 cents)
- Alternative tuning support:
  - Standard: A4 = 440 Hz
  - Alternative: A4 = 432 Hz
  - Baroque: A4 = 415 Hz
- MIDI note number conversion

### Unity C# API
```csharp
using (var detector = new PitchDetector(44100, 2048))
{
    var result = detector.Detect(audioBuffer);
    if (result.Voiced)
    {
        var note = PitchDetector.FrequencyToNote(result.Frequency);
        Debug.Log($"{note.Name}{note.Octave} ({note.Cents:+0;-0} cents)");
    }
}
```

### Examples & Documentation
- **SimplePitchTuner:** Complete microphone-based tuner
  - Visual tuning indicator (green/yellow/red)
  - Configurable detection parameters
  - Instrument presets (guitar, vocals, bass)
- **Comprehensive docs:** `docs/PitchDetection.md`
  - API reference and usage guide
  - Performance benchmarks
  - Best practices

## 📦 Downloads

- **Source Code:** SpectraCore-v1.3.0-source.zip / .tar.gz
- **Unity Package:** SpectraCore-v1.3.0-unity.zip
- **Linux Library:** libspectra-v1.3.0-linux-x64.so
- **Example:** PitchDetectionExample-v1.3.0.zip

## 🎮 Use Cases

- Musical instrument tuners (guitar, chromatic, vocal)
- Real-time pitch correction and auto-tune
- Music transcription and analysis
- Voice analysis and singing training
- Pitch-based game mechanics
- Melody detection

## 🧪 Testing

- **Native Tests:** 32 new tests (YIN, autocorrelation, note conversion)
- **Unity Tests:** 23 C# integration tests
- **Total:** 142+ tests passing (100% pass rate)

## 📊 Statistics

- **Lines Added:** ~1,880 (C++, C#, docs, tests, examples)
- **Performance:** < 0.5ms per frame
- **Accuracy:** ±5 Hz (pure tones), ±10 Hz (complex signals)
- **Platforms:** All 6 platforms from v1.2.0 supported

## 🔧 Breaking Changes

None - fully backward compatible with v1.2.0

## 🆕 API Additions

**C++:**
- `PitchDetector` class
- `PitchResult`, `MusicalNote` structs
- `frequencyToNote()`, `noteToFrequency()`, `frequencyToMIDI()`

**C#:**
- `PitchDetector` class (IDisposable)
- `PitchResult`, `MusicalNote` structs
- `PitchMethod` enum
- Static utility methods

## 📚 Documentation

- [Pitch Detection Guide](docs/PitchDetection.md)
- [Example README](unity/Samples~/PitchDetectionExample/README.md)
- [CHANGELOG](CHANGELOG.md)

## 🙏 References

- de Cheveigné, A., & Kawahara, H. (2002). "YIN, a fundamental frequency estimator for speech and music." *JASA*, 111(4), 1917-1930.

---

**Full Changelog:** https://github.com/yourusername/SpectraCore/compare/v1.2.0...v1.3.0
EOF
) \
  /tmp/spectracore-v1.3.0/SpectraCore-v1.3.0-source.zip \
  /tmp/spectracore-v1.3.0/SpectraCore-v1.3.0-source.tar.gz \
  /tmp/spectracore-v1.3.0/SpectraCore-v1.3.0-unity.zip \
  /tmp/spectracore-v1.3.0/libspectra-v1.3.0-linux-x64.so \
  /tmp/spectracore-v1.3.0/PitchDetectionExample-v1.3.0.zip

echo "✅ v1.3.0 release created successfully!"
```

**Option B: Using GitHub Web Interface**

1. Go to: https://github.com/yourusername/SpectraCore/releases/new
2. Select tag: `v1.3.0`
3. Release title: `v1.3.0 - Real-Time Pitch Detection`
4. Copy release notes from above
5. Upload artifacts from `/tmp/spectracore-v1.3.0/`
6. Click "Publish release"

---

## Post-Release Tasks

### 1. Verify Releases on GitHub

```bash
# List all releases
gh release list

# View v1.2.0 release
gh release view v1.2.0

# View v1.3.0 release
gh release view v1.3.0
```

Or visit:
- https://github.com/yourusername/SpectraCore/releases/tag/v1.2.0
- https://github.com/yourusername/SpectraCore/releases/tag/v1.3.0

### 2. Update Documentation Links

Ensure links in README.md point to correct releases:

```bash
# Check release links
grep -n "releases/tag" README.md

# Update if needed
sed -i 's|releases/tag/v1.0.0|releases/tag/v1.3.0|g' README.md
```

### 3. Announce Releases

**GitHub Discussions:**
- Post in Announcements about v1.2.0 features
- Post in Announcements about v1.3.0 features

**Social Media / Forums:**
- Share on Unity forums
- Share on relevant Discord servers
- Tweet/post about new features

### 4. Update Unity Asset Store (if applicable)

1. Upload Unity package from releases
2. Update description with new features
3. Update screenshots/videos if needed
4. Submit for review

### 5. Backup Release Artifacts

```bash
# Copy artifacts to safe location
mkdir -p ~/SpectraCore-Releases
cp -r /tmp/spectracore-v1.2.0 ~/SpectraCore-Releases/
cp -r /tmp/spectracore-v1.3.0 ~/SpectraCore-Releases/

# Create archive
cd ~/SpectraCore-Releases
tar -czf SpectraCore-Releases-v1.2.0-v1.3.0.tar.gz spectracore-v1.2.0 spectracore-v1.3.0

echo "✅ Backup created: ~/SpectraCore-Releases/SpectraCore-Releases-v1.2.0-v1.3.0.tar.gz"
```

### 6. Clean Up Temporary Files

```bash
# Remove temporary release directories
rm -rf /tmp/spectracore-v1.2.0
rm -rf /tmp/spectracore-v1.3.0

echo "✅ Cleanup complete"
```

---

## Quick Reference: Release Checklist

### For Each Release:

- [ ] Verify all features are implemented
- [ ] Build and run all tests (100% pass rate)
- [ ] Update version numbers in all files
- [ ] Update CHANGELOG.md
- [ ] Update README.md
- [ ] Commit all changes
- [ ] Create and push git tag
- [ ] Build release artifacts (Linux, Windows, macOS, etc.)
- [ ] Create source archives
- [ ] Create Unity package
- [ ] Create GitHub release with notes and artifacts
- [ ] Verify release on GitHub
- [ ] Update documentation links
- [ ] Announce release
- [ ] Backup artifacts

---

## Troubleshooting

### GitHub CLI Not Installed

```bash
# Install on Ubuntu/Debian
sudo apt install gh

# Install on macOS
brew install gh

# Login
gh auth login
```

### Permission Denied When Pushing Tags

```bash
# Check remote URL
git remote -v

# If using HTTPS, switch to SSH
git remote set-url origin git@github.com:yourusername/SpectraCore.git

# Or configure credential helper
git config --global credential.helper store
```

### Build Failures

```bash
# Clean build directory
cd native/build
rm -rf *
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Test Failures

```bash
# Run specific test
ctest -R test_name --output-on-failure

# Run with verbose output
ctest -V

# Run single test binary directly
./bin/test_pitch_detection --gtest_filter=*DetectA440*
```

---

## Notes

- **Both releases are independent** - users can download either v1.2.0 or v1.3.0
- **v1.3.0 includes all v1.2.0 features** - it's a superset
- **Tags are permanent** - don't delete or modify tags after pushing
- **Releases can be edited** - you can update release notes after creation
- **Draft releases** - use `--draft` flag to create draft releases for review

---

## Success Criteria

✅ Both tags (v1.2.0, v1.3.0) pushed to GitHub
✅ Both releases created with artifacts
✅ All tests passing (110+ for v1.2.0, 142+ for v1.3.0)
✅ Source code archives downloadable
✅ Unity packages ready for import
✅ Documentation complete and accessible
✅ CHANGELOG.md updated
✅ README.md reflects latest version

---

**Questions?** See individual documentation files or open an issue on GitHub.
