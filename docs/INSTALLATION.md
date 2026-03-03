# SpectraCore Installation Guide

This guide provides detailed instructions for installing SpectraCore in your Unity project.

---

## Prerequisites

- **Unity**: 2021.3 LTS or newer
- **Platforms**: Windows 10/11, macOS 10.15+, or Linux (Ubuntu 18.04+)
- **Git**: For installation via Unity Package Manager (recommended)

---

## Installation Methods

Choose one of the following methods based on your workflow:

### Method 1: Unity Package Manager (Git URL) - Recommended

This is the easiest method and enables automatic updates.

**Steps:**

1. Open your Unity project
2. Go to `Window → Package Manager`
3. Click the `+` button in the top-left corner
4. Select `Add package from git URL...`
5. Enter the following URL:
   ```
   https://github.com/yogt1984/SpectraCore.git
   ```
6. Click `Add`
7. Wait for Unity to download and import the package

**Advantages:**
- Automatic dependency resolution
- Easy updates (just click "Update" in Package Manager)
- No manual file management

**Note**: This method requires Git to be installed and accessible from command line.

---

### Method 2: Download Release Package

Install from a specific release version.

**Steps:**

1. Go to the [Releases page](https://github.com/yogt1984/SpectraCore/releases)
2. Download the latest release (e.g., `SpectraCore-v1.0.0.zip`)
3. Extract the ZIP file
4. Copy the `unity/` folder contents to your project:
   - Copy to `Assets/SpectraCore/` or any preferred location
5. Unity will automatically import the package

**Advantages:**
- Specific version control
- Works without Git
- Can be committed to your project repository

---

### Method 3: Clone Repository

For developers who want to contribute or modify the library.

**Steps:**

1. Open a terminal/command prompt
2. Navigate to your Unity project's `Packages/` directory:
   ```bash
   cd /path/to/YourUnityProject/Packages
   ```

3. Clone the repository:
   ```bash
   git clone https://github.com/yogt1984/SpectraCore.git com.spectracore.dsp
   ```

4. Unity will automatically detect and import the package

**Advantages:**
- Full source code access
- Easy to contribute changes
- Can create branches for custom modifications

**For development in Assets folder:**
```bash
cd /path/to/YourUnityProject/Assets
git clone https://github.com/yogt1984/SpectraCore.git SpectraCore
```

---

### Method 4: Manual Package Manager Entry

Add SpectraCore to your project's `manifest.json` file.

**Steps:**

1. Navigate to your project's `Packages/` directory
2. Open `manifest.json` in a text editor
3. Add the following line to the `dependencies` section:
   ```json
   {
     "dependencies": {
       "com.spectracore.dsp": "https://github.com/yogt1984/SpectraCore.git",
       ...other dependencies...
     }
   }
   ```

4. Save the file
5. Return to Unity - it will automatically import the package

**Advantages:**
- Version control friendly
- Declarative package management
- Easy to share with team (commit manifest.json)

---

## Platform-Specific Setup

SpectraCore includes pre-built native libraries for all major platforms. No additional setup is required for most users.

### Included Platform Libraries

The following platform libraries are included in the package:

- **Linux**: `libspectra.so` (x86_64)
- **Windows**: `spectra.dll` (x64)
- **macOS**: `libspectra.dylib` (Universal Binary: Intel + Apple Silicon)

### Downloading Latest Artifacts (Optional)

If you want the latest CI-built libraries:

1. Go to [GitHub Actions](https://github.com/yogt1984/SpectraCore/actions)
2. Click on the latest successful "CI" workflow run
3. Scroll to "Artifacts" section
4. Download the artifacts you need:
   - `native-ubuntu-latest` (Linux)
   - `native-windows-latest` (Windows)
   - `native-macos-latest` (macOS)

5. Extract and copy to appropriate directories:
   ```
   unity/Runtime/Plugins/
   ├── Linux/
   │   └── x86_64/
   │       └── libspectra.so
   ├── Windows/
   │   └── x86_64/
   │       └── spectra.dll
   └── macOS/
       └── libspectra.dylib
   ```

### Building From Source (Advanced)

If you need to customize the native library:

**Linux:**
```bash
cd native
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cp build/lib/libspectra.so ../unity/Runtime/Plugins/Linux/x86_64/
```

**Windows** (Visual Studio):
```cmd
cd native
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
copy build\Release\spectra.dll ..\unity\Runtime\Plugins\Windows\x86_64\
```

**macOS** (Universal Binary):
```bash
cd native
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build
cp build/lib/libspectra.dylib ../unity/Runtime/Plugins/macOS/
```

---

## Verification

After installation, verify that SpectraCore is working correctly:

### 1. Check Package Manager

1. Open `Window → Package Manager`
2. Look for "SpectraCore DSP Library" in the package list
3. Version should show `1.0.0`

### 2. Run Test Script

Create a test script to verify the installation:

```csharp
using UnityEngine;
using SpectraCore;

public class SpectraCoreTest : MonoBehaviour
{
    void Start()
    {
        // Test 1: Version info
        Debug.Log("SpectraCore Version: " + SpectraNative.GetVersion());

        // Test 2: FFT creation
        using (var fft = new FFTAnalyzer(256))
        {
            Debug.Log($"FFT size: {fft.Size}, Spectrum size: {fft.SpectrumSize}");
        }

        // Test 3: Filter design
        var (b, a) = DSP.Butter(4, 0.3f, FilterType.Lowpass);
        Debug.Log($"Filter designed: {b.Length} b coefficients, {a.Length} a coefficients");

        Debug.Log("✅ SpectraCore is working correctly!");
    }
}
```

**Expected output:**
```
SpectraCore Version: 1.0.0
FFT size: 256, Spectrum size: 129
Filter designed: 5 b coefficients, 5 a coefficients
✅ SpectraCore is working correctly!
```

### 3. Try Editor Tools

1. Open `Window → Spectra → Signal Inspector`
   - Should open without errors
   - Load an AudioClip to test visualization

2. Open `Window → Spectra → Filter Designer`
   - Should display filter design interface
   - Try designing a filter and viewing Bode plot

---

## Common Issues

### Issue 1: DllNotFoundException

**Error**: `DllNotFoundException: spectra`

**Cause**: Native library not found or wrong platform.

**Solution**:
1. Check that platform libraries are in the correct `Plugins/` folder
2. Verify `.meta` files have correct platform settings:
   - Linux: `libspectra.so.meta` should target Linux
   - Windows: `spectra.dll.meta` should target Windows x64
   - macOS: `libspectra.dylib.meta` should target macOS

3. Try reimporting the package:
   - Right-click on `SpectraCore` folder
   - Select `Reimport`

### Issue 2: EntryPointNotFoundException

**Error**: `EntryPointNotFoundException: spectra_version`

**Cause**: Old version of native library or ABI mismatch.

**Solution**:
1. Download latest artifacts from GitHub Actions
2. Replace old library files with new ones
3. Restart Unity Editor

### Issue 3: Package Not Found (Git URL)

**Error**: `Cannot perform upm operation: Unable to add package...`

**Cause**: Git not installed or not in PATH.

**Solution**:
1. Install Git from https://git-scm.com/
2. Add Git to system PATH
3. Restart Unity Editor
4. Try again

Alternatively, use Method 2 (Download Release Package).

### Issue 4: Import Errors

**Error**: Various compilation errors after import.

**Cause**: Unity version too old or .NET Standard compatibility issues.

**Solution**:
1. Upgrade to Unity 2021.3 LTS or newer
2. Check `Player Settings → Other Settings → API Compatibility Level`
   - Should be `.NET Standard 2.1` or `.NET 4.x`
3. Restart Unity Editor

### Issue 5: macOS Library Not Loading

**Error**: Library loading fails on macOS.

**Cause**: Code signing or Gatekeeper restrictions.

**Solution**:
1. Open Terminal
2. Navigate to the library location
3. Remove quarantine attribute:
   ```bash
   xattr -d com.apple.quarantine libspectra.dylib
   ```
4. Restart Unity

---

## Uninstallation

### If installed via Package Manager (Git):

1. Open `Window → Package Manager`
2. Find "SpectraCore DSP Library"
3. Click `Remove`

### If installed manually:

1. Delete the `SpectraCore/` folder from your project
2. Delete any `.meta` files associated with it
3. Unity will automatically update references

---

## Next Steps

After successful installation:

1. **Read the Quick Start**: See [README.md](README.md#quick-start)
2. **Follow Tutorials**: Check out [docs/Tutorials.md](docs/Tutorials.md)
3. **Explore Examples**: See [docs/ExampleProjects.md](docs/ExampleProjects.md)
4. **Browse API Reference**: Read [docs/API_Reference.md](docs/API_Reference.md)
5. **Try Editor Tools**:
   - Signal Inspector: `Window → Spectra → Signal Inspector`
   - Filter Designer: `Window → Spectra → Filter Designer`

---

## Support

If you encounter issues not covered in this guide:

- **Check Documentation**: [docs/](docs/)
- **Search Issues**: https://github.com/yogt1984/SpectraCore/issues
- **Ask Questions**: https://github.com/yogt1984/SpectraCore/discussions
- **Report Bugs**: https://github.com/yogt1984/SpectraCore/issues/new

---

**Happy coding! 🎵**
