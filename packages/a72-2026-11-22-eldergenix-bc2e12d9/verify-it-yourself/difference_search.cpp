#include <algorithm>
#include <array>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <unistd.h>

// A set of indices fits in 72 bits. Bit v denotes vertex v in the private permutation.
using Set = unsigned __int128;
using U64 = uint64_t;
using Clock = std::chrono::steady_clock;
constexpr int M = 72;
constexpr int undo_capacity = 512;
Set bit(int x) { return Set(1) << x; }
int count(Set s) { return __builtin_popcountll(U64(s)) + __builtin_popcountll(U64(s >> 64)); }
int first(Set s) { const U64 lo = U64(s); return lo ? __builtin_ctzll(lo) : 64 + __builtin_ctzll(U64(s >> 64)); }
int take(Set &s) { int v = first(s); s &= s - 1; return v; }

struct Edge {
    Set vertices;
    int size;
    std::array<int, 4> v;
};
struct Pair { unsigned char a, b; };
struct Rng {
    U64 state = 0xec2f55149e214723ULL;
    U64 word() {
        state ^= state << 13;
        state ^= state >> 7;
        return state ^= state << 17;
    }
    int uniform(int n) { return word() % n; }
};

int n, steps, goal, partition_depth, part_from, part_end;
int order[M], original[M];
Set all_vertices;
std::vector<Edge> edges;
std::vector<int> incidence[M];
std::vector<Pair> activation1[M], activation2[M][M];
Set conflict[M]; // Pairs currently forbidden by already chosen vertices.
U64 calls, selections, pack_cuts, size_cuts;
Set result;
Clock::time_point started, previous_update;
Rng rng;

void stamp() {
    std::time_t now = std::time(nullptr);
    char str[32];
    std::strftime(str, sizeof str, "%FT%TZ", std::gmtime(&now));
    std::printf("%s ", str);
}
void witness(Set s) {
    // Deliberately verify with inclusive unordered SUMS, independent of difference construction.
    std::set<int> sums;
    for (int a = 1; a <= n; ++a) {
        if (!(s & bit(order[a-1]))) continue;
        for (int b = a; b <= n; ++b) if (s & bit(order[b-1])) {
            if (!sums.insert(a*a+b*b).second) {
                std::fprintf(stderr, "invalid sum witness\n");
                std::exit(3);
            }
        }
    }
    stamp();
    std::printf("witness size=%d sums=%zu roots=", count(s), sums.size());
    for (int a = 1; a <= n; ++a) if (s & bit(order[a-1])) std::printf(" %d", a);
    std::puts("");
}
void build() {
    // Generate all equalities of positive differences, counting each forbidden index set once.
    std::map<int, std::vector<std::pair<int, int>>> differences;
    for (int i = 1; i <= n; ++i)
        for (int j = i + 1; j <= n; ++j) differences[j*j-i*i].push_back({i-1, j-1});
    std::set<Set> raw;
    for (const auto &bucket : differences) {
        const auto &pairs = bucket.second;
        for (int i = 0; i < int(pairs.size()); ++i)
            for (int j = 0; j < i; ++j)
                raw.insert(bit(pairs[i].first) | bit(pairs[i].second) | bit(pairs[j].first) | bit(pairs[j].second));
    }
    int degree[M] = {};
    for (Set e : raw) for (Set q = e; q;) ++degree[take(q)];
    for (int i = 0; i < n; ++i) original[i] = i;
    std::sort(original, original + n, [&](int a, int b) {
        if (degree[a] != degree[b]) return degree[a] < degree[b];
        return a < b;
    });
    for (int v = 0; v < n; ++v) order[original[v]] = v;
    int triples = 0;
    for (Set raw_edge : raw) {
        Edge e = {};
        for (Set q = raw_edge; q;) e.vertices |= bit(order[take(q)]);
        for (Set q = e.vertices; q;) {
            int v = take(q);
            e.v[e.size++] = v;
            incidence[v].push_back(int(edges.size()));
        }
        triples += (e.size == 3);
        for (int i = 0; i < e.size; ++i) for (int j = 0; j < i; ++j) {
            Set rest = e.vertices ^ bit(e.v[i]) ^ bit(e.v[j]);
            Pair p = { (unsigned char)e.v[j], (unsigned char)e.v[i] };
            int a = take(rest);
            if (!rest) activation1[a].push_back(p);
            else {
                int b = first(rest);
                activation2[a][b].push_back(p);
                activation2[b][a].push_back(p);
            }
        }
        edges.push_back(e);
    }
    all_vertices = bit(n) - 1;
    stamp(); std::printf("order=%d differences=%zu clauses3=%d clauses4=%zu pid=%d\n",
                       n, differences.size(), triples, edges.size()-triples, getpid());
}

