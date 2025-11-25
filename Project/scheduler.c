#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Proc { char pid[64]; double arrival; double burst; int priority; };
struct Seg { char pid[64]; double start; double end; };

static int cmp_arrival(const void* a,const void* b){
    const struct Proc* x=a; const struct Proc* y=b;
    if(x->arrival<y->arrival) return -1; if(x->arrival>y->arrival) return 1;
    return strcmp(x->pid,y->pid);
}

static void print_metrics(struct Seg* segs,int nsegs,struct Proc* procs,int n, double* completion){
    double avg_wait=0,avg_turn=0,avg_resp=0;
    double* wait=(double*)calloc(n,sizeof(double));
    double* turn=(double*)calloc(n,sizeof(double));
    double* resp=(double*)calloc(n,sizeof(double));
    double* first=(double*)malloc(n*sizeof(double));
    for(int i=0;i<n;i++) first[i]=1e18;
    for(int i=0;i<nsegs;i++){
        for(int j=0;j<n;j++){
            if(strcmp(segs[i].pid,procs[j].pid)==0){ if(first[j]>1e17) first[j]=segs[i].start; break; }
        }
    }
    for(int i=0;i<n;i++){
        turn[i]=completion[i]-procs[i].arrival;
        wait[i]=turn[i]-procs[i].burst;
        resp[i]=((first[i]>1e17)?0.0:(first[i]-procs[i].arrival));
        avg_wait+=wait[i]; avg_turn+=turn[i]; avg_resp+=resp[i];
    }
    avg_wait/=n; avg_turn/=n; avg_resp/=n;
    printf("{\"schedule\":[");
    for(int i=0;i<nsegs;i++){
        printf("{\"pid\":\"%s\",\"start\":%.6f,\"end\":%.6f}%s",segs[i].pid,segs[i].start,segs[i].end,(i<nsegs-1?",":""));
    }
    printf("],\"waiting\":{");
    for(int i=0;i<n;i++){
        printf("\"%s\":%.6f%s",procs[i].pid,wait[i],(i<n-1?",":""));
    }
    printf("},\"turnaround\":{");
    for(int i=0;i<n;i++){
        printf("\"%s\":%.6f%s",procs[i].pid,turn[i],(i<n-1?",":""));
    }
    printf("},\"response\":{");
    for(int i=0;i<n;i++){
        printf("\"%s\":%.6f%s",procs[i].pid,resp[i],(i<n-1?",":""));
    }
    printf("},\"avg_wait\":%.6f,\"avg_turn\":%.6f,\"avg_response\":%.6f}\n",avg_wait,avg_turn,avg_resp);
    free(wait); free(turn); free(resp); free(first);
}

static void fcfs(struct Proc* in,int n){
    struct Proc* procs=(struct Proc*)malloc(n*sizeof(struct Proc)); memcpy(procs,in,n*sizeof(struct Proc));
    qsort(procs,n,sizeof(struct Proc),cmp_arrival);
    struct Seg* segs=NULL; int nsegs=0, cap=0;
    double* completion=(double*)malloc(n*sizeof(double));
    double t=0;
    for(int i=0;i<n;i++){
        if(t<procs[i].arrival) t=procs[i].arrival;
        double start=t, end=t+procs[i].burst;
        if(nsegs==cap){ cap=cap?cap*2:8; segs=(struct Seg*)realloc(segs,cap*sizeof(struct Seg)); }
        strncpy(segs[nsegs].pid,procs[i].pid,sizeof(segs[nsegs].pid)); segs[nsegs].start=start; segs[nsegs].end=end; nsegs++;
        t=end; completion[i]=end;
    }
    print_metrics(segs,nsegs,procs,n,completion);
    free(segs); free(completion); free(procs);
}

static void sjf(struct Proc* in,int n){
    struct Proc* procs=(struct Proc*)malloc(n*sizeof(struct Proc)); memcpy(procs,in,n*sizeof(struct Proc));
    qsort(procs,n,sizeof(struct Proc),cmp_arrival);
    struct Proc* ready=(struct Proc*)malloc(n*sizeof(struct Proc)); int rsz=0;
    struct Seg* segs=NULL; int nsegs=0, cap=0;
    double* completion=(double*)malloc(n*sizeof(double));
    double t=0; int i=0;
    while(i<n || rsz>0){
        while(i<n && procs[i].arrival<=t){ ready[rsz++]=procs[i++]; }
        if(rsz==0){ t=procs[i].arrival; continue; }
        int idx=0;
        for(int k=1;k<rsz;k++){
            if(ready[k].burst<ready[idx].burst || (ready[k].burst==ready[idx].burst && ready[k].arrival<ready[idx].arrival)) idx=k;
        }
        struct Proc p=ready[idx];
        for(int k=idx;k<rsz-1;k++) ready[k]=ready[k+1]; rsz--;
        double start=t,end=t+p.burst;
        if(nsegs==cap){ cap=cap?cap*2:8; segs=(struct Seg*)realloc(segs,cap*sizeof(struct Seg)); }
        strncpy(segs[nsegs].pid,p.pid,sizeof(segs[nsegs].pid)); segs[nsegs].start=start; segs[nsegs].end=end; nsegs++;
        t=end;
        for(int k=0;k<n;k++) if(strcmp(procs[k].pid,p.pid)==0) { completion[k]=end; break; }
    }
    print_metrics(segs,nsegs,procs,n,completion);
    free(ready); free(segs); free(completion); free(procs);
}

