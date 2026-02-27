# Emscripten Setup Guide for WebAssembly Builds

This guide explains how to install and configure Emscripten to build SpectraCore for WebAssembly/WebGL.

---

## 📋 Overview

**Emscripten** is a complete compiler toolchain that compiles C/C++ to WebAssembly (WASM), allowing native code to run in web browsers at near-native speed.

**What you'll get:**
- `libspectra.wasm` - WebAssembly binary (~200-300 KB compressed)
- `libspectra.js` - JavaScript glue code (~50 KB)
- Compatible with Unity WebGL builds

**Performance:** ~50-70% of native speed (sufficient for real-time DSP at 44.1 kHz)

---

## 🚀 Quick Start (TL;DR)

```bash
# Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# Build SpectraCore for WASM
cd /home/onat/SpectraCore/native
./scripts/build_wasm.sh
```

---

## 📦 Installing Emscripten

### Method 1: emsdk (Recommended)

**Step 1: Clone emsdk repository**
```bash
cd ~
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
```

**Step 2: Install latest version**
```bash
./emsdk install latest
```

This downloads:
- Emscripten compiler (emcc)
- LLVM backend
- Node.js (for testing)
- Binaryen (WASM optimizer)

Download size: ~300-400 MB
Time: 2-5 minutes

**Step 3: Activate**
```bash
./emsdk activate latest
```

**Step 4: Set up environment**
```bash
source ./emsdk_env.sh
```

**Step 5: Verify installation**
```bash
emcc --version
```

Expected output:
```
emcc (Emscripten gcc/clang-like replacement) 3.1.x (...)
```

### Method 2: System Package Manager (Alternative)

**Ubuntu/Debian:**
```bash
sudo apt-get install emscripten
```

**macOS:**
```bash
brew install emscripten
```

**Arch Linux:**
```bash
sudo pacman -S emscripten
```

⚠️ **Warning:** Package manager versions may be outdated. emsdk is recommended.

---

## 🔧 Configuration

### Making Emscripten Permanent

Add to `~/.bashrc` (Linux) or `~/.zshrc` (macOS):

```bash
# Emscripten SDK
source ~/emsdk/emsdk_env.sh > /dev/null 2>&1
```

Then reload:
```bash
source ~/.bashrc  # or ~/.zshrc
```

Verify:
```bash
emcc --version  # Should work without running source manually
```

### Directory Structure

After installation, emsdk directory contains:

```
~/emsdk/
├── emsdk              # Management script
├── emsdk_env.sh       # Environment setup
├── upstream/          # Emscripten toolchain
│   ├── emscripten/    # Compiler
│   ├── bin/           # emcc, em++, etc.
│   └── lib/           # System libraries
└── node/              # Node.js for testing
```

---

## 🏗️ Building SpectraCore for WebAssembly

### Basic Build (Recommended)

```bash
cd native
./scripts/build_wasm.sh
```

Output:
- `build-wasm/lib/libspectra.wasm` (~250 KB)
- `build-wasm/lib/libspectra.js` (~50 KB)
- Automatically copied to `unity/Runtime/Plugins/WebGL/`

### Build Profiles

**1. MinSizeRel (Default - Smallest)**
```bash
BUILD_TYPE=MinSizeRel ./scripts/build_wasm.sh
```
- Optimization: `-Oz` (optimize for size)
- Output: ~200-250 KB (compressed: ~70-90 KB)
- Best for: Production web deployment

**2. Release (Balanced)**
```bash
BUILD_TYPE=Release ./scripts/build_wasm.sh
```
- Optimization: `-O3` (optimize for speed)
- Output: ~300-350 KB (compressed: ~100-120 KB)
- Best for: Performance-critical applications

**3. Debug (Debugging)**
```bash
BUILD_TYPE=Debug ./scripts/build_wasm.sh
```
- Optimization: `-O0 -g` (no optimization, debug symbols)
- Output: ~800-1000 KB (with source maps)
- Best for: Development and debugging

### Manual Build

```bash
cd native

# Configure
emcmake cmake -B build-wasm \
    -DCMAKE_TOOLCHAIN_FILE=cmake/wasm.toolchain.cmake \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DSPECTRA_BUILD_TESTS=OFF

# Build
cmake --build build-wasm

# Output: build-wasm/lib/libspectra.wasm
```

---

## 🧪 Testing WASM Build

### Method 1: Standalone HTML Test

Create `test.cpp`:
```cpp
#include <spectra/spectra.h>
#include <iostream>

int main() {
    std::cout << "SpectraCore WASM Test" << std::endl;

    // Create FFT analyzer
    // ... test code ...

    return 0;
}
```

Compile and test:
```bash
emcc test.cpp -o test.html \
    -Lbuild-wasm/lib -lspectra \
    -Inative/include

# Serve locally
python3 -m http.server 8000

# Open in browser: http://localhost:8000/test.html
```

### Method 2: Unity WebGL Build

1. Open Unity project
2. File → Build Settings → WebGL
3. Click "Build and Run"
4. Test filters in browser

### Browser Compatibility

| Browser | WASM Support | SIMD Support | Status |
|---------|--------------|--------------|--------|
| **Chrome 91+** | ✅ Yes | ✅ Yes | Full support |
| **Edge 91+** | ✅ Yes | ✅ Yes | Full support |
| **Firefox 89+** | ✅ Yes | ✅ Yes | Full support |
| **Safari 15+** | ✅ Yes | ✅ Yes | iOS 15+ required |
| **Opera 77+** | ✅ Yes | ✅ Yes | Chromium-based |

**WASM SIMD:** Enabled by default in SpectraCore builds (`-msimd128`)
- Provides 2-4× performance boost
- Supported by 95%+ of browsers (2024+)

