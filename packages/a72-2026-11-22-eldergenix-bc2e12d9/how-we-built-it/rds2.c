/* Referee's own exact search for A390813 (version 2: adds a clique-cover bound).
   Problem: max |X|, X subset of {1..n}, all sums x^2+y^2 (x<=y; x,y in X) distinct.
   Forbidden configurations (complete list, from the definition):
     4-sets {a<b<c<d} with a^2+d^2 = b^2+c^2 (all four distinct; largest must pair with smallest),
     3-sets {a<b<c}  with a^2+c^2 = 2 b^2   (diagonal collision).
   (x^2+y^2=z^2+w^2 with a shared element forces equal multisets, so nothing else exists; a sum with
    3+ representations simply yields several forbidden sets, one per pair of representations.)
   Russian-doll search over prefixes: for k=start..N decide whether a valid set of size a(k-1)+1 exists
   in {1..k}; such a set must contain k (else it lies in {1..k-1}); a(k) in {a(k-1), a(k-1)+1}.
   State: X (chosen, descending), C = candidates below min-chosen-so-far-branch that can individually be
   added (no forbidden set inside X+c), G = graph on C: p~q iff X+{p,q} contains a forbidden set.
   Any valid extension I of X inside C is an independent set of G. Bound used at every node:
     |I| <= min over i of ( L(i) + U(i) ),
       L(i) >= max |I ∩ [1..i]| : L(i)=min(a(i), ascending greedy clique-cover count of C∩[1..i], L(i-1)+[i in C])
       U(i) >= max |I ∩ (i..top]| : number of cliques opened by a descending greedy clique cover on C∩(i..top]
     a(i) for i<k are values proved earlier in this same run (or b-file values, i<=68 only, with -b).
   Branching: z = largest candidate; branch "z in X" then "z not in X" (complete binary split).
   No node cap, no time limit inside the program. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#define MAXN 80
typedef unsigned __int128 u128;
typedef unsigned long long u64;
#define BIT(i) (((u128)1)<<(i))
static int a[MAXN+1], sq[MAXN+1];
static int pstart[MAXN+1][MAXN+1], pend[MAXN+1][MAXN+1]; /* for u<v */
static unsigned char *pl_p, *pl_q; /* q==0 : 3-set, forbid p */
static int t3start[MAXN+2]; static unsigned char t3p[20000], t3q[20000];
static long n4sets, n3sets;
static atomic_int found; static int witness[MAXN], wlen; static pthread_mutex_t mu=PTHREAD_MUTEX_INITIALIZER;
static int useClique=1;
typedef struct { u128 adj[MAXN+1][MAXN+1]; unsigned char inX[MAXN+1]; int X[MAXN]; u64 nodes, bounds; int target; } St;
static inline int hi_bit(u128 c){ u64 h=(u64)(c>>64); if(h) return 127-__builtin_clzll(h); return 63-__builtin_clzll((u64)c); }
static inline int lo_bit(u128 c){ u64 l=(u64)c; if(l) return __builtin_ctzll(l); return 64+__builtin_ctzll((u64)(c>>64)); }
static inline int pc(u128 c){ return __builtin_popcountll((u64)c)+__builtin_popcountll((u64)(c>>64)); }
/* add z as element number n (0-based) of X; adjacency level n -> n+1; returns new candidate set */
static inline u128 add(St*S,int n,int z,u128 C){
  u128 forb=0; u128 *A=S->adj[n+1]; memcpy(A,S->adj[n],sizeof(u128)*(z));  /* rows 0..z-1 */
  for(int i=0;i<n;i++){ int x=S->X[i]; int u=x<z?x:z, v=x<z?z:x;
    for(int r=pstart[u][v]; r<pend[u][v]; r++){ int p=pl_p[r], q=pl_q[r];
      if(q==0){ forb|=BIT(p); continue; }
      if(S->inX[p]) forb|=BIT(q); else if(S->inX[q]) forb|=BIT(p);
      else if(p<z && q<z){ A[p]|=BIT(q); A[q]|=BIT(p); } } }
  for(int r=t3start[z]; r<t3start[z+1]; r++){ int p=t3p[r], q=t3q[r]; if(p<z&&q<z){ A[p]|=BIT(q); A[q]|=BIT(p);} }
  S->inX[z]=1; S->X[n]=z;
  return C & (BIT(z)-1) & ~forb;
}
/* upper bound on number of further elements choosable from C */
static inline int bound(St*S,int n,u128 C,int need){
  int m=pc(C); if(m<need) return m;
  int v[MAXN]; unsigned char L[MAXN]; u128 cl[MAXN]; int nc=0; const u128*A=S->adj[n];
  u128 t=C; int j=0, Lp=0;
  /* cheap pass first: prefix bounds only */
  { u128 tt=C; int f=0; while(tt){ int x=lo_bit(tt); tt&=tt-1; f++; if(f>a[x]) f=a[x]; } if(f<need) return f; if(!useClique) return f; }
  while(t){ int x=lo_bit(t); t&=t-1; u128 nb=A[x]; int k;
    for(k=0;k<nc;k++) if(!(cl[k]&~nb)){ cl[k]|=BIT(x); break; }
    if(k==nc){ cl[nc++]=BIT(x); }
    Lp=Lp+1; if(Lp>nc) Lp=nc; if(Lp>a[x]) Lp=a[x]; v[j]=x; L[j]=Lp; j++; }
  if(Lp<need) return Lp;
  int B=Lp; nc=0;
  for(j=m-1;j>=0;j--){ int x=v[j]; if(L[j]+nc<B){ B=L[j]+nc; if(B<need) return B; }
    u128 nb=A[x]; int k; for(k=0;k<nc;k++) if(!(cl[k]&~nb)){ cl[k]|=BIT(x); break; }
    if(k==nc) cl[nc++]=BIT(x); }
  if(nc<B) B=nc;
  return B;
}
#define MAXT 4000000
static unsigned char (*tasks)[8]; static int ntask; static int D; static atomic_int next_task; static atomic_int done_tasks;
static void rec(St*S,int n,u128 C,int gen){
  if(gen && n==D){ if(ntask>=MAXT){fprintf(stderr,"task overflow\n");exit(2);} for(int i=0;i<D;i++) tasks[ntask][i]=S->X[i]; ntask++; return; }
  S->nodes++;
  int need=S->target-n;
  for(;;){
    if(atomic_load_explicit(&found,memory_order_relaxed)) return;
    S->bounds++;
    if(bound(S,n,C,need)<need) return;
    int z=hi_bit(C);
    u128 C2=add(S,n,z,C);
    if(n+1==S->target){ pthread_mutex_lock(&mu); if(!atomic_load(&found)){ wlen=n+1; memcpy(witness,S->X,sizeof(int)*wlen); atomic_store(&found,1);} pthread_mutex_unlock(&mu); S->inX[z]=0; return; }
    rec(S,n+1,C2,gen);
    S->inX[z]=0;
    C&=~BIT(z);
  }
}
static int gK,gT; static u64 thr_nodes[16], thr_bounds[16];
static void* worker(void*arg){ long id=(long)arg; St*S=calloc(1,sizeof(St)); S->target=gT;
  for(;;){ int ti=atomic_fetch_add(&next_task,1); if(ti>=ntask) break; if(atomic_load(&found)) break;
    u128 C=BIT(gK+1)-2;
    for(int i=0;i<D;i++) C=add(S,i,tasks[ti][i],C);
    rec(S,D,C,0);
    for(int i=0;i<D;i++) S->inX[tasks[ti][i]]=0;
    atomic_fetch_add(&done_tasks,1); }
  thr_nodes[id]=S->nodes; thr_bounds[id]=S->bounds; free(S); return 0; }
