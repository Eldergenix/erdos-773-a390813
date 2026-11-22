# How we found it (contributor text, unverified)

After the run's lenses had settled a(72) = 33, two of them kept the same Russian-doll branch and bound going. A
valid set of size a(k-1)+1 in {1..k} must contain k, so each new term needs one decision. At 73 and 75 a witness
turned up (so a(73) = 34, a(75) = 35 with no search needed, since a(n+1) <= a(n)+1); at 74 and 76 the tree was
exhausted without one (a(74) = 34, a(76) = 35). Two separately written programs (bb2, bb3) did every exclusion; a
referee session then wrote a third program from the definition and re-derived the whole chain 69..76 from the OEIS
prefix, and the panel's reproduction skeptic wrote a fourth.

What failed on the way: SAT with DRAT proofs and ILP/CP-SAT on the collision model (far too slow beyond n of about
55 to 60). What made the long trees tractable: turning nearly complete forbidden sets into pair conflicts, greedy
clique covers of the conflict graph, and Knuth-style tree-size estimates to schedule the split runs.
