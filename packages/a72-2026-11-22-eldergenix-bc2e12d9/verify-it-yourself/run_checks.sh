#!/bin/sh
# Run from the package folder: sh verify-it-yourself/run_checks.sh   (last line: PASS or FAIL)
# 1. witness from the definition (lower bound). 2. exhaustive, uncapped whole-instance search (upper bound):
#    rebuilds search B from source and requires "maximum=<value in object/a72.json>" with every part "answer=no".
fail() { echo "$1"; echo FAIL; exit 1; }
python3 -I -B verify-it-yourself/check_witness.py object/a72.json || fail "witness check failed"
V=$(python3 -I -c "import json;print(json.load(open('object/a72.json'))['value'])") || fail "cannot read value"
N=$(python3 -I -c "import json;print(json.load(open('object/a72.json'))['n'])") || fail "cannot read n"
D=$(mktemp -d) || fail "mktemp"
c++ -std=c++17 -O3 verify-it-yourself/difference_search.cpp -o "$D/ds" 2> "$D/build.log" || fail "compile failed"
"$D/ds" "$N" 100000 0 5 > "$D/run.log" 2>&1 || fail "search did not exit normally"
grep -q "target=$((V+1)) answer=no parts=\[0,32)/32" "$D/run.log" || fail "search did not exclude size $((V+1)) over all 32 parts"
[ "$(grep -c "^maximum=" "$D/run.log")" = 1 ] && grep -q "^maximum=$V\$" "$D/run.log" || fail "search maximum differs from the claimed value"
grep "answer=no parts=\[0,32)" "$D/run.log" | sed 's/^[^ ]* //; s/ elapsed=.*//'
echo "maximum=$V"
echo PASS
