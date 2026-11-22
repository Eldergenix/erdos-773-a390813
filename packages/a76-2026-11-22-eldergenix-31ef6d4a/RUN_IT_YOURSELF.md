# Run it yourself (fresh shell, from this folder)

    sh verify-it-yourself/run_checks.sh

Expected: `witnesses 35@75 34@73 33@70 and chain table: OK`, `N=76 T=36 parts exhausted: 8 of 8`, `PASS`.

The whole chain from the OEIS prefix, with two programs (usage: prog N T D P part A_1 ... A_{N-1}, where A_k = a(k)
are the prefix bounds; D=8 P=8 splits the tree into 8 parts):

    cc -O3 -o bb2 how-we-built-it/bb2.c ; cc -O3 -o bb3 how-we-built-it/bb3.c
    B="$(awk '$1 ~ /^[0-9]+$/ {printf "%s ", $2}' object/oeis_b390813_prefix.txt)"
    ./bb3 69 33 0 1 0 $B                     # EXHAUSTED            -> a(69) = 32
    ./bb3 71 34 0 1 0 $B 32 33               # EXHAUSTED            -> a(71) = 33 (a(70) = 33 by witness_70)
    for p in 0 1 2 3 4 5 6 7; do ./bb3 72 34 8 8 $p $B 32 33 33; done              # -> a(72) = 33
    for p in 0 1 2 3 4 5 6 7; do ./bb3 74 35 8 8 $p $B 32 33 33 33 34; done        # -> a(74) = 34 (a(73) = 34 by witness_73)
    for p in 0 1 2 3 4 5 6 7; do ./bb3 76 36 8 8 $p $B 32 33 33 33 34 34 35; done  # -> a(76) = 35 (a(75) = 35 by witness)

and the same with ./bb2. Every line must print EXHAUSTED. Saved logs of both programs: object/logs/. A third program
written in another session: `cc -O3 -o rds2 how-we-built-it/rds2.c`, then for p in 0 1 2:
`./rds2 76 36 how-we-built-it/bounds_through_75.txt $p 3 62` (each prints NONE ... completed=1); its chain summary is
object/search3_chain_summary.txt.

How to rebuild the object: the witnesses are printed by the searches when a set exists (FOUND lines in
object/logs/); the value 35 is the witness size together with the exclusion of size 36.
