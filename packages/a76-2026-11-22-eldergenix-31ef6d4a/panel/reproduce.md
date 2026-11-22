# PANEL_OUT — REPRODUCE seat

context: claude -p fresh session, claude-opus-5-5 — a NEW cold session in the REPRODUCE seat; the other vendor's model (GPT-6.2 Astra via codex exec) is unavailable today (usage limit).

Claim checked: A390813(76) = 35. This seat also checked the chain a(69..76) = 32, 33, 33, 33, 34, 34, 35, 35.

## 0. Definition used (quoted from CLAIM.md)
"the largest subset X of {1..76} for which all sums x^2 + y^2 (x <= y, x, y in X) are distinct"
Note that x = y is allowed, so 2x^2 also counts as a sum.

I wrote my own programs from this definition alone, before opening any author code (bb2.c, bb3.c, rds2.c, sidon_check.py):
- `reproduce_work/mycheck.py` checks a set directly from the definition (all sums over pairs x <= y must be distinct).
- `reproduce_work/mysolve4.c` (clang -O3) is an exact branch and bound, with no node cap and no time cap. It decides whether some valid T-subset of {1..N} contains N. This is enough: if T = a(N-1)+1, any valid T-subset of {1..N} must contain N. Its rules, each proved sound in the source header:
  - The forbidden sets are the unions {a,b} ∪ {c,d} of two distinct pairs with a²+b² = c²+d². The program builds them by grouping all pairs by their sum. At N=76 it finds 901 of them, the same count bb2 and rds2 print.
  - Elements are decided from N downward. A candidate w is dropped exactly when some forbidden F contains the element just added and F \ S = {w}.
  - Prefix bound: |S' ∩ [1..j]| ≤ a(j).
  - Greedy clique cover of the candidates in the conflict graph. Edges come from forbidden sets with exactly 2 elements still open. Each clique holds at most one chosen element.
  - Disjoint packing of 3- and 4-element remainders among singleton candidates. Each one forces at least one element to be left out.
  - These are combined at every split point j as min(a(j), left part) + right part.
  - Any FOUND set is re-checked from the definition inside the program (`valid_direct`).
- `reproduce_work/mysolve.c` (v1, plain prefix bound only) is a simpler second version, used for calibration.

## 1. Own checks (commands run from reproduce_work/)
Witnesses:
- `python3 -I mycheck.py 75 <35-set>` → `N=75 size=35 max=75 valid=True info=630` (630 = C(36,2) distinct sums). The same result at N=76.
- `python3 -I mycheck.py 73 <34-set>` → `N=73 size=34 max=73 valid=True info=595`.
- The witness in CLAIM.md is the same as `chain_73_76.json` a75.witness (checked: True).
- The author's checker also prints `VERDICT: PASS` for both witnesses, and for the 35-set read in {1..76}.

Calibration, from scratch. The only input is a(1)=1; every later value is derived from my own earlier values:
- `./chain.sh 60` with mysolve v1 → a(1..60) is identical to the b-file.
- `./chain3.sh 2 68 "1" ./mysolve4 12 7000` (3 parts, 7000 s alarm, which never fired) → a(1..68) is identical to all 68 b-file values (`68 68 True`). For example a(60..68) = 29 30 30 30 31 31 31 32 32. a(68): T=33, NONE in all 3 parts.

Decisive range, using only my own a(1..68) (they match the b-file) and my own later values. Run: `./chain3.sh 69 76 "<own a(1..68)>" ./mysolve4 12 7000`. Every NONE line means all 3 parts printed `NONE completed=1`. Every FOUND set has `valid_direct=1` and was re-checked with mycheck.py.
```
a(69)=32  T=33 NONE   nodes 45005799 44878606 44804612
a(70)=33  T=33 FOUND  (3 4 6 7 8 10 11 18 22 28 30 31 33 36 37 38 39 40 43 44 45 48 49 55 60 62 63 64 65 67 68 69 70)
a(71)=33  T=34 NONE   nodes 31171927 31366690 30923634
a(72)=33  T=34 NONE   nodes 139223891 137595250 138933328
a(73)=34  T=34 FOUND  (a different 34-set; valid)
a(74)=34  T=35 NONE   nodes 67734493 67867090 67883288
a(75)=35  T=35 FOUND  (found the same 35-set as the claim's witness)
a(76)=35  T=36 NONE   nodes 85791514 85520322 84837227
VALUES ... 32 32 33 33 33 34 34 35 35
```
- Determinism: rerunning N=76 with the same split gave identical node counts in all 3 parts.
- Unsplit runs (no partitioning at all):
  - `mysolve4 76 36 0 1 0 <own a(1..75)>` → `nodes=256146317 NONE completed=1 rc=0`
  - `mysolve4 74 35 0 1 0 <own a(1..73)>` → `nodes=203481509 NONE completed=1 rc=0`

