# Quick Start: Cross-Platform Building from Linux

## TL;DR - What's Feasible?

From your Linux machine **right now**, you can:

### ✅ Fully Achievable
- Build Windows DLL via cross-compilation (10 minutes)
- Build macOS library via GitHub Actions (free, automated)
- Create production-ready binaries for all platforms

### ⚠️ Requires Additional Resources
- Testing on Windows (needs Windows VM or PC)
- Testing on macOS (needs Mac hardware or cloud)

### ❌ Not Possible from Linux
- Running Unity Editor on Windows/macOS to test P/Invoke

**Bottom line:** You can BUILD for all platforms from Linux, but TESTING requires those platforms.

---

## Option 1: Build Windows DLL Right Now (10 min)

### Step 1: Install MinGW

```bash
sudo apt-get update
sudo apt-get install mingw-w64 cmake
```

### Step 2: Run the build script

```bash
cd /home/onat/SpectraCore
./build-windows-from-linux.sh
```

**Output:**
```
✓ SUCCESS: Windows DLL built successfully
Output: /home/onat/SpectraCore/native/build-windows/lib/spectra.dll
✓ Copied to: unity/Runtime/Plugins/Windows/x86_64/spectra.dll
```

### Step 3: Verify the DLL

```bash
file native/build-windows/lib/spectra.dll
# Expected: PE32+ executable (DLL) (console) x86-64, for MS Windows
```

**You now have a Windows DLL** but need a Windows machine to test it.

---

## Option 2: Use GitHub Actions for All Platforms (Best)

### Step 1: Push to GitHub

```bash
git add .
git commit -m "Add multi-platform CI workflow"
git push origin main
```

### Step 2: Watch the build

1. Go to your GitHub repository
2. Click "Actions" tab
3. Wait ~5-10 minutes for build to complete

### Step 3: Download artifacts

After build completes:
1. Click on the workflow run
2. Scroll to "Artifacts" section
3. Download:
   - `libspectra-linux` (Linux .so)
   - `spectra-windows` (Windows .dll)
   - `libspectra-macos` (macOS .dylib - universal binary)

**You now have production binaries for all platforms** without any local cross-compilation!

---

## Option 3: Quick Windows Testing (Low Cost)

If you want to actually TEST on Windows:

### Free Option: Windows in Cloud

**Microsoft provides free credits:**
- Azure: $200 free credit (first month)
- AWS: Free tier with Windows instance

### Cheap Option: Rent a Windows VM

**Cost: ~$0.10/hour** for testing only

```bash
# 1. Create Windows VM on Azure/AWS
# 2. RDP into the VM
# 3. Install Unity Hub
# 4. Copy your DLL
# 5. Run tests (30 min)
# 6. Destroy VM

Total cost: $0.10 × 1 hour = $0.10
```

### Local Option: Windows in VirtualBox

**Free but slower:**

```bash
# 1. Install VirtualBox
sudo apt-get install virtualbox

# 2. Download Windows 10 ISO (legal evaluation version)
# From: https://www.microsoft.com/en-us/software-download/windows10ISO

# 3. Create Windows VM (evaluation is valid for 90 days)
# 4. Install Unity Editor
# 5. Test your DLL
```

**Cost:** Free (90-day eval license)
**Time:** ~2 hours setup, then reusable

---

## Recommended Workflow for UNITY-001/002

### Week 1: Building (Linux Only)

**Day 1-2: Get builds working**
```bash
# 1. Test MinGW cross-compilation
./build-windows-from-linux.sh

# 2. Set up GitHub Actions
git push  # Triggers CI build

# 3. Verify all platforms build
# Check GitHub Actions results
```

**Deliverable:** DLLs for all platforms ✅

### Week 2: Testing (Requires other platforms)

**For Windows (UNITY-001):**
```bash
# Option A: Use free Azure credit
1. Create Azure account (free $200 credit)
2. Spin up Windows 10 VM
3. Install Unity Editor
4. Transfer DLL
5. Run Unity Test Runner
6. Take screenshots of passing tests
7. Destroy VM

Total cost: ~$1 (1 hour of VM time)
```

**For macOS (UNITY-002):**
```bash
# Option A: Find a Mac user (beta tester)
1. Build dylib via GitHub Actions (free)
2. Send to Mac user for testing
3. They run Unity Test Runner
4. Report results back

Cost: Free
```

```bash
# Option B: Rent Mac in cloud
1. Use MacStadium or AWS Mac instances
2. ~$1/hour minimum
3. Install Unity, test, done

Cost: ~$2-5 for testing session
```

```bash
# Option C: Delay macOS testing
1. Mark UNITY-002 as "built but not tested"
2. Wait until you have Mac access
3. Build is verified via CI, tests are there
4. Testing can happen later

Cost: Free, but incomplete
```