struct Heuristic {
    Set selected = 0;
    int damage[M] = {}, tabu[M] = {}, num_bad = 0;
    std::vector<int> missing, missing_xor;
    Heuristic() : missing(edges.size()), missing_xor(edges.size()) {
        for (int i = 0; i < int(edges.size()); ++i) {
            const Edge &e = edges[i];
            missing[i] = e.size;
            for (int k = 0; k < e.size; ++k) missing_xor[i] ^= e.v[k];
        }
    }
    void toggle(int v) {
        int delta = (selected & bit(v)) ? 1 : -1;
        selected ^= bit(v);
        for (int i : incidence[v]) {
            const Edge &e = edges[i];
            int &m = missing[i], &x = missing_xor[i];
            if (m == 0) { --num_bad; for (int k = 0; k < e.size; ++k) --damage[e.v[k]]; }
            else if (m == 1) --damage[x];
            x ^= v;
            m += delta;
            if (m == 0) { ++num_bad; for (int k = 0; k < e.size; ++k) ++damage[e.v[k]]; }
            else if (m == 1) ++damage[x];
        }
    }
    int pick(Set q, int sign, int turn, int forbidden = -1) {
        int best = -1000000, chosen = -1, ties = 0;
        while (q) {
            int v = take(q);
            if (v == forbidden) continue;
            int value = sign * damage[v] - (tabu[v] > turn ? 1000 : 0);
            if (value > best) { best = value; chosen = v; ties = 1; }
            else if (value == best && rng.uniform(++ties) == 0) chosen = v;
        }
        return chosen;
    }
};
Set find_lower_bound() {
    Set best = 0;
    Heuristic h;
    for (int i = 0; i < n; ++i) {
        int v = h.pick(all_vertices ^ h.selected, -1, 0);
        if (h.damage[v]) break;
        h.toggle(v);
    }
    best = h.selected;
    // Grow the best set one vertex at a time with a reproducible single-exchange tabu walk.
    int i = 1, last = 0;
    while (i <= steps && count(best) < n) {
        if (h.selected == best || i - last > 3000) {
            last = i;
            Set changes = h.selected ^ best;
            while (changes) h.toggle(take(changes));
            std::fill(h.tabu, h.tabu+n, 0);
            h.toggle(h.pick(all_vertices ^ h.selected, -1, i));
        }
        if (!h.num_bad) { best = h.selected; last = i; ++i; continue; }
        int x = h.pick(h.selected, 1, i);
        h.toggle(x);
        int y = h.pick(all_vertices ^ h.selected, -1, i, x);
        if (y < 0) break;
        h.toggle(y);
        h.tabu[x] = i + 2 + rng.uniform(6);
        h.tabu[y] = i + 2 + rng.uniform(6);
        if (!h.num_bad) { best = h.selected; last = i; }
        ++i;
    }
    return best;
}

// Only newly active pairs with both ends still available matter below this decision.
// Undoing the log restores exactly the parent's graph; duplicate proofs are harmless.
int activate(int v, Set selected, Set available, Pair *undo) {
    int added = 0;
    auto insert = [&](const std::vector<Pair> &pairs) {
        for (Pair p : pairs) {
            Set a = bit(p.a), b = bit(p.b);
            if ((available & (a | b)) != (a | b) || (conflict[p.a] & b)) continue;
            if (added == undo_capacity) {
                std::fputs("undo log capacity exceeded\n", stderr);
                std::exit(4);
            }
            conflict[p.a] |= b;
            conflict[p.b] |= a;
            undo[added++] = p;
        }
    };
    insert(activation1[v]);
    while (selected) insert(activation2[v][take(selected)]);
    return added;
}
void restore(Pair *undo, int added) {
    while (added) {
        Pair p = undo[--added];
        conflict[p.a] ^= bit(p.b);
        conflict[p.b] ^= bit(p.a);
    }
}

struct Coloring {
    unsigned char vertex[M], bound[M];
    int length = 0, upper = 0;
    void group(Set q, int cap) {
        const int base = upper;
        int taken = 0;
        while (q) {
            vertex[length] = take(q);
            bound[length++] = base + std::min(cap, ++taken);
        }
        upper += cap;
    }
};

Coloring cover(Set available, Set selected) {
    Coloring out;
    Set singletons = 0;
    Set pending = available;
    while (pending) {
        int v = take(pending);
        Set more = pending & conflict[v];
        if (!more) { singletons |= bit(v); continue; }
        Set clique = bit(v);
        while (more) {
            int w = take(more);
            pending ^= bit(w);
            clique |= bit(w);
            more &= conflict[w];
        }
        out.group(clique, 1);
    }
    // No two of the leftover singleton vertices conflict in the current graph.
    // A remaining complete ternary or quaternary clause costs another slot.
    Set plain = 0;
    while (singletons) {
        int v = first(singletons);
        Set scope = selected | singletons, best = 0;
        int best_size = 5;
        for (int id : incidence[v]) {
            const Set e = edges[id].vertices;
            if (e & ~scope) continue;
            Set residual = e & singletons;
            int size = count(residual);
            if (size < best_size) { best_size = size; best = residual; if (size <= 3) break; }
        }
        if (best) {
            out.group(best, best_size-1);
            singletons ^= best;
        } else {
            plain |= bit(v);
            singletons ^= bit(v);
        }
    }
    out.group(plain, count(plain));
    return out;
}

