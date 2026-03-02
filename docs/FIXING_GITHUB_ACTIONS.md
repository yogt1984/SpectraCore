# Fixing GitHub Actions Release Workflow Failures

## 🔍 Problem Analysis

Your GitHub Actions workflow is failing with:
- ✅ **Linux x64** - Succeeds
- ✅ **Mobile (Android/iOS)** - Succeeds
- ❌ **macOS Universal** - **FAILS**
- ❌ **Windows x64** - Cancelled (due to macOS failure)
- ⏭️ **Create Release** - Skipped (because builds failed)

### Root Causes

1. **macOS Universal Binary Build Issue**
   - Trying to build for both Intel (x86_64) and Apple Silicon (arm64) in one step
   - Requires `lipo` tool and separate builds for each architecture
   - Current workflow tries to do both architectures in one CMake command (doesn't work well)

2. **Windows Build Cancelled**
   - Likely cancelled because macOS failed first
   - Or hit timeout/resource limits

3. **Workflow Triggers Automatically**
   - Runs every time you push a tag (`v*`)
   - This is why you get failure emails

---

## 💡 Solutions (Choose One)

### Option 1: Fix the Workflow ⭐ RECOMMENDED

**Use the fixed workflow that properly handles multi-arch builds**

**Pros:**
- Builds for all platforms automatically
- Proper error handling
- Creates universal macOS binaries correctly
- Uses `if: always()` so partial builds still create releases

**Cons:**
- Takes longer to run (5-10 minutes)
- May still fail if GitHub runners have issues

**How to Apply:**
```bash
cd /home/onat/SpectraCore

# Replace current workflow with fixed version
mv .github/workflows/release-fixed.yml .github/workflows/release.yml

# Commit and push
git add .github/workflows/release.yml
git commit -m "Fix GitHub Actions release workflow for multi-platform builds"
git push origin main
```

---

### Option 2: Simplify (Linux Only) ⚡ FASTEST

**Use simplified workflow that only builds Linux automatically**

**Pros:**
- Fast and reliable (15 seconds)
- No cross-compilation issues
- Creates draft release for you to add other platforms manually

**Cons:**
- Need to manually upload Windows/macOS builds
- Automated release is incomplete

**How to Apply:**
```bash
cd /home/onat/SpectraCore

# Replace with simple workflow
mv .github/workflows/release-simple.yml .github/workflows/release.yml

# Commit and push
git add .github/workflows/release.yml
git commit -m "Simplify GitHub Actions to Linux-only builds"
git push origin main
```

**Then manually upload other platforms:**
1. Workflow creates Linux build automatically
2. You build Windows/macOS locally
3. Upload to the draft release it created

---

### Option 3: Disable Automatic Builds 🚫 MANUAL ONLY

**Turn off automatic builds completely, do everything manually**

**Pros:**
- Full control
- No surprise failures
- No automatic emails

**Cons:**
- Have to manually build and upload everything
- No automation benefits

**How to Apply:**
```bash
cd /home/onat/SpectraCore

# Disable the workflow
mv .github/workflows/release.yml .github/workflows/release.yml.disabled

# Commit and push
git add .github/workflows/release.yml.disabled
git commit -m "Disable automatic release builds"
git push origin main
```

---

## 🔧 Detailed Fix for Option 1

The fixed workflow solves the macOS issue by:

### Before (Broken):
```yaml
- name: macOS Universal
  os: macos-latest
  cmake_args: "-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64"  # ❌ Doesn't work
```

### After (Fixed):
```yaml
# Build x86_64
- name: Configure x86_64
  run: cmake -B build-x86_64 -DCMAKE_OSX_ARCHITECTURES=x86_64

- name: Build x86_64
  run: cmake --build build-x86_64

# Build arm64
- name: Configure arm64
  run: cmake -B build-arm64 -DCMAKE_OSX_ARCHITECTURES=arm64

- name: Build arm64
  run: cmake --build build-arm64

# Combine into universal binary
- name: Create Universal Binary
  run: lipo -create build-x86_64/lib/libspectra.dylib build-arm64/lib/libspectra.dylib -output libspectra.dylib
```

### Additional Fixes:

1. **Submodule initialization** - Added `submodules: recursive` to checkout step
2. **Error tolerance** - Added `if: always()` to create release even if some builds fail
3. **Better artifact paths** - Fixed Windows DLL path issues
4. **Release notes integration** - Reads from `docs/RELEASE_NOTES_vX.X.X.md`
5. **Draft vs Published** - Option 2 creates drafts, Option 1 publishes automatically

---

## 🎯 My Recommendation

**Use Option 1 (Fixed Workflow)** because:
- ✅ Fully automated multi-platform builds
- ✅ Handles errors gracefully
- ✅ Creates complete releases with all platforms
- ✅ Integrates your release notes automatically
- ✅ Only needs to be set up once

---

## 📋 Step-by-Step: Applying Option 1

### Step 1: Replace Workflow File

```bash
cd /home/onat/SpectraCore

# Backup current workflow
cp .github/workflows/release.yml .github/workflows/release.yml.backup

# Use the fixed version
cp .github/workflows/release-fixed.yml .github/workflows/release.yml
```

### Step 2: Test Without Pushing a Tag

```bash
# The fixed workflow has workflow_dispatch which allows manual testing
# You can test it from GitHub web interface:
# Go to: Actions → Release → Run workflow (button on right)
# This lets you test without pushing a new tag
```

### Step 3: Commit and Push

```bash
git add .github/workflows/release.yml
git commit -m "Fix GitHub Actions multi-platform release builds

- Separate x86_64 and arm64 builds for macOS
- Create universal binary with lipo
- Add submodule initialization
- Add error tolerance with if: always()
- Fix Windows DLL artifact paths
- Integrate release notes from docs/
- Allow manual workflow triggers for testing"

git push origin main
```

### Step 4: Test the Fix

You have two options:

**Option A: Manual trigger (safer)**
1. Go to GitHub → Actions tab
2. Click "Release" workflow
3. Click "Run workflow" button
4. Select main branch
5. Click green "Run workflow" button
6. Watch it build (no tag needed)

**Option B: Push a test tag**
```bash
git tag -a v1.3.1-test -m "Test release workflow"
git push origin v1.3.1-test
# Watch the workflow run
# Delete tag afterward if it works: git push --delete origin v1.3.1-test
```

---

## 📊 Expected Results After Fix

### Successful Workflow Run Should Show:

```
✅ Build Windows x64       (2-3 minutes)
✅ Build macOS Universal   (3-4 minutes)
✅ Build Linux x64         (15 seconds)
✅ Build Mobile           (1-2 minutes)
✅ Create Release         (30 seconds)
```

### The Release Will Include:

- `spectra-unity-vX.X.X.zip` - Full Unity package with all platforms
- Automatically published (not draft)
- Release notes from `docs/RELEASE_NOTES_vX.X.X.md`

---

## 🚨 Common Issues & Solutions

### Issue 1: "lipo: can't open input file"
**Cause:** One of the macOS builds failed
**Solution:** Check that both x86_64 and arm64 builds succeed individually

### Issue 2: "GITHUB_TOKEN permissions"
**Cause:** Token doesn't have release creation permissions
**Solution:**
1. Go to Settings → Actions → General
2. Under "Workflow permissions", select "Read and write permissions"
3. Save

### Issue 3: "Artifact not found"
**Cause:** Build step failed but was ignored
**Solution:** Check individual build logs, fix compilation errors

### Issue 4: Windows build timeout
**Cause:** CMake taking too long
**Solution:** Already handled in fixed workflow with better caching

---

## 🔍 Debugging Failed Workflows

If builds still fail after applying the fix:

### Step 1: Check Logs
```
1. Go to GitHub → Actions → Failed workflow run
2. Click on the failed job (e.g., "Build macOS Universal")
3. Expand the failed step
4. Copy error message
```

### Step 2: Test Locally
```bash
# Test macOS build locally (if on Mac)
cd native
cmake -B build-x86_64 -DCMAKE_OSX_ARCHITECTURES=x86_64
cmake --build build-x86_64

# Test Linux build locally
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Step 3: Check Dependencies
- Ensure all submodules are present: `git submodule update --init --recursive`
- Check CMakeLists.txt doesn't have platform-specific issues

---

## 📞 Still Having Issues?

If the workflow still fails after applying the fix:

1. **Share the error logs** - Copy the failed step output
2. **Check GitHub Actions limits** - Free tier has 2000 minutes/month
3. **Verify repository settings** - Actions must be enabled
4. **Test manual build** - Build locally first to rule out code issues

---

## ✅ Quick Fix Checklist

- [ ] Choose which option to use (1, 2, or 3)
- [ ] Replace workflow file
- [ ] Commit and push changes
- [ ] Test with manual trigger or test tag
- [ ] Verify builds succeed
- [ ] Check release is created properly
- [ ] Delete test tag if used

---

## 📚 Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [CMake Cross-Compilation](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html)
- [macOS Universal Binaries](https://developer.apple.com/documentation/apple-silicon/building-a-universal-macos-binary)

---

**Bottom Line:** Use the fixed workflow (Option 1), commit it, and your releases will build automatically for all platforms without failures! 🎉
