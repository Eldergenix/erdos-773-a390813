# Panel output: NOVELTY skeptic

context: claude -p fresh session, claude-opus-5-5 (Claude Code agent, non-interactive, unattended panel run)

target: erdos-773-sidon-squares-next-term. Claim: A390813(72) = 33.

## Status fetches (one plain curl request per URL, User-Agent "V3R1T4S-novelty-skeptic/1.0", no retries)
Times are from `date -u` at each request. Raw copies are saved in novelty_fetch/.

| time (UTC) | URL | HTTP / bytes | value read |
|---|---|---|---|
| Sun Nov 22 11:50:00 2026 | https://oeis.org/A390813 | 200 / 20036 | DATA runs n=1..68 and ends "..., 31, 32, 32", so a(68) = 32. "a(43)-a(68) from Christian Sievers, Nov 27 2025". STATUS approved. The page says "Last modified November 22 06:45 EST 2026". There is no a(69) or later term. |
| Sun Nov 22 11:50:01 2026 | https://oeis.org/A390813/b390813.txt | 200 / 438 | 68 lines, last line "68 32". sha256 dce8e0fa…6916, the same as status/oeis_b390813.txt and both earlier fetches by the claimant. |
| Sun Nov 22 11:50:01 2026 | https://oeis.org/history?seq=A390813 | 200 / 25210 | The newest change is #25, Sean A. Irvine, Dec 02 2025 (approved). The history gives no value beyond n=68. |
| Sun Nov 22 11:50:02 2026 | https://erdosproblemaday.com/day/773-square-sidon-frontier | 200 / 51564 | "Conditional on the full published exact OEIS A390813 prefix through N=68, a(69)=32, a(70)=33, and a(71)=33" (Aug 12 2026). The page says nothing about a(72). sha256 is identical to the claimant's copy. |
| Sun Nov 22 11:50:02 2026 | https://www.erdosproblems.com/forum/thread/773 | 200 / 59113 | 11 comments and 0 proof claims; the problem page was last edited 24 April 2026. The comments are about the Croot–Mao–Yip upper bound, random-model heuristics and g(A). The only OEIS mention is the 20 Nov 2025 submission note. No a(72) or any N=72 value appears. |

The first index beyond both the b-file (68) and the Erdos Problem a Day page (71) is 72. The target is therefore a(72), as the claim says.

## Literature and code search (window: last 24 months)
- **OEIS:** the entry, b-file and history above stop at n=68.
- **Erdos Problem a Day:** the page gives conditional values up to a(71) only. Its verification repository (github.com/pw/erdosproblemaday-verification, folder 773/) holds only n69-frontier and n71-frontier. Its last commit touching 773/ is dated 2026-10-25. The repository has no releases or release assets.
- **erdosproblems.com forum thread 773:** nothing on a(72).
- **V3R1T4S repository (github.com/elder-plinius/V3R1T4S), read through the unauthenticated API and raw files:**
  - TROPHY_CASE.md has no 773, Sidon-squares or A390813 entry.
  - The full repository tree (8714 paths) has no known-knowns/ folder for erdos-773. The only matching file is the bank entry known-unknowns/erdos-and-proofs/erdos-773-sidon-squares-next-term.md, which says "open as of 2026-10-30" and "So a(72) is 33 or 34".
  - I listed all pull requests, open and closed, numbers 1–1217 on 12 pages. No title or body mentions 773, A390813 or Sidon squares.
  - Issue/PR searches for "773", "sidon", "A390813", "sidon squares" and "erdos-773" return nothing relevant. The hits are issue #773, which is about Birkhoff, plus the Erdos 156 and 757 Sidon items.
- **arXiv:**
  - 2606.17487 (Croot, Mao, Pohoata, Sheffer, Yip, combinatorial large sieve) has only asymptotic bounds.
  - 2607.02728 (Bosio, Riblet, Tarr, B2[g] subsets of the first squares) has only asymptotic lower bounds.
  - 2605.03181 (Bailleul, Riblet) treats general finite sets.
  - Searching the text of all three for A390813, a(7x) or N=72 finds no exact small-N table.
- **Web (several standard and extended searches for A390813, a(72), "Erdos 773", Sidon squares exhaustive and GitHub):**
  - No source gives a(72) or settles it.
  - The DumbPrime mirror page (primereboot.pages.dev) only restates the asymptotic problem.
- **Limits:**
  - GitHub-wide code search needs authentication, so I did not run it; I only did web searches covering github.com.
  - I cannot rule out unpublished private work.

## Comparison with published objects in this cell and neighbouring cells
- **Lower bound a(72) ≥ 33.** This part is NOT new on its own. The published, unconditional 33-set for N=70 on the Erdos Problem a Day page is already a valid 33-element set inside {1..72}, so a(72) ≥ a(70) ≥ 33 was publicly known.
  - The claimed witness is a different set. It contains 72 and overlaps the published N=70 set in only 19 of 33 elements; under shifts of −3..+3 the overlap is at most 19.
  - The witness passes checker/sidon_check.py ("VERDICT: PASS", N=72, k=33), which I re-ran.
  - The witness is not what makes the claim new.
- **Upper bound a(72) ≤ 33 (no 34-element set).** This is the new part. From published material the best one can derive is a(72) ≤ a(71)+1 = 34, and only conditionally, through the Erdos Problem a Day values. No trivial edit of a published object gives the upper bound. It needs an exhaustive search at N=72, and nothing like that is published anywhere I could find.
- **A tie or a new value?** The claim is not a tie with a published record. The criterion asks for the exact value of a(72), which before this claim was publicly undetermined (33 or 34). Settling it is strictly beyond what is known. It also does not depend on the conditional published values (see the next section).

## What the certificate depends on
- **Search C** (searchC/rds2.c; log searchC/run_selfcontained_1_72.log) recomputes every a(k), k=1..72, within the same run. The log reads:
  - "k=71 a=33 [EXHAUSTED-NONE size 34 containing 71]"
  - "k=72 a=33 [EXHAUSTED-NONE size 34 containing 72]"
  - "# DONE normal completion"
- **Search A** (searchAB/code/doll.c) fills its prefix bounds within the same run ("Every prefix bound is filled by this process"). searchAB/code/prefix.csv runs through 72, and searchAB/code/doll.log ends with "finished through=72".
- **Search B** (searchAB/code/independent/difference_search.cpp) treats the whole instance and uses no prefix values. Its log searchAB/code/independent/order72-complete.log reads "target=34 answer=no parts=[0,32)/32" and "maximum=33".
- **Certificate D** (cert_parity/n72.cover.gz) comes with its own standalone checker, cert_parity/check_cover.cpp.
- None of the four upper-bound certificates takes the unpublished, conditional Erdos Problem a Day values a(69..71) as input. The package states that no prefix value beyond the OEIS b-file (n ≤ 68) is taken from publications, and the logs I read agree.
- **Status of the certificate itself:** it is the claimant's own work, unpublished and not externally certified. That is expected for a new term. Its correctness is for the reproduction skeptics; I did not re-run the long searches. My NOVELTY finding is limited to "not previously published".

novelty: NEW. No source I could reach publishes A390813(72), a 34-set at N=72, or an upper bound a(72) ≤ 33. The status sources (OEIS entry, b-file and history; Erdos Problem a Day #773; erdosproblems.com forum thread 773) were all read successfully and stop at n=68 (OEIS) and n=71 (conditional). The claimed exact value goes strictly beyond the published frontier.

pass: true
