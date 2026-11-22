/*
 * Russian-doll maximum Sidon subset of the squares. Input is just an order
 * and an output filename. Every prefix bound is filled by this process.
 * Coordinates are zero-based internally; reported values are square roots.
 */
#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

enum { MAX_N = 72, LOW = 64, MAX_E = 2048, MAX_SUM = 2*MAX_N*MAX_N };
typedef uint64_t Mask;
typedef struct { int len, v[4]; } Support;
typedef struct { int i, j, next; } Pair;
typedef struct { Mask trigger, ends; unsigned char a, b; } Event;
typedef struct {
    uint64_t nodes, splits, count, prefix, cover, mixed, found;
    uint64_t cubes, impossible;
} Stats;

static Support supports[MAX_E];
static int nsupports, optimum[MAX_N+1];
static Mask adj[LOW];
static Event events[LOW][MAX_E];
static int nevents[LOW];
/* Only edges absent on entry are recorded. The live stack contains each
 * unordered pair at most once, even if multiple supports imply it. */
static unsigned short undo[LOW*(LOW-1)/2];
static int top;
static int level, goal, low_count;
static unsigned int cube_id;
static unsigned char fixed_v[MAX_N], fixed_witness[MAX_N], best_witness[MAX_N];
static Mask answer;
static Stats stats;
static double started, last_progress;

static void die(const char *s) { fprintf(stderr, "ERROR %s\n", s); exit(2); }
static double now(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts)) die("clock_gettime");
    return (double)ts.tv_sec + (double)ts.tv_nsec/1e9;
}
static Mask bit(int i) { return UINT64_C(1) << i; }
static Mask below(int k) { return k == LOW ? UINT64_MAX : bit(k)-1; }
static int highest(Mask p) { return 63-__builtin_clzll(p); }
static int count(Mask p) { return __builtin_popcountll(p); }

static void make_supports(int n) {
    int heads[MAX_SUM+1];
    Pair pairs[MAX_N*(MAX_N+1)/2];
    int npairs = 0;
    for (int s=0; s<=MAX_SUM; ++s) heads[s] = -1;
    for (int j=0; j<n; ++j) for (int i=0; i<=j; ++i) {
        int s = (i+1)*(i+1) + (j+1)*(j+1);
        for (int t=heads[s]; t>=0; t=pairs[t].next) {
            int x[4] = {i, j, pairs[t].i, pairs[t].j};
            for (int k=1; k<4; ++k) {
                int v=x[k], h=k;
                while (h && x[h-1]>v) { x[h]=x[h-1]; --h; }
                x[h]=v;
            }
            if (nsupports == MAX_E) die("support storage");
            Support *e = &supports[nsupports++];
            e->len=0;
            for (int k=0; k<4; ++k)
                if (!k || x[k]!=x[k-1]) e->v[e->len++]=x[k];
            if (e->len<3) die("degenerate collision");
        }
        pairs[npairs]=(Pair){i,j,heads[s]};
        heads[s]=npairs++;
    }
}

static void add_edge(int a, int b) {
    adj[a] |= bit(b);
    adj[b] |= bit(a);
}

/* Remove all fixed upper coordinates from every remaining support. The
 * returned mask excludes every singleton. Pair and higher constraints may
 * harmlessly retain a reference to an excluded coordinate. */
static int prepare(Mask *live) {
    memset(adj, 0, sizeof(adj));
    memset(nevents, 0, sizeof(nevents));
    top=0;
    *live=below(low_count);
    for (int j=0; j<nsupports; ++j) {
        const Support *e=&supports[j];
        int r[4], len=0, active=1;
        for (int h=0; h<e->len; ++h) {
            int v=e->v[h];
            if (v<low_count) r[len++]=v;
            else if (!fixed_v[v]) { active=0; break; }
        }
        if (!active) continue;
        if (!len) return 0;
        if (len==1) { *live &= ~bit(r[0]); continue; }
        if (len==2) { add_edge(r[0],r[1]); continue; }
        int v=r[2];
        Event *ev=&events[v][nevents[v]++];
        ev->a=(unsigned char)r[0]; ev->b=(unsigned char)r[1];
        ev->ends=bit(r[0])|bit(r[1]);
        ev->trigger=len==4 ? bit(r[3]) : 0;
    }
    return 1;
}

