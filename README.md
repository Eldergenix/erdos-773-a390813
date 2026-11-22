# A390813: exact values a(69)–a(76) (Erdős problem #773, Sidon subsets of the squares)

a(n) is the size of the largest Sidon subset of {1², 2², …, n²}: all sums x² + y² with x ≤ y distinct
(OEIS A390813, https://oeis.org/A390813). The OEIS entry gives a(1)–a(68).

| n | 69 | 70 | 71 | 72 | 73 | 74 | 75 | 76 |
|---|---|---|---|---|---|---|---|---|
| a(n) | 32 | 33 | 33 | 33 | 34 | 34 | 35 | 35 |

a(69)–a(71) agree with the values reported by Erdős Problem a Day
(https://erdosproblemaday.com/day/773-square-sidon-frontier). a(72)–a(76) are, as far as we can find, new.
This is computational data. It says nothing new about the asymptotic question (is a(n) = n^(1−o(1))?).

## Check it yourself
Each package verifies itself from its own folder (python3 and a C/C++ compiler):

    cd packages/a72-2026-11-22-eldergenix-bc2e12d9 && sh verify-it-yourself/run_checks.sh   # ~3 CPU min, last line PASS
    cd packages/a76-2026-11-22-eldergenix-31ef6d4a && sh verify-it-yourself/run_checks.sh   # ~10 CPU min, last line PASS

The full chain a(69)–a(76) with both search programs is in `packages/a76-*/RUN_IT_YOURSELF.md` (about one CPU
hour). Each package's README states exactly what is and is not claimed.

## How the bounds are proved
Lower bounds: explicit witness sets (in each package's `object/`). Upper bounds: exhaustive, uncapped
branch-and-bound searches that use the OEIS values a(1)–a(68) as pruning bounds, each exclusion run by two
separately written programs that agree. a(72) was also checked by a whole-instance search with no prefix values.
Together with a(n) ≤ a(n+1) ≤ a(n) + 1 this gives the whole row.

## Dated proof
`proofs/` holds a reproducible tar of each package, its sha256 (`SHA256SUMS.txt`) and an OpenTimestamps proof
(`*.ots`, submitted 2026-11-22T18:30:48Z; verify with `ots verify proofs/<file>.tar.ots`).

## OEIS
`oeis/b390813.txt` is a b-file for n = 1..76 (lines 1–68 are the current entry).

## How it was made
The search programs were written by AI model sessions (Claude Opus 5.5, Claude Fable 5.2, GPT-6.2 Astra) under the
V3R1T4S protocol (https://github.com/elder-plinius/V3R1T4S): frozen success criterion, three independent reviewer
sessions, reruns. Not peer reviewed.

## Credits
Noga Alon and Paul Erdős (the problem); Thomas Bloom (https://www.erdosproblems.com/773); Giorgos Kalogeropoulos,
Michael S. Branicky and Christian Sievers (OEIS A390813, terms to n = 68); Patrick White (Erdős Problem a Day #773,
conditional a(69)–a(71)); Hanno Lefmann and Torsten Thiele; Ernie Croot, Junzhe Mao and Kyle Yip.

## License
Code: MIT (LICENSE). Data, results and text: CC BY 4.0 (LICENSE-DATA).

Please cite as: Stefan Creadore, "A390813: exact values a(69)–a(76)", 2026,
https://github.com/Eldergenix/erdos-773-a390813
