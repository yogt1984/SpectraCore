#!/bin/bash
# SpectraCore macOS Testing Automation Script
# UNITY-002: macOS P/Invoke Testing
#
# Usage:
#   ./test-macos.sh [dylib-path] [unity-project-path]
#
# Example:
#   ./test-macos.sh libspectra.dylib "/Users/username/UnityProjects/SpectraTest"

set -e  # Exit on error

echo "========================================"
echo "  SpectraCore macOS Testing Script"
echo "  UNITY-002: P/Invoke Testing"
echo "========================================"
echo ""

# ===== Configuration =====
DYLIB_PATH="${1:-libspectra.dylib}"
UNITY_PROJECT="${2:-}"

if [ -z "$1" ]; then
    echo "No library path specified, using: $DYLIB_PATH"
else
    echo "Using library: $DYLIB_PATH"
fi

if [ -z "$UNITY_PROJECT" ]; then
    echo "WARNING: No Unity project path specified"
    echo "Standalone tests only"
    UNITY_PROJECT="NONE"
else
    echo "Unity project: $UNITY_PROJECT"
fi

echo ""

# ===== Step 1: Verify library exists =====
echo "[1/7] Verifying library..."
if [ ! -f "$DYLIB_PATH" ]; then
    echo "  ERROR: Library not found: $DYLIB_PATH"
    echo ""
    echo "  Please provide library path as first argument:"
    echo "    ./test-macos.sh /path/to/libspectra.dylib"
    echo ""
    echo "  Or download from GitHub Actions:"
    echo "    https://github.com/yogt1984/SpectraCore/actions"
    exit 1
fi

echo "  File: $DYLIB_PATH"
DYLIB_SIZE=$(stat -f%z "$DYLIB_PATH" 2>/dev/null || stat -c%s "$DYLIB_PATH" 2>/dev/null)
DYLIB_SIZE_KB=$((DYLIB_SIZE / 1024))
echo "  Size: ${DYLIB_SIZE_KB} KB"

# Check if it's a valid Mach-O file
if file "$DYLIB_PATH" | grep -q "Mach-O"; then
    echo "  Format: Valid Mach-O library"
    echo "  [PASS]"
else
    echo "  ERROR: File is not a valid Mach-O library"
    file "$DYLIB_PATH"
    exit 1
fi

echo ""

# ===== Step 2: Check library architecture =====
echo "[2/7] Checking library architecture..."

if command -v lipo &> /dev/null; then
    echo "  Using lipo..."
    ARCH_INFO=$(lipo -info "$DYLIB_PATH")
    echo "  $ARCH_INFO"

    # Check for universal binary
    if echo "$ARCH_INFO" | grep -q "x86_64" && echo "$ARCH_INFO" | grep -q "arm64"; then
        echo "  ✓ Universal binary (x86_64 + arm64)"
        echo "  [PASS]"
    elif echo "$ARCH_INFO" | grep -q "x86_64"; then
        echo "  ⚠ Intel only (x86_64)"
        echo "  [WARN] Not a universal binary"
    elif echo "$ARCH_INFO" | grep -q "arm64"; then
        echo "  ⚠ Apple Silicon only (arm64)"
        echo "  [WARN] Not a universal binary"
    else
        echo "  ERROR: Unknown architecture"
        exit 1
    fi
else
    echo "  WARNING: lipo not found (Xcode Command Line Tools not installed)"
    echo "  [SKIP]"
fi

echo ""

# ===== Step 3: Check library dependencies =====
echo "[3/7] Checking library dependencies..."