static void include_events(int v, Mask chosen, Mask remaining) {
    const Event *list=events[v];
    for (int k=0; k<nevents[v]; ++k) {
        const Event *e=&list[k];
        if (e->trigger && !(chosen & e->trigger)) continue;
        if ((remaining & e->ends) != e->ends) continue;
        int a=e->a, b=e->b;
        if (adj[a] & bit(b)) continue;
        add_edge(a,b);
        undo[top++]=(unsigned short)(a*LOW+b);
    }
}
static void restore(int mark) {
    while (top>mark) {
        unsigned int pair=undo[--top];
        int a=(int)(pair/LOW), b=(int)(pair%LOW);
        adj[a] &= ~bit(b); adj[b] &= ~bit(a);
    }
}

/* Each emitted clique contributes at most one. Unpartitioned vertices
 * lie in the prefix ending at their highest bit; its bound is in-run.
 * Return a reason only if a strict upper bound below need was obtained. */
static int cover_reason(Mask rest, int size, int need) {
    int groups=0;
    while (rest && groups<need) {
        int v=highest(rest);
        rest ^= bit(v);
        Mask neighbors=rest & adj[v];
        while (neighbors) {
            int u=highest(neighbors);
            rest ^= bit(u);
            neighbors &= adj[u]; /* no loops, so u is removed here */
            --size;
            if (size<need) return 1;
        }
        ++groups;
        if (rest && groups+optimum[highest(rest)+1]<need) return 2;
    }
    return groups<need ? 1 : 0;
}

static void heartbeat(void) {
    if ((stats.nodes & UINT64_C(0xfffff))!=0) return;
    double t=now();
    if (t-last_progress>=30.0) {
        printf("progress pid=%ld n=%d target=%d cube=%u target_nodes=%" PRIu64
               " elapsed=%.3f\n",(long)getpid(),level,goal,cube_id,stats.nodes,t-started);
        last_progress=t;
    }
}

/* The exclusion child is a tail iteration. Count it exactly as a node. */
static int search(Mask live, Mask chosen, int need) {
    for (;;) {
        ++stats.nodes;
        heartbeat();
        if (need==0) { answer=chosen; ++stats.found; return 1; }
        int size=count(live);
        if (size<need) { ++stats.count; return 0; }
        int v=highest(live);
        if (optimum[v+1]<need) { ++stats.prefix; return 0; }
        if (need==1) { answer=chosen|bit(v); ++stats.found; return 1; }
        int why=cover_reason(live,size,need);
        if (why) {
            if (why==1) ++stats.cover; else ++stats.mixed;
            return 0;
        }
        ++stats.splits;
        live ^= bit(v);
        Mask q=live & ~adj[v];
        int mark=top;
        include_events(v,chosen,q);
        if (search(q,chosen|bit(v),need-1)) return 1;
        restore(mark);
    }
}

static int decide(int target) {
    goal=target;
    int upper_bits=level>LOW ? level-LOW-1 : 0;
    unsigned int cubes=1u<<upper_bits;
    low_count=level<=LOW ? level-1 : LOW;
    memset(&stats,0,sizeof(stats));
    for (unsigned int i=cubes; i>0; --i) {
        cube_id=i-1;
        memset(fixed_v,0,sizeof(fixed_v));
        fixed_v[level-1]=1;
        for (int k=0; k<upper_bits; ++k)
            fixed_v[LOW+k]=(unsigned char)((cube_id>>k)&1u);
        int needed=goal-1-__builtin_popcount(cube_id);
        uint64_t before=stats.nodes;
        ++stats.cubes;
        Mask live=0;
        int feasible=needed>=0 && prepare(&live);
        int ok=0;
        if (feasible) ok=search(live,0,needed);
        else ++stats.impossible;
        printf("cube n=%d target=%d mask=%u outcome=%s nodes=%" PRIu64 "\n",
               level,goal,cube_id,ok?"SAT":feasible?"UNSAT":"FIXED_CONFLICT",stats.nodes-before);
        if (ok) {
            memcpy(fixed_witness,fixed_v,sizeof(fixed_witness));
            for (int k=0; k<low_count; ++k)
                fixed_witness[k]=(unsigned char)((answer>>k)&1u);
            return 1;
        }
    }
    return 0;
}

