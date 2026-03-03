# CI Test Run #2 - Verification

This is a second test to confirm the CI pipeline is completely clean.

## Expected: 6 Jobs Only

1. Build Native (ubuntu-latest) ✅
2. Build Native (macos-latest) ✅
3. Static Analysis ✅
4. Build Android ✅
5. Build iOS ✅
6. Unity Tests ✅

## Should NOT appear:
- Code Coverage (removed)
- Test with Sanitizers (removed)
- Build Native (windows-latest) (removed)

**Test run:** 2026-03-03 15:00 UTC
