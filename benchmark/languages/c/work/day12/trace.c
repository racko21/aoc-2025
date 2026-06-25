#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SHAPES 20
#define MAX_CELLS_PER_SHAPE 12
#define MAX_ORIENTS 8
#define MAX_W 20
#define MAX_H 20
#define MAX_TYPES 20

typedef struct {
    int dr[MAX_CELLS_PER_SHAPE], dc[MAX_CELLS_PER_SHAPE], n, maxR, maxC;
} Orient;
typedef struct {
    Orient orients[MAX_ORIENTS]; int num_orients; int num_cells;
} ShapeSet;
typedef struct {
    int r[MAX_CELLS_PER_SHAPE], c[MAX_CELLS_PER_SHAPE], n;
} RawShape;

int num_shapes = 0;
ShapeSet shapes[MAX_SHAPES];

static void normalize_raw(RawShape *s) {
    int minr=999,minc=999;
    for(int i=0;i<s->n;i++){if(s->r[i]<minr)minr=s->r[i];if(s->c[i]<minc)minc=s->c[i];}
    for(int i=0;i<s->n;i++){s->r[i]-=minr;s->c[i]-=minc;}
    for(int i=0;i<s->n-1;i++)for(int j=i+1;j<s->n;j++)
        if(s->r[j]<s->r[i]||(s->r[j]==s->r[i]&&s->c[j]<s->c[i])){
            int t=s->r[i];s->r[i]=s->r[j];s->r[j]=t;
            t=s->c[i];s->c[i]=s->c[j];s->c[j]=t;
        }
}
static int same_raw(RawShape *a, RawShape *b) {
    if(a->n!=b->n)return 0;
    for(int i=0;i<a->n;i++)if(a->r[i]!=b->r[i]||a->c[i]!=b->c[i])return 0;
    return 1;
}
static RawShape rotate90(RawShape *s) {
    RawShape t; t.n=s->n;
    for(int i=0;i<s->n;i++){t.r[i]=s->c[i];t.c[i]=-s->r[i];}
    normalize_raw(&t); return t;
}
static RawShape flip_h(RawShape *s) {
    RawShape t; t.n=s->n;
    for(int i=0;i<s->n;i++){t.r[i]=s->r[i];t.c[i]=-s->c[i];}
    normalize_raw(&t); return t;
}
static void build_orient(Orient *o, RawShape *s) {
    o->n=s->n; int mr=0,mc=0;
    for(int i=0;i<s->n;i++){o->dr[i]=s->r[i];o->dc[i]=s->c[i];if(s->r[i]>mr)mr=s->r[i];if(s->c[i]>mc)mc=s->c[i];}
    o->maxR=mr; o->maxC=mc;
}
static void compute_orientations(RawShape *base, ShapeSet *ss) {
    RawShape all[8]; int num=0;
    RawShape cur=*base;
    for(int f=0;f<2;f++){
        for(int r=0;r<4;r++){
            int dup=0;
            for(int i=0;i<num;i++)if(same_raw(&cur,&all[i])){dup=1;break;}
            if(!dup)all[num++]=cur;
            RawShape next=rotate90(&cur); cur=next;
        }
        RawShape fl=flip_h(&cur); cur=fl;
    }
    ss->num_orients=num; ss->num_cells=base->n;
    for(int i=0;i<num;i++) build_orient(&ss->orients[i],&all[i]);
}

static char grid[MAX_H][MAX_W];
static int gW,gH;
static int remain[MAX_TYPES];
static int total_remaining;
static int n_types;
static int min_piece_cells;

static int find_first_empty(int *r, int *c) {
    for(int i=0;i<gH;i++) for(int j=0;j<gW;j++) if(!grid[i][j]){*r=i;*c=j;return 1;}
    return 0;
}
static int place(int si, int oi, int pr, int pc) {
    Orient *o=&shapes[si].orients[oi];
    for(int k=0;k<o->n;k++){int r=pr+o->dr[k],c=pc+o->dc[k];if(r<0||r>=gH||c<0||c>=gW||grid[r][c])return 0;}
    for(int k=0;k<o->n;k++) grid[pr+o->dr[k]][pc+o->dc[k]]=1;
    return 1;
}
static void unplace(int si, int oi, int pr, int pc) {
    Orient *o=&shapes[si].orients[oi];
    for(int k=0;k<o->n;k++) grid[pr+o->dr[k]][pc+o->dc[k]]=0;
}

int solve_depth=0;
int solve() {
    int fr,fc;
    if(!find_first_empty(&fr,&fc)) return (total_remaining==0);
    if(total_remaining==0) return 1;
    
    if(solve_depth < 3) printf("depth=%d first_empty=(%d,%d) total_rem=%d\n",solve_depth,fr,fc,total_remaining);
    
    for(int si=0;si<n_types;si++){
        if(remain[si]==0) continue;
        ShapeSet *ss=&shapes[si];
        for(int oi=0;oi<ss->num_orients;oi++){
            Orient *o=&ss->orients[oi];
            for(int k=0;k<o->n;k++){
                int pr=fr-o->dr[k], pc=fc-o->dc[k];
                if(pr<0||pr+o->maxR>=gH||pc<0||pc+o->maxC>=gW) continue;
                
                int valid=1;
                int fr_idx=fr*gW+fc;
                for(int j=0;j<o->n;j++){
                    if(j==k) continue;
                    int idx=(pr+o->dr[j])*gW+(pc+o->dc[j]);
                    if(idx<fr_idx){valid=0;break;}
                }
                if(!valid) continue;
                
                if(solve_depth<3) printf("  trying si=%d oi=%d k=%d at (%d,%d)\n",si,oi,k,pr,pc);
                if(!place(si,oi,pr,pc)) continue;
                remain[si]--; total_remaining--;
                solve_depth++;
                if(solve()) {
                    unplace(si,oi,pr,pc); remain[si]++; total_remaining++;
                    solve_depth--;
                    return 1;
                }
                solve_depth--;
                unplace(si,oi,pr,pc); remain[si]++; total_remaining++;
            }
        }
    }
    return 0;
}

int main() {
    // Shape 4 from example: ### / #.. / ###
    RawShape r4 = {{0,0,0,1,2,2,2},{0,1,2,0,0,1,2},7};
    normalize_raw(&r4);
    compute_orientations(&r4,&shapes[0]);
    num_shapes=1;
    
    printf("Shape 4 has %d orientations:\n",shapes[0].num_orients);
    for(int i=0;i<shapes[0].num_orients;i++){
        Orient *o=&shapes[0].orients[i];
        printf("  Orient %d (maxR=%d,maxC=%d):",i,o->maxR,o->maxC);
        for(int j=0;j<o->n;j++) printf(" (%d,%d)",o->dr[j],o->dc[j]);
        printf("\n");
    }
    
    gW=4; gH=4;
    memset(grid,0,sizeof(grid));
    n_types=1; remain[0]=2; total_remaining=2; min_piece_cells=7;
    
    int r = solve();
    printf("4x4 with 2x shape4: %s\n",r?"FEASIBLE":"INFEASIBLE");
    return 0;
}