void heartbeat(int need) {
    auto now = Clock::now();
    if (std::chrono::duration<double>(now-previous_update).count() < 60) return;
    previous_update = now;
    stamp(); std::printf("progress calls=%" PRIu64 " selections=%" PRIu64 " need=%d elapsed=%.1fs\n",
                         calls, selections, need, std::chrono::duration<double>(now-started).count());
}

bool dfs(Set selected, Set available, int need) {
    ++calls;
    if ((calls & 262143) == 0) heartbeat(need);
    if (need == 0) { result = selected; return true; }
    if (count(available) < need) { ++size_cuts; return false; }
    if (need == 1) { result = selected | bit(first(available)); return true; }
    Coloring colors = cover(available, selected);
    if (colors.upper < need) { ++pack_cuts; return false; }
    for (int i = colors.length - 1; i >= 0 && colors.bound[i] >= need; --i) {
        int v = colors.vertex[i];
        available ^= bit(v);
        Set remainder = available & ~conflict[v];
        if (count(remainder) < need-1) continue;
        Pair undo[undo_capacity];
        int added = activate(v, selected, remainder, undo);
        ++selections;
        bool found = dfs(selected | bit(v), remainder, need-1);
        restore(undo, added);
        if (found) return true;
    }
    return false;
}

// Split into all assignments on the partition_depth highest-incidence indices.
bool search(int k) {
    calls = selections = pack_cuts = size_cuts = 0;
    const int parts = 1 << partition_depth;
    for (int p = part_from; p < std::min(parts,part_end); ++p) {
        Set s = 0, a = all_vertices;
        bool valid = true;
        for (int z = 0; z < partition_depth; ++z) {
            int v = n-1-z;
            bool included = (p >> z) & 1;
            if (included && !(a & bit(v))) { valid = false; break; }
            a &= ~bit(v);
            if (included) {
                a &= ~conflict[v];
                Pair undo[undo_capacity];
                activate(v, s, a, undo);
                s |= bit(v);
            }
        }
        U64 before = calls;
        auto t = Clock::now();
        stamp(); std::printf("begin target=%d part=%d/%d roots_chosen=%d\n",k,p,parts,count(s));
        bool found = false;
        if (valid) {
            if (count(s) >= k) {
                result = s; while (count(result) > k) take(result);
                found = true;
            } else found = dfs(s,a,k-count(s));
        }
        std::fill(conflict, conflict+n, Set(0));
        stamp(); std::printf("end target=%d part=%d/%d answer=%s calls=%" PRIu64 " wall=%.4fs\n",
                             k,p,parts,found?"yes":"no",calls-before,std::chrono::duration<double>(Clock::now()-t).count());
        if (found) { witness(result); return true; }
    }
    return false;
}

int main(int argc, char **argv) {
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    if (argc < 2) {
        std::fprintf(stderr,"usage: difference_search n [heuristic_steps=20000] [goal=0, -1 for witness only] [split_bits=0] [first=0] [past=2^split_bits]\n");
        return 2;
    }
    n = std::atoi(argv[1]);
    steps = argc > 2 ? std::atoi(argv[2]) : 20000;
    goal = argc > 3 ? std::atoi(argv[3]) : 0;
    partition_depth = argc > 4 ? std::atoi(argv[4]) : 0;
    part_from = argc > 5 ? std::atoi(argv[5]) : 0;
    if (n < 1 || n > M || steps < 0 || goal < -1 || goal > n+1 || partition_depth < 0 || partition_depth > std::min(n,20)) return 2;
    int parts = 1 << partition_depth;
    part_end = argc > 6 ? std::atoi(argv[6]) : parts;
    if (part_from < 0 || part_from >= part_end || part_end > parts) return 2;
    if (goal == 0 && (part_from || part_end != parts)) return 2;
    started = previous_update = Clock::now();
    build();
    Set best = 0;
    if (goal <= 0 || steps) { best=find_lower_bound(); witness(best); }
    if (goal < 0) return 0;
    if (goal > 0 && part_from == 0 && part_end == parts && count(best) >= goal) {
        std::printf("decision target=%d answer=yes heuristic\n", goal); return 0;
    }
    for (int k = (goal ? goal : count(best)+1);; ++k) {
        bool feasible = search(k);
        stamp(); std::printf("target=%d answer=%s parts=[%d,%d)/%d calls=%" PRIu64 " selections=%" PRIu64 " pack_cuts=%" PRIu64 " size_cuts=%" PRIu64 " elapsed=%.4fs\n",
                            k,feasible?"yes":"no",part_from,part_end,parts,calls,selections,pack_cuts,size_cuts,std::chrono::duration<double>(Clock::now()-started).count());
        if (goal || !feasible) {
            if (!goal) std::printf("maximum=%d\n",k-1);
            break;
        }
        best = result;
    }
}
