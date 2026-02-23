# CI/CD Testing Checklist

Quick checklist for testing the GitHub Actions workflow.

## Pre-Flight Check

- [ ] Workflow file exists at `.github/workflows/build-multiplatform.yml`
- [ ] Git repository initialized
- [ ] Main branch exists
- [ ] No uncommitted changes in workflow file

```bash
# Verify
ls -la .github/workflows/build-multiplatform.yml
git status
git branch
```

---

## Step 1: Commit Workflow

```bash
cd /home/onat/SpectraCore

# Check status
git status

# Add workflow
git add .github/workflows/build-multiplatform.yml

# Commit
git commit -m "Add CI/CD: Multi-platform build workflow"

# Push
git push origin main
```

**Status:** ⬜ Not started | ⏳ In progress | ✅ Complete

---

## Step 2: Monitor Build

### Access GitHub Actions

1. **Open Browser:**
   - [ ] Navigate to: `https://github.com/YOUR_USERNAME/SpectraCore`
   - [ ] Click "Actions" tab

2. **Find Workflow Run:**
   - [ ] See "Multi-Platform Build" workflow
   - [ ] Status shows "In progress" (yellow dot)

3. **Watch Jobs:**
   - [ ] `build-linux` running
   - [ ] `build-windows` running
   - [ ] `build-macos` running
   - [ ] `package-unity` waiting

**Expected Time:** 10-15 minutes

---

## Step 3: Verify Success

### Check Each Job

**build-linux:**
- [ ] Install Dependencies ✓
- [ ] Build Native Library ✓
- [ ] Run Tests ✓ (68/68 passed)
- [ ] Upload Artifact ✓

**build-windows:**
- [ ] Setup MSVC ✓
- [ ] Build Native Library ✓
- [ ] Run Tests ✓ (68/68 passed)
- [ ] Upload Artifact ✓

**build-macos:**
- [ ] Install Dependencies ✓
- [ ] Build Native Library (Universal) ✓
- [ ] Run Tests ✓ (68/68 passed)
- [ ] Upload Artifact ✓

**package-unity:**
- [ ] Download Linux Library ✓
- [ ] Download Windows Library ✓
- [ ] Download macOS Library ✓
- [ ] Create Unity Package ✓
- [ ] Upload Unity Package ✓

**All jobs:** ✅ Green checkmarks

---

## Step 4: Download Artifacts

### Locate Artifacts

1. **Scroll to bottom** of workflow run page

2. **Verify Artifacts Present:**
   - [ ] `libspectra-linux` (~140 KB)
   - [ ] `spectra-windows` (~100 KB)
   - [ ] `libspectra-macos` (~250 KB)
   - [ ] `SpectraCore-Unity-Package` (~500 KB)

3. **Download Each:**
   - [ ] Click artifact name
   - [ ] File downloads as .zip

---

## Step 5: Validate Downloads

### Linux Library

```bash
unzip libspectra-linux.zip
file libspectra.so
```

**Expected:**
```
libspectra.so: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV),
dynamically linked, BuildID[sha1]=..., not stripped
```

- [ ] File type correct
- [ ] Size reasonable (~140 KB)
- [ ] Not corrupted

---

### Windows Library

```bash
unzip spectra-windows.zip
file spectra.dll
```

**Expected:**
```
spectra.dll: PE32+ executable (DLL) (console) x86-64, for MS Windows
```

- [ ] File type correct
- [ ] Size reasonable (~100 KB)
- [ ] Not corrupted

---

### macOS Library

```bash
unzip libspectra-macos.zip
file libspectra.dylib
```

**Expected:**
```
libspectra.dylib: Mach-O universal binary with 2 architectures:
[x86_64:Mach-O 64-bit dynamically linked shared library x86_64]
[arm64:Mach-O 64-bit dynamically linked shared library arm64]
```

- [ ] Universal binary (both architectures)
- [ ] Size reasonable (~250 KB)
- [ ] Not corrupted

---

### Unity Package

```bash
unzip SpectraCore-Unity-Package.zip
tar -tzf SpectraCore-Unity.tar.gz | head -20
```