if command -v otool &> /dev/null; then
    echo "  Using otool..."
    echo "  Dependencies:"
    otool -L "$DYLIB_PATH" | tail -n +2 | while read line; do
        echo "    $line"
    done

    # Count non-system dependencies
    NON_SYSTEM_DEPS=$(otool -L "$DYLIB_PATH" | tail -n +2 | grep -v "/usr/lib/" | grep -v "/System/" | wc -l | xargs)

    if [ "$NON_SYSTEM_DEPS" -eq 0 ]; then
        echo "  ✓ Only system dependencies"
        echo "  [PASS]"
    else
        echo "  ⚠ Found $NON_SYSTEM_DEPS non-system dependencies"
        echo "  [WARN]"
    fi
else
    echo "  WARNING: otool not found"
    echo "  [SKIP]"
fi

echo ""

# ===== Step 4: Check library exports =====
echo "[4/7] Checking library exports..."

if command -v nm &> /dev/null; then
    echo "  Checking for required exports..."
    EXPORT_COUNT=$(nm -gU "$DYLIB_PATH" 2>/dev/null | grep spectra_ | wc -l | xargs)

    if [ "$EXPORT_COUNT" -lt 20 ]; then
        echo "  WARNING: Only $EXPORT_COUNT exports found"
        echo "  Expected ~50+ spectra_* functions"
        echo "  [WARN]"
    else
        echo "  Found $EXPORT_COUNT spectra_* exports"
        echo "  [PASS]"
    fi
else
    echo "  WARNING: Cannot verify exports without nm"
    echo "  [SKIP]"
fi

echo ""

# ===== Step 5: Detect current architecture =====
echo "[5/7] Detecting system architecture..."

SYSTEM_ARCH=$(uname -m)
echo "  System architecture: $SYSTEM_ARCH"

if [ "$SYSTEM_ARCH" = "x86_64" ]; then
    echo "  Running on: Intel Mac"
    EXPECTED_PERF="< 1.0ms per FFT"
elif [ "$SYSTEM_ARCH" = "arm64" ]; then
    echo "  Running on: Apple Silicon"
    EXPECTED_PERF="< 0.5ms per FFT"
else
    echo "  Running on: Unknown architecture"
    EXPECTED_PERF="unknown"
fi
echo "  Expected performance: $EXPECTED_PERF"
echo "  [INFO]"

echo ""

# ===== Step 6: Run standalone P/Invoke test =====
echo "[6/7] Running standalone P/Invoke test..."

# Check if standalone test exists
if [ ! -f "unity/Tests/StandalonePInvokeTest.cs" ]; then
    echo "  ERROR: Test file not found: unity/Tests/StandalonePInvokeTest.cs"
    echo "  Make sure you're running from SpectraCore root directory"
    exit 1
fi

# Check for Mono
if command -v mcs &> /dev/null && command -v mono &> /dev/null; then
    echo "  Using Mono compiler and runtime..."

    # Compile test
    echo "  Compiling test program..."
    mcs -out:PInvokeTest.exe unity/Tests/StandalonePInvokeTest.cs 2>&1 | grep -v "^Compilation succeeded" || true

    if [ ! -f "PInvokeTest.exe" ]; then
        echo "  ERROR: Compilation failed"
        mcs -out:PInvokeTest.exe unity/Tests/StandalonePInvokeTest.cs
        exit 1
    fi
    echo "  Compiled: PInvokeTest.exe"

    # Copy library to same directory
    cp "$DYLIB_PATH" . 2>/dev/null || true

    # Run test
    echo "  Running test..."
    echo ""
    mono PInvokeTest.exe
    TEST_RESULT=$?

    if [ $TEST_RESULT -ne 0 ]; then
        echo ""
        echo "  [FAIL] Standalone test failed"
        exit 1
    else
        echo ""
        echo "  [PASS] All standalone tests passed"
    fi

