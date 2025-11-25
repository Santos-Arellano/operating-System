#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_trace(int* refs,int n,int* hits,int* evict,int** frames,int frames_n){
    int faults=0,h=0; for(int i=0;i<n;i++){ if(hits[i]) h++; else faults++; }
    printf("{\"trace\":[");
    for(int i=0;i<n;i++){
        printf("{\"ref\":%d,\"hit\":%d,\"evict\":%d,\"frames\":[",refs[i],hits[i],evict[i]);
        for(int j=0;j<frames_n;j++){
            printf("%d%s",frames[i][j],(j<frames_n-1?",":""));
        }
        printf("]}%s",(i<n-1?",":""));
    }
    printf("],\"faults\":%d,\"hits\":%d,\"fault_rate\":%.6f}\n",faults,h,(n?((double)faults/n):0.0));
}

static void fifo(int* refs,int n,int frames_n){
    int* frames=(int*)malloc(frames_n*sizeof(int)); for(int i=0;i<frames_n;i++) frames[i]=-1;
    int** snapshot=(int**)malloc(n*sizeof(int*));
    int* hits=(int*)calloc(n,sizeof(int));
    int* evict=(int*)malloc(n*sizeof(int)); for(int i=0;i<n;i++) evict[i]=-1;
    int ptr=0;
    for(int i=0;i<n;i++){
        int p=refs[i]; int pos=-1; for(int j=0;j<frames_n;j++) if(frames[j]==p){ pos=j; break; }
        if(pos!=-1){ hits[i]=1; }
        else {
            int replaced=frames[ptr]; frames[ptr]=p; evict[i]=replaced; ptr=(ptr+1)%frames_n;
        }
        snapshot[i]=(int*)malloc(frames_n*sizeof(int)); memcpy(snapshot[i],frames,frames_n*sizeof(int));
    }
    print_trace(refs,n,hits,evict,snapshot,frames_n);
    for(int i=0;i<n;i++) free(snapshot[i]); free(snapshot); free(hits); free(evict); free(frames);
}

static void lru(int* refs,int n,int frames_n){
    int* frames=(int*)malloc(frames_n*sizeof(int)); for(int i=0;i<frames_n;i++) frames[i]=-1;
    int* last=(int*)malloc(frames_n*sizeof(int)); for(int i=0;i<frames_n;i++) last[i]=-1;
    int** snapshot=(int**)malloc(n*sizeof(int*));
    int* hits=(int*)calloc(n,sizeof(int));
    int* evict=(int*)malloc(n*sizeof(int)); for(int i=0;i<n;i++) evict[i]=-1;
    for(int i=0;i<n;i++){
        int p=refs[i]; int pos=-1;
        for(int j=0;j<frames_n;j++) if(frames[j]==p){ pos=j; break; }
        if(pos!=-1){ hits[i]=1; last[pos]=i; }
        else {
            int free_pos=-1; for(int j=0;j<frames_n;j++) if(frames[j]==-1){ free_pos=j; break; }
            if(free_pos!=-1){ frames[free_pos]=p; last[free_pos]=i; }
            else {
                int victim=0; for(int j=1;j<frames_n;j++) if(last[j]<last[victim]) victim=j;
                evict[i]=frames[victim]; frames[victim]=p; last[victim]=i;
            }
        }
        snapshot[i]=(int*)malloc(frames_n*sizeof(int)); memcpy(snapshot[i],frames,frames_n*sizeof(int));
    }
    print_trace(refs,n,hits,evict,snapshot,frames_n);
    for(int i=0;i<n;i++) free(snapshot[i]); free(snapshot); free(hits); free(evict); free(frames); free(last);
}

static int next_use(int* refs,int n,int start,int val){
    for(int i=start;i<n;i++) if(refs[i]==val) return i; return 1e9;
}