static void srtf(struct Proc* in,int n){
    struct Proc* procs=(struct Proc*)malloc(n*sizeof(struct Proc)); memcpy(procs,in,n*sizeof(struct Proc));
    qsort(procs,n,sizeof(struct Proc),cmp_arrival);
    double* rem=(double*)malloc(n*sizeof(double)); for(int i=0;i<n;i++) rem[i]=procs[i].burst;
    int* done=(int*)calloc(n,sizeof(int));
    struct Seg* segs=NULL; int nsegs=0, cap=0;
    double* completion=(double*)malloc(n*sizeof(double));
    double t=0; int finished=0; int cur=-1; double seg_start=0;
    int i=0;
    while(finished<n){
        while(i<n && procs[i].arrival<=t) i++;
        int best=-1;
        for(int k=0;k<i;k++){
            if(done[k]) continue;
            if(best==-1 || rem[k]<rem[best]) best=k;
        }
        if(best==-1){
            t=procs[i].arrival; continue;
        }
        if(cur!=best){
            if(cur!=-1){
                if(nsegs==cap){ cap=cap?cap*2:8; segs=(struct Seg*)realloc(segs,cap*sizeof(struct Seg)); }
                strncpy(segs[nsegs].pid,procs[cur].pid,sizeof(segs[nsegs].pid)); segs[nsegs].start=seg_start; segs[nsegs].end=t; nsegs++;
            }
            cur=best; seg_start=t;
        }
        double next_arr=(i<n?procs[i].arrival:1e18);
        double delta=rem[cur]; double gap=next_arr-t; if(gap<delta && gap>1e-9) delta=gap; if(delta<1e-9) delta=1e-9;
        t+=delta; rem[cur]-=delta;
        if(rem[cur]<=0 && !done[cur]){
            done[cur]=1; completion[cur]=t; finished++;
        }
    }
    if(cur!=-1){
        if(nsegs==cap){ cap=cap?cap*2:8; segs=(struct Seg*)realloc(segs,cap*sizeof(struct Seg)); }
        strncpy(segs[nsegs].pid,procs[cur].pid,sizeof(segs[nsegs].pid)); segs[nsegs].start=seg_start; segs[nsegs].end=t; nsegs++;
    }
    print_metrics(segs,nsegs,procs,n,completion);
    free(rem); free(done); free(segs); free(completion); free(procs);
}

static void prio(struct Proc* in,int n){
    struct Proc* procs=(struct Proc*)malloc(n*sizeof(struct Proc)); memcpy(procs,in,n*sizeof(struct Proc));
    qsort(procs,n,sizeof(struct Proc),cmp_arrival);
    struct Proc* ready=(struct Proc*)malloc(n*sizeof(struct Proc)); int rsz=0;
    struct Seg* segs=NULL; int nsegs=0, cap=0;
    double* completion=(double*)malloc(n*sizeof(double));
    double t=0; int i=0;
    while(i<n || rsz>0){
        while(i<n && procs[i].arrival<=t){ ready[rsz++]=procs[i++]; }
        if(rsz==0){ t=procs[i].arrival; continue; }
        int idx=0;
        for(int k=1;k<rsz;k++){
            if(ready[k].priority<ready[idx].priority || (ready[k].priority==ready[idx].priority && ready[k].arrival<ready[idx].arrival)) idx=k;
        }
        struct Proc p=ready[idx];
        for(int k=idx;k<rsz-1;k++) ready[k]=ready[k+1]; rsz--;
        double start=t,end=t+p.burst;
        if(nsegs==cap){ cap=cap?cap*2:8; segs=(struct Seg*)realloc(segs,cap*sizeof(struct Seg)); }
        strncpy(segs[nsegs].pid,p.pid,sizeof(segs[nsegs].pid)); segs[nsegs].start=start; segs[nsegs].end=end; nsegs++;
        t=end;
        for(int k=0;k<n;k++) if(strcmp(procs[k].pid,p.pid)==0) { completion[k]=end; break; }
    }
    print_metrics(segs,nsegs,procs,n,completion);
    free(ready); free(segs); free(completion); free(procs);
}

