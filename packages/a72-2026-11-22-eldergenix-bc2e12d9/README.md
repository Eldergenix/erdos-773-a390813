# A390813(72) = 33 (Erdős problem 773, Sidon subsets of the squares)

## What
Under the definition with x = y sums included, the listed 33-element subset of {1..72} is valid and no 34-element subset exists, as shown by my reruns of two independently written, uncapped exact searches (searchC/rds2.c self-contained from k = 1, and difference_search.cpp on the whole instance with no prefix values) that agree with each other, with the saved search A log and with the b-file for n <= 68, so a(72) = 33; non-appearance of the value is supported only by the scout's recorded reads of the entry, b-file, Erdős Problem a Day page and forum, and is not evidenced for the OEIS history page.

## Check it yourself
`sh verify-it-yourself/run_checks.sh` from this folder (needs python3 and a C++17 compiler). Expected last line:
`PASS`. About 2 minutes of CPU on a laptop (it checks the witness from the definition, then rebuilds and reruns one
exhaustive, uncapped whole-instance search: 123,914,694 calls, 32 of 32 parts "answer=no" at size 34).

## Compared to
| table | cell | value there | URL | fetched (UTC) |
|---|---|---|---|---|
| OEIS A390813 b-file | a(72) | absent (last line "68 32") | https://oeis.org/A390813/b390813.txt | 2026-11-22T11:48:28Z |
| Erdős Problem a Day #773 | a(72) | absent (conditional values through a(71) = 33) | https://erdosproblemaday.com/day/773-square-sidon-frontier | 2026-11-22T11:48:29Z |
The value here: a(72) = 33. Before it, a(72) was known only to be 33 or 34 (conditionally on a(71) = 33).

## Not claimed
- Nothing about a(73) or later terms (a(73) = 34 follows from a 34-element witness, which is not part of this package).
- No machine-checked (DRAT/LRAT) proof: the upper bound rests on independently written exhaustive searches that agree.
- The lower bound a(72) >= 33 alone is not new (the published 33-set for n = 70 already gives it).
- a(69) = 32, a(70) = 33, a(71) = 33 are re-derived here as lemmas; they were first reported, conditionally, by
  Patrick White with Claude (Erdős Problem a Day).
- The three panel seats were separate fresh sessions of Anthropic models (the planned cross-vendor seat was
  unavailable); see panel/gate.md.

## Credits
Noga Alon and Paul Erdős (the problem); Thomas Bloom (https://www.erdosproblems.com/773); Giorgos Kalogeropoulos,
Michael S. Branicky and Christian Sievers (https://oeis.org/A390813, terms to n = 68); Patrick White with Claude
(https://erdosproblemaday.com/day/773-square-sidon-frontier, conditional a(69..71), the natural-prefix and
conflict-clique bounds that search A adapts); Hanno Lefmann and Torsten Thiele; Ernie Croot, Junzhe Mao and Kyle Yip
(asymptotic bounds). Searches written by AI model sessions (GPT-6.2 Astra, Claude Fable 5.2, Claude Opus 5.5) run by
Eldergenix under the V3R1T4S kit.
