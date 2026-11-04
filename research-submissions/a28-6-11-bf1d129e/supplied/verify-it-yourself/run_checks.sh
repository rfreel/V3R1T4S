#!/bin/sh
set -eu
HERE="$(cd "$(dirname "$0")/.." && pwd)"
g++ -O3 -fopenmp "$HERE/verify-it-yourself/verify_code.cpp" -o "$HERE/verify-it-yourself/verify_code_bin"
"$HERE/verify-it-yourself/verify_code_bin" "$HERE/object/A28_6_11_29484_hex.txt" 28 6 11 29484
rm -f "$HERE/verify-it-yourself/verify_code_bin"
echo "ALL CHECKS PASSED"
echo "PASS"
