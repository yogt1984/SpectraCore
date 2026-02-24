# SpectraCore CI/CD Guide

Continuous Integration and Deployment setup for automated multi-platform builds.

## Table of Contents

1. [Overview](#overview)
2. [GitHub Actions Workflow](#github-actions-workflow)
3. [Testing the Workflow](#testing-the-workflow)
4. [Understanding the Build Process](#understanding-the-build-process)
5. [Troubleshooting](#troubleshooting)
6. [Advanced Configuration](#advanced-configuration)
7. [Local Testing](#local-testing)

---

## Overview

The SpectraCore project uses GitHub Actions to automatically build native libraries for all supported platforms (Linux, Windows, macOS) on every commit.

### What Gets Built

**Platforms:**
- Linux (x86_64) - Ubuntu latest
- Windows (x86_64) - Visual Studio 2022
- macOS (Universal Binary) - x86_64 + ARM64

**Artifacts:**
- `libspectra.so` (Linux)
- `spectra.dll` (Windows)
- `libspectra.dylib` (macOS universal)
- Complete Unity package with all platforms

**Testing:**
- Runs full native test suite (68 tests)
- Validates builds on all platforms
- Ensures cross-platform compatibility

---

## GitHub Actions Workflow

### Workflow File

**Location:** `.github/workflows/ci.yml`

**Triggers:**
- Push to `main` branch
- Pull requests targeting `main`

### Workflow Structure

```yaml
jobs:
  build-linux:    # Build on Ubuntu
  build-windows:  # Build on Windows
  build-macos:    # Build on macOS
  package-unity:  # Combine all artifacts
```

**Execution Time:** ~10-15 minutes total (parallel builds)

---

## Testing the Workflow

### Prerequisites

1. **GitHub Repository**
   - SpectraCore repository pushed to GitHub
   - GitHub Actions enabled (default for public repos)

2. **Repository Structure**
   ```
   SpectraCore/
   ├── .github/
   │   └── workflows/
   │       └── ci.yml
   ├── native/
   │   ├── CMakeLists.txt
   │   ├── src/
   │   ├── include/
   │   └── tests/
   └── unity/
       └── Runtime/
           └── Plugins/
   ```

3. **Permissions**
   - Write access to repository
   - GitHub Actions enabled

---

### Step 1: Initial Setup

**Verify workflow file exists:**

```bash
cd /home/onat/SpectraCore
ls -la .github/workflows/ci.yml
```

**Expected output:**
```
-rw-rw-r-- 1 user user 3500 Feb 23 10:00 .github/workflows/ci.yml
```

---

### Step 2: Commit and Push

**Check current status:**

```bash
git status
```

**Stage workflow file:**

```bash
git add .github/workflows/ci.yml
```

**Commit:**

```bash
git commit -m "Add CI/CD: Multi-platform build workflow

- Automated builds for Linux, Windows, macOS
- Run full test suite on all platforms
- Generate Unity package artifacts
- Parallel execution for fast builds"
```

**Push to GitHub:**

```bash
git push origin main
```

---

### Step 3: Monitor the Build

**1. Open GitHub Repository:**
- Navigate to your repository on GitHub
- Example: `https://github.com/YOUR_USERNAME/SpectraCore`

**2. Go to Actions Tab:**
- Click "Actions" in the top navigation bar

**3. View Workflow Run:**
- You should see a new workflow run starting
- Name: "CI"
- Triggered by: Your commit

**4. Watch Progress:**
- Click on the workflow run
- See multiple jobs running in parallel:
  - build-native (ubuntu-latest)
  - build-native (windows-latest)
  - build-native (macos-latest)
  - test-sanitizers
  - static-analysis
  - build-android (optional)
  - build-ios (optional)
  - unity-tests (optional, requires UNITY_LICENSE secret)
  - code-coverage (optional)

**Typical Timeline:**
```
0:00 - Jobs start
2:00 - Linux build completes
5:00 - Windows build completes
8:00 - macOS build completes
10:00 - All jobs complete
```

---

### Step 4: Download Artifacts

**After successful build:**

1. **Scroll to bottom** of workflow run page

2. **Find "Artifacts" section:**
   ```
   Artifacts
   ├── native-ubuntu-latest (142 KB)
   ├── native-windows-latest (98 KB)
   ├── native-macos-latest (256 KB)
   ├── native-android (optional)
   └── native-ios (optional)
   ```

3. **Download artifacts:**
   - Click on any artifact name to download
   - Downloads as `.zip` file

4. **Extract and verify:**
   ```bash
   unzip native-ubuntu-latest.zip
   file lib/libspectra.so
   # Expected: ELF 64-bit LSB shared object, x86-64
   ```

---

### Step 5: Verify Build Success

**Check logs for each job:**

1. **Click on job name** (e.g., "build-linux")

2. **Expand steps** to see output:
   - Install Dependencies ✓
   - Build Native Library ✓
   - Run Tests ✓
   - Upload Artifact ✓

3. **Look for test results:**
   ```
   Test project /home/runner/work/SpectraCore/SpectraCore/native/build
   Start 1: test_version
   1/68 Test #1: test_version .....................   Passed    0.01 sec
   ...
   100% tests passed, 0 tests failed out of 68
   ```

4. **All green checkmarks** = success!

---

## Understanding the Build Process

### Linux Build (Ubuntu)

**Steps:**

1. **Checkout Code**
   ```yaml
   - uses: actions/checkout@v3
   ```
   Clones your repository

2. **Install Dependencies**
   ```bash
   sudo apt-get update
   sudo apt-get install -y cmake build-essential
   ```
   Installs CMake and GCC

3. **Build Library**
   ```bash
   cd native
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```
   Compiles `libspectra.so`

4. **Run Tests**
   ```bash
   cd native/build
   ctest --output-on-failure
   ```
   Runs 68 unit tests

5. **Upload Artifact**
   ```yaml
   - uses: actions/upload-artifact@v4
     with:
       name: native-${{ matrix.os }}
       path: |
         native/build/lib/*
         native/build/bin/*
   ```
   Makes library downloadable

---

### Windows Build (MSVC)

**Steps:**

1. **Checkout Code**
   Same as Linux

2. **Setup MSVC**
   ```yaml
   - uses: microsoft/setup-msbuild@v1
   ```
   Configures Visual Studio 2022

3. **Build Library**
   ```bash
   cd native
   cmake -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release
   ```
   Compiles `spectra.dll` with MSVC

4. **Run Tests**
   ```bash
   cd native/build
   ctest -C Release --output-on-failure
   ```
   Runs tests on Windows

5. **Upload Artifact**
   ```yaml
   path: native/build/Release/spectra.dll
   ```
   Note: Different path than Linux (Release subdirectory)

---

### macOS Build (Universal Binary)

**Steps:**

1. **Checkout Code**
   Same as others

2. **Install Dependencies**
   ```bash
   brew install cmake
   ```
   Homebrew package manager

3. **Build Universal Binary**
   ```bash
   cmake -B build \
     -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
   cmake --build build
   ```
   **Important:** Builds for both Intel (x86_64) and Apple Silicon (arm64)

4. **Run Tests**
   Tests run on current architecture (x86_64 on GitHub runners)

5. **Upload Artifact**
   ```yaml
   path: native/build/lib/libspectra.dylib
   ```
   Universal binary works on both Mac types

---

### Package Unity Job

**Purpose:** Combines all platform binaries into Unity package structure

**Steps:**

1. **Wait for all builds** (`needs: [build-linux, build-windows, build-macos]`)

2. **Download all artifacts** to correct Unity plugin locations:
   ```
   unity/Runtime/Plugins/
   ├── Linux/x86_64/libspectra.so
   ├── Windows/x86_64/spectra.dll
   └── macOS/libspectra.dylib
   ```

3. **Create tarball:**
   ```bash
   cd unity
   tar -czf ../SpectraCore-Unity.tar.gz .
   ```

4. **Upload complete package**
   Ready to import into Unity

---

## Troubleshooting

### Issue 1: Workflow Not Triggering

**Symptoms:**
- Pushed to GitHub but no workflow runs
- Actions tab is empty

**Causes:**
1. Pushed to wrong branch (not `main`)
2. GitHub Actions disabled
3. Workflow file in wrong location

**Solutions:**

**Check current branch:**
```bash
git branch
# Should show: * main
```

**If on different branch:**
```bash
git checkout main
git merge your-branch
git push origin main
```

**Check workflow file location:**
```bash
ls -la .github/workflows/
# Should show: ci.yml
```

**Enable GitHub Actions:**
1. Go to repository Settings
2. Click "Actions" in left sidebar
3. Select "Allow all actions"
4. Save

---

### Issue 2: Linux Build Fails

**Common errors:**

**Error:** `CMake not found`
```
/bin/sh: 1: cmake: not found
```

**Cause:** Dependencies not installed

**Solution:** Workflow already includes:
```yaml
- name: Install Dependencies
  run: |
    sudo apt-get update
    sudo apt-get install -y cmake build-essential
```
Should auto-fix. If persists, check Ubuntu runner version.

---

**Error:** `undefined reference to 'fftw_...'`

**Cause:** Missing FFTW dependency (if using FFTW instead of KissFFT)

**Solution:** Add to dependencies:
```yaml
sudo apt-get install -y cmake build-essential libfftw3-dev
```

---

**Error:** Tests fail
```
99% tests passed, 1 tests failed out of 68
```

**Cause:** Platform-specific test failure

**Solution:**
1. Click on failed test in logs
2. Read error message
3. Check if test expects specific platform behavior
4. Add platform checks to test if needed

---

### Issue 3: Windows Build Fails

**Common errors:**

**Error:** `MSVC not found`

**Cause:** MSVC setup failed

**Solution:** Workflow uses:
```yaml
- uses: microsoft/setup-msbuild@v1
```

If fails, try specific version:
```yaml
- uses: microsoft/setup-msbuild@v1.1
```

---

**Error:** `spectra.dll not found` during artifact upload

**Cause:** Different output path on Windows

**Check CMakeLists.txt:**
```cmake
# Should output to:
# Release/spectra.dll (MSVC)
# lib/libspectra.so (GCC/Clang)
```

**Current workflow path is correct:**
```yaml
path: native/build/Release/spectra.dll
```

---

### Issue 4: macOS Build Fails

**Common errors:**

**Error:** `clang: error: unsupported option '-arch arm64'`

**Cause:** Old Xcode version doesn't support ARM64

**Solution:** Workflow uses `macos-latest` which includes ARM64 support.

If using older macOS runner, build x86_64 only:
```yaml
-DCMAKE_OSX_ARCHITECTURES="x86_64"
```

---

**Error:** Tests fail on macOS

**Check for:**
- Endianness issues (macOS is little-endian, same as Linux/Windows)
- Path separators (use `/` or platform-agnostic paths)
- Floating-point precision differences

---

### Issue 5: Artifact Upload Fails

**Error:** `Artifact not found: native-ubuntu-latest`

**Cause:** Build job failed or artifact path incorrect

**Solution:**
1. Check which build-native job failed (ubuntu-latest, windows-latest, or macos-latest)
2. Verify artifact paths in workflow match actual build output
3. Fix the failing job first
3. Package job will succeed once all dependencies pass

---

**Error:** `Permission denied` when creating tarball

**Cause:** File permissions issue

**Solution:** Add before tar command:
```yaml
- name: Fix Permissions
  run: chmod -R +r unity/
```

---

## Advanced Configuration

### Customizing Triggers

**Build on all branches:**
```yaml
on:
  push:
    branches: [ '*' ]
```

**Build on tags only:**
```yaml
on:
  push:
    tags:
      - 'v*'  # Triggers on v1.0.0, v2.1.0, etc.
```

**Manual trigger:**
```yaml
on:
  workflow_dispatch:  # Adds "Run workflow" button
```

**Scheduled builds:**
```yaml
on:
  schedule:
    - cron: '0 0 * * 0'  # Every Sunday at midnight
```

---

### Adding More Platforms

**Android:**
```yaml
build-android:
  runs-on: ubuntu-latest
  steps:
    - uses: actions/checkout@v3

    - name: Setup NDK
      uses: nttld/setup-ndk@v1
      with:
        ndk-version: r25b

    - name: Build for Android
      run: |
        cd native
        cmake -B build-android \
          -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
          -DANDROID_ABI=arm64-v8a \
          -DANDROID_PLATFORM=android-21
        cmake --build build-android
```

**iOS:**
```yaml
build-ios:
  runs-on: macos-latest
  steps:
    - uses: actions/checkout@v3

    - name: Build for iOS
      run: |
        cd native
        cmake -B build-ios \
          -GXcode \
          -DCMAKE_SYSTEM_NAME=iOS \
          -DCMAKE_OSX_ARCHITECTURES="arm64"
        cmake --build build-ios
```

---

### Build Matrix

**Test multiple configurations:**

```yaml
build-linux:
  strategy:
    matrix:
      compiler: [gcc-9, gcc-10, gcc-11, clang-12, clang-13]
      build-type: [Debug, Release]

  runs-on: ubuntu-latest

  steps:
    - uses: actions/checkout@v3

    - name: Install Compiler
      run: |
        sudo apt-get install -y ${{ matrix.compiler }}

    - name: Build
      run: |
        cmake -B build -DCMAKE_BUILD_TYPE=${{ matrix.build_type }}
        cmake --build build
```

**Result:** Tests 10 configurations (5 compilers × 2 build types)

---

### Caching Dependencies

**Speed up builds:**

```yaml
- name: Cache CMake Build
  uses: actions/cache@v3
  with:
    path: native/build
    key: ${{ runner.os }}-cmake-${{ hashFiles('**/CMakeLists.txt') }}
    restore-keys: |
      ${{ runner.os }}-cmake-
```

**Benefits:**
- Faster builds (use cached dependencies)
- Reduced build time from ~10 min to ~5 min

---

### Release Automation

**Auto-create releases:**

```yaml
release:
  needs: [build-linux, build-windows, build-macos, package-unity]
  runs-on: ubuntu-latest
  if: startsWith(github.ref, 'refs/tags/v')

  steps:
    - name: Download Artifacts
      uses: actions/download-artifact@v4

    - name: Create Release
      uses: softprops/action-gh-release@v1
      with:
        files: |
          native-ubuntu-latest.zip
          native-windows-latest.zip
          native-macos-latest.zip
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
```

**Usage:**
```bash
git tag v1.0.0
git push origin v1.0.0
# Automatically creates GitHub release with binaries
```

---

## Local Testing

### Test Workflow Locally with Act

**Act** runs GitHub Actions locally using Docker.

**Install:**
```bash
# Linux/macOS
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# Or with package manager
sudo apt-get install act  # Ubuntu
brew install act          # macOS
```

**Run workflow:**
```bash
cd /home/onat/SpectraCore

# Run all jobs
act

# Run specific job
act -j build-linux

# Run with verbose output
act -v
```

**Limitations:**
- May not perfectly match GitHub runners
- Some actions might not work
- Good for quick validation

---

### Manual Build Test

**Replicate CI build locally:**

**Linux:**
```bash
cd /home/onat/SpectraCore/native
cmake -B build-ci -DCMAKE_BUILD_TYPE=Release
cmake --build build-ci
cd build-ci
ctest --output-on-failure
```

**Cross-compile for Windows (from Linux):**
```bash
cd /home/onat/SpectraCore
./build-windows-from-linux.sh
```

**macOS:**
```bash
# On Mac
cd native
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build
cd build
ctest --output-on-failure
```

---

## Best Practices

### 1. Version Control

**Tag releases:**
```bash
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0
```

**Benefits:**
- Easy to track which version is which
- Can trigger release builds
- Clear history

---

### 2. Status Badges

**Add to README.md:**

```markdown
[![Build Status](https://github.com/YOUR_USERNAME/SpectraCore/workflows/Multi-Platform%20Build/badge.svg)](https://github.com/YOUR_USERNAME/SpectraCore/actions)
```

**Shows build status on repository page**

---

### 3. Branch Protection

**Require CI to pass before merge:**

1. Go to Settings → Branches
2. Add rule for `main`
3. Check "Require status checks to pass"
4. Select "build-linux", "build-windows", "build-macos"
5. Save

**Result:** Can't merge PR if builds fail

---

### 4. Notifications

**Get notified of build failures:**

1. GitHub → Settings → Notifications
2. Enable "GitHub Actions" notifications
3. Choose email or mobile notifications

**Or use workflow:**
```yaml
- name: Notify on Failure
  if: failure()
  run: |
    curl -X POST YOUR_WEBHOOK_URL \
      -d "Build failed: ${{ github.repository }}"
```

---

## Monitoring and Metrics

### Build Times

**Check workflow execution time:**
1. Actions tab
2. Click workflow run
3. See total time and per-job breakdown

**Optimize if:**
- Total time > 20 minutes
- Individual job > 10 minutes

**Optimization strategies:**
- Enable caching
- Reduce test scope
- Use faster runners (paid)

---

### Success Rate

**Monitor:**
- How many builds pass vs fail
- Which jobs fail most often
- Common error patterns

**Tools:**
- GitHub Insights (for private repos)
- Custom analytics (parse workflow logs)

---

### Artifact Size

**Check artifact sizes:**
- Linux .so: ~140 KB
- Windows .dll: ~100 KB
- macOS .dylib: ~250 KB (universal)
- Unity package: ~500 KB

**If much larger:**
- Debug symbols included (use -DCMAKE_BUILD_TYPE=Release)
- Static linking (check CMakeLists.txt)
- Not stripped (add strip command)

---

## Conclusion

The CI/CD workflow provides:

✅ **Automated builds** for all platforms
✅ **Quality assurance** via automated tests
✅ **Artifact delivery** for easy distribution
✅ **Fast feedback** on code changes
✅ **Reproducible builds** across environments

**Next Steps:**
1. Push workflow to GitHub
2. Monitor first build
3. Download and verify artifacts
4. Integrate into development workflow

**Resources:**
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [CMake Documentation](https://cmake.org/documentation/)
- [Act (Local Testing)](https://github.com/nektos/act)

---

## Quick Reference

### Common Commands

```bash
# Check workflow syntax
yamllint .github/workflows/ci.yml

# Local build test
cd native && cmake -B build && cmake --build build

# View workflow runs
gh run list  # GitHub CLI

# Download artifacts
gh run download RUN_ID  # GitHub CLI

# Trigger manual workflow
gh workflow run "CI"  # If workflow_dispatch enabled
```

### Useful GitHub CLI Commands

```bash
# Install GitHub CLI
sudo apt install gh  # Ubuntu
brew install gh      # macOS

# Login
gh auth login

# View recent runs
gh run list --limit 10

# View specific run
gh run view RUN_ID

# Watch run in real-time
gh run watch

# Download artifacts
gh run download

# Re-run failed jobs
gh run rerun RUN_ID --failed
```

---

## Support

**If builds fail:**
1. Check this troubleshooting guide
2. Review GitHub Actions logs
3. Test locally
4. Open issue with:
   - Error message
   - Workflow run link
   - Steps to reproduce

**Resources:**
- GitHub Actions logs (most detailed)
- This CI/CD guide
- CMake documentation
- Community forums
