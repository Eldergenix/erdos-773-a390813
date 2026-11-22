# Panel skeptic: NOVELTY

context: fresh non-interactive Claude Agent SDK session (claude -p style), claude-opus-5-5; started with only this claim package as input
claim checked: A390813(76) = 35 (target erdos-773-sidon-squares-next-term, partial win p1)

## Status sources, one plain request each (curl, User-Agent "V3R1T4S-novelty-skeptic/1.0", no retries)
Raw responses and the full fetch log are in novelty_fetch/ (fetch_record.txt).

| time (date -u) | URL | HTTP | what it shows |
|---|---|---|---|
| Sun Nov 22 16:00:44 UTC 2026 | https://oeis.org/search?q=id:A390813&fmt=text | 200, 2844 B, sha256 35842c05… | revision #25 (Dec 02 2025). DATA ends at a(68) = 32 (68 terms). "a(43)-a(68) from Christian Sievers, Nov 27 2025". No a(69) or later. |
| Sun Nov 22 16:00:45 UTC 2026 | https://oeis.org/A390813/b390813.txt | 200, 438 B, sha256 dce8e0fa… | last line "68 32". It is byte-identical to status/oeis_b390813.txt (same hash). |
| Sun Nov 22 16:00:45 UTC 2026 | https://oeis.org/history?seq=A390813 (and &start=10 at 16:01:12) | 200 | revisions #1–#25. Nothing past n = 68, no pending edits, no a(69) or later. |
| Sun Nov 22 16:00:45 UTC 2026 | https://erdosproblemaday.com/day/773-square-sidon-frontier | 200, 51564 B, sha256 1c7be380… (same as this run's earlier fetches) | "Conditional on the full published exact OEIS A390813 prefix through N=68, a(69)=32, a(70)=33, and a(71)=33" (Aug 12, 2026). Nothing about a(72) or later. |
| Sun Nov 22 16:00:46 UTC 2026 | https://www.erdosproblems.com/forum/thread/773 | 200, 59113 B | 11 comments, 0 proof claims, page last edited 24 April 2026. The comments cover asymptotic bounds (Croot–Mao–Yip, random-model heuristics) and the OEIS submission. They give no finite values past the OEIS data. |

The forum and history page hashes differ from the hashes in status/refetch_record.txt. The likely cause is dynamic page parts (the footer shows "Last modified November 22 10:58 EST 2026"). The text I extracted shows the same content.

Published frontier: a(68) = 32 unconditionally (OEIS). a(69) = 32, a(70) = 33 and a(71) = 33 conditionally (Erdős Problem a Day, which assumes the OEIS prefix). Nothing anywhere for n ≥ 72.

## Literature and code search (last 24 months)
- arXiv API, searching "Sidon" plus "squares" sorted by date, and "A390813":
  - 2606.17487: combinatorial large sieve, upper bound N·exp(−c log N / log log N).
  - 2607.02728: B_2[g] subsets of the first squares, a lower-bound asymptotic.
  - 2602.08807: Sidon sets in short intervals.
  - 2605.03181: largest Sidon subset of arbitrary sets.
  - 2609.25050: FrontierMath Erdős benchmark.
  - None of these gives exact finite values of A390813. The "A390813" query returned no results.
- GitHub:
  - pw/erdosproblemaday-verification holds the Erdős Problem a Day bundles. Its 773 folder contains only n69-frontier and n71-frontier. It has no releases, and its last 773 commit is a sync on 2026-10-25.
  - vibemathing/problem-um-ep-773-… holds a single "planned" attempt with no claims or artifacts.
  - grandchallenge/MATH-PROGRAMME#2044 is a research design with no values.
  - morluto/jacobian#2884 mentions A390813(69) only as demand for a search.
  - thatnealpatel/proofs#7 is a formalisation todo.
  - teorth/erdosproblems#176 adds the OEIS link to 773.
  - Repository search for "A390813" returned 0 results. Searching "sidon squares" and "erdos 773" found nothing with values.
