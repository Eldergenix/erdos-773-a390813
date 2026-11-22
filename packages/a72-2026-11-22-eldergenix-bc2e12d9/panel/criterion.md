# PANEL_OUT — role: CRITERION

context: Claude Agent SDK, non-interactive fresh session, claude-fable-5-2; saw only this claim-package directory; no network access used.

## Verdict

The mathematical part of the criterion (case a(72) = 33) is met and I reproduced it myself. The "value must not appear" clause is supported by the status records for the entry, the b-file, the Erdős Problem a Day page and the forum, but not for the OEIS history page (hash only, see the last section). I pass on that basis and leave the history page to the NOVELTY skeptic.

## Which target index

- `status/oeis_b390813.txt` ends at "68 32". Its sha256 is dce8e0fa…6916, the same hash recorded by the scout at 10:16 UTC and by `status/refetch_record.txt` at 11:48 UTC.
- The scout read the Erdős Problem a Day page as giving values through a(71) and nothing about a(72). The refetch hash of that page is identical to the scout's.
- The first index beyond both is therefore 72, so a(72) is the right target.

## Witness (lower bound)

- The claimed 33-set (2 5 7 … 72) passes `checker/sidon_check.py 72 33 …` with VERDICT: PASS.
- My own inline check gives 33 distinct elements in 1..72 and 561 unordered sums x^2+y^2 with x <= y, x = y included, all 561 distinct.
- `cert_parity/witness.json` holds a different 33-set, which is also valid (561 of 561 distinct).
- The checker's definition includes the diagonal sums (`combinations_with_replacement`). My own brute force from the definition (`panel_tmp/own.py`) matches the b-file for n = 1..26, so the definition used is the sequence's.

## Upper bound (no 34-set)

The criterion asks for two independently written exact searches that agree. I read all three search sources and reran two of them.

| Search | File | Method | Prefix bounds used | My rerun |
|---|---|---|---|---|
| C | `searchC/rds2.c` | Russian doll, forbidden 3- and 4-sets from equal sums, clique-cover bound | Only a(i), i < k, proved earlier in the same run (no `-b`) | `./rds2 -n 72 -t 2`: k = 1..68 all equal the b-file; k=69 a=32, k=70 a=33, k=71 a=33, k=72 a=33 [EXHAUSTED-NONE size 34 containing 72], tasks 728/728, "# DONE normal completion", exit 0 |
| B | `searchAB/code/independent/difference_search.cpp` | Whole instance, forbidden sets from equal differences of squares, group-cover bound | None | `./difference_search 72 100000 0 5`: 32 of 32 parts "answer=no" at target 34, `maximum=33`, exit 0; call count 123914694 identical to the saved log |
| A | `searchAB/code/doll.c` | Russian doll, supports from equal sums | `optimum[]` filled in-run from zero | Not rerun; source read, saved `searchAB/code/doll.log` ends "finished through=72" with k=34 UNSAT over 128 of 128 cubes |

- **Exhaustive and uncapped:** none of the three has a node cap or internal time limit. Search C's branching is a complete include/exclude split and it checks tasks done = tasks generated. Search B covers every last-root choice and all 32 outer assignments; the 100000 budget applies only to the heuristic witness walk. The wall-clock alarm in the search A/B wrappers kills the process rather than truncating silently, and the saved logs show exit 0.
- **Bound validity:** I checked each pruning rule by reading the source (clique groups contribute at most 1, residual clause groups at most size−1, prefix bounds a(i) only for proved i) and found no unsound step.
- **Forbidden-set generation:** search B reports 23 triples and 762 quadruples at n = 72. My own enumeration from equal sums gives the same 23 and 762.
- **Independence:** B and C differ in language, formulation (differences vs sums), strategy (whole instance vs prefix doll) and, per the claim, authoring session. They agree with each other and with A.
- **Prefix values k <= 71:** no value from outside the run is used anywhere. a(69) = 32, a(70) = 33, a(71) = 33 are re-proved in-run by A and C. B's saved runs in `searchB_calibration/` give 29 30 30 30 31 31 31 32 32 for n = 60..68 (matching the b-file) and 32 33 33 for n = 69..71; I did not rerun those.
- **Certificate D:** `./check_cover 72 34 cert_parity/n72.cover.gz` printed "PASS N=72 excluded=34 even_upper=20 odd_upper=19 strata=6 hitting_clauses=141168", as expected. I ran it but did not audit `cert_parity/check_cover.cpp` line by line, so I count it as corroboration only; it is not needed for the criterion.

## Status clause ("value must not appear …")

What `status/` supports:
- **Entry and b-file:** terms stop at a(68) = 32 (scout read; b-file hash unchanged at refetch).
- **Erdős Problem a Day #773:** scout read values through a(71) and "nothing about a(72)"; page hash unchanged at refetch.
- **Forum thread 773:** scout read 11 comments, 0 proof claims, about the Croot–Mao–Yip bound and heuristics.

Gaps, for the NOVELTY skeptic:
1. **History page:** `https://oeis.org/history?seq=A390813` appears only in `status/refetch_record.txt` as a status/size/hash line. No record says its content was read or searched for a(72), so the "its history" part of the clause is not evidenced in this package.
2. **Forum grep result missing:** `status/scout_record.md` says the forum page was grepped for "a(72)", "a(7" and "34": "see below", but no grep output follows.
3. **Forum hash changed:** scout 17534332…, refetch 6c52526f…, same 59113 bytes, with no explanation or re-read recorded.
4. **Raw pages absent:** the saved pages (`data/…`) and `checker/test_output.txt` cited by the scout are not in the package, so none of the reads can be rechecked here.

Nothing in the package suggests the value is published; the best published statement recorded is a(71) = 33 (conditional) on the Erdős Problem a Day page. But I could not confirm non-appearance in the history page, and my pass does not cover it.

## Files I created

Everything I built or wrote is in `panel_tmp/` (binaries, `rds2.log`, `diff72.log`, `cover.log`, `own.py`).

confirmed_scope: Under the definition with x = y sums included, the listed 33-element subset of {1..72} is valid and no 34-element subset exists, as shown by my reruns of two independently written, uncapped exact searches (searchC/rds2.c self-contained from k = 1, and difference_search.cpp on the whole instance with no prefix values) that agree with each other, with the saved search A log and with the b-file for n <= 68, so a(72) = 33; non-appearance of the value is supported only by the scout's recorded reads of the entry, b-file, Erdős Problem a Day page and forum, and is not evidenced for the OEIS history page.

pass: true
