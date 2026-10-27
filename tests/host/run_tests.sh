#!/usr/bin/env bash
#
# Host-side tests for the FastLED-free color module (src/color/color.h).
# Single entrypoint: `bash tests/host/run_tests.sh`.
#
# Builds the test in a temporary workdir and removes it on exit. Exits non-zero
# on any build or test failure.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

CXX="${CXX:-c++}"
if ! command -v "$CXX" >/dev/null 2>&1; then
    echo "FAIL: C++ compiler '$CXX' not found" >&2
    exit 1
fi

WORKDIR="$(mktemp -d "${TMPDIR:-/tmp}/ed-common-color-tests.XXXXXX")"
cleanup() {
    rm -rf "$WORKDIR"
}
trap cleanup EXIT

FLAGS=(-std=gnu++11 -Wall -Wextra -Werror -I"$REPO_ROOT/src")

echo "--- Building color tests ---"
"$CXX" "${FLAGS[@]}" "$SCRIPT_DIR/test_color.cpp" -o "$WORKDIR/test_color"

echo "--- Running color tests ---"
"$WORKDIR/test_color"

echo "ALL HOST TESTS PASSED"
