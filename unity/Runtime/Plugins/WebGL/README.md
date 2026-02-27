# WebGL/WebAssembly Native Plugin

This directory contains the WebAssembly binaries for SpectraCore to run in web browsers via Unity WebGL builds.

## Building

From the `native/` directory, run:

```bash
# Install Emscripten first (see docs/EmscriptenSetupGuide.md)
source ~/emsdk/emsdk_env.sh

# Build for WebGL
./scripts/build_wasm.sh
```

## Output

- **libspectra.wasm** - WebAssembly binary (~200-300 KB)
- **libspectra.js** - JavaScript glue code (~50 KB)

## Unity Import Settings

The `.meta` files should have these settings:

### libspectra.wasm
- **Platform:** WebGL
- **Plugin Type:** Native (.wasm)

### libspectra.js
- **Platform:** WebGL
- **Plugin Type:** Native (.js)

## Browser Compatibility

| Browser | Version | WASM | SIMD | Status |
|---------|---------|------|------|--------|
| **Chrome** | 91+ | ✅ | ✅ | Full support |
| **Edge** | 91+ | ✅ | ✅ | Full support |
| **Firefox** | 89+ | ✅ | ✅ | Full support |
| **Safari** | 15+ | ✅ | ✅ | iOS 15+ required |
| **Opera** | 77+ | ✅ | ✅ | Chromium-based |

**Coverage:** 95%+ of active browser installations (2024+)

## Performance

**Real-time Audio (44.1 kHz):**
- ✅ Real-time capable for typical DSP workloads
- Order 4 filter: ~0.5ms per 1024-sample buffer
- 1024-point FFT: ~1.5ms

**Comparison to Native:**
- MinSizeRel build: ~50-60% native speed
- Release build: ~60-70% native speed
- With SIMD enabled: ~70-90% native speed

**Memory:**
- WASM heap: Dynamic growth (starts ~16 MB)
- Browser limit: 2-4 GB (sufficient for DSP)

## Testing

### Unity WebGL Build

1. Open Unity project
2. File → Build Settings → WebGL
3. Click "Build and Run"
4. Test in browser console:
   ```javascript
   // Unity should automatically load SpectraCore WASM
   console.log("SpectraCore loaded in browser");
   ```

### Standalone Browser Test

```bash
# Create test HTML
cd native
emcc test.cpp -o test.html -Lbuild-wasm/lib -lspectra

# Serve locally
python3 -m http.server 8000

# Open: http://localhost:8000/test.html
```

## Build Profiles

**MinSizeRel (Default):**
- Smallest binary (~200-250 KB)
- Optimization: `-Oz`
- Best for web deployment

```bash
BUILD_TYPE=MinSizeRel ./scripts/build_wasm.sh
```

**Release (Performance):**
- Balanced size/speed (~300-350 KB)
- Optimization: `-O3`
- Best for performance-critical apps

```bash
BUILD_TYPE=Release ./scripts/build_wasm.sh
```

**Debug (Development):**
- Debug symbols and source maps (~800-1000 KB)
- Optimization: `-O0 -g`
- Best for debugging

```bash
BUILD_TYPE=Debug ./scripts/build_wasm.sh
```

## Platform-Specific Notes

### WebGL Differences from Native

**1. Library Import**
- Native platforms: `[DllImport("spectra")]`
- WebGL: `[DllImport("__Internal")]`
- SpectraCore handles this automatically with `#if UNITY_WEBGL`

**2. No File System**
- WebGL has no direct file I/O
- All data must be in memory
- SpectraCore doesn't use file I/O, so this doesn't affect functionality

**3. Single-Threaded**
- WebGL is single-threaded by default
- SpectraCore is thread-safe but doesn't require threading
- Performance is still real-time capable

**4. Memory Management**
- WASM heap grows dynamically
- No need to pre-allocate large buffers
- Browser handles memory limits

### Optimization Flags

**Enabled in build:**
- `-msimd128` - WASM SIMD (2-4× speedup)
- `-ffast-math` - Fast floating-point math
- `-Oz` or `-O3` - Size or speed optimization
- `-s ALLOW_MEMORY_GROWTH=1` - Dynamic heap
- `-s WASM=1` - Pure WASM (no asm.js fallback)

### Browser Developer Tools

**Chrome DevTools:**
1. Open Console (F12)
2. Enable WebAssembly debugging:
   - Sources → Enable "WebAssembly Debugging"
3. Set breakpoints in C++ code
4. Performance tab for profiling

