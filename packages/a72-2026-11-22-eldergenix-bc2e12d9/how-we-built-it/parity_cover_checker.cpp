// Verify the monotone hitting clauses in a compact Sidon certificate.
// No source, precomputed catalogue, or OEIS prefix is shared with the producer.
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>
#include <zlib.h>

using Word=uint64_t;
using Columns=std::array<Word,3>;
using Pattern=std::array<unsigned,3>;
constexpr int Axes=20, Size=1<<Axes, Low=8, High=Axes-Low;
struct Entry {Columns bits{};Pattern projections{};};
struct Part {
    int width=0,first_width=0;
    Word packedbit[36]{};
    std::vector<std::pair<Word,Word>> forbidden[36];
    std::vector<Entry> local[2];
    std::vector<int> group[2];
};
int n,features,axis[3][Axes],number[5200],which[2][36][36];
Part parts[2];
gzFile cert;
uint64_t emitted;
bool enumerate_all;
static void require(bool yes,const char *why){if(!yes){std::fprintf(stderr,"REJECT %s\n",why);std::exit(1);}}
static int count(Word a){return std::popcount(a);}
static std::string line(){
    char buf[512];require(gzgets(cert,buf,sizeof(buf))!=nullptr,"unexpected end");
    std::string s=buf;require(!s.empty()&&s.back()=='\n',"invalid line");s.pop_back();return s;
}
static void endline(std::istringstream &r){std::string extra;require(!(r>>extra),"extra field");}
static void prepare(){
    std::vector<std::vector<Word>> equal[2];
    for(int side=0;side<2;side++){
        auto &a=parts[side];equal[side].resize(5200);
        for(int x=2-side;x<=n;x+=2){
            int v=a.width++,g=x*x%16==(side?1:0)?0:1;
            a.group[g].push_back(v);
            for(int y=2-side;y<x;y+=2)
                equal[side][x*x-y*y].push_back((Word(1)<<v)|(Word(1)<<((y-1)/2)));
        }
        a.first_width=a.group[0].size();
        for(int g=0;g<2;g++)for(int j=0;j<(int)a.group[g].size();j++)
            a.packedbit[a.group[g][j]]=Word(1)<<(j+(g?a.first_width:0));
    }
    std::fill(std::begin(number),std::end(number),-1);
    for(int d=1;d<n*n;d++){
        int f=-1;if(!equal[0][d].empty()&&!equal[1][d].empty())number[d]=f=features++;
        for(int side=0;side<2;side++){
            auto &rr=equal[side][d];
            for(int i=0;i<(int)rr.size();i++){
                Word pair=rr[i];int v=std::countr_zero(pair),w=63-std::countl_zero(pair);
                which[side][v][w]=which[side][w][v]=f+1;
                if(f>=0){
                    int x=2*v+2-side,y=2*w+2-side;
                    require(x*x%16==y*y%16,"cross-class shared difference");
                }
                for(int j=0;j<i;j++){
                    Word support=rr[i]|rr[j],low=support&-support,other=support^low;
                    int pivot=std::countr_zero(other);
                    parts[side].forbidden[pivot].push_back({other&(other-1),low});
                }
            }
        }
    }
    require(features<=192,"signature width");
}
static Pattern projection(const Columns &b){
    Pattern p{};
    for(int t=0;t<3;t++)for(int j=0;j<Axes;j++){
        int f=axis[t][j];if(b[f/64]&(Word(1)<<(f%64)))p[t]|=1U<<j;
    }
    return p;
}
static void local_tables(int side){
    auto &part=parts[side];
    Pattern label[192]{};
    for(int t=0;t<3;t++)for(int j=0;j<Axes;j++)label[axis[t][j]][t]|=1U<<j;
    for(int g=0;g<2;g++){
        auto &table=part.local[g];auto &vs=part.group[g];
        table.assign(1U<<vs.size(),{});
        for(unsigned m=1;m<table.size();m++){
            unsigned rest=m&(m-1);int v=vs[std::countr_zero(m)];
            auto &e=table[m];e=table[rest];
            for(unsigned w=rest;w;w&=w-1){
                int f=which[side][v][vs[std::countr_zero(w)]]-1;
                if(f<0)continue;
                e.bits[f/64]|=Word(1)<<(f%64);
                for(int t=0;t<3;t++)e.projections[t]|=label[f][t];
            }
        }
    }
}
struct Never {bool operator()(Word) const {return false;}};
template<class Fn,class Skip>
static void enumerate(const Part &a,int need,Word selected,Word remaining,Word packed,Fn &use,Skip &skip){
    if(!need){use(packed);++emitted;return;}
    if((need==3||need==6||need==9)&&skip(packed))return;
    while(count(remaining)>=need){
        int v=63-std::countl_zero(remaining);Word b=Word(1)<<v;
        remaining^=b;Word next=remaining;
        for(auto [higher,low]:a.forbidden[v])if(!(higher&~selected))next&=~low;
        enumerate(a,need-1,selected|b,next,packed|a.packedbit[v],use,skip);
    }
}
template<class Fn,class Skip=Never>
static uint64_t scan(int p,int k,Fn fn,Skip skip={}){
    emitted=0;enumerate(parts[p],k,0,(Word(1)<<parts[p].width)-1,0,fn,skip);return emitted;
}
static Entry signature(const Part &a,Word packed){
    const auto &x=a.local[0][packed&((Word(1)<<a.first_width)-1)];
    const auto &y=a.local[1][packed>>a.first_width];Entry z;
    for(int t=0;t<3;t++){z.bits[t]=x.bits[t]|y.bits[t];z.projections[t]=x.projections[t]|y.projections[t];}
    return z;
}
struct SubsetIndex {
    std::vector<unsigned> frequency;
    std::vector<int> head,next;
    std::vector<Word> high_by_low,low_by_high;
    void build(const std::vector<Columns>&terms,int t){
        frequency.assign(Size,0);head.assign(Size,-1);next.resize(terms.size());
        high_by_low.assign((1<<Low)*(1<<(High-6)),0);
        low_by_high.assign((1<<High)*(1<<(Low-6)),0);
        for(int i=0;i<(int)terms.size();i++){
            unsigned m=projection(terms[i])[t];int h=m>>Low,l=m&255;
            next[i]=head[m];head[m]=i;frequency[m]++;
            low_by_high[h*4+l/64]|=Word(1)<<(l%64);
        }
        // At this stage only the low dimensions have been summed. High bits are exact.
        for(int b=1;b<Size;b*=2){
            for(int start=0;start<Size;start+=2*b)for(int j=0;j<b;j++)
                frequency[start+b+j]+=frequency[start+j];
            if(b==(1<<(Low-1)))for(int m=0;m<Size;m++)if(frequency[m]){
                int h=m>>Low,l=m&255;high_by_low[l*64+h/64]|=Word(1)<<(h%64);
            }
        }
    }
};
Word high_sub[1<<High][1<<(High-6)],low_sub[1<<Low][1<<(Low-6)];
static void submasks(){
    for(unsigned a=0;a<(1<<High);a++)for(unsigned b=a;;b=(b-1)&a){
        high_sub[a][b/64]|=Word(1)<<(b%64);if(b==0)break;
    }
    for(unsigned a=0;a<(1<<Low);a++)for(unsigned b=a;;b=(b-1)&a){
        low_sub[a][b/64]|=Word(1)<<(b%64);if(b==0)break;
    }
}
static bool meets(Columns a,Columns b){return ((a[0]&b[0])|(a[1]&b[1])|(a[2]&b[2]))!=0;}
static bool transversal(const Entry &q,const std::vector<Columns> &terms,const SubsetIndex index[3]){
    if(terms.size()<=16){
        for(auto s:terms)if(!meets(q.bits,s))return false;
        return true;
    }
    unsigned least=~0U,chosen=0,m=0;
    for(int t=0;t<3;t++){
        unsigned a=(Size-1)^q.projections[t],z=index[t].frequency[a];
        if(!z)return true;
        if(z<least){least=z;m=a;chosen=t;}
    }
    const auto &s=index[chosen];int hi=m>>Low,lo=m&255;
    for(int w=0;w<64;w++)for(Word hs=high_sub[hi][w]&s.high_by_low[lo*64+w];hs;hs&=hs-1){
        int h=w*64+std::countr_zero(hs);
        for(int j=0;j<4;j++)for(Word ls=low_sub[lo][j]&s.low_by_high[h*4+j];ls;ls&=ls-1){
            int pattern=(h<<Low)+64*j+std::countr_zero(ls);
            for(int i=s.head[pattern];i>=0;i=s.next[i])if(!meets(q.bits,terms[i]))return false;
        }
    }
    return true;
}
static int check_case(int target,uint64_t &total_terms){
    std::istringstream desc(line());std::string name;int p,s,t,rows,nt;
    require(bool(desc>>name>>p>>s>>t>>rows>>nt)&&name=="CASE"&&(p==0||p==1)&&s>=0&&t>=0&&s+t==target&&s<=parts[p].width&&t<=parts[p^1].width&&nt>0&&nt<=rows&&rows<=10000000,"case header");
    endline(desc);
    for(auto &proj:axis){
        std::istringstream a(line());require(bool(a>>name)&&name=="AXES","axes line");Columns used{};
        for(int &f:proj){int d;require(bool(a>>d)&&d>0&&d<n*n&&number[d]>=0,"unknown difference");f=number[d];Word bit=Word(1)<<(f%64);require(!(used[f/64]&bit),"repeated axis");used[f/64]|=bit;}
        endline(a);
    }
    std::vector<Columns> terms(nt);std::vector<int> spans(nt);int64_t cover=0;
    for(int j=0;j<nt;j++){
        std::istringstream a(line());auto &h=terms[j];int len;
        require(bool(a>>len>>std::hex>>h[0]>>h[1]>>h[2])&&len>0&&len<=rows,"clause");
        endline(a);spans[j]=len;cover+=len;
    }
    require(cover==rows,"run lengths");
    local_tables(p);int at=0,left=spans[0];
    uint64_t seen=scan(p,s,[&](Word m){
        require(at<nt,"missing clause");auto b=signature(parts[p],m).bits,h=terms[at];
        require(!((h[0]&~b[0])|(h[1]&~b[1])|(h[2]&~b[2])),"clause not contained in indexed row");
        if(--left==0&&++at<nt)left=spans[at];
    });
    require(seen==(unsigned)rows&&at==nt,"incomplete indexing family");
    local_tables(p^1);SubsetIndex idx[3];
    if(nt>16)for(int i=0;i<3;i++)idx[i].build(terms,i);
    uint64_t closed=0;
    auto implied=[&](Word m){
        if(enumerate_all)return false;
        const auto q=signature(parts[p^1],m);
        if(nt<=16){
            for(auto h:terms)if(!meets(q.bits,h))return false;
        }else{
            // Zero for this projection proves that every clause already has a hit.
            bool zero=false;
            for(int j=0;j<3;j++)if(!idx[j].frequency[(Size-1)^q.projections[j]]){zero=true;break;}
            if(!zero)return false;
        }
        ++closed;return true;
    };
    uint64_t opposite=scan(p^1,t,[&](Word m){
        require(transversal(signature(parts[p^1],m),terms,idx),"opposite row misses a clause");
    },implied);
    std::printf("case parity=%d sizes=%d+%d indexed_rows=%llu clauses=%d tested_rows=%llu closed_prefixes=%llu PASS\n",p,s,t,(unsigned long long)seen,nt,(unsigned long long)opposite,(unsigned long long)closed);
    std::fflush(stdout);total_terms+=nt;return p?t:s;
}
int main(int argc,char **argv){
    if(argc<4||argc>5||(argc==5&&std::strcmp(argv[4],"--enumerate"))){std::fprintf(stderr,"usage: check_cover N K certificate.gz [--enumerate]\n");return 2;}
    enumerate_all=argc==5;
    cert=gzopen(argv[3],"rb");require(cert,"open");gzbuffer(cert,1<<17);
    std::istringstream in(line());std::string name;int version,target,max_even,max_odd,cases;
    require(bool(in>>name>>version>>n>>target>>max_even>>max_odd>>cases)&&name=="SIDON-COVER"&&version==1&&n>=1&&n<=72&&target>0&&target<=n,"header");endline(in);
    require(n==std::atoi(argv[1])&&target==std::atoi(argv[2]),"wrong target");
    prepare();submasks();
    require(max_even>=0&&max_even<=parts[0].width&&max_odd>=0&&max_odd<=parts[1].width,"parity bounds");
    for(int p=0;p<2;p++)scan(p,(p?max_odd:max_even)+1,[](Word){require(false,"incorrect parity upper bound");});
    int lo=std::max(0,target-max_odd),hi=std::min(target,max_even);
    require(cases==hi-lo+1,"number of strata");std::vector<int> done(n+1);uint64_t total=0;
    for(int i=0;i<cases;i++){int e=check_case(target,total);require(e>=lo&&e<=hi&&!done[e]++,"duplicate or out-of-range stratum");}
    require(line()=="END"&&gzgetc(cert)==-1&&gzclose(cert)==Z_OK,"footer");
    std::printf("PASS N=%d excluded=%d even_upper=%d odd_upper=%d strata=%d hitting_clauses=%llu\n",n,target,max_even,max_odd,cases,(unsigned long long)total);
}
