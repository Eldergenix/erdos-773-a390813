# How we found it (contributor text, unverified)

The run gave six independent AI lens sessions the same brief (definition, the OEIS prefix to n = 68, the
conditional values a(69..71), pitfalls) and a rule frozen beforehand: a new value counts only with a witness and two
independently written, uncapped exhaustive searches that agree, using no prefix value beyond the b-file unless
re-proved in the same run.

What failed: SAT with cardinality encodings and DRAT proofs (about 15 times more conflicts per 4 indices; stalled
near n = 55); CP-SAT and HiGHS on the full collision model (about 100 times slower than branch and bound at n = 60).

What worked: Russian-doll branch and bound over prefixes. A valid set of size a(k-1)+1 in {1..k} must contain k;
forbidden configurations are the 762 four-sets a^2+d^2 = b^2+c^2 and 23 three-sets a^2+c^2 = 2b^2 in 1..72; a
partial set turns longer forbidden sets into pair conflicts, and a greedy clique cover of the conflict graph plus
already-proved prefix optima bounds the extension. Four separately written programs (three of them in this package)
agree: no 34-set exists in {1..72}, and 33-sets exist. A parity/difference-table cover certificate from a fifth
session agrees as well (its checker is in how-we-built-it/; the 2.4 MB certificate file is not included).
A hostile-referee session and then a three-skeptic panel reran everything with their own code.
