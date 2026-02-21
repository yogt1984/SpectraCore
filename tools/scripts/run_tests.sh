#!/bin/bash
# Run all tests

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
NATIVE_DIR="$PROJECT_ROOT/native"

echo "Running SpectraCore tests..."

cd "$NATIVE_DIR"

# Configure and build with tests
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DSPECTRA_BUILD_TESTS=ON
cmake --build build --config Debug

# Run tests
ctest --test-dir build -C Debug --output-on-failure --verbose

echo "All tests passed!"