---

## What You Can Complete Today (Linux Only)

### Immediate (Next 30 minutes)

1. **Install MinGW:**
   ```bash
   sudo apt-get install mingw-w64 cmake
   ```

2. **Build Windows DLL:**
   ```bash
   ./build-windows-from-linux.sh
   ```

3. **Verify output:**
   ```bash
   ls -lh native/build-windows/lib/spectra.dll
   file native/build-windows/lib/spectra.dll
   ```

4. **Set up CI:**
   ```bash
   git add .github/workflows/build-multiplatform.yml
   git add native/cmake/mingw-w64.cmake
   git add build-windows-from-linux.sh
   git add docs/CrossPlatformBuildGuide.md
   git commit -m "Add cross-platform build support"
   git push
   ```

**Result:**
- ✅ Windows DLL built and ready
- ✅ CI pipeline set up for automatic builds
- ✅ macOS build will be available via GitHub Actions

### This Week (With minimal external resources)

5. **Test Windows DLL:**
   - Rent Windows VM for 1 hour ($0.10-1.00)
   - Or use free Azure/AWS credit
   - Run Unity Test Runner
   - Document results

6. **macOS build:**
   - Let GitHub Actions handle it (free)
   - Download artifact when ready

7. **Update TASKS.md:**
   - Mark UNITY-001 as "Complete (Windows DLL tested on VM)"
   - Mark UNITY-002 as "Partial (macOS build available via CI)"

---

## Decision Matrix

| Approach | Time | Cost | Completeness | Recommendation |
|----------|------|------|--------------|----------------|
| **Build Windows DLL locally** | 10 min | Free | 50% (build only) | ✅ Do now |
| **GitHub Actions (all platforms)** | 5 min setup | Free | 80% (builds + Linux tests) | ✅ Do now |
| **Windows VM testing** | 1 hour | $0-5 | 95% (Windows verified) | ✅ Do this week |
| **macOS cloud testing** | 1 hour | $2-5 | 100% (all platforms) | ⚠️ Optional |
| **Buy a Mac Mini** | N/A | $600 | 100% | ❌ Not worth it |
| **Wait for Mac access** | Indefinite | Free | 80% | ⚠️ If budget-constrained |

---

## My Recommendation

**For your situation (Linux developer, no Mac):**

### Phase 1: Today (Free)
```bash
1. Run ./build-windows-from-linux.sh
2. Push to GitHub (triggers Actions)
3. Verify builds complete on all platforms
```
**Result:** All platform binaries ready ✅

### Phase 2: This Week ($0-5)
```bash
1. Rent Windows VM for 1 hour
2. Test Windows DLL in Unity
3. Document results
4. Mark UNITY-001 as complete
```
**Result:** Windows verified ✅

### Phase 3: Mark macOS as "Available"
```bash
1. Note in TASKS.md: "macOS build available via CI"
2. Add note: "Testing deferred pending Mac hardware access"
3. Move forward with other tasks
```
**Result:** macOS build exists, testing deferred ⚠️

### Total Investment
- **Time:** 2-3 hours
- **Money:** $0-5 (optional Windows VM)
- **Completeness:** 90% (Linux + Windows verified, macOS built but untested)

---

## Next Steps

**Right now, run:**

```bash
cd /home/onat/SpectraCore

# 1. Install prerequisites
sudo apt-get install mingw-w64 cmake

# 2. Build Windows DLL
./build-windows-from-linux.sh

# 3. Set up CI
git add .github/workflows/build-multiplatform.yml
git add native/cmake/mingw-w64.cmake
git add build-windows-from-linux.sh
git add docs/CrossPlatformBuildGuide.md
git add QUICK_START_CROSS_PLATFORM.md
git commit -m "Add cross-platform build infrastructure"
git push

# 4. Watch GitHub Actions build all platforms
# Go to: https://github.com/YOUR_USERNAME/SpectraCore/actions
```

**Then decide:** Rent a Windows VM for testing, or move on to other tasks with builds ready?

---

## Questions?

**"Can I skip Windows/macOS testing entirely?"**
- For personal projects: Yes, you have the builds
- For release: Recommended to test at least Windows
- For commercial: Must test both

**"Will the Windows DLL work if built on Linux?"**
- Yes, if MinGW is properly configured (which our script handles)
- 95%+ success rate for well-written C++
- Still recommended to test on real Windows

**"Do I need a Mac for macOS development?"**
- For building: No (use GitHub Actions)
- For testing Unity integration: Yes (or cloud Mac)
- For releasing: Recommended to test on real hardware

**"Can I mark UNITY-001/002 as complete without testing?"**
- UNITY-001: Partially (build complete, testing pending)
- UNITY-002: Partially (build via CI, testing pending)
- Document: "Builds available, manual testing deferred"
