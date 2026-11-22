# PANEL_OUT — REPRODUCE seat

context: claude -p fresh session, claude-opus-5-5 — a NEW cold session of claude-opus-5-5 taking the REPRODUCE seat because the other vendor's model (GPT-6.2 Astra via codex exec) hit its usage limit.

Claim checked: A390813(72) = 33. The largest X ⊆ {1..72} with all x^2+y^2 (x ≤ y, x, y ∈ X) distinct has 33 elements.

Definition used (quoted from CLAIM.md): "the largest subset X of {1..72} for which all sums x^2 + y^2 (x <= y, x, y in X) are distinct".
My programs were written from this sentence alone, before I opened any of the author's code.

Limits kept: at most 3 CPU cores at any time; memory per process was a few MB. Every compile used clang -O3 (the cert checker was built with `c++ -O3`, as REPRODUCE.md says). Every long job was run under `perl -e 'alarm shift; exec @ARGV' SECONDS ...`, and no alarm fired. The machine was shared and heavily loaded (load average about 45 on 14 cores), so the wall times below are longer than the CPU times.

## 1. My own programs

### 1a. Witness checker: mine/wcheck.py
It lists every unordered pair with repetition (x ≤ y), checks that the elements are distinct and lie in range, and checks that the n(n+1)/2 sums are all distinct.
- `python3 -I mine/wcheck.py 72 2 5 7 9 11 12 13 14 18 22 24 28 31 36 42 43 45 46 51 55 56 58 60 61 64 65 66 67 68 69 70 71 72`
  → `size 33 pairs 561 distinct 561 expected 561` / `OK`
- The same OK result holds for the other packaged 33-sets: searchAB/result/answer.json (the independent witness), the 71-only witness, cert_parity/witness.json, and the k=64, 67 and 70 witnesses in searchC's log. It also holds for a new 33-element witness that my search found in {1..70}: 4 9 10 12 16 17 19 21 22 24 27 34 37 40 42 43 46 48 49 52 54 55 57 58 60 61 63 64 65 66 67 69 70.
- I counted the forbidden configurations independently in Python, by grouping pairs by their sum. For N=72 there are 23 3-sets and 762 4-sets. These counts agree with searches A, B and C (triples=23, quads=762).

### 1b. Exhaustive search: mine/rds_v4.c
Earlier versions are mine/rds_mine.c (bound by prefix threshold only) and mine/rds_cc.c.

Method: Russian doll over k. Since a(k) ≤ a(k-1)+1, any valid set of size a(k-1)+1 inside {1..k} must contain k. The program enumerates exhaustively, choosing elements in decreasing order.
- The candidate set C holds exactly the z for which S ∪ {z} is still valid.
- The conflict graph on C joins z and w when S ∪ {z, w} is invalid. Three kinds of edges are added:
  - (i) z^2+w^2 equals an existing sum;
  - (ii) z^2 - w^2 = ±(x^2 - y^2), for the new element y and an x in S;
  - (iii) 2z^2 = w^2 + y^2.
  Every edge is a real collision, so the bound stays sound.
- Pruning uses two bounds, both sound:
  - min over t of ( |C ∩ (t, max]| + a(t) );
  - an ascending greedy clique cover of the conflict graph restricted to C ∩ [1..u]. A valid extension is independent in that graph, so it takes at most one vertex from each clique. Recoloring moves a vertex v only into a clique it fully conflicts with.
- The work is split into tasks, one for each pair (second element, third element) under k, and every task runs to completion. There is no node cap and no time cap. A witness found for any k is re-verified from the definition before it is accepted.
- Node counts are deterministic for the "NONE" levels: k=62 gave 6073795 nodes with 1 thread and with 3 threads.

Calibration and results. In the table, "b-file" means status/oeis_b390813.txt.

| run | prefix bounds used | result |
|---|---|---|
| mine/cc64.log (rds_cc, from k=1) | only its own | a(1..64) identical to the b-file |
| mine/run65_66.log | b-file a(1..64) | a(65)=31, a(66)=31 (match) |
| mine/run67_72.log | b-file a(1..66), then its own | a(67)=32 (witness), a(68)=32 (match); a(69)=32 NONE of size 33 containing 69 (64798787 nodes); a(70)=33 (witness above); a(71)=33 NONE of size 34 containing 71 (55733088 nodes); **k=72: NONE of size 34 containing 72, 205583137 nodes**; `# DONE normal completion`, exit=0, 472 s CPU |
| mine/selfcontained_2_72.log (rds_v4, 1 thread, no b-file at all) | only its own | a(1..68) identical to the b-file. a(69..72) = 32 33 33 33. k=72: NONE of size 34 containing 72, with 205583137 nodes. The NONE node counts for k=69, 71 and 72 are identical to the 3-thread run (64798787, 55733088, 205583137), so the run is deterministic. All 32 witnesses it found pass mine/wcheck.py. `# DONE normal completion`, exit=0, 581 s CPU. |

Conclusion of part 1: my own program decides exhaustively that {1..72} has no valid 34-element subset. It uses only the b-file values a(n) for n ≤ 68, or none in the self-contained run, plus values it proved itself. a(69), a(70) and a(71) were recomputed. Together with the 33-element witness this gives a(72) = 33.

## 2. Reruns of the author's searches and certificate

All of these were rebuilt in rerun/ from the packaged sources.