---

## 🎯 Performance Expectations

### Benchmarks

**Real-time Audio Processing (44.1 kHz):**
- Order 4 Butterworth lowpass: ~0.5ms per 1024-sample buffer
- 1024-point FFT: ~1.5ms
- **Conclusion:** ✅ Real-time capable for typical DSP workloads

**Comparison to Native:**
- MinSizeRel: ~50-60% native speed
- Release: ~60-70% native speed
- With SIMD: ~70-90% native speed

**Memory Usage:**
- WASM heap: Dynamic growth (starts ~16 MB, grows as needed)
- Browser limit: 2-4 GB (plenty for DSP)

---

## 🐛 Troubleshooting

### "emcc: command not found"

**Solution:**
```bash
# Run emsdk environment setup
source ~/emsdk/emsdk_env.sh

# Or add to .bashrc permanently
echo 'source ~/emsdk/emsdk_env.sh' >> ~/.bashrc
```

### "CMake could not find a package configuration file provided by Emscripten"

**Solution:**
Use `emcmake` wrapper instead of plain `cmake`:
```bash
emcmake cmake -B build-wasm ...
```

### WASM binary too large

**Solutions:**
1. Use MinSizeRel build type:
   ```bash
   BUILD_TYPE=MinSizeRel ./scripts/build_wasm.sh
   ```

2. Strip unused code (automatic in MinSizeRel)

3. Enable additional size optimizations in `wasm.toolchain.cmake`

### "ReferenceError: Module is not defined" in browser

**Cause:** JavaScript module loading issue

**Solution:**
- Ensure Unity WebGL build settings are correct
- Check that `libspectra.js` is included in build
- Verify `MODULARIZE=1` in linker flags

### SIMD not working in Safari

**Cause:** Safari requires iOS 15+ / macOS 12+ for WASM SIMD

**Solution:**
- Update browser to latest version
- Or build without SIMD (remove `-msimd128` from toolchain)

### "RuntimeError: memory access out of bounds"

**Cause:** Buffer overflow or invalid memory access

**Solution:**
1. Build in Debug mode:
   ```bash
   BUILD_TYPE=Debug ./scripts/build_wasm.sh
   ```

2. Enable WASM debugging in browser:
   - Chrome DevTools → Sources → Enable "WebAssembly Debugging"

3. Check for buffer size mismatches between C++ and C#

---

## 🔬 Advanced Topics

### Optimizing for Size

**Already Applied in SpectraCore:**
- `-Oz` optimization level
- Dead code elimination
- Function inlining disabled for large functions
- WASM binary stripping

**Additional Techniques (if needed):**

1. **Enable LTO (Link-Time Optimization):**
   ```cmake
   set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
   ```

2. **Aggressive compression:**
   ```bash
   gzip -9 libspectra.wasm  # ~3-4× compression
   ```

3. **Brotli compression (better than gzip):**
   ```bash
   brotli -9 libspectra.wasm  # ~4-5× compression
   ```

### Debugging WASM

**Source Maps (included in Debug builds):**
```bash
BUILD_TYPE=Debug ./scripts/build_wasm.sh
```

Browser DevTools can now:
- Set breakpoints in C++ code
- Step through execution
- Inspect variables

**Emscripten Tracing:**
```bash
emcc -s ASSERTIONS=2 -s SAFE_HEAP=1 ...
```

**Profiling:**
- Chrome DevTools → Performance tab
- Firefox DevTools → Performance tab
- Look for `wasm-function` entries

### Threading (Advanced - Not Enabled by Default)

WASM threading is possible but complex in WebGL:

**Pros:**
- Parallel processing
- Better performance

**Cons:**
- Requires SharedArrayBuffer (CORS headers required)
- Browser support limited
- Unity WebGL doesn't support threads by default

**Not recommended for SpectraCore** (single-threaded is sufficient).

---

## 📚 Resources

**Official Documentation:**
- Emscripten: https://emscripten.org/
- WebAssembly: https://webassembly.org/
- WASM SIMD: https://github.com/WebAssembly/simd

**Tutorials:**
- [Emscripten Tutorial](https://emscripten.org/docs/getting_started/Tutorial.html)
- [Unity WebGL Guide](https://docs.unity3d.com/Manual/webgl-building.html)

**Browser Compatibility:**
- [Can I Use - WebAssembly](https://caniuse.com/wasm)
- [Can I Use - WASM SIMD](https://caniuse.com/wasm-simd)

---

## ✅ Quick Checklist

Before building:
- [ ] Emscripten installed (`emcc --version` works)
- [ ] emsdk_env.sh sourced (or in .bashrc)
- [ ] CMake 3.20+ installed
- [ ] Python 3.6+ available (for testing)

After building:
- [ ] libspectra.wasm exists (~200-300 KB)
- [ ] libspectra.js exists (~50 KB)
- [ ] Files copied to unity/Runtime/Plugins/WebGL/
- [ ] Unity .meta files created (run setup_meta_files.sh)

For testing:
- [ ] Unity WebGL build succeeds
- [ ] Browser opens without errors
- [ ] Filter creation works in browser
- [ ] Audio processing is real-time

---

## 🆘 Getting Help

**Common Issues:**
- Check [Troubleshooting](#troubleshooting) section above
- Review build output for errors
- Verify Emscripten version (3.1.x recommended)

**Still stuck?**
- GitHub Issues: https://github.com/yogt1984/SpectraCore/issues
- Emscripten Discussions: https://github.com/emscripten-core/emscripten/discussions

---

**Guide Version:** v1.2.0
**Last Updated:** February 26, 2026