**Firefox DevTools:**
1. Console (F12)
2. Debugger → WASM files appear
3. Performance profiling available

## Troubleshooting

### "Could not load libspectra.wasm"

**Causes:**
- WASM file not included in WebGL build
- Incorrect path in Unity build settings
- Server MIME type issue

**Solutions:**
```bash
# Verify files exist
ls unity/Runtime/Plugins/WebGL/

# Check Unity import settings
# Select libspectra.wasm → Inspector → Platform: WebGL

# If serving manually, ensure correct MIME type:
# application/wasm for .wasm files
# application/javascript for .js files
```

### "DllNotFoundException: __Internal"

**Cause:**
- Not building for WebGL platform
- Running in Unity Editor (not WebGL player)

**Solution:**
- Build for WebGL target, not Editor
- `#if UNITY_WEBGL && !UNITY_EDITOR` ensures Editor uses native library

### Performance Too Slow

**Solutions:**
1. Use Release build instead of MinSizeRel:
   ```bash
   BUILD_TYPE=Release ./scripts/build_wasm.sh
   ```

2. Verify SIMD is enabled:
   ```bash
   # Check build output for: -msimd128
   ```

3. Test in Chrome/Edge (fastest WASM engines)

4. Profile in DevTools Performance tab

### WASM Module Won't Load in Safari

**Cause:**
- Safari < 15 doesn't support WASM SIMD
- iOS < 15 doesn't support WASM SIMD

**Solutions:**
- Update to Safari 15+ / iOS 15+
- Or build without SIMD (remove `-msimd128` from toolchain)

### Memory Errors

**"RuntimeError: memory access out of bounds"**

**Solution:**
```bash
# Build in Debug mode for better error messages
BUILD_TYPE=Debug ./scripts/build_wasm.sh

# Enable WASM debugging in browser
# Check buffer sizes match between C++ and C#
```

## Advanced Topics

### Compressing WASM for Faster Downloads

**Gzip (default):**
```bash
gzip -9 libspectra.wasm
# ~3-4× compression: 300 KB → ~80 KB
```

**Brotli (better):**
```bash
brotli -9 libspectra.wasm
# ~4-5× compression: 300 KB → ~60 KB
```

Most web servers automatically compress WASM files.

### Source Maps for Debugging

**Included in Debug builds:**
```bash
BUILD_TYPE=Debug ./scripts/build_wasm.sh
```

Browser can now:
- Show C++ source code
- Set breakpoints
- Step through execution
- Inspect variables

### Profiling WASM Performance

**Browser DevTools:**
1. Performance tab
2. Record profile
3. Look for `wasm-function[XXX]` entries
4. Identify bottlenecks

**Emscripten Profiler:**
```bash
emcc -profiling ...
```

## Next Steps

After building WASM libraries:

1. **Set up .meta files:**
   ```bash
   cd unity/Runtime/Plugins
   ./setup_meta_files.sh
   ```

2. **Open Unity project:**
   - Verify plugins import correctly
   - Check Inspector settings

3. **Build for WebGL:**
   - File → Build Settings → WebGL
   - Build and Run

4. **Test in browsers:**
   - Chrome (recommended)
   - Firefox
   - Safari (iOS 15+)
   - Edge

5. **Validate functionality:**
   - Create filters
   - Process audio
   - Verify real-time performance

## Resources

- **Emscripten Setup:** See `docs/EmscriptenSetupGuide.md`
- **Build Guide:** See `docs/CrossPlatformBuildGuide.md` (WebGL section)
- **Unity WebGL Docs:** https://docs.unity3d.com/Manual/webgl-building.html
- **WASM Spec:** https://webassembly.org/

## Performance Benchmarks

Measured on:
- Chrome 120 on Ubuntu 22.04
- Intel Core i7-10700K @ 3.8GHz

| Operation | Native (Linux) | WASM (MinSizeRel) | WASM (Release) | Ratio |
|-----------|----------------|-------------------|----------------|-------|
| Order 4 Butterworth LP | 0.25 ms | 0.45 ms | 0.35 ms | 55-70% |
| 1024-point FFT | 0.8 ms | 1.5 ms | 1.2 ms | 53-67% |
| FIR filter (51 taps) | 0.15 ms | 0.25 ms | 0.20 ms | 60-75% |

**Conclusion:** ✅ Real-time capable for typical DSP at 44.1 kHz

---

**Guide Version:** v1.2.0
**Last Updated:** February 26, 2026
