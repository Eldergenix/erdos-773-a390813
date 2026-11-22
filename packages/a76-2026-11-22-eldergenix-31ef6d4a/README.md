# A390813(76) = 35 (Erdős problem 773, Sidon subsets of the squares; a certified term in the range 73..100)

## What
The listed 35-element set is Sidon in squares inside {1..75}, and three separately written exhaustive searches (search 3 rerun by me for N = 69, 71, 72, 74, 76; searches 1 and 2 rerun by me at N = 76 and taken from saved logs at N = 69, 71, 72, 74) agree that no larger set exists in {1..76} given the b-file values a(1..68), so a(76) = 35; novelty is confirmed only as far as the status record states it, with the OEIS history and forum contents not shown.

## Check it yourself
`sh verify-it-yourself/run_checks.sh` from this folder (python3 and a C compiler; uses 8 processes). Expected last
line: `PASS`. About 10 CPU minutes on a laptop: it checks the witnesses from the definition and exhausts
"a 36-element set in {1..76} containing 76" (8 of 8 parts, 442,644,384 nodes in all). The full chain from the OEIS
prefix (exclusions at 69, 71, 72, 74 as well) is in RUN_IT_YOURSELF.md, about one CPU hour.

## Compared to
| table | cell | value there | URL | fetched (UTC) |
|---|---|---|---|---|
| OEIS A390813 b-file | a(76) | absent (last line "68 32") | https://oeis.org/A390813/b390813.txt | 2026-11-22T11:48:28Z |
| Erdős Problem a Day #773 | a(76) | absent (conditional values through a(71) = 33) | https://erdosproblemaday.com/day/773-square-sidon-frontier | 2026-11-22T11:48:29Z |
The value here: a(76) = 35, with the chain a(69..76) = 32, 33, 33, 33, 34, 34, 35, 35 certified inside the package.

## Not claimed
- This package claims one cell, a(76). The term a(72) = 33 is the subject of a separate package
  (2026-11-22-eldergenix-bc2e12d9) from the same run; a(73), a(74), a(75) appear here only as certified lemmas.
- Nothing about a(77) or later terms, and nothing about which maximum sets contain 76^2.
- No machine-checked (DRAT/LRAT) proof: the upper bounds rest on separately written exhaustive searches that agree.
- The three panel seats were separate fresh sessions of Anthropic models (the planned cross-vendor seat was
  unavailable); see panel/gate.md.

## Credits
Noga Alon and Paul Erdős (the problem); Thomas Bloom (https://www.erdosproblems.com/773); Giorgos Kalogeropoulos,
Michael S. Branicky and Christian Sievers (https://oeis.org/A390813, terms to n = 68); Patrick White with Claude
(https://erdosproblemaday.com/day/773-square-sidon-frontier, conditional a(69..71) and the prefix / conflict-clique
bounding idea). Searches written by AI model sessions (Claude Opus 5.5, Claude Fable 5.2) run by Eldergenix under
the V3R1T4S kit.