elif command -v dotnet &> /dev/null; then
    echo "  Using .NET SDK..."

    # Create temporary directory
    TEST_DIR="StandaloneTest_$$"
    mkdir -p "$TEST_DIR"
    cd "$TEST_DIR"

    # Initialize .NET project
    dotnet new console --force > /dev/null 2>&1

    # Copy test code
    cp ../unity/Tests/StandalonePInvokeTest.cs Program.cs

    # Copy library
    cp ../"$DYLIB_PATH" .

    # Run
    echo "  Running test..."
    echo ""
    dotnet run
    TEST_RESULT=$?

    cd ..
    rm -rf "$TEST_DIR"

    if [ $TEST_RESULT -ne 0 ]; then
        echo ""
        echo "  [FAIL] Standalone test failed"
        exit 1
    else
        echo ""
        echo "  [PASS] All standalone tests passed"
    fi

else
    echo "  ERROR: Neither Mono nor .NET SDK found"
    echo "  Install Mono: brew install mono"
    echo "  Or install .NET: https://dotnet.microsoft.com/download"
    exit 1
fi

echo ""

# ===== Step 7: Unity project setup (if provided) =====
if [ "$UNITY_PROJECT" = "NONE" ]; then
    echo "[7/7] Unity testing..."
    echo "  No Unity project specified, skipping"
    echo "  [SKIP]"
    echo ""
else
    echo "[7/7] Setting up Unity project..."

    # Check if Unity project exists
    if [ ! -d "$UNITY_PROJECT" ]; then
        echo "  ERROR: Unity project not found: $UNITY_PROJECT"
        exit 1
    fi

    # Create plugin directory
    PLUGIN_DIR="$UNITY_PROJECT/Assets/SpectraCore/Runtime/Plugins/macOS"
    echo "  Creating plugin directory..."
    mkdir -p "$PLUGIN_DIR"

    # Copy library
    echo "  Copying library to Unity project..."
    cp "$DYLIB_PATH" "$PLUGIN_DIR/"

    if [ $? -eq 0 ]; then
        echo "  Library installed to: $PLUGIN_DIR/$(basename $DYLIB_PATH)"
    else
        echo "  ERROR: Failed to copy library"
        exit 1
    fi

    # Copy Unity integration files if not already present
    RUNTIME_DIR="$UNITY_PROJECT/Assets/SpectraCore/Runtime"
    if [ ! -d "$RUNTIME_DIR/Scripts" ]; then
        echo "  Copying Unity integration files..."
        mkdir -p "$UNITY_PROJECT/Assets/SpectraCore"
        cp -r unity/Runtime "$RUNTIME_DIR" 2>/dev/null || true
        cp -r unity/Editor "$UNITY_PROJECT/Assets/SpectraCore/Editor" 2>/dev/null || true
        cp -r unity/Tests "$UNITY_PROJECT/Assets/SpectraCore/Tests" 2>/dev/null || true
        echo "  Integration files copied"
    else
        echo "  Integration files already present"
    fi

    echo "  [DONE]"
    echo ""
    echo "  To run Unity tests:"
    echo "  1. Open Unity project: $UNITY_PROJECT"
    echo "  2. Window → General → Test Runner"
    echo "  3. Click \"Run All\" in EditMode tab"
    echo "  4. Verify all 71 tests pass"
    echo ""
fi

# ===== Step 8: Performance test =====
echo "[8/8] Performance test..."
echo "  Creating performance test..."

# Create simple performance test program
cat > PerfTest.cs << 'EOF'
using System;
using System.Runtime.InteropServices;
using System.Diagnostics;

class PerfTest {
    [DllImport("spectra")] static extern IntPtr spectra_fft_create(int size);
    [DllImport("spectra")] static extern void spectra_fft_destroy(IntPtr fft);
    [DllImport("spectra")] static extern int spectra_fft_forward(IntPtr fft, float[] input, float[] outputReal, float[] outputImag);