static void validate(const unsigned char *w, int size) {
    unsigned char seen[MAX_SUM+1]={0};
    int num=0;
    for (int j=0; j<level; ++j) if (w[j]) {
        ++num;
        for (int i=0; i<=j; ++i) if (w[i]) {
            int s=(i+1)*(i+1)+(j+1)*(j+1);
            if (seen[s]++) die("witness collision");
        }
    }
    if (num!=size) die("witness size");
}
static void print_witness(FILE *f, const unsigned char *w) {
    int sep=0;
    for (int i=0; i<level; ++i) if (w[i]) {
        fprintf(f,"%s%d",sep?" ":"",i+1); sep=1;
    }
}

int main(int argc, char **argv) {
    if (argc!=3) die("usage: doll maximum_n prefix.csv");
    int last=atoi(argv[1]);
    if (last<1 || last>MAX_N) die("unsupported order");
    FILE *tab=fopen(argv[2],"w");
    if (!tab) die("open table");
    setvbuf(stdout,NULL,_IOLBF,0); setvbuf(tab,NULL,_IOLBF,0);
    fprintf(tab,"n,a,with_n,without_n,status,targets,nodes,seconds,witness,with_witness\n");
    started=last_progress=now();
    make_supports(last);
    printf("start pid=%ld max_n=%d generated_supports=%d word_bits=%d\n",
           (long)getpid(),last,nsupports,LOW);
    for (level=1; level<=last; ++level) {
        double base=now();
        int previous=optimum[level-1], conditional=previous+1, targets=0;
        uint64_t nodes=0;
        for (;; --conditional) {
            if (conditional<1) die("missing singleton");
            double t=now();
            int ok=decide(conditional);
            ++targets; nodes+=stats.nodes;
            if (stats.nodes!=stats.splits+stats.count+stats.prefix+stats.cover+stats.mixed+stats.found)
                die("node accounting");
            printf("target n=%d k=%d outcome=%s cubes=%" PRIu64 " fixed_conflict=%" PRIu64
                   " nodes=%" PRIu64 " splits=%" PRIu64 " cardinality=%" PRIu64
                   " prefix=%" PRIu64 " clique=%" PRIu64 " clique_prefix=%" PRIu64
                   " accepts=%" PRIu64 " seconds=%.3f\n",level,conditional,ok?"SAT":"UNSAT",
                   stats.cubes,stats.impossible,stats.nodes,stats.splits,stats.count,stats.prefix,
                   stats.cover,stats.mixed,stats.found,now()-t);
            if (ok) break;
        }
        validate(fixed_witness,conditional);
        if (!fixed_witness[level-1]) die("missing required last index");
        if (conditional>=previous) memcpy(best_witness,fixed_witness,sizeof(best_witness));
        optimum[level]=conditional>previous ? conditional : previous;
        validate(best_witness,optimum[level]);
        const char *status=conditional>previous?"required":conditional<previous?"excluded":"optional";
        int triples=0, quads=0;
        for (int k=0; k<nsupports; ++k)
            if (supports[k].v[supports[k].len-1]<level) {
                if (supports[k].len==3) ++triples; else ++quads;
            }
        double seconds=now()-base;
        fprintf(tab,"%d,%d,%d,%d,%s,%d,%" PRIu64 ",%.6f,",
                level,optimum[level],conditional,previous,status,targets,nodes,seconds);
        print_witness(tab,best_witness); fputc(',',tab);
        print_witness(tab,fixed_witness); fputc('\n',tab);
        printf("level n=%d a=%d with_n=%d without_n=%d status=%s triples=%d quads=%d"
               " nodes=%" PRIu64 " seconds=%.3f elapsed=%.3f\n",level,optimum[level],
               conditional,previous,status,triples,quads,nodes,seconds,now()-started);
    }
    if (fclose(tab)) die("close table");
    printf("finished through=%d elapsed=%.3f\n",last,now()-started);
    return 0;
}
