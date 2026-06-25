#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct { long long x, y, z; } Point;
typedef struct { double dist; int a, b; } Edge;

int par[20], rnk[20], sz[20];
int find2(int x){while(par[x]!=x){par[x]=par[par[x]];x=par[x];}return x;}
int unite(int a,int b){a=find2(a);b=find2(b);if(a==b)return 0;if(rnk[a]<rnk[b]){int t=a;a=b;b=t;}par[b]=a;sz[a]+=sz[b];if(rnk[a]==rnk[b])rnk[a]++;return 1;}
int cmpedge(const void*p1,const void*p2){const Edge*e1=p1;const Edge*e2=p2;if(e1->dist<e2->dist)return -1;if(e1->dist>e2->dist)return 1;return 0;}
int cmpd(const void*a,const void*b){return *(int*)b-*(int*)a;}

int main(){
    FILE*f=fopen("input.txt","r");
    Point pts[100]; int n=0;
    char line[256];
    while(fgets(line,256,f)){
        if(strlen(line)>3)sscanf(line,"%lld,%lld,%lld",&pts[n].x,&pts[n].y,&pts[n].z),n++;
    }
    fclose(f);
    printf("n=%d\n",n);
    int ne=n*(n-1)/2;
    Edge*edges=malloc(ne*sizeof(Edge));
    int ei=0;
    for(int i=0;i<n;i++)for(int j=i+1;j<n;j++){
        double dx=(double)(pts[i].x-pts[j].x),dy=(double)(pts[i].y-pts[j].y),dz=(double)(pts[i].z-pts[j].z);
        edges[ei].dist=sqrt(dx*dx+dy*dy+dz*dz);
        edges[ei].a=i;edges[ei].b=j;ei++;
    }
    qsort(edges,ne,sizeof(Edge),cmpedge);
    for(int i=0;i<n;i++){par[i]=i;rnk[i]=0;sz[i]=1;}
    int comps=n;
    for(int i=0;i<10;i++){
        int merged=unite(edges[i].a,edges[i].b);
        if(merged)comps--;
        printf("Edge %d: (%lld,%lld,%lld) -- (%lld,%lld,%lld) dist=%.2f merged=%d\n",
            i+1,pts[edges[i].a].x,pts[edges[i].a].y,pts[edges[i].a].z,
            pts[edges[i].b].x,pts[edges[i].b].y,pts[edges[i].b].z,
            edges[i].dist,merged);
    }
    printf("Components: %d\n",comps);
    int sizes[100],sc=0;
    for(int j=0;j<n;j++)if(find2(j)==j)sizes[sc++]=sz[j];
    qsort(sizes,sc,sizeof(int),cmpd);
    printf("Top sizes: %d %d %d\n",sizes[0],sizes[1],sizes[2]);
    printf("Product: %lld\n",(long long)sizes[0]*sizes[1]*sizes[2]);
    free(edges);
}
