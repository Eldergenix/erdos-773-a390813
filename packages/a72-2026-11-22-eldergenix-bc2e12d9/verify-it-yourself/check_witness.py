#!/usr/bin/env python3
"""Standalone check from the definition (OEIS A390813): a set X of indices is valid iff all sums x^2 + y^2 with
x <= y, x, y in X (x = y included) are distinct. Usage: check_witness.py object/a72.json"""
import json, sys
from itertools import combinations_with_replacement
o = json.load(open(sys.argv[1]))
n, v, w = o["n"], o["value"], o["witness"]
ok = (n == 72 and v == 33 and len(w) == v and len(set(w)) == v and all(isinstance(x, int) and 1 <= x <= n for x in w))
sums = [x * x + y * y for x, y in combinations_with_replacement(sorted(w), 2)] if ok else []
ok = ok and len(sums) == len(set(sums)) == v * (v + 1) // 2
print("witness n=%s size=%s distinct_sums=%d %s" % (n, len(w), len(set(sums)), "OK" if ok else "BAD"))
sys.exit(0 if ok else 1)