- **Certificate D**: `c++ -O3 -std=c++20 cert_parity/check_cover.cpp -lz -o rerun/check_cover; rerun/check_cover 72 34 cert_parity/n72.cover.gz`
  → it printed 6 `case ... PASS` lines, then `PASS N=72 excluded=34 even_upper=20 odd_upper=19 strata=6 hitting_clauses=141168` with exit 0. This is exactly the expected line. (rerun/check_cover.log, 19 s CPU)
- **Search B, n=72**: `rerun/difference_search 72 100000 0 5`
  → 32 parts `answer=no`, `target=34 answer=no parts=[0,32)/32 calls=123914694 selections=123914662 pack_cuts=50429856 size_cuts=0`, `maximum=33`, exit 0. The counts are identical to searchAB/code/independent/order72-complete.log. (rerun/diff72.log)
- **Search B, n=71**: `rerun/difference_search 71 100000 0 0`
  → `calls=60352801 ... pack_cuts=24826473`, `maximum=33`. This is identical to searchB_calibration/diff_lemma_71.log. (rerun/diff71.log)
- **Search C**: `clang -O3 -o rerun/rds2 searchC/rds2.c -lpthread; rerun/rds2 -n 72 -t 2`. I used 2 threads so the total stayed within 3 cores.
  → `# DONE normal completion`, exit 0. Every a(k) for k=1..72 equals the saved searchC/run_selfcontained_1_72.log, and every EXHAUSTED row has the same node count (k=72: 171850734). a(1..68) equals the b-file. The k=69, 71 and 72 rows are EXHAUSTED-NONE. Node counts differ only on FOUND rows (for example k=67: 8574674 against the saved 8412490). Those rows end at the first witness a thread reaches, which depends on scheduling, so the difference is expected. (rerun/rds2.log, 456 s CPU)
- Search A (doll.c) was not rerun. I audited its source and its saved log only.

## 3. Audit of the pruning rules and logs

- **Completeness of the forbidden structure.** If x^2+y^2 = z^2+w^2 and the two pairs share an element, the multisets are equal. So the only forbidden sets are 4-sets {a<b<c<d} with a^2+d^2 = b^2+c^2 and 3-sets with a^2+c^2 = 2b^2. These are equivalent to equal positive differences of squares of distinct pairs, which is what searches A and B use. My counts (23/762) agree with all three.
- **Search C (rds2.c).**
  - The candidate filter is exact.
  - Edges are added only for real pairwise conflicts.
  - L(i) and U(i) are valid bounds on |I ∩ [1..i]| and |I ∩ (i..top]|.
  - Tasks are generated by always branching on the top candidate, so the sibling exclusions are implied by the mask below z.
  - It checks that done_tasks equals ntask before declaring NONE.
  - Sound.
- **Search A (doll.c).**
  - Supports come from equal sums.
  - Residual supports become singleton, pair or "event" constraints. An event edge r0–r1 is installed when r2 is chosen (and r3, if present, is selected). This is correct because the search order is descending.
  - cover_reason counts groups + leftover vertices, and groups + optimum[highest(rest)+1]. This is valid because every leftover vertex lies in the prefix ending at highest(rest).
  - Cubes cover all assignments of the indices 65..level-1.
  - Sound. The saved doll.log ends with `finished through=72`, and its level rows give a(60..72) = 29 30 30 30 31 31 31 32 32 32 33 33 33.
- **Search B (difference_search.cpp).**
  - It uses MCQ-style branching. Any solution is found at the branch of its last vertex in the coloring order, and the bound at that position is valid:
    - clique groups have cap 1;
    - a clause group whose other members are all selected has cap s-1;
    - plain vertices have cap 1 each.
  - Activations add only real conflicts, and the undo is exact.
  - The 2^5 root split is exhaustive.
  - No prefix values are used. Sound.
- **Certificate D.**
  - Every collision is either within one parity, or between an even pair and an odd pair with equal difference. Mixed collisions have exactly one odd element on each side, as shown mod 4/8. So "shared difference features" capture every cross-parity collision exactly. The checker requires that feature pairs have equal x^2 mod 16, which justifies the two-group signature tables.
  - even ≤ 20 and odd ≤ 19 are re-proved inside the checker by an empty enumeration at size +1. That gives the strata #even ∈ [15, 20].
  - In each stratum, every valid one-side row contains its clause, and every valid opposite-side row hits every clause. So every combination contains a cross collision.
  - The early "closed prefix" skip and the subset-index transversal test are monotone and correct.
  - Sound.
- **Logs.**
  - All saved logs end normally: rds2 `# DONE normal completion` exit=0; doll `finished through=72`; difference_search `maximum=33` exit=0, and searchB_calibration/* give the maxima 29 30 30 30 31 31 31 32 32 32 33 33 for n=60..71.
  - My reruns reproduced them exactly where the result is deterministic.
- **Minor observation, not a defect in the mathematics.** status/oeis_b390813.txt is headed "b-file synthesized from sequence entry". Every value my programs used from it (n ≤ 66) was also recomputed from scratch, both by my cc64 run for n ≤ 64 and by my self-contained run and search C for all n. So nothing depends on it.

## Verdict
- The 33-element witness is valid.
- My independent exhaustive search finds no 34-element valid subset of {1..72}, and its calibration on a(60..68) matches the b-file.
- Search C and search B, rerun, reproduce their saved logs, and certificate D's checker passes.
- No mismatch, missing object or non-determinism (outside FOUND-row node counts) was found.

pass: true