static int selfcheck(int*X,int n,int K){ static unsigned char u[2*MAXN*MAXN+1]; memset(u,0,sizeof u);
  for(int i=0;i<n;i++){ if(X[i]<1||X[i]>K) return 0; for(int j=i;j<n;j++){ if(j>i&&X[i]==X[j]) return 0; int s=sq[X[i]]+sq[X[j]]; if(u[s]) return 0; u[s]=1; } } return 1; }
static double now(void){ struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts); return ts.tv_sec+ts.tv_nsec*1e-9; }
int main(int argc,char**argv){
  int N=72, nthr=3, start=1; const char*bf=0; int bmax=68; D=6;
  for(int i=1;i<argc;i++){ if(!strcmp(argv[i],"-n")) N=atoi(argv[++i]); else if(!strcmp(argv[i],"-t")) nthr=atoi(argv[++i]);
    else if(!strcmp(argv[i],"-b")) bf=argv[++i]; else if(!strcmp(argv[i],"-s")) start=atoi(argv[++i]);
    else if(!strcmp(argv[i],"-d")) D=atoi(argv[++i]); else if(!strcmp(argv[i],"-noclique")) useClique=0; else {fprintf(stderr,"bad arg\n");return 2;} }
  if(N>MAXN||nthr>3||nthr<1||D>8||D<2) return 2;
  for(int i=0;i<=MAXN;i++) sq[i]=i*i;
  /* enumerate forbidden sets directly from the definition */
  { int cap=4000000; pl_p=malloc(cap); pl_q=malloc(cap); int (*cnt)[MAXN+1]=calloc(MAXN+1,sizeof *cnt);
    for(int pass=0;pass<2;pass++){
      n4sets=n3sets=0;
      if(pass==1){ int r=0; for(int u=1;u<=N;u++)for(int v=u+1;v<=N;v++){ pstart[u][v]=r; pend[u][v]=r; r+=cnt[u][v]; } if(r>cap){fprintf(stderr,"cap\n");return 2;} }
      #define ADDP(U,V,P,Q) do{ int u_=(U)<(V)?(U):(V), v_=(U)<(V)?(V):(U); if(pass==0) cnt[u_][v_]++; else { pl_p[pend[u_][v_]]=(P); pl_q[pend[u_][v_]]=(Q); pend[u_][v_]++; } }while(0)
      for(int A=1;A<=N;A++)for(int B=A+1;B<=N;B++)for(int Cc=B+1;Cc<=N;Cc++)for(int Dd=Cc+1;Dd<=N;Dd++)
        if(sq[A]+sq[Dd]==sq[B]+sq[Cc]){ n4sets++; int e[4]={A,B,Cc,Dd};
          for(int i=0;i<4;i++)for(int j=i+1;j<4;j++){ int o[2],k=0; for(int l=0;l<4;l++) if(l!=i&&l!=j) o[k++]=e[l]; ADDP(e[i],e[j],o[0],o[1]); } }
      for(int A=1;A<=N;A++)for(int B=A+1;B<=N;B++)for(int Cc=B+1;Cc<=N;Cc++)
        if(sq[A]+sq[Cc]==2*sq[B]){ n3sets++; ADDP(A,B,Cc,0); ADDP(A,Cc,B,0); ADDP(B,Cc,A,0); }
    }
    int r=0; for(int z=1;z<=N+1;z++){ t3start[z]=r; if(z>N) break;
      for(int A=1;A<=N;A++)for(int B=A+1;B<=N;B++)for(int Cc=B+1;Cc<=N;Cc++) if(sq[A]+sq[Cc]==2*sq[B] && (A==z||B==z||Cc==z)){
        int o[2],k=0; if(A!=z)o[k++]=A; if(B!=z)o[k++]=B; if(Cc!=z)o[k++]=Cc; t3p[r]=o[0]; t3q[r]=o[1]; r++; } }
    free(cnt); }
  tasks=malloc((size_t)MAXT*8);
  a[0]=0;
  if(bf){ FILE*f=fopen(bf,"r"); if(!f){perror("bfile");return 2;} char line[256]; int k,v; int got=0;
    while(fgets(line,sizeof line,f)){ if(line[0]=='#') continue; if(sscanf(line,"%d %d",&k,&v)==2 && k>=1 && k<=bmax && k<start){ a[k]=v; got++; } } fclose(f);
    if(got!=start-1 || start-1>bmax){ fprintf(stderr,"b-file must supply exactly a(1..%d), start-1<=%d\n",start-1,bmax); return 2; }
    printf("# bounds a(1..%d) loaded from b-file; a(k) for k>=%d proved by this run\n",start-1,start);
  } else { if(start!=1){fprintf(stderr,"-s needs -b\n");return 2;} printf("# self-contained: all a(k) proved by this run, no external bounds\n"); }
  printf("# N=%d threads=%d splitdepth=%d clique=%d forbidden4sets=%ld forbidden3sets=%ld ; no node cap, no internal time limit\n",N,nthr,D,useClique,n4sets,n3sets); fflush(stdout);
  double T0=now();
  for(int K=start;K<=N;K++){
    double t0=now(); int target=a[K-1]+1; atomic_store(&found,0); wlen=0; ntask=0; atomic_store(&next_task,0); atomic_store(&done_tasks,0);
    gK=K; gT=target; u64 total=0, totalb=0;
    St*S=calloc(1,sizeof(St)); S->target=target;
    u128 C=BIT(K+1)-2; /* bits 1..K */
    u128 C2=add(S,0,K,C);
    if(target==1){ wlen=1; witness[0]=K; atomic_store(&found,1); total=1; }
    else if(target<=D+1 || K<30){ rec(S,1,C2,0); total=S->nodes; totalb=S->bounds; }
    else { rec(S,1,C2,1); total=S->nodes; totalb=S->bounds;
      pthread_t th[3]; memset(thr_nodes,0,sizeof thr_nodes); memset(thr_bounds,0,sizeof thr_bounds);
      for(long i=0;i<nthr;i++) pthread_create(&th[i],0,worker,(void*)i);
      for(int i=0;i<nthr;i++) pthread_join(th[i],0);
      for(int i=0;i<nthr;i++){ total+=thr_nodes[i]; totalb+=thr_bounds[i]; } }
    free(S);
    int fd=atomic_load(&found);
    if(fd){ if(!selfcheck(witness,wlen,K)||wlen!=target){ printf("INTERNAL ERROR: bad witness at K=%d\n",K); return 3; } a[K]=target; }
    else { if(atomic_load(&done_tasks)!=ntask){ printf("INTERNAL ERROR: tasks incomplete\n"); return 3; } a[K]=a[K-1]; }
    printf("k=%d a=%d  [%s size %d containing %d] nodes=%llu boundcalls=%llu tasks=%d/%d time=%.2fs",K,a[K],fd?"FOUND":"EXHAUSTED-NONE",target,K,total,totalb,atomic_load(&done_tasks),ntask,now()-t0);
    if(fd){ printf(" witness:"); for(int i=wlen-1;i>=0;i--) printf(" %d",witness[i]); }
    printf("\n"); fflush(stdout);
  }
  printf("# DONE normal completion, total time %.2fs\n",now()-T0);
  return 0;
}
