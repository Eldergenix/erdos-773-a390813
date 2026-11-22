/*
 * bb3.c -- exact branch-and-bound for "Sidon in squares" sets (OEIS A390813).
 *
 * A set X of positive integers is Sidon in squares if all x^2+y^2 (x<=y in X,
 * x=y allowed) are distinct.  a(n) = max |X|, X subset of {1..n}.
 *
 * usage: bb3 N T D P part A1 ... A_{N-1}
 *   Decides whether a Sidon-in-squares S subset of {1..N} with N in S and
 *   |S| >= T exists.  A_k must be valid upper bounds on a(k).
 *   D>0: the subtrees rooted at search depth D (first node on a path whose
 *   depth N-1-k >= D, k = next undecided index) are numbered 0,1,2,... in DFS
 *   order and only those with number % P == part are explored.  D=0: no split.
 *   Compile with -DCHECK for (slow) internal self-checks against brute force.
 *
 * ---------------------------------------------------------------------------
 * SEARCH.  S is the current partial set (all elements > k, N in S), "used" is
 * a byte table indexed by the sums x^2+y^2 (x<=y in S), size 2N^2+1.
 * Indices are decided in decreasing order (N first, then N-1, ...), include
 * before exclude.
 *
 * FEASIBILITY (from the definition).  S u {w} (w not in S) is Sidon iff the
 * new sums 2w^2 and w^2+s^2 (s in S) are all unused.  (The new sums are
 * pairwise distinct: w^2+s^2 = w^2+s'^2 forces s = s', and w^2+s^2 = 2w^2
 * forces s = w.)  Every inclusion is tested this way, and the candidate set
 * cand = { w <= k : S u {w} Sidon } is recomputed with exactly this test after
 * every inclusion (after an exclusion only k is removed: S is unchanged).
 * Indices k not in cand cannot be included, so the search jumps directly to
 * the largest member of cand (equivalent to excluding the skipped indices).
 *
 * PAIR CONFLICTS.  For u != w both in cand, S u {u,w} is Sidon iff none of
 * the following holds (the only possible coincidences between the new sums
 * {u^2+s^2, 2u^2, w^2+t^2, 2w^2, u^2+w^2 : s,t in S} and the old sums or
 * among themselves, given that u and w are individually addable):
 *   (a) u^2+w^2 is a used sum  (= s^2+t^2, s,t in S, s=t allowed);
 *   (b) u^2+s^2 = w^2+t^2, s != t in S, i.e. |u^2-w^2| = |s^2-t^2|;
 *   (c) u^2+s^2 = 2w^2 or w^2+s^2 = 2u^2, s in S.
 *   (u^2+w^2 vs u^2+s^2 / w^2+t^2 / 2u^2 / 2w^2 / 2u^2 vs 2w^2 would force
 *   w=s, u=t, u=w, so they never coincide.)
 * conf[L][u] is the bitset of w with (a)/(b)/(c) for the S of size L on the
 * current path.  When x is added to S (x below all of S), the new conflicts
 * are exactly the pairs with u,w < x such that
 *   (a) u^2+w^2 = x^2+t^2 (t in S) or = 2x^2,
 *   (b) |u^2-w^2| = t^2-x^2 (t in S),
 *   (c) u^2+x^2 = 2w^2 (either orientation);
 * these are enumerated from precomputed representation lists of integers as
 * u^2+w^2, w^2-u^2 and 2w^2-u^2 (pure arithmetic, no collision list), and
 * added to a copy of the parent matrix.  SOUNDNESS: each recorded pair is a
 * genuine coincidence of sums of S u {u,w}, and S only grows along a path, so
 * every recorded conflict is real: if u,w conflict then no superset of
 * S u {u,w} is Sidon.  (Completeness is not needed for validity; it is
 * nevertheless cross-checked under -DCHECK, together with cand.)
 *
 * BOUNDS (every one is an upper bound on the number of elements that can
 * still be added to S; the node is pruned if |S| + bound < T):
 *  - prefix bound: any completion S' is a subset of {1..k} and is itself
 *    Sidon in squares (subset of a Sidon set), so |S'| <= a(k) <= A_k.
 *    More generally S' n [1..v-1] is Sidon in {1..v-1}: |S' n [1..v-1]| <=
 *    A_{v-1} (A_0 = 0).
 *  - candidates: S' must be a subset of cand (an element that cannot be
 *    added to S cannot be added to any superset of S).
 *  - clique cover: cand is partitioned greedily (in decreasing order, first
 *    fit) into cliques of the conflict graph.  Any two members of a clique
 *    conflict, so S' contains at most one member of each clique.  Hence for
 *    every threshold v in 1..k+1:
 *       |S'| = |S' n [v..k]| + |S' n [1..v-1]|
 *            <= #{cliques with a member in [v..k]}            (= top >= v)
 *             + min(A_{v-1}, #{cliques with a member < v})    (= bottom < v)
 *    and the bound is the minimum over v.  (v = k+1 gives min(A_k, #cliques),
 *    v = 1 gives #cliques.)  Cliques straddling v are counted twice, which
 *    only weakens (never invalidates) the bound.
 *  - skipping a non-candidate index never loses pruning: with cand unchanged
 *    the bound at k-1 is <= the bound at k (term v=k becomes
 *    0 + min(A_{k-1}, ...) <= min(A_k, ...)).
 * ---------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define MAXN 100

typedef struct { uint64_t w[2]; } bs_t;

static inline int bs_test(const bs_t *b, int i) { return (int)((b->w[i >> 6] >> (i & 63)) & 1); }
static inline void bs_set(bs_t *b, int i) { b->w[i >> 6] |= 1ULL << (i & 63); }
static inline void bs_clr(bs_t *b, int i) { b->w[i >> 6] &= ~(1ULL << (i & 63)); }
static inline int bs_top(const bs_t *b) {  /* highest set bit, 0 if empty (bit 0 never used) */
    if (b->w[1]) return 64 + 63 - __builtin_clzll(b->w[1]);
    if (b->w[0]) return 63 - __builtin_clzll(b->w[0]);
    return 0;
}

