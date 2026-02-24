#!/bin/bash
# CI/CD Status Checker for SpectraCore
# This script helps check and validate GitHub Actions workflow status

REPO_URL="https://github.com/yogt1984/SpectraCore"
ACTIONS_URL="${REPO_URL}/actions"

echo "========================================"
echo "  SpectraCore CI/CD Status Checker"
echo "========================================"
echo ""
echo "Repository: $REPO_URL"
echo "Actions URL: $ACTIONS_URL"
echo ""

# Get the latest commit
LATEST_COMMIT=$(git log -1 --format="%H %s")
COMMIT_HASH=$(git log -1 --format="%h")
COMMIT_MSG=$(git log -1 --format="%s")

echo "Latest Commit:"
echo "  Hash: $COMMIT_HASH"
echo "  Message: $COMMIT_MSG"
echo ""

# Check if GitHub CLI is available
if command -v gh &> /dev/null; then
    echo "GitHub CLI detected. Fetching workflow status..."
    echo ""

    # Get latest workflow runs
    gh run list --limit 5 --json databaseId,status,conclusion,name,createdAt,displayTitle

    echo ""
    echo "To view detailed logs:"
    echo "  gh run view --log"
    echo ""
    echo "To download artifacts:"
    echo "  gh run download --name spectra-windows"
    echo "  gh run download --name libspectra-macos"
    echo "  gh run download --name libspectra-linux"
    echo "  gh run download --name SpectraCore-Unity-Package"
else
    echo "GitHub CLI not installed."
    echo ""
    echo "To check CI/CD status manually:"
    echo "  1. Open your browser"
    echo "  2. Navigate to: $ACTIONS_URL"
    echo "  3. Look for the workflow run with commit: $COMMIT_HASH"
    echo "  4. Check if all jobs completed successfully (green checkmarks)"
    echo ""
    echo "Expected Jobs:"
    echo "  - build-linux (Ubuntu)"
    echo "  - build-windows (Windows)"
    echo "  - build-macos (macOS)"
    echo "  - package-unity (Ubuntu)"
    echo ""
    echo "To install GitHub CLI:"
    echo "  sudo apt install gh"
    echo "  or visit: https://cli.github.com/"
fi

echo ""
echo "========================================"
echo "  What to Check For"
echo "========================================"
echo ""
echo "✅ All 4 jobs should have green checkmarks"
echo "✅ build-linux: Compiles libspectra.so + runs 68 tests"
echo "✅ build-windows: Compiles spectra.dll + runs 68 tests"
echo "✅ build-macos: Compiles libspectra.dylib (universal) + runs 68 tests"
echo "✅ package-unity: Creates Unity package with all libraries"
echo ""
echo "Total tests run: 204 (68 tests × 3 platforms)"
echo ""

echo "========================================"
echo "  Expected Artifacts"
echo "========================================"
echo ""
echo "After successful completion, these artifacts should be available:"
echo ""
echo "1. libspectra-linux (~140 KB)"
echo "   - Linux shared library (.so)"
echo "   - For x86_64 architecture"
echo ""
echo "2. spectra-windows (~100 KB)"
echo "   - Windows DLL"
echo "   - For x64 architecture"
echo ""
echo "3. libspectra-macos (~250 KB)"
echo "   - macOS universal binary"
echo "   - Contains both x86_64 and arm64 slices"
echo ""
echo "4. SpectraCore-Unity-Package (~500 KB)"
echo "   - Complete Unity package (tar.gz)"
echo "   - Contains all 3 platform libraries"
echo "   - Ready for Unity import"
echo ""

echo "========================================"
echo "  Next Steps"
echo "========================================"
echo ""
echo "If CI/CD SUCCEEDED:"
echo "  1. Download artifacts (optional)"
echo "  2. Update UNITY-001 status to complete"
echo "  3. Update UNITY-002 status to complete"
echo "  4. Update UNITY_INTEGRATION_SUMMARY.md"
echo "  5. Create comprehensive README"
echo ""
echo "If CI/CD FAILED:"
echo "  1. Check error logs in Actions tab"
echo "  2. Identify which job failed"
echo "  3. Read error messages"
echo "  4. Fix issues and push again"
echo ""
echo "If CI/CD IN PROGRESS:"
echo "  Expected duration: 10-15 minutes from commit"
echo "  Check back in a few minutes"
echo ""

echo "========================================"
echo "  Quick Actions"
echo "========================================"
echo ""
echo "Open Actions page in browser:"
echo "  xdg-open '$ACTIONS_URL' 2>/dev/null || open '$ACTIONS_URL' 2>/dev/null || echo 'Please visit: $ACTIONS_URL'"
echo ""
echo "Check if workflow file is valid:"
echo "  cat .github/workflows/build-multiplatform.yml | head -20"
echo ""

exit 0
