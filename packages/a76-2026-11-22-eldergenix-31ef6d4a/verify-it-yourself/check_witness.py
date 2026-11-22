#!/usr/bin/env python3
"""Standalone check from the definition (OEIS A390813): a set X of indices is valid iff all sums x^2 + y^2 with
x <= y, x, y in X (x = y included) are distinct. Checks the three witnesses in object/a76.json and the chain's
internal consistency. Usage: check_witness.py object/a76.json"""
import json, sys
from itertools import combinations_with_replacement
def valid(w, n, size):
    if len(w) != size or len(set(w)) != size or not all(isinstance(x, int) and 1 <= x <= n for x in w): return False
    s = [x * x + y * y for x, y in combinations_with_replacement(sorted(w), 2)]
    return len(s) == len(set(s)) == size * (size + 1) // 2
o = json.load(open(sys.argv[1])); c = {int(k): v for k, v in o["chain"].items()}
ok = (o["n"] == 76 and o["value"] == 35 and c == {69: 32, 70: 33, 71: 33, 72: 33, 73: 34, 74: 34, 75: 35, 76: 35}
      and valid(o["witness"], 75, 35) and valid(o["witness_73"], 73, 34) and valid(o["witness_70"], 70, 33))
print("witnesses 35@75 34@73 33@70 and chain table: %s" % ("OK" if ok else "BAD"))
sys.exit(0 if ok else 1)