static int N, T, D, P, part;
static int A[MAXN + 2];
static int sq[MAXN + 2];
static uint8_t *used;                 /* used[x^2+y^2] = 1 for x<=y in S */
static int S[MAXN + 2], ns;
static bs_t conf[MAXN + 2][MAXN + 2]; /* conf[|S|][u] */
static long long nodes = 0, subtree_ctr = 0;
static double t0;

/* representation lists (CSR) */
static int *sumStart, *sumU, *sumW;   /* sigma = u^2+w^2, u<w */
static int *difStart, *difU, *difW;   /* d = w^2-u^2, u<w */
static int midStart[MAXN + 2], *midU, *midW; /* for x: u^2+x^2 = 2w^2, u!=w */

static double now(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void build_lists(void) {
    int maxs = 2 * N * N + 1, maxd = N * N + 1;
    sumStart = calloc(maxs + 1, sizeof(int)); difStart = calloc(maxd + 1, sizeof(int));
    int np = N * (N - 1) / 2 + 1;
    sumU = malloc(np * sizeof(int)); sumW = malloc(np * sizeof(int));
    difU = malloc(np * sizeof(int)); difW = malloc(np * sizeof(int));
    for (int u = 1; u <= N; u++) for (int w = u + 1; w <= N; w++) {
        sumStart[sq[u] + sq[w] + 1]++; difStart[sq[w] - sq[u] + 1]++;
    }
    for (int i = 0; i < maxs; i++) sumStart[i + 1] += sumStart[i];
    for (int i = 0; i < maxd; i++) difStart[i + 1] += difStart[i];
    int *fs = malloc((maxs + 1) * sizeof(int)), *fd = malloc((maxd + 1) * sizeof(int));
    memcpy(fs, sumStart, (maxs + 1) * sizeof(int)); memcpy(fd, difStart, (maxd + 1) * sizeof(int));
    for (int u = 1; u <= N; u++) for (int w = u + 1; w <= N; w++) {
        int s = sq[u] + sq[w], d = sq[w] - sq[u];
        sumU[fs[s]] = u; sumW[fs[s]] = w; fs[s]++;
        difU[fd[d]] = u; difW[fd[d]] = w; fd[d]++;
    }
    free(fs); free(fd);
    /* mid lists */
    int cnt = 0;
    midU = malloc((size_t)(N + 1) * (N + 1) * sizeof(int)); midW = malloc((size_t)(N + 1) * (N + 1) * sizeof(int));
    for (int x = 1; x <= N; x++) {
        midStart[x] = cnt;
        for (int w = 1; w <= N; w++) {
            if (w == x) continue;
            int v = 2 * sq[w] - sq[x];
            if (v <= 0) continue;
            int u = 1; while (u * u < v) u++;
            if (u * u == v && u <= N && u != w && u != x) { midU[cnt] = u; midW[cnt] = w; cnt++; }
        }
    }
    midStart[N + 1] = cnt;
}

static inline int feasible(int w) {
    if (used[2 * sq[w]]) return 0;
    for (int i = 0; i < ns; i++) if (used[sq[w] + sq[S[i]]]) return 0;
    return 1;
}

static inline void addconf(bs_t *cf, int u, int w) { bs_set(&cf[u], w); bs_set(&cf[w], u); }

/* add x (feasible, below every element of S) */
static void include(int x) {
    bs_t *src = conf[ns], *dst = conf[ns + 1];
    memcpy(&dst[1], &src[1], (size_t)(x - 1) * sizeof(bs_t));
    int x2 = sq[x];
    for (int i = 0; i < ns; i++) {
        int t = S[i];
        int s = x2 + sq[t];
        used[s] = 1;
        for (int j = sumStart[s]; j < sumStart[s + 1]; j++) if (sumW[j] < x) addconf(dst, sumU[j], sumW[j]);
        int d = sq[t] - x2; /* t > x */
        for (int j = difStart[d]; j < difStart[d + 1]; j++) if (difW[j] < x) addconf(dst, difU[j], difW[j]);
    }
    int s = 2 * x2; used[s] = 1;
    for (int j = sumStart[s]; j < sumStart[s + 1]; j++) if (sumW[j] < x) addconf(dst, sumU[j], sumW[j]);
    for (int j = midStart[x]; j < midStart[x + 1]; j++)
        if (midU[j] < x && midW[j] < x) addconf(dst, midU[j], midW[j]);
    S[ns++] = x;
}

static void removelast(void) {
    int x = S[--ns];
    for (int i = 0; i < ns; i++) used[sq[x] + sq[S[i]]] = 0;
    used[2 * sq[x]] = 0;
}

/* independent from-scratch check of a set */
static int sidon_scratch(const int *X, int m) {
    uint8_t *tab = calloc(2 * N * N + 1, 1); int ok = 1;
    for (int i = 0; i < m && ok; i++) for (int j = i; j < m; j++) {
        int s = X[i] * X[i] + X[j] * X[j];
        if (tab[s]) { ok = 0; break; }
        tab[s] = 1;
    }
    free(tab); return ok;
}

static int cmpint(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

static void found(void) {
    int X[MAXN + 2]; memcpy(X, S, ns * sizeof(int)); qsort(X, ns, sizeof(int), cmpint);
    if (!sidon_scratch(X, ns) || X[ns - 1] != N) { fprintf(stderr, "INTERNAL ERROR: bad witness\n"); exit(2); }
    printf("FOUND size=%d :", ns);
    for (int i = 0; i < ns; i++) printf(" %d", X[i]);
    printf("\n");
    printf("nodes=%lld time=%.2f\n", nodes, now() - t0);
    fflush(stdout); exit(0);
}

static int bound(int k, const bs_t *cand) {
    const bs_t *cf = conf[ns];
    bs_t M[MAXN + 2]; int bot[MAXN + 2]; int nc = 0;
    bs_t topB = {{0, 0}}, botB = {{0, 0}};
    for (int wi = 1; wi >= 0; wi--) {
        uint64_t x = cand->w[wi];
        while (x) {
            int b = 63 - __builtin_clzll(x); x &= ~(1ULL << b);
            int w = wi * 64 + b, j;
            for (j = 0; j < nc; j++) if (bs_test(&M[j], w)) break;
            if (j < nc) { M[j].w[0] &= cf[w].w[0]; M[j].w[1] &= cf[w].w[1]; bot[j] = w; }
            else { M[nc] = cf[w]; bot[nc] = w; bs_set(&topB, w); nc++; }
        }
    }
    for (int j = 0; j < nc; j++) bs_set(&botB, bot[j]);
    int best = A[k] < nc ? A[k] : nc;      /* v = k+1 */
    int hi = 0, botGE = 0;
    for (int v = k; v >= 1; v--) {
        hi += bs_test(&topB, v); botGE += bs_test(&botB, v);
        int lo = nc - botGE;
        int val = hi + (A[v - 1] < lo ? A[v - 1] : lo);
        if (val < best) best = val;
    }
    return best;
}

#ifdef CHECK
static void check_state(int k, const bs_t *cand) {
    for (int w = 1; w <= N; w++) {
        int inS = 0; for (int i = 0; i < ns; i++) if (S[i] == w) inS = 1;
        int want = (w <= k && !inS);
        if (want) { int X[MAXN + 2]; memcpy(X, S, ns * sizeof(int)); X[ns] = w; want = sidon_scratch(X, ns + 1); }
        if (want != bs_test(cand, w)) { fprintf(stderr, "CHECK FAIL cand w=%d k=%d\n", w, k); exit(3); }
    }
    for (int u = 1; u <= k; u++) if (bs_test(cand, u)) for (int w = u + 1; w <= k; w++) if (bs_test(cand, w)) {
        int X[MAXN + 2]; memcpy(X, S, ns * sizeof(int)); X[ns] = u; X[ns + 1] = w;
        int c = !sidon_scratch(X, ns + 2);
        if (c != bs_test(&conf[ns][u], w) || c != bs_test(&conf[ns][w], u)) {
            fprintf(stderr, "CHECK FAIL conf u=%d w=%d brute=%d\n", u, w, c); exit(3);
        }
    }
}
#endif

static void dfs(bs_t cand, int split_done) {
    nodes++;
    if ((nodes & ((1LL << 24) - 1)) == 0) {
        fprintf(stderr, "progress N=%d T=%d nodes=%lld subtrees=%lld |S|=%d time=%.1f S:", N, T, nodes, subtree_ctr, ns, now() - t0);
        for (int i = 0; i < ns; i++) fprintf(stderr, " %d", S[i]);
        fprintf(stderr, "\n");
    }
    if (ns >= T) found();
    int k = bs_top(&cand);
    if (k == 0) return;
    if (D > 0 && !split_done && N - 1 - k >= D) {
        split_done = 1;
        if ((subtree_ctr++ % P) != part) return;
    }
#ifdef CHECK
    check_state(k, &cand);
#endif
    if (ns + bound(k, &cand) < T) return;
    /* include k */
    if (!feasible(k)) { fprintf(stderr, "INTERNAL ERROR: candidate infeasible\n"); exit(2); }
    include(k);
    bs_t nc = {{0, 0}};
    for (int wi = 0; wi < 2; wi++) {
        uint64_t x = cand.w[wi];
        while (x) {
            int b = __builtin_ctzll(x); x &= x - 1;
            int w = wi * 64 + b;
            if (w < k && feasible(w)) bs_set(&nc, w);
        }
    }
#ifdef CHECK
    { bs_t alt = cand; bs_clr(&alt, k);
      alt.w[0] &= ~conf[ns - 1][k].w[0]; alt.w[1] &= ~conf[ns - 1][k].w[1];
      for (int w = k; w < 128; w++) bs_clr(&alt, w);
      if (alt.w[0] != nc.w[0] || alt.w[1] != nc.w[1]) { fprintf(stderr, "CHECK FAIL newcand vs conf\n"); exit(3); } }
#endif
    dfs(nc, split_done);
    removelast();
    /* exclude k */
    bs_clr(&cand, k);
    dfs(cand, split_done);
}

int main(int argc, char **argv) {
    if (argc < 6) { fprintf(stderr, "usage: bb3 N T D P part A1 ... A_{N-1}\n"); return 1; }
    N = atoi(argv[1]); T = atoi(argv[2]); D = atoi(argv[3]); P = atoi(argv[4]); part = atoi(argv[5]);
    if (N < 1 || N > MAXN || P < 1 || part < 0 || part >= P || D < 0) { fprintf(stderr, "bad args\n"); return 1; }
    if (argc != 6 + N - 1) { fprintf(stderr, "need exactly N-1=%d values A1..A_{N-1}, got %d\n", N - 1, argc - 6); return 1; }
    A[0] = 0;
    for (int k = 1; k < N; k++) A[k] = atoi(argv[5 + k]);
    for (int i = 0; i <= N + 1; i++) sq[i] = i * i;
    used = calloc(2 * N * N + 1, 1);
    build_lists();
    t0 = now();
    ns = 0;
    include(N);                       /* N is always in S */
    bs_t cand = {{0, 0}};
    for (int w = 1; w < N; w++) if (feasible(w)) bs_set(&cand, w);
    dfs(cand, 0);
    printf("EXHAUSTED N=%d T=%d nodes=%lld time=%.2f", N, T, nodes, now() - t0);
    if (D > 0) printf(" D=%d P=%d part=%d subtrees=%lld", D, P, part, subtree_ctr);
    printf("\n");
    return 0;
}