My own program therefore establishes a(69..76) = 32, 33, 33, 33, 34, 34, 35, 35, and in particular a(76) = 35.

## 2. Reruns of the packaged searches (in reproduce_work/author_rerun/, all three rebuilt with clang -O3)
I gave each program my own proved bound values; they equal search3/bounds_chain.txt for every index below K.
- **rds2**, K=76 T=36 and K=74 T=35, 3 parts, splitlevel K-14 (also K=72): all 9 runs print NONE, completed=1, rc=0. The node and splitstate counts are **identical** to search3/logs (for example K=76: 364485904/363607984/364296761, splitstates 2644).
- **bb3**, N=74 and N=76, D=8 P=8: all 16 parts EXHAUSTED, rc=0. Every per-part node count is **identical** to logs/ (sums 323,242,324 and 442,644,384).
- **bb2**, N=74 and N=76, D=8 P=8: all 16 parts EXHAUSTED, rc=0. Every per-part node count is **identical** to logs/ (sums 203,432,704 and 256,186,164, as claimed).
- Spot checks of earlier links: bb3 N=69 T=33 unsplit gives 231529645 nodes, and bb2 N=71 T=34 unsplit gives 93286221 nodes, both identical to the saved logs. The FOUND sets in logs/ for N=70, 71(T=33), 73 and 75 are valid under mycheck.py.
- No alarm fired (all 43 reruns have rc=0), and no INTERNAL/CHECK errors appeared.
- Because the node counts match exactly, the saved runs very probably used the same bound vector as mine.

## 3. Soundness audit of the author's pruning rules
- **bb2.** It builds the supports from all pairs with equal sums (901 at N=76, with no non-minimal ones). The kill step removes a vertex when exactly one element of a support is still unchosen; it adds a conflict edge when exactly two are. The greedy clique partition really produces cliques (it intersects the candidate set with adjacency). Hyperedge parts are required to be disjoint and made only of live singletons. In the threshold bound, run(v..k) + A[v-1], each part contributes min(m(P), |P∩[v..k]|); I re-derived this. Sound.
- **bb3.** Each recorded conflict ((a) u²+w² = an old or new sum, (b) |u²−w²| = t²−x², (c) u²+x² = 2w²) is a genuine coincidence of sums in S∪{u,w}. Feasibility is tested directly against the sum table. The clique bound hi(v) + min(A[v−1], lo(v)) is sound; cliques straddling v are counted twice. The split numbering is deterministic. Sound.
- **rds2.** The enumeration a<c≤d<b covers every triple (c=d) and every quadruple exactly once (901). The conflict pairs are triggered by the largest decided elements of the support, which is valid for descending order. The feasibility test checks ub[i] + (number of matched-reduced candidates in (i..j]) against need; a greedy matching gives an upper bound on the independent set. Sound.
- Every program uses the prefix bounds only as upper bounds on the size of S'∩[1..i], which is valid because a subset of a valid set is valid.

## 4. Defects found (none changes the result)
- REPRODUCE.md's `B="$(awk '{printf "%s ", $2}' status/oeis_b390813.txt)"` also picks up the `#` header line of that file (token `A390813`). That gives 69 tokens, and bb2 refuses with `need 68 prefix values`. The documented commands do not run as written; you have to drop the comment line. It is not a silent shift: the programs check the argument count.
- logs/bb2_N72_T34.log is empty. bb2's a(72) evidence exists only as logs/cond_N72_* (bb2 format; the bounds used are not recorded). bb3 logs for N=73 and for N=71 T=33 are absent. Those are only the SAT direction, which the witnesses cover.
- chain_73_76.json cites N=77 logs that are not in the package. That is outside this claim.
- No log records the bound vector it used. The exact node-count reproduction makes up for this.
- None of these affect the claim. My own program certifies every link 69..76 independently, and two packaged searches (in fact all three) reproduce exactly at N=74 and N=76.

## Verdict
The witness is valid. My own exhaustive search, starting from a(1)=1, without node or time caps and with no unproved bounds, gives a(76) = 35, and the whole chain a(69..76) = 32, 33, 33, 33, 34, 34, 35, 35. bb2, bb3 and rds2, rebuilt and rerun, reproduce their saved logs exactly at N=74 and N=76. The pruning rules are sound.

pass: true
