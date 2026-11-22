/* bb.c -- exact branch and bound for Sidon sets in the squares (lens search-engineer).
   Decides: does a Sidon-in-squares set S subset {1..N} with N in S and |S| >= T exist?
   Supports (minimal collision sets) are recomputed here from the definition, independent of the Python code.
   Bounds used at a node (vertices k..1 still to decide, c already chosen):
     prefix bound   : c + A[k] where A[k] = a(k) is an upper bound on any Sidon subset of {1..k}
     clique bound   : greedy clique partition (descending order) of live vertices in the pair-conflict graph,
                      combined with the prefix bound: c + q(v..k) + A[v-1], minimised over v.
   Usage: bb N T split_depth parts part  A1 A2 ... A_{N-1}
   Prints FOUND + witness, or EXHAUSTED with node counts.  No time limit, no node cap. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef unsigned __int128 u128;
#define MAXN 128
#define MAXS 20000
#define BIT(i) (((u128)1) << (i))
static int N, T, D, P, PART;
static int A[MAXN + 1];
static int nsup, ssz[MAXS];
static u128 smask[MAXS];
static int vs[MAXN + 1][2000], vdeg[MAXN + 1];
static int cnt[MAXS];
static u128 chosen;
static int chosenc;
static long long nodes = 0, leafid = 0;
static int found = 0;
static time_t t0;

static void gen_supports(void) {
    /* all pairs x<=y, group by sum, every pair of representations gives a support */
    static int px[MAXN * MAXN], py[MAXN * MAXN];
    static long long key[MAXN * MAXN];
    int np = 0;
    for (int x = 1; x <= N; x++)
        for (int y = x; y <= N; y++) { px[np] = x; py[np] = y; key[np] = (long long)x * x + (long long)y * y; np++; }
    nsup = 0;
    for (int i = 0; i < np; i++)
        for (int j = i + 1; j < np; j++)
            if (key[i] == key[j]) {
                u128 m = BIT(px[i]) | BIT(py[i]) | BIT(px[j]) | BIT(py[j]);
                int dup = 0;
                for (int s = 0; s < nsup; s++) if (smask[s] == m) { dup = 1; break; }
                if (dup) continue;
                if (nsup >= MAXS) { fprintf(stderr, "MAXS\n"); exit(2); }
                smask[nsup] = m;
                int c = 0; for (int v = 1; v <= N; v++) if (m & BIT(v)) c++;
                ssz[nsup] = c; nsup++;
            }
    /* minimality check: no support strictly contains another */
    int nonmin = 0;
    for (int a = 0; a < nsup; a++) for (int b = 0; b < nsup; b++)
        if (a != b && (smask[a] & smask[b]) == smask[a] && smask[a] != smask[b]) nonmin++;
    int t3 = 0, t4 = 0;
    for (int s = 0; s < nsup; s++) { if (ssz[s] == 3) t3++; else if (ssz[s] == 4) t4++; else { fprintf(stderr, "size %d?\n", ssz[s]); exit(2); } }
    printf("supports=%d triples=%d quads=%d nonminimal_pairs=%d\n", nsup, t3, t4, nonmin);
    memset(vdeg, 0, sizeof vdeg);
    for (int s = 0; s < nsup; s++) for (int v = 1; v <= N; v++) if (smask[s] & BIT(v)) vs[v][vdeg[v]++] = s;
}

static inline int popc(u128 x) { return __builtin_popcountll((unsigned long long)x) + __builtin_popcountll((unsigned long long)(x >> 64)); }
static inline int lowbit(u128 x) { unsigned long long lo = (unsigned long long)x; if (lo) return __builtin_ctzll(lo); return 64 + __builtin_ctzll((unsigned long long)(x >> 64)); }
static inline int highbit(u128 x) { unsigned long long hi = (unsigned long long)(x >> 64); if (hi) return 127 - __builtin_clzll(hi); return 63 - __builtin_clzll((unsigned long long)x); }

/* upper bound on additional elements from live vertices <= k.
   Partition of L = live & [1..k] into disjoint parts with a valid max m(P):
     greedy cliques of the pair-conflict graph (m=1), then among singleton vertices disjoint residual supports
     whose unchosen part is 3 live vertices (m=2) or 4 live vertices (m=3).
   For every threshold v: extra <= sum_P min(m(P), |P cap [v..k]|) + A[v-1]. */