static void opt(int* refs,int n,int frames_n){
    int* frames=(int*)malloc(frames_n*sizeof(int)); for(int i=0;i<frames_n;i++) frames[i]=-1;
    int** snapshot=(int**)malloc(n*sizeof(int*));
    int* hits=(int*)calloc(n,sizeof(int));
    int* evict=(int*)malloc(n*sizeof(int)); for(int i=0;i<n;i++) evict[i]=-1;
    for(int i=0;i<n;i++){
        int p=refs[i]; int pos=-1; for(int j=0;j<frames_n;j++) if(frames[j]==p){ pos=j; break; }
        if(pos!=-1){ hits[i]=1; }
        else {
            int free_pos=-1; for(int j=0;j<frames_n;j++) if(frames[j]==-1){ free_pos=j; break; }
            if(free_pos!=-1){ frames[free_pos]=p; }
            else {
                int victim=0; int best=-1; for(int j=0;j<frames_n;j++){ int nu=next_use(refs,n,i+1,frames[j]); if(nu>best){ best=nu; victim=j; } }
                evict[i]=frames[victim]; frames[victim]=p;
            }
        }
        snapshot[i]=(int*)malloc(frames_n*sizeof(int)); memcpy(snapshot[i],frames,frames_n*sizeof(int));
    }
    print_trace(refs,n,hits,evict,snapshot,frames_n);
    for(int i=0;i<n;i++) free(snapshot[i]); free(snapshot); free(hits); free(evict); free(frames);
}

static void lfu(int* refs,int n,int frames_n){
    int* frames=(int*)malloc(frames_n*sizeof(int)); for(int i=0;i<frames_n;i++) frames[i]=-1;
    int* freq=(int*)malloc(frames_n*sizeof(int)); for(int i=0;i<frames_n;i++) freq[i]=0;
    int* age=(int*)malloc(frames_n*sizeof(int)); for(int i=0;i<frames_n;i++) age[i]=0;
    int** snapshot=(int**)malloc(n*sizeof(int*));
    int* hits=(int*)calloc(n,sizeof(int));
    int* evict=(int*)malloc(n*sizeof(int)); for(int i=0;i<n;i++) evict[i]=-1;
    for(int i=0;i<n;i++){
        for(int j=0;j<frames_n;j++) age[j]++;
        int p=refs[i]; int pos=-1; for(int j=0;j<frames_n;j++) if(frames[j]==p){ pos=j; break; }
        if(pos!=-1){ hits[i]=1; freq[pos]++; age[pos]=0; }
        else {
            int free_pos=-1; for(int j=0;j<frames_n;j++) if(frames[j]==-1){ free_pos=j; break; }
            if(free_pos!=-1){ frames[free_pos]=p; freq[free_pos]=1; age[free_pos]=0; }
            else {
                int victim=0; for(int j=1;j<frames_n;j++){
                    if(freq[j]<freq[victim] || (freq[j]==freq[victim] && age[j]>age[victim])) victim=j;
                }
                evict[i]=frames[victim]; frames[victim]=p; freq[victim]=1; age[victim]=0;
            }
        }
        snapshot[i]=(int*)malloc(frames_n*sizeof(int)); memcpy(snapshot[i],frames,frames_n*sizeof(int));
    }
    print_trace(refs,n,hits,evict,snapshot,frames_n);
    for(int i=0;i<n;i++) free(snapshot[i]); free(snapshot); free(hits); free(evict); free(frames); free(freq); free(age);
}

void run_memory(const int* in_refs,int n,int frames_n,const char* algo){
    int* refs=(int*)malloc(n*sizeof(int)); memcpy(refs,in_refs,n*sizeof(int));
    if(strcmp(algo,"FIFO")==0){ fifo(refs,n,frames_n); free(refs); return; }
    if(strcmp(algo,"LRU")==0){ lru(refs,n,frames_n); free(refs); return; }
    if(strcmp(algo,"OPT")==0){ opt(refs,n,frames_n); free(refs); return; }
    if(strcmp(algo,"LFU")==0){ lfu(refs,n,frames_n); free(refs); return; }
    fprintf(stderr,"algo inválido\n");
    free(refs);
}