static void prio_preempt(struct Proc* in,int n){
    struct Proc* procs=(struct Proc*)malloc(n*sizeof(struct Proc)); memcpy(procs,in,n*sizeof(struct Proc));
    qsort(procs,n,sizeof(struct Proc),cmp_arrival);
    double* rem=(double*)malloc(n*sizeof(double)); for(int i=0;i<n;i++) rem[i]=procs[i].burst;
    int* done=(int*)calloc(n,sizeof(int));
    struct Seg* segs=NULL; int nsegs=0, cap=0;
    double* completion=(double*)malloc(n*sizeof(double));
    double t=0; int finished=0; int cur=-1; double seg_start=0;
    int i=0;
    while(finished<n){
        while(i<n && procs[i].arrival<=t) i++;
        int best=-1;
        for(int k=0;k<i;k++){
            if(done[k]) continue;
            if(best==-1 || procs[k].priority<procs[best].priority || (procs[k].priority==procs[best].priority && rem[k]<rem[best])) best=k;
        }
        if(best==-1){
            t=procs[i].arrival; continue;
        }
        if(cur!=best){
            if(cur!=-1){
                if(nsegs==cap){ cap=cap?cap*2:8; segs=(struct Seg*)realloc(segs,cap*sizeof(struct Seg)); }
                strncpy(segs[nsegs].pid,procs[cur].pid,sizeof(segs[nsegs].pid)); segs[nsegs].start=seg_start; segs[nsegs].end=t; nsegs++;
            }
            cur=best; seg_start=t;
        }
        double next_arr=(i<n?procs[i].arrival:1e18);
        double delta=rem[cur]; double gap=next_arr-t; if(gap<delta && gap>1e-9) delta=gap; if(delta<1e-9) delta=1e-9;
        t+=delta; rem[cur]-=delta;
        if(rem[cur]<=0 && !done[cur]){
            done[cur]=1; completion[cur]=t; finished++;
        }
    }
    if(cur!=-1){
        if(nsegs==cap){ cap=cap?cap*2:8; segs=(struct Seg*)realloc(segs,cap*sizeof(struct Seg)); }
        strncpy(segs[nsegs].pid,procs[cur].pid,sizeof(segs[nsegs].pid)); segs[nsegs].start=seg_start; segs[nsegs].end=t; nsegs++;
    }
    print_metrics(segs,nsegs,procs,n,completion);
    free(rem); free(done); free(segs); free(completion); free(procs);
}

static void rr(struct Proc* in,int n,double quantum){
    struct Proc* procs=(struct Proc*)malloc(n*sizeof(struct Proc)); memcpy(procs,in,n*sizeof(struct Proc));
    qsort(procs,n,sizeof(struct Proc),cmp_arrival);
    double* rem=(double*)malloc(n*sizeof(double)); for(int i=0;i<n;i++) rem[i]=procs[i].burst;
    int* done=(int*)calloc(n,sizeof(int));
    int* q=(int*)malloc(n*sizeof(int)); int qs=0, qh=0;
    struct Seg* segs=NULL; int nsegs=0, cap=0;
    double* completion=(double*)malloc(n*sizeof(double));
    double t=0; int i=0; int finished=0;
    while(finished<n){
        while(i<n && procs[i].arrival<=t){ q[qs++]=i; i++; }
        if(qs==qh){
            t=procs[i].arrival; continue;
        }
        int idx=q[qh++]; if(qh==qs){ qh=0; qs=0; }
        double run=quantum; if(rem[idx]<run) run=rem[idx];
        double start=t,end=t+run;
        if(nsegs==cap){ cap=cap?cap*2:8; segs=(struct Seg*)realloc(segs,cap*sizeof(struct Seg)); }
        strncpy(segs[nsegs].pid,procs[idx].pid,sizeof(segs[nsegs].pid)); segs[nsegs].start=start; segs[nsegs].end=end; nsegs++;
        t=end; rem[idx]-=run;
        while(i<n && procs[i].arrival<=t){ q[qs++]=i; i++; }
        if(rem[idx]>0){ q[qs++]=idx; } else { done[idx]=1; completion[idx]=t; finished++; }
    }
    print_metrics(segs,nsegs,procs,n,completion);
    free(rem); free(done); free(q); free(segs); free(completion); free(procs);
}

void run_scheduler(struct Proc* procs,int n,const char* algo,double quantum){
    if(strcmp(algo,"FCFS")==0){ fcfs(procs,n); return; }
    if(strcmp(algo,"SJF")==0){ sjf(procs,n); return; }
    if(strcmp(algo,"SRTF")==0){ srtf(procs,n); return; }
    if(strcmp(algo,"PRIO")==0){ prio(procs,n); return; }
    if(strcmp(algo,"PRIO_PREEMPT")==0){ prio_preempt(procs,n); return; }
    if(strcmp(algo,"RR")==0){ rr(procs,n,quantum); return; }
    fprintf(stderr,"algo inválido\n");
}