# Contributing to SpectraCore

Thank you for your interest in contributing to SpectraCore! This document provides guidelines and instructions for contributing to the project.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [How to Contribute](#how-to-contribute)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Features](#suggesting-features)
- [Community](#community)

---

## Code of Conduct

This project adheres to a Code of Conduct that all contributors are expected to follow. Please read [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) before contributing.

---

## Getting Started

### Prerequisites

- **Git**: For version control
- **CMake** 3.20 or newer
- **C++17 compatible compiler**:
  - Linux: GCC 7+ or Clang 6+
  - Windows: MSVC 2019+ or MinGW-w64
  - macOS: Xcode 10+ (Clang)
- **Unity** 2021.3 LTS or newer (for Unity integration work)
- **Python** 3.7+ (for scripts and tools)

### Setting Up Development Environment

1. **Fork the repository** on GitHub

2. **Clone your fork:**
   ```bash
   git clone https://github.com/YOUR_USERNAME/SpectraCore.git
   cd SpectraCore
   ```

3. **Add upstream remote:**
   ```bash
   git remote add upstream https://github.com/yogt1984/SpectraCore.git
   ```

4. **Build the native library:**
   ```bash
   cd native
   cmake -B build -DCMAKE_BUILD_TYPE=Debug
   cmake --build build
   ```

5. **Run tests:**
   ```bash
   cd build
   ctest --output-on-failure
   ```

---

## How to Contribute

### Types of Contributions

We welcome contributions in many forms:

- **Bug fixes**: Fixing issues in existing code
- **New features**: Implementing new DSP functions or Unity components
- **Documentation**: Improving docs, tutorials, or examples
- **Tests**: Adding test coverage
- **Performance improvements**: Optimizing existing algorithms
- **Platform support**: Enabling new platforms
- **Examples**: Creating new sample projects

---

## Development Workflow

### 1. Create a Feature Branch

```bash
# Update main branch
git checkout main
git pull upstream main

# Create feature branch
git checkout -b feature/your-feature-name
# or
git checkout -b fix/your-bug-fix
```

**Branch naming conventions:**
- `feature/feature-name` - New features
- `fix/bug-name` - Bug fixes
- `docs/topic` - Documentation changes
- `test/test-name` - Test additions
- `refactor/component-name` - Code refactoring
- `perf/optimization-name` - Performance improvements

### 2. Make Your Changes

- Write clean, readable code
- Follow the coding standards (see below)
- Add tests for new functionality
- Update documentation as needed

### 3. Test Your Changes

```bash
# Build and run native tests
cd native
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure

# Run Unity tests (if applicable)
# Open Unity project and run tests via Test Runner
```

### 4. Commit Your Changes

Follow conventional commit format (see [Commit Messages](#commit-messages)):

```bash
git add .
git commit -m "feat: add bandpass filter implementation"
```

### 5. Push to Your Fork

```bash
git push origin feature/your-feature-name
```

### 6. Create Pull Request

- Go to GitHub and create a Pull Request from your fork to `yogt1984/SpectraCore:main`
- Fill in the PR template with details
- Link any related issues

---

## Coding Standards

### C++ Code (Native Library)

**Style:**
- **Indentation**: 4 spaces (no tabs)
- **Naming**:
  - Classes: `PascalCase` (e.g., `FFTAnalyzer`)
  - Functions: `camelCase` (e.g., `computeSpectrum`)
  - Variables: `camelCase` (e.g., `fftSize`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_FFT_SIZE`)
  - Private members: `m_` prefix (e.g., `m_buffer`)
- **Braces**: Opening brace on same line
- **Header guards**: `#pragma once`

**Example:**
```cpp
#pragma once

namespace spectra {

class FFTAnalyzer {
public:
    FFTAnalyzer(size_t fftSize);
    ~FFTAnalyzer();

    void computeSpectrum(const float* input, float* output);

private:
    size_t m_fftSize;
    float* m_buffer;
};

} // namespace spectra
```

**Best Practices:**
- Use RAII for resource management
- Prefer `std::unique_ptr` and `std::shared_ptr` over raw pointers
- Avoid manual memory management
- Use `const` wherever possible
- Document public APIs with comments
- Keep functions focused and small (<50 lines typically)

### C# Code (Unity Integration)

**Style:**
- **Indentation**: 4 spaces (no tabs)
- **Naming**:
  - Classes: `PascalCase` (e.g., `FFTAnalyzer`)
  - Methods: `PascalCase` (e.g., `GetMagnitudeSpectrum`)
  - Properties: `PascalCase` (e.g., `FftSize`)
  - Local variables: `camelCase` (e.g., `fftSize`)
  - Private fields: `camelCase` (e.g., `fftHandle`)
- **Braces**: Opening brace on new line

**Example:**
```csharp
using System;
using UnityEngine;

namespace SpectraCore
{
    public class FFTAnalyzer : IDisposable
    {
        private IntPtr fftHandle;

        public int Size { get; private set; }

        public FFTAnalyzer(int fftSize)
        {
            Size = fftSize;
            fftHandle = SpectraNative.spectra_fft_create(fftSize);
        }

        public float[] GetMagnitudeSpectrum(float[] input)
        {
            // Implementation
        }

        public void Dispose()
        {
            if (fftHandle != IntPtr.Zero)
            {
                SpectraNative.spectra_fft_destroy(fftHandle);
                fftHandle = IntPtr.Zero;
            }
        }
    }
}
```

**Best Practices:**
- Implement `IDisposable` for classes wrapping native resources
- Use `SafeHandle` pattern for native handle management
- Always dispose of native resources
- Use properties instead of getters/setters
- Document public APIs with XML comments
- Follow Unity C# coding conventions

### Documentation

- **Code comments**: Document complex algorithms and non-obvious behavior
- **API documentation**: All public APIs must have documentation
- **Markdown**: Use GitHub-flavored Markdown for docs
- **Examples**: Include code examples in documentation
- **Keep it updated**: Update docs when changing functionality

---

## Testing Guidelines

### Native C++ Tests

**Location**: `native/tests/unit/`

**Framework**: Google Test (included via CMake)

**Example:**
```cpp
#include <gtest/gtest.h>
#include "spectra/fft/fft.hpp"

TEST(FFTTest, ForwardTransform_ValidInput_ReturnsCorrectSize)
{
    spectra::FFT fft(256);
    std::vector<float> input(256, 0.0f);
    std::vector<std::complex<float>> output;

    fft.forward(input.data(), output);

    EXPECT_EQ(output.size(), 129);  // N/2 + 1 for real input
}

TEST(FFTTest, RoundTrip_RecreatesSignal)
{
    spectra::FFT fft(256);
    std::vector<float> input(256);
    // ... test implementation

    // Assertions
    EXPECT_NEAR(reconstructed[i], input[i], 1e-5);
}
```

**Coverage Requirements:**
- All new functions must have tests
- Aim for >80% code coverage
- Test edge cases (empty input, null pointers, invalid sizes)
- Test error handling

### Unity C# Tests

**Location**: `unity/Tests/Editor/`

**Framework**: Unity Test Framework (NUnit)

**Example:**
```csharp
using NUnit.Framework;
using SpectraCore;

public class FFTAnalyzerTests
{
    [Test]
    public void FFTAnalyzer_Create_ValidSize_Success()
    {
        using (var fft = new FFTAnalyzer(256))
        {
            Assert.AreEqual(256, fft.Size);
            Assert.AreEqual(129, fft.SpectrumSize);
        }
    }

    [Test]
    public void GetMagnitudeSpectrum_ValidInput_ReturnsCorrectSize()
    {
        using (var fft = new FFTAnalyzer(256))
        {
            float[] input = new float[256];
            float[] spectrum = fft.GetMagnitudeSpectrum(input);

            Assert.AreEqual(129, spectrum.Length);
        }
    }
}
```

### Running Tests

```bash
# Native tests
cd native/build
ctest --output-on-failure

# Unity tests
# Open Unity project, Window → General → Test Runner → Run All
```

---

## Commit Messages

We follow **Conventional Commits** specification.

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `test`: Adding or updating tests
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `build`: Build system changes
- `ci`: CI/CD changes
- `chore`: Maintenance tasks

### Scope (Optional)

- `fft`: FFT-related changes
- `filter`: Filter-related changes
- `stft`: STFT-related changes
- `unity`: Unity integration
- `docs`: Documentation
- `ci`: CI/CD

### Examples

```bash
# Feature
git commit -m "feat(filter): add bandpass filter implementation"

# Bug fix
git commit -m "fix(fft): resolve memory leak in FFT destructor"

# Documentation
git commit -m "docs: add tutorial for beat detection"

# Test
git commit -m "test(filter): add unit tests for Chebyshev filters"

# With body
git commit -m "feat(stft): add streaming mode support

Implemented streaming STFT with push/pop interface.
Includes frame buffering and overlap handling.

Closes #42"
```

### Co-Authoring

If collaborating with AI:
```
feat: add new feature

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

---

## Pull Request Process

### Before Submitting

- [ ] Code builds successfully on all platforms (Linux, Windows, macOS)
- [ ] All tests pass
- [ ] New code has tests
- [ ] Documentation is updated
- [ ] Commit messages follow conventional format
- [ ] Code follows style guidelines
- [ ] No merge conflicts with main branch

### PR Title

Use conventional commit format:
```
feat: add bandpass filter implementation
fix: resolve FFT memory leak
docs: update installation guide
```

### PR Description Template

```markdown
## Description
Brief description of changes.

## Type of Change
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to change)
- [ ] Documentation update

## Testing
Describe tests you ran and how to reproduce.

## Checklist
- [ ] My code follows the style guidelines
- [ ] I have performed a self-review
- [ ] I have commented my code where needed
- [ ] I have updated the documentation
- [ ] My changes generate no new warnings
- [ ] I have added tests that prove my fix/feature works
- [ ] New and existing tests pass locally

## Related Issues
Closes #123
Fixes #456
```

### Review Process

1. **Automated checks** run (CI/CD, linting, tests)
2. **Maintainer review** (1-2 reviewers)
3. **Feedback** addressed
4. **Approval** and merge

**Review criteria:**
- Code quality and readability
- Test coverage
- Documentation completeness
- Performance impact
- Breaking changes justified

---

## Reporting Bugs

### Before Reporting

1. **Search existing issues** to avoid duplicates
2. **Try the latest version** to see if it's already fixed
3. **Gather information** about the bug

### Bug Report Template

```markdown
**Describe the bug**
A clear description of what the bug is.

**To Reproduce**
Steps to reproduce the behavior:
1. Call function '...'
2. With parameters '...'
3. See error

**Expected behavior**
What you expected to happen.

**Actual behavior**
What actually happened.

**Environment:**
- OS: [e.g., Windows 10, macOS 12, Ubuntu 20.04]
- Unity version: [e.g., 2021.3.16f1]
- SpectraCore version: [e.g., 1.0.0]

**Additional context**
Add any other context, screenshots, or code samples.
```

**Submit at**: https://github.com/yogt1984/SpectraCore/issues/new

---

## Suggesting Features

### Feature Request Template

```markdown
**Is your feature request related to a problem?**
A clear description of the problem. Ex. I'm always frustrated when [...]

**Describe the solution you'd like**
Clear description of what you want to happen.

**Describe alternatives you've considered**
Other solutions or features you've considered.

**Additional context**
Any other context, examples, or references.

**MATLAB equivalent (if applicable)**
If this exists in MATLAB, provide the function name and reference.
```

**Submit at**: https://github.com/yogt1984/SpectraCore/issues/new

---

## Community

### Communication Channels

- **GitHub Issues**: Bug reports, feature requests
- **GitHub Discussions**: Questions, ideas, showcases
- **Pull Requests**: Code contributions

### Getting Help

- Read the [documentation](docs/)
- Search [existing issues](https://github.com/yogt1984/SpectraCore/issues)
- Ask in [discussions](https://github.com/yogt1984/SpectraCore/discussions)

### Recognition

Contributors are recognized in:
- Release notes
- CHANGELOG.md
- GitHub contributors page

---

## License

By contributing to SpectraCore, you agree that your contributions will be licensed under the MIT License.

---

## Questions?

If you have questions about contributing, feel free to:
- Open a discussion: https://github.com/yogt1984/SpectraCore/discussions
- Comment on an issue

---

**Thank you for contributing to SpectraCore! 🎵**
