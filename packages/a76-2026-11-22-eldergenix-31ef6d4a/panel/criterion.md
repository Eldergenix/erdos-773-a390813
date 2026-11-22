# PANEL_OUT — role: CRITERION (partial win p1, claim A390813(76) = 35)

context: Claude Agent SDK, non-interactive fresh session, claude-fable-5-2; saw only this directory; no network use.

## Verdict in one paragraph
The package meets p1 literally for N = 76: a definition-valid 35-element witness exists inside {1..75}, and the
upper bound a(76) <= 35 is certified by three separately written exact searches that agree at every exhaustive step.
I reran the third search's whole chain and both other searches at N = 76 myself; every rerun was exhaustive and
reproduced the saved node counts exactly. The novelty clause is supported by the status record for the entry,
b-file and Erdős Problem a Day page, and only weakly for the forum and the OEIS history (see "Gaps").

## What I checked

1. Witness (x = y sums included). The 35 listed indices are distinct, lie in 2..75, and give 630 pairwise
   distinct sums x^2 + y^2 over all 630 pairs x <= y (own check, plus `checker/sidon_check.py` with N = 75 and
   N = 76: VERDICT: PASS). It equals the `a75` witness in `chain_73_76.json`. So a(76) >= a(75) >= 35.

2. Certificate form. The criterion's first option applies (two independently written exact searches that agree).
   `search1_bb2/bb2.c` (collision-support list with counters, clique + residual-support bound),
   `search2_bb3/bb3.c` (used-sum table, conflicts derived arithmetically, clique-cover bound) and
   `search3/rds2.c` (trigger lists, matching bound) are structurally different programs. No UNSAT proof is offered
   and none is needed.

3. Exhaustive and uncapped. Read all three sources line by line: no time limit, node cap, depth cap or early
   exit other than FOUND. Each decides "is there a set in {1..N} containing N with at least T elements".
   Pruning is sound in each: prefix bounds A[v-1], clique/matching bounds built only from genuine collisions.
   Split runs number the split states identically in every part, so the parts partition the tree; the logs show
   equal split counts across parts (bb2 88, bb3 88, rds2 2644 at N = 76) and all parts present.

4. Logical chain to a(76) <= 35. From a(68) = 32 (b-file): no 33-set containing 69, no 34-set containing 71,
   no 34-set containing 72, no 35-set containing 74, no 36-set containing 76, with a(k+1) <= a(k) + 1 for
   k = 69, 72, 74. Saved logs cover each of the five exhaustive steps for all three programs (bb2's N = 72 step
   is in `logs/cond_N72_T34_D8_P8_p*.log`).

5. Prefix bounds. The only pruning values beyond n = 68 are a(69..75) = 32 33 33 33 34 34 35. Values for n <= 68
   in `search3/bounds68.txt`, `search3/bounds_chain.txt` and `search3/bounds_through_75.txt` are identical to
   `status/oeis_b390813.txt` (sha256 dce8e0fa…, 438 bytes, matching both fetch records). Each value above 68 is an
   upper bound certified inside the package, and no step uses a bound it has not already established.

6. My reruns (clang -O3, 3 processes at a time, outputs in `panel_build/`):
   - `rds2` K = 69, 71, 72, 74, 76 (T = 33, 34, 34, 35, 36), 3 parts each, bounds cut to indices below K:
     15/15 parts NONE, completed=1, node counts identical to `search3/logs/`.
   - `bb3` N = 76 T = 36, 8/8 parts EXHAUSTED, node counts identical to `logs/`, sum 442,644,384.
   - `bb2` N = 76 T = 36, 8/8 parts EXHAUSTED, node counts identical to `logs/`, sum 256,186,164.
   - Small-N validation of my own (`panel_build/small.py`): for every N = 2..52 all three programs (and bb2 in
     split mode) answer FOUND exactly when the b-file has a(N) = a(N-1) + 1; 51/51 agree.
   Not rerun by me: bb2 and bb3 at N = 69, 71, 72, 74. Those steps rest on their saved logs plus my rds2 rerun.

7. Novelty clause, from `status/` only. Entry and b-file end at a(68) = 32. The Erdős Problem a Day page is
   recorded as giving conditional values through a(71) and nothing later. The refetch record shows the b-file
   and that page unchanged (same hashes) after the scout.

## Gaps and defects (none changes the mathematics)
- OEIS history: `status/refetch_record.txt` records only a fetch (HTTP 200, hash); no reading of its content is
  recorded anywhere. Forum: `status/scout_record.md` says the page was grepped "see below", but no grep output
  follows. The raw pages are not in the package, so I could not check either. Novelty for these two sources is
  asserted, not shown; the NOVELTY skeptic should carry that.
- `REPRODUCE.md` as written does not run: the `awk` line picks up the b-file's comment header, so bb2 exits with
  "need 68 prefix values". Skipping lines starting with `#` fixes it (that is what I ran).
- Saved bb2/bb3 logs do not record the prefix values passed, and the search3 logs name
  `logs/bounds_used_k*.txt` files that are not in the package. Identical node counts in my reruns show the
  stated bounds were the ones used for every step I reran.
- Empty logs: `logs/bb2_N72_T34.log`, `logs/bb2_N73_T34_D8_P8_p1.log`, `logs/bb2_N75_T35_D8_P8_p3.log`
  (aborted runs; none is needed for the chain).
- `chain_73_76.json` also asserts a(77) = 35 and cites logs that are not in the package. That is outside the
  claim and unsupported here.
- "Independently written" is taken from the sources' visibly different designs; authorship cannot be checked.

confirmed_scope: The listed 35-element set is Sidon in squares inside {1..75}, and three separately written exhaustive searches (search 3 rerun by me for N = 69, 71, 72, 74, 76; searches 1 and 2 rerun by me at N = 76 and taken from saved logs at N = 69, 71, 72, 74) agree that no larger set exists in {1..76} given the b-file values a(1..68), so a(76) = 35; novelty is confirmed only as far as the status record states it, with the OEIS history and forum contents not shown.

pass: true
