# Creating GitHub Release for v1.0.0

This guide explains how to create the official GitHub Release for SpectraCore v1.0.0.

---

## Prerequisites

- Git tag `v1.0.0` has been pushed to GitHub ✅ (Already done)
- You have maintainer access to the repository
- All CI/CD workflows have passed

---

## Method 1: GitHub Web Interface (Recommended)

### Step 1: Navigate to Releases

1. Go to https://github.com/yogt1984/SpectraCore
2. Click on **"Releases"** (right sidebar, or `/releases`)
3. Click **"Draft a new release"** button

### Step 2: Select Tag

1. Click the **"Choose a tag"** dropdown
2. Select **`v1.0.0`** from the list
   - It should show "Existing tag"
   - If not visible, ensure the tag was pushed correctly

### Step 3: Fill Release Information

**Release Title:**
```
SpectraCore v1.0.0 - Production Ready
```

**Release Description:**

Copy the entire contents of `GITHUB_RELEASE_TEMPLATE.md` into the description field.

Alternatively, you can use a shorter version:

```markdown
# SpectraCore v1.0.0 - Production Ready 🎉

**First official production release!**

A high-performance DSP library for Unity with MATLAB-compatible APIs, cross-platform support, and professional editor tools.

## 🚀 Key Features

- 13 MATLAB-compatible DSP functions (FFT, filters, spectral analysis)
- Cross-platform validated: Linux, Windows, macOS (Universal Binary)
- Professional Unity integration with editor tools
- 13,000+ lines of comprehensive documentation
- 204 automated tests (100% pass rate)

## 📦 Installation

**Unity Package Manager:**
```
Window → Package Manager → + → Add package from git URL
https://github.com/yogt1984/SpectraCore.git
```

## 📚 Documentation

- [Full Release Notes](https://github.com/yogt1984/SpectraCore/blob/main/RELEASE_NOTES_v1.0.0.md)
- [Installation Guide](https://github.com/yogt1984/SpectraCore/blob/main/INSTALLATION.md)
- [API Reference](https://github.com/yogt1984/SpectraCore/blob/main/docs/API_Reference.md)
- [Tutorials](https://github.com/yogt1984/SpectraCore/blob/main/docs/Tutorials.md)
- [Examples](https://github.com/yogt1984/SpectraCore/blob/main/docs/ExampleProjects.md)

## 📦 Platform Libraries

Download the appropriate library for your platform from the release assets below.

**Happy coding! 🎵**
```

### Step 4: Attach Artifacts (Optional but Recommended)

You can download the latest build artifacts from CI/CD and attach them:

1. Go to https://github.com/yogt1984/SpectraCore/actions
2. Click on the latest successful "CI" workflow run
3. Download the following artifacts:
   - `native-ubuntu-latest`
   - `native-windows-latest`
   - `native-macos-latest`

4. Back in the release creation page, drag and drop these ZIP files to the "Attach binaries" section

**Rename for clarity:**
- `native-ubuntu-latest.zip` → `SpectraCore-v1.0.0-linux-x86_64.zip`
- `native-windows-latest.zip` → `SpectraCore-v1.0.0-windows-x64.zip`
- `native-macos-latest.zip` → `SpectraCore-v1.0.0-macos-universal.zip`

### Step 5: Release Options

- ✅ **Set as the latest release** (check this)
- ⬜ **Set as a pre-release** (leave unchecked)
- ⬜ **Create a discussion for this release** (optional)

### Step 6: Publish

Click **"Publish release"** button

---

## Method 2: GitHub CLI (If Available)

If you have GitHub CLI installed:

```bash
cd /home/onat/SpectraCore

# Create release using the template
gh release create v1.0.0 \
  --title "SpectraCore v1.0.0 - Production Ready" \
  --notes-file GITHUB_RELEASE_TEMPLATE.md \
  --latest

# Optionally attach artifacts
# First download them from GitHub Actions, then:
gh release upload v1.0.0 \
  native-ubuntu-latest.zip \
  native-windows-latest.zip \
  native-macos-latest.zip
```

