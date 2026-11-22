// Exhaustive decision: is there X subset of {1..K}, |X|=T, K in X, all x^2+y^2 (x<=y in X) distinct?
// Top-down DFS (K, K-1, ..., 1).
// State: S chosen (all > j), C candidates in [1..j], adj[v] = set of w such that {v,w} plus chosen elements
//   contain a full collision support (so v,w cannot both be added).
// Pruning (sound): for every i<=j, |X ∩ [1..i]| <= ub[i]  (ub from b-file for i<=68, or proved by this program),
//   and |X ∩ (i..j]| <= |C ∩ (i..j]| - M, M = size of a matching of conflict pairs inside C ∩ (i..j].
// usage: rds2 K T boundsfile [part P splitlevel]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
typedef unsigned __int128 u128;
#define MAXN 100
static int K,T,ub[MAXN+2];
typedef struct { u128 need; int a,b; } Trig;   // when v included and need ⊂ S : conflict pair (a,b)
static Trig *tl[MAXN+1]; static int nt[MAXN+1];
static uint64_t nodes=0, splitcnt=0; static int part=0,P=1,splitlevel=0;
static int sol[MAXN],found=0; static int USEM=1;
static uint64_t lvl[MAXN+1];
static inline int pc(u128 x){return __builtin_popcountll((uint64_t)x)+__builtin_popcountll((uint64_t)(x>>64));}
#define BIT(i) ((u128)1<<(i))
typedef struct { u128 adj[MAXN+1]; } Adj;
static inline int feasible(u128 C,int j,int need,const Adj*A){
  if(need<=0) return 1;
  int cnt=0; u128 matched=0, seen=0;
  for(int i=j;i>=1;i--){
    if(ub[i]+cnt<need) return 0;
    if((C>>i)&1){
      if(USEM){
        u128 m=A->adj[i]&seen&~matched;   // unmatched higher candidates conflicting with i
        if(m){ // match i with the highest such
          int w=(uint64_t)(m>>64)?127-__builtin_clzll((uint64_t)(m>>64)):63-__builtin_clzll((uint64_t)m);
          matched|=BIT(w)|BIT(i); seen|=BIT(i); continue; }
        seen|=BIT(i);
      }
      cnt++; if(cnt>=need) return 1;
    }
  }
  return 0;
}
static void dfs(int j,u128 S,u128 C,int size,const Adj*A){
  nodes++;
  if(size==T){found=1; int m=0; for(int i=1;i<=K;i++) if((S>>i)&1) sol[m++]=i; return;}
  while(j>=1 && !((C>>j)&1)) j--;
  if(j<1) return;
  lvl[j]++;
  if(P>1 && j<=splitlevel && j+0>=0){ }
  int need=T-size;
  { // include j
    u128 S2=S|BIT(j); u128 C2=C&~BIT(j)&~A->adj[j];
    Adj B; memcpy(&B,A,sizeof(u128)*(j));  // only entries < j matter
    Trig*t=tl[j]; int n=nt[j];
    for(int q=0;q<n;q++) if((t[q].need & S2)==t[q].need){ B.adj[t[q].a]|=BIT(t[q].b); B.adj[t[q].b]|=BIT(t[q].a); }
    if(feasible(C2,j-1,need-1,&B)){ dfs(j-1,S2,C2,size+1,&B); if(found) return; }
  }
  { u128 C2=C&~BIT(j);
    if(feasible(C2,j-1,need,A)) dfs(j-1,S,C2,size,A); }
}
// top part enumerator for parallel split: handled by wrapper dfs_top
static void dfs_top(int j,u128 S,u128 C,int size,const Adj*A){
  if(j<=splitlevel){ uint64_t id=splitcnt++; if((int)(id%P)!=part) return; dfs(j,S,C,size,A); return; }
  nodes++;
  if(size==T){found=1; int m=0; for(int i=1;i<=K;i++) if((S>>i)&1) sol[m++]=i; return;}
  if(!((C>>j)&1)){ dfs_top(j-1,S,C,size,A); return; }
  int need=T-size;
  { u128 S2=S|BIT(j); u128 C2=C&~BIT(j)&~A->adj[j];
    Adj B; memcpy(&B,A,sizeof(u128)*(j));
    Trig*t=tl[j]; int n=nt[j];
    for(int q=0;q<n;q++) if((t[q].need & S2)==t[q].need){ B.adj[t[q].a]|=BIT(t[q].b); B.adj[t[q].b]|=BIT(t[q].a); }
    if(feasible(C2,j-1,need-1,&B)){ dfs_top(j-1,S2,C2,size+1,&B); if(found) return; }
  }
  { u128 C2=C&~BIT(j);
    if(feasible(C2,j-1,need,A)) dfs_top(j-1,S,C2,size,A); }
}
int main(int argc,char**argv){
  if(argc<4){fprintf(stderr,"usage: rds2 K T boundsfile [part P splitlevel]\n");return 2;}
  K=atoi(argv[1]);T=atoi(argv[2]);
  if(getenv("NOMATCH")) USEM=0;
  FILE*f=fopen(argv[3],"r"); if(!f){perror("bounds");return 2;}
  char line[256]; for(int i=0;i<=MAXN;i++)ub[i]=-1; ub[0]=0;
  while(fgets(line,sizeof line,f)){ if(line[0]=='#')continue; int a,b; if(sscanf(line,"%d %d",&a,&b)==2&&a>=1&&a<=MAXN)ub[a]=b; }
  fclose(f);
  for(int i=1;i<K;i++) if(ub[i]<0){fprintf(stderr,"missing bound for %d\n",i);return 2;}
  if(argc>=7){part=atoi(argv[4]);P=atoi(argv[5]);splitlevel=atoi(argv[6]);}
  int nedges=0,ntri=0,nquad=0;
  static Trig all[1<<16]; static int trigv[1<<16];
  for(int a=1;a<=K;a++)for(int b=a;b<=K;b++)for(int c=a+1;c<=K;c++)for(int d=c;d<=K;d++){
    if(a*a+b*b!=c*c+d*d) continue;   // then a<c<=d<b
    if(c==d){ // support {a,c,b}: triple; trigger when b included -> conflict (a,c)
      all[nedges].need=0; all[nedges].a=a; all[nedges].b=c; trigv[nedges]=b; ntri++; }
    else { // support a<c<d<b : trigger when d included and b in S -> conflict (a,c)
      all[nedges].need=BIT(b); all[nedges].a=a; all[nedges].b=c; trigv[nedges]=d; nquad++; }
    nedges++;
  }
  for(int v=1;v<=K;v++){ int n=0; for(int e=0;e<nedges;e++) if(trigv[e]==v)n++; tl[v]=malloc(sizeof(Trig)*(n+1)); nt[v]=0;
    for(int e=0;e<nedges;e++) if(trigv[e]==v) tl[v][nt[v]++]=all[e]; }
  printf("rds2 K=%d T=%d supports=%d (triples=%d quads=%d) part=%d/%d splitlevel=%d bounds=%s matching=%d\n",K,T,nedges,ntri,nquad,part,P,splitlevel,argv[3],USEM);
  fflush(stdout);
  u128 S=BIT(K); u128 C=0; for(int i=1;i<K;i++)C|=BIT(i);
  static Adj A0; memset(&A0,0,sizeof A0);
  { Trig*t=tl[K]; for(int q=0;q<nt[K];q++) if((t[q].need & S)==t[q].need){ A0.adj[t[q].a]|=BIT(t[q].b); A0.adj[t[q].b]|=BIT(t[q].a);} }
  if(feasible(C,K-1,T-1,&A0)){ if(P>1) dfs_top(K-1,S,C,1,&A0); else dfs(K-1,S,C,1,&A0); }
  if(found){ printf("FOUND size %d:",T); for(int i=0;i<T;i++)printf(" %d",sol[i]); printf("\n"); }
  else printf("NONE: no %d-subset of {1..%d} containing %d (this part exhausted)\n",T,K,K);
  printf("nodes=%llu splitstates=%llu completed=1\n",(unsigned long long)nodes,(unsigned long long)splitcnt);
  if(getenv("LVL")){ for(int i=K;i>=1;i--) printf("%d:%llu ",i,(unsigned long long)lvl[i]); printf("\n"); }
  return 0;
}
