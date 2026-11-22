# Run it yourself (fresh shell, from this folder)

    sh verify-it-yourself/run_checks.sh

Expected output: a line `witness n=72 size=33 distinct_sums=561 OK`, the line
`target=34 answer=no parts=[0,32)/32 calls=123914694 selections=123914662 pack_cuts=50429856 size_cuts=0`,
`maximum=33`, then `PASS`.

Further independent searches (sources in how-we-built-it/, saved logs in object/):

    cc -O3 -o rds2 how-we-built-it/rds2.c -lpthread && ./rds2 -n 72     # search C: proves every a(k), k = 1..72, in one run
    cc -O3 -std=c11 how-we-built-it/doll.c -o doll && ./doll 72 prefix.csv   # search A: prefix Russian doll

Search C ends with `k=72 a=33  [EXHAUSTED-NONE size 34 containing 72]` and `# DONE normal completion` (about 8 to
17 CPU minutes); its column a(k) for k <= 68 equals object/oeis_b390813_prefix_used_for_calibration.txt. Search A
ends with `finished through=72`.

How to rebuild the object: any of the searches prints a 33-element witness; object/a72.json holds the one found by
search A. The value 33 is the witness size together with the exclusion of size 34.