**To install GitHub CLI:**
```bash
# Ubuntu/Debian
sudo apt install gh

# macOS
brew install gh

# Then authenticate
gh auth login
```

---

## Method 3: Using the GitHub API

If you prefer automation:

```bash
# Create release
curl -X POST \
  -H "Authorization: token YOUR_GITHUB_TOKEN" \
  -H "Accept: application/vnd.github.v3+json" \
  https://api.github.com/repos/yogt1984/SpectraCore/releases \
  -d '{
    "tag_name": "v1.0.0",
    "name": "SpectraCore v1.0.0 - Production Ready",
    "body": "See RELEASE_NOTES_v1.0.0.md for complete details.",
    "draft": false,
    "prerelease": false
  }'
```

---

## Verification

After creating the release:

### 1. Check Release Page

Visit: https://github.com/yogt1984/SpectraCore/releases/tag/v1.0.0

Verify:
- ✅ Title shows "SpectraCore v1.0.0 - Production Ready"
- ✅ Description is formatted correctly
- ✅ Badge shows "Latest"
- ✅ Source code archives are available
- ✅ Artifacts are attached (if applicable)

### 2. Test Installation

Test that users can install via Unity Package Manager:

1. Create a new Unity project (or use existing)
2. Go to `Window → Package Manager`
3. Click `+` → `Add package from git URL`
4. Enter: `https://github.com/yogt1984/SpectraCore.git`
5. Verify package installs successfully

### 3. Verify Release Badge

Check that the README badge updates:

Visit: https://github.com/yogt1984/SpectraCore

The badge should show: `release v1.0.0`

---

## Post-Release Tasks

### 1. Update README.md Release Link (If Needed)

If you hardcoded the release link in README.md, verify it works:

https://github.com/yogt1984/SpectraCore/releases/tag/v1.0.0

### 2. Announce Release (Optional)

Consider announcing on:
- Project discussions
- Social media
- Unity forums
- Reddit (r/Unity3D, r/gamedev)
- Twitter/LinkedIn

### 3. Monitor Feedback

Watch for:
- GitHub Issues
- Discussions
- Questions about installation

---

## Troubleshooting

### Issue: Tag Not Found

**Problem**: `v1.0.0` doesn't appear in tag dropdown

**Solution**:
```bash
# Verify tag exists locally
git tag -l

# Verify tag was pushed
git ls-remote --tags origin

# If missing, push again
git push origin v1.0.0
```

### Issue: Can't Create Release

**Problem**: "You don't have permission" error

**Solution**:
- Ensure you have "Write" access to the repository
- Check that you're logged in to the correct GitHub account

### Issue: Artifacts Too Large

**Problem**: Artifact upload fails due to size

**Solution**:
- GitHub has a 2GB limit per file
- Our artifacts are ~500KB, well within limits
- If needed, compress further with `zip -9`

---

## Quick Checklist

Before publishing:

- [ ] Tag `v1.0.0` exists and is pushed
- [ ] CI/CD workflow passed successfully
- [ ] Release title: "SpectraCore v1.0.0 - Production Ready"
- [ ] Release description is complete
- [ ] "Latest release" is checked
- [ ] "Pre-release" is unchecked
- [ ] Artifacts attached (optional)
- [ ] Release is published

After publishing:

- [ ] Release page loads correctly
- [ ] Installation via UPM works
- [ ] README badge shows v1.0.0
- [ ] Documentation links work

---

## Success!

Once published, your release will be available at:

**Release Page**: https://github.com/yogt1984/SpectraCore/releases/tag/v1.0.0

**Installation**: Users can now install via Unity Package Manager:
```
https://github.com/yogt1984/SpectraCore.git
```

---

**Congratulations on the v1.0.0 release! 🎉**