- V3R1T4S (github.com/elder-plinius/V3R1T4S):
  - TROPHY_CASE.md has no erdos-773, Sidon-squares or A390813 entry. Its only "773" match is the unrelated constant-weight-code bound 3773.
  - known-knowns/ has 46 entries, none matching 773, sidon or A390813.
  - The issue and PR search covers open and closed items. "A390813", "erdos-773", "erdos 773", "square-sidon" and "squares erdos" all returned 0 relevant items. "sidon" returned only erdos-757 and erdos-156 items. #773 is the issue number of an unrelated Birkhoff bank issue.
- Web search found only the OEIS entry, the Erdős Problem a Day page (through a(71)), the forum, the arXiv papers above and a primereboot.pages.dev summary page with no values.

Limits of this search: GitHub code search needs authentication, so I could not run it. I used the issue and PR search API plus repository trees instead. The bulk PR listing returned only the 100 newest PRs, so older PRs are covered only through the issue search API.

Result: I found no published value of A390813(72), (73), (74), (75) or (76), and no claim that would settle any of them.

## Does a(76) go beyond a(72) plus trivial steps?
Yes.
- a(73) = 34 needs only a witness on top of a(72) = 33, because a(n+1) ≤ a(n) + 1. The same goes for a(75) = 35 on top of a(74) = 34.
- a(74) = 34 and a(76) = 35 are not implied by anything published or by trivial edits. Without new exclusions, the best that a(72) = 33 plus witnesses gives is 35 ≤ a(76) ≤ 36.
- a(76) = 35 therefore needs its own exhaustive exclusion: no 36-subset of {1..76} contains 76. It also needs the exclusion at 74, so that a(75) ≤ 35.
- The package supplies these as separate runs:
  - logs/bb2_N76_T36_D8_P8_p*.log and logs/bb3_N76_T36_D8_P8_p*.log: 8/8 parts EXHAUSTED each.
  - search3/logs/rds2_k76_T36_p*.log: 3/3 "NONE … completed=1".
  - The same three programs cover N = 74.
- The value a(76) lies strictly beyond the published frontier (68 unconditional, 71 conditional) and is 4 places beyond today's unpublished a(72). It is not a tie.

## Dependencies of the certificate
- a(1)..a(68) come from the OEIS b-file. The criterion allows this. search3/bounds68.txt matches the b-file except for a single comment line added at the top.
- a(69)..a(75) are used as pruning bounds. None of them is taken from a publication. They are re-certified inside this package in order (search3/chain_summary.txt; logs for N = 69..75).
- a(72) = 33 is unpublished. It comes only from this workspace: logs/cond_N72_T34_D8_P8_p*.log, logs/bb3_N72_T34_D8_P8_p*.log and search3/logs/rds2_k72_T34_p*.log. a(69), a(70) and a(71) agree with the conditional values on Erdős Problem a Day.
- So the a(76) certificate rests on the OEIS b-file plus a chain certified only in-package by the claimant's own programs. No outside party has certified any value past a(71). The criterion permits this ("or be re-certified in the same run"). Whether the in-package chain is correct is for the correctness and reproduction skeptics to check, not this one.
- Housekeeping: three logs are empty runs that were stopped (logs/bb2_N72_T34.log, logs/bb2_N73_T34_D8_P8_p1.log, logs/bb2_N75_T35_D8_P8_p3.log). The certificate does not need them, because the witnesses for 73 and 75 were found in other parts. The bb2-style exclusion at N = 72 is in the logs/cond_N72_* files, not in a bb2_N72_*_p* set as REPRODUCE.md implies.

novelty: NEW. As of Sun Nov 22 16:00–16:04 UTC 2026, A390813(76) = 35 appears in no source I could find. It strictly extends the known frontier (a(68) published, a(71) conditional) and needs its own exhaustive exclusion beyond a(72).
pass: true
