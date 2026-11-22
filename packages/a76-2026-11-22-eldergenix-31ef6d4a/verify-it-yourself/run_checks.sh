#!/bin/sh
# Run from the package folder: sh verify-it-yourself/run_checks.sh   (last line: PASS or FAIL)
# 1. the witnesses, from the definition (lower bounds a(70) >= 33, a(73) >= 34, a(75) >= 35).
# 2. the decisive exclusion at N = 76: rebuilds search 2 (bb3) and exhausts "a 36-set of {1..76} containing 76"
#    in 8 parts (run in parallel), with the b-file prefix and the chain values a(69..75) from object/a76.json.
#    The earlier links (exclusions at 69, 71, 72, 74) are reproduced by RUN_IT_YOURSELF.md; their logs are in object/logs/.
fail() { echo "$1"; echo FAIL; exit 1; }
python3 -I -B verify-it-yourself/check_witness.py object/a76.json || fail "witness check failed"
D=$(mktemp -d) || fail "mktemp"
cc -O3 -o "$D/bb3" verify-it-yourself/bb3.c 2> "$D/build.log" || fail "compile failed"
B=$(awk '$1 ~ /^[0-9]+$/ {printf "%s ", $2}' object/oeis_b390813_prefix.txt)
C=$(python3 -I -c "import json;c=json.load(open('object/a76.json'))['chain'];print(' '.join(str(c[str(k)]) for k in range(69,76)))") || fail "chain"
for p in 0 1 2 3 4 5 6 7; do ( "$D/bb3" 76 36 8 8 $p $B $C > "$D/p$p.log" 2>&1; echo "exit=$?" >> "$D/p$p.log" ) & done
wait
n=0; for p in 0 1 2 3 4 5 6 7; do grep -q "EXHAUSTED" "$D/p$p.log" && grep -q "^exit=0" "$D/p$p.log" && ! grep -q "FOUND" "$D/p$p.log" && n=$((n+1)); done
echo "N=76 T=36 parts exhausted: $n of 8"
[ "$n" = 8 ] || fail "not all parts exhausted"
echo PASS