**Expected to see:**
```
Runtime/
Runtime/Plugins/
Runtime/Plugins/Linux/
Runtime/Plugins/Linux/x86_64/
Runtime/Plugins/Linux/x86_64/libspectra.so
Runtime/Plugins/Windows/
Runtime/Plugins/Windows/x86_64/
Runtime/Plugins/Windows/x86_64/spectra.dll
Runtime/Plugins/macOS/
Runtime/Plugins/macOS/libspectra.dylib
...
```

- [ ] All three libraries present
- [ ] Correct directory structure
- [ ] Package can be extracted

---

## Step 6: Integration Test (Optional)

### Test in Unity

**If you have Unity installed:**

1. **Extract Unity package:**
   ```bash
   mkdir unity-test
   cd unity-test
   tar -xzf ../SpectraCore-Unity.tar.gz
   ```

2. **Create test Unity project:**
   - [ ] Open Unity Hub
   - [ ] Create new project
   - [ ] Copy extracted files to `Assets/SpectraCore/`

3. **Run verification script:**
   ```csharp
   using Spectra;

   var (b, a) = DSP.Butter(4, 0.25f);
   Debug.Log($"Filter created: {b.Length} coefficients");
   // Should see: Filter created: 5 coefficients
   ```

4. **Test on each platform** (if available):
   - [ ] Windows build
   - [ ] macOS build
   - [ ] Linux build

---

## Troubleshooting

### Build Failed

**If any job fails:**

1. **Click on failed job**
2. **Expand failed step**
3. **Read error message**
4. **Check:**
   - [ ] Error message saved
   - [ ] Relevant code section identified
   - [ ] Fix attempted locally first

**Common fixes:**
- Update workflow file
- Fix CMake configuration
- Update dependencies
- Adjust test expectations

5. **After fix:**
   ```bash
   git add .
   git commit -m "Fix CI: [describe fix]"
   git push origin main
   ```

6. **Monitor new build**

---

### No Artifacts

**If artifacts don't appear:**

- [ ] Check all jobs passed (green checkmarks)
- [ ] Refresh page
- [ ] Check workflow permissions (read/write)

---

### Download Failed

**If download doesn't work:**

- [ ] Try different browser
- [ ] Check GitHub status
- [ ] Use GitHub CLI: `gh run download RUN_ID`

---

## Success Criteria

✅ **All checks passed:**

- [ ] All 4 jobs completed successfully
- [ ] All artifacts available for download
- [ ] Libraries verified as correct file type
- [ ] Unity package structure correct
- [ ] No build errors or warnings
- [ ] All 68 tests passed on all platforms

**If all checked:** CI/CD is working! 🎉

---

## Next Steps

After successful CI/CD setup:

1. **Enable branch protection:**
   - Settings → Branches
   - Require status checks before merge

2. **Add status badge to README:**
   ```markdown
   [![Build Status](https://github.com/USER/SpectraCore/workflows/Multi-Platform%20Build/badge.svg)]
   ```

3. **Set up notifications:**
   - Get alerted on build failures
   - Review regularly

4. **Document for contributors:**
   - Add CI/CD guide link to README
   - Explain workflow to team

5. **Monitor over time:**
   - Track build times
   - Watch for flaky tests
   - Update as needed

---

## Final Verification

**Complete CI/CD setup includes:**

- [x] Workflow file created
- [ ] Workflow pushed to GitHub
- [ ] First build completed successfully
- [ ] All artifacts downloaded and verified
- [ ] Unity package tested (optional)
- [ ] Documentation reviewed
- [ ] Team notified

**Mark CI-001 as complete when all checked!**

---

## Quick Commands Reference

```bash
# Push workflow
git add .github/workflows/build-multiplatform.yml
git commit -m "Add CI/CD workflow"
git push origin main

# Check status (with GitHub CLI)
gh run list

# Watch current run
gh run watch

# Download artifacts
gh run download

# View logs
gh run view --log

# Trigger manual run (if workflow_dispatch enabled)
gh workflow run "Multi-Platform Build"
```

---

## Support

**Need help?**

1. Check [CI/CD Guide](docs/CI_CD_Guide.md)
2. Review GitHub Actions logs
3. Test locally first
4. Search GitHub Actions docs
5. Ask for help with specific error message

**Common resources:**
- GitHub Actions docs: https://docs.github.com/en/actions
- CMake docs: https://cmake.org/documentation/
- This checklist!