static int USE_HYPER = 1;
static int bound_extra(int k, u128 live, const u128 *adj) {
    u128 L = live & ((BIT(k + 1)) - 1);
    int best = A[k];
    int pc = popc(L);
    if (pc < best) best = pc;
    if (!L) return 0;
    u128 cand[MAXN + 1], memb[MAXN + 1];
    int q = 0;
    u128 LL = L;
    while (LL) {
        int v = highbit(LL); LL &= ~BIT(v);
        int j;
        for (j = 0; j < q; j++) if (cand[j] & BIT(v)) { cand[j] &= adj[v]; memb[j] |= BIT(v); break; }
        if (j == q) { cand[q] = adj[v]; memb[q] = BIT(v); q++; }
    }
    /* contribution flag: vertex v contributes +1 when scanned (descending) if its rank in its part < m(P) */
    u128 zero = 0; /* vertices contributing 0 */
    u128 single = 0;
    for (int j = 0; j < q; j++) {
        if (popc(memb[j]) == 1) single |= memb[j];
        else { u128 m = memb[j]; m &= ~BIT(highbit(m)); zero |= m; }
    }
    if (USE_HYPER && popc(single) >= 3) {
        for (int pass = 3; pass <= 4; pass++)
            for (int s = 0; s < nsup; s++) {
                u128 rem = smask[s] & ~chosen;
                if ((rem & ~single) == 0 && popc(rem) == pass) {
                    single &= ~rem;
                    zero |= BIT(lowbit(rem)); /* lowest is scanned last: contributes 0 */
                    if (popc(single) < 3) goto done;
                }
            }
    }
done:;
    int run = 0;
    LL = L;
    while (LL) {
        int v = highbit(LL); LL &= ~BIT(v);
        if (!(zero & BIT(v))) run++;
        int b = run + A[v - 1];
        if (b < best) best = b;
    }
    if (run < best) best = run;
    return best;
}

static void report_found(void) {
    printf("FOUND size=%d :", chosenc);
    for (int v = 1; v <= N; v++) if (chosen & BIT(v)) printf(" %d", v);
    printf("\n"); fflush(stdout);
}

/* depth = number of decided indices below N (for splitting) */
static void search(int k, u128 live, const u128 *adj, int depth) {
    if (found) return;
    nodes++;
    if ((nodes & ((1LL << 24) - 1)) == 0) {
        fprintf(stderr, "progress nodes=%lld k=%d c=%d t=%lds\n", nodes, k, chosenc, (long)(time(NULL) - t0));
    }
    if (chosenc >= T) { found = 1; report_found(); return; }
    while (k >= 1 && !(live & BIT(k))) { k--; }
    if (k < 1) return;
    if (chosenc + bound_extra(k, live, adj) < T) return;
    if (D > 0 && depth == D) {
        long long id = leafid++;
        if (id % P != PART) return;
    }
    /* include k */
    {
        u128 nadj[MAXN + 1];
        memcpy(nadj, adj, sizeof(u128) * (N + 1));
        u128 nlive = live & ~BIT(k);
        chosen |= BIT(k); chosenc++;
        for (int i = 0; i < vdeg[k]; i++) {
            int s = vs[k][i];
            cnt[s]++;
            u128 rem = smask[s] & ~chosen;
            int r = ssz[s] - cnt[s];
            if (r == 1) nlive &= ~rem;
            else if (r == 2) { int u = lowbit(rem), w = highbit(rem); nadj[u] |= BIT(w); nadj[w] |= BIT(u); }
        }
        search(k - 1, nlive, nadj, depth + 1);
        for (int i = 0; i < vdeg[k]; i++) cnt[vs[k][i]]--;
        chosen &= ~BIT(k); chosenc--;
        if (found) return;
    }
    /* exclude k */
    search(k - 1, live & ~BIT(k), adj, depth + 1);
}

int main(int argc, char **argv) {
    if (argc < 6) { fprintf(stderr, "usage: bb N T split_depth parts part A1..A_{N-1}\n"); return 1; }
    if (getenv("NOHYPER")) USE_HYPER = 0;
    N = atoi(argv[1]); T = atoi(argv[2]); D = atoi(argv[3]); P = atoi(argv[4]); PART = atoi(argv[5]);
    if (argc != 6 + N - 1) { fprintf(stderr, "need %d prefix values\n", N - 1); return 1; }
    A[0] = 0;
    for (int k = 1; k < N; k++) A[k] = atoi(argv[5 + k]);
    t0 = time(NULL);
    gen_supports();
    memset(cnt, 0, sizeof cnt);
    u128 adj[MAXN + 1]; memset(adj, 0, sizeof adj);
    u128 live = 0; for (int v = 1; v <= N; v++) live |= BIT(v);
    /* choose N */
    chosen = 0; chosenc = 0;
    int k = N;
    chosen |= BIT(k); chosenc = 1; live &= ~BIT(k);
    for (int i = 0; i < vdeg[k]; i++) {
        int s = vs[k][i]; cnt[s]++;
        u128 rem = smask[s] & ~chosen; int r = ssz[s] - cnt[s];
        if (r == 1) live &= ~rem;
        else if (r == 2) { int u = lowbit(rem), w = highbit(rem); adj[u] |= BIT(w); adj[w] |= BIT(u); }
    }
    search(N - 1, live, adj, 0);
    if (!found) printf("EXHAUSTED N=%d T=%d split=%d parts=%d part=%d nodes=%lld leaves_at_split=%lld time=%lds\n", N, T, D, P, PART, nodes, leafid, (long)(time(NULL) - t0));
    else printf("nodes=%lld time=%lds\n", nodes, (long)(time(NULL) - t0));
    return 0;
}