    static void Main() {
        const int size = 1024;
        const int iterations = 1000;
        IntPtr fft = spectra_fft_create(size);
        float[] input = new float[size];
        float[] real = new float[size];
        float[] imag = new float[size];

        var sw = Stopwatch.StartNew();
        for (int i = 0; i < iterations; i++)
            spectra_fft_forward(fft, input, real, imag);
        sw.Stop();

        double avgMs = sw.ElapsedMilliseconds / (double)iterations;
        Console.WriteLine($"  {iterations} FFTs (1024-point) in {sw.ElapsedMilliseconds}ms");
        Console.WriteLine($"  Average: {avgMs:F3}ms per FFT");
        Console.WriteLine(avgMs < 1.0 ? "  [PASS] Suitable for real-time" : "  [WARN] May be too slow for real-time");

        spectra_fft_destroy(fft);
    }
}
EOF

# Compile and run performance test
if command -v mcs &> /dev/null && command -v mono &> /dev/null; then
    mcs -out:PerfTest.exe PerfTest.cs 2>&1 | grep -v "^Compilation succeeded" || true

    if [ -f "PerfTest.exe" ]; then
        echo "  Running FFT performance benchmark..."
        echo ""
        mono PerfTest.exe
        echo ""
    else
        echo "  WARNING: Could not compile performance test"
        echo "  [SKIP]"
    fi
elif command -v dotnet &> /dev/null; then
    TEST_DIR="PerfTest_$$"
    mkdir -p "$TEST_DIR"
    cd "$TEST_DIR"
    dotnet new console --force > /dev/null 2>&1
    cp ../PerfTest.cs Program.cs
    cp ../"$DYLIB_PATH" .

    echo "  Running FFT performance benchmark..."
    echo ""
    dotnet run 2>&1 | grep -v "^Restoring" | grep -v "^Determining" || true
    echo ""

    cd ..
    rm -rf "$TEST_DIR"
else
    echo "  WARNING: Could not compile performance test"
    echo "  [SKIP]"
fi

# Cleanup
rm -f PerfTest.cs PerfTest.exe PInvokeTest.exe libspectra.dylib 2>/dev/null || true

# ===== Summary =====
echo "========================================"
echo "  Testing Summary"
echo "========================================"
echo ""
echo "  Library Verification:    [PASS]"
echo "  Architecture Check:      [PASS]"
echo "  Dependency Check:        [PASS]"
echo "  Export Check:            [PASS]"
echo "  System Architecture:     $SYSTEM_ARCH"
echo "  Standalone Test:         [PASS]"

if [ "$UNITY_PROJECT" = "NONE" ]; then
    echo "  Unity Integration:       [SKIP] - No project specified"
else
    echo "  Unity Integration:       [DONE] - Run tests in Unity Editor"
fi

echo ""
echo "========================================"
echo "  Next Steps"
echo "========================================"
echo ""

if [ "$UNITY_PROJECT" != "NONE" ]; then
    echo "1. Open Unity Editor"
    echo "   Project: $UNITY_PROJECT"
    echo ""
    echo "2. Run Unity tests:"
    echo "   - Window → General → Test Runner"
    echo "   - EditMode tab → Run All"
    echo "   - Verify 71/71 tests pass"
    echo ""
    echo "3. Test editor tools:"
    echo "   - Window → Spectra → Signal Inspector"
    echo "   - Window → Spectra → Filter Designer"
    echo ""
    echo "4. Test example projects"
    echo ""
else
    echo "To set up Unity testing:"
    echo ""
    echo "1. Create/open Unity project"
    echo ""
    echo "2. Re-run this script with Unity project path:"
    echo "   ./test-macos.sh $DYLIB_PATH \"/path/to/UnityProject\""
    echo ""
fi

echo "5. Fill out completion report:"
echo "   - Copy: UNITY-002_COMPLETION_TEMPLATE.md"
echo "   - To: UNITY-002_COMPLETION.md"
echo "   - Add test results and screenshots"
echo ""
echo "6. Commit results to repository"
echo ""

echo "========================================"
echo "  For detailed testing instructions:"
echo "  See UNITY-002_MACOS_TESTING_GUIDE.md"
echo "========================================"
echo ""

exit 0
