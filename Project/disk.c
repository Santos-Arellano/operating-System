#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp_int(const void* a,const void* b){ int x=*(const int*)a,y=*(const int*)b; return (x<y?-1:(x>y?1:0)); }

static void print_result(int start,int* seq,int seqn,int* order,int n,int movement){
    printf("{\"start\":%d,\"sequence\":[",start);
    for(int i=0;i<seqn;i++) printf("%d%s",seq[i],(i<seqn-1?",":""));
    printf("],\"order\":[");
    for(int i=0;i<n;i++) printf("%d%s",order[i],(i<n-1?",":""));
    printf("],\"total_movement\":%d}\n",movement);
}

static void fcfs(int* req,int n,int start){
    int* order=(int*)malloc(n*sizeof(int)); memcpy(order,req,n*sizeof(int));
    int* seq=(int*)malloc(n*sizeof(int)); memcpy(seq,req,n*sizeof(int));
    int movement=0; int cur=start; for(int i=0;i<n;i++){ movement+=abs(seq[i]-cur); cur=seq[i]; }
    print_result(start,seq,n,order,n,movement);
    free(order); free(seq);
}

static void sstf(int* req,int n,int start){
    int* used=(int*)calloc(n,sizeof(int));
    int* order=(int*)malloc(n*sizeof(int));
    int* seq=(int*)malloc(n*sizeof(int));
    int movement=0; int cur=start;
    for(int k=0;k<n;k++){
        int best=-1; int bestd=1e9;
        for(int i=0;i<n;i++) if(!used[i]){ int d=abs(req[i]-cur); if(d<bestd){ bestd=d; best=i; } }
        used[best]=1; order[k]=req[best]; seq[k]=req[best]; movement+=abs(req[best]-cur); cur=req[best];
    }
    print_result(start,seq,n,order,n,movement);
    free(used); free(order); free(seq);
}

static void scan(int* req,int n,int start,const char* direction,int limit){
    int* arr=(int*)malloc(n*sizeof(int)); memcpy(arr,req,n*sizeof(int)); qsort(arr,n,sizeof(int),cmp_int);
    int* order=(int*)malloc((n+limit+1)*sizeof(int));
    int* seq=(int*)malloc((n+limit+1)*sizeof(int));
    int k=0; int movement=0; int cur=start;
    int idx=0; while(idx<n && arr[idx]<start) idx++;
    if(strcmp(direction,"down")==0){
        for(int i=idx-1;i>=0;i--){ order[k]=arr[i]; seq[k]=arr[i]; movement+=abs(arr[i]-cur); cur=arr[i]; k++; }
        if(cur>0){ movement+=abs(cur-0); cur=0; }
        for(int i=idx;i<n;i++){ order[k]=arr[i]; seq[k]=arr[i]; movement+=abs(arr[i]-cur); cur=arr[i]; k++; }
    } else {
        for(int i=idx;i<n;i++){ order[k]=arr[i]; seq[k]=arr[i]; movement+=abs(arr[i]-cur); cur=arr[i]; k++; }
        if(cur<limit){ movement+=abs(limit-cur); cur=limit; }
        for(int i=idx-1;i>=0;i--){ order[k]=arr[i]; seq[k]=arr[i]; movement+=abs(arr[i]-cur); cur=arr[i]; k++; }
    }
    print_result(start,seq,k,order,k,movement);
    free(arr); free(order); free(seq);
}

static void cscan(int* req,int n,int start,const char* direction,int limit){
    int* arr=(int*)malloc(n*sizeof(int)); memcpy(arr,req,n*sizeof(int)); qsort(arr,n,sizeof(int),cmp_int);
    int* order=(int*)malloc(n*sizeof(int));
    int* seq=(int*)malloc((n+2)*sizeof(int));
    int k=0; int movement=0; int cur=start;
    int idx=0; while(idx<n && arr[idx]<start) idx++;
    if(strcmp(direction,"down")==0){
        for(int i=idx-1;i>=0;i--){ order[k]=arr[i]; seq[k]=arr[i]; movement+=abs(arr[i]-cur); cur=arr[i]; k++; }
        if(cur>0){ movement+=abs(cur-0); cur=0; }
        movement+=abs(limit-cur); cur=limit;
        for(int i=n-1;i>=idx;i--){ order[k]=arr[i]; seq[k]=arr[i]; movement+=abs(arr[i]-cur); cur=arr[i]; k++; }
    } else {
        for(int i=idx;i<n;i++){ order[k]=arr[i]; seq[k]=arr[i]; movement+=abs(arr[i]-cur); cur=arr[i]; k++; }
        if(cur<limit){ movement+=abs(limit-cur); cur=limit; }
        movement+=abs(cur-0); cur=0;
        for(int i=idx-1;i>=0;i--){ order[k]=arr[i]; seq[k]=arr[i]; movement+=abs(arr[i]-cur); cur=arr[i]; k++; }
    }
    print_result(start,seq,k,order,k,movement);
    free(arr); free(order); free(seq);
}

void run_disk(const int* in_req,int n,int start,const char* algo,const char* direction,int limit){
    int* req=(int*)malloc(n*sizeof(int)); memcpy(req,in_req,n*sizeof(int));
    if(strcmp(algo,"FCFS")==0){ fcfs(req,n,start); free(req); return; }
    if(strcmp(algo,"SSTF")==0){ sstf(req,n,start); free(req); return; }
    if(strcmp(algo,"SCAN")==0){ scan(req,n,start,direction,limit); free(req); return; }
    if(strcmp(algo,"CSCAN")==0){ cscan(req,n,start,direction,limit); free(req); return; }
    fprintf(stderr,"algo inválido\n");
    free(req);
}