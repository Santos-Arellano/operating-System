#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct Proc { char pid[64]; double arrival; double burst; int priority; };

void run_scheduler(struct Proc* procs,int n,const char* algo,double quantum);
void run_memory(const int* refs,int n,int frames,const char* algo);
void run_disk(const int* req,int n,int start,const char* algo,const char* direction,int limit);
void run_pc(int buffer_size,int producers,int consumers,int seconds);
void run_dp(int n,int seconds);
void run_shell(void);

static int parse_int_list(const char* s,int** out){
    int count=1; for(const char* p=s;*p;p++) if(*p==',') count++;
    int* arr=(int*)malloc(count*sizeof(int)); int i=0; const char* p=s;
    while(*p){
        int v=0; int sign=1;
        if(*p=='-'){sign=-1; p++;}
        while(*p && isdigit((unsigned char)*p)){ v=v*10+(*p-'0'); p++; }
        arr[i++]=v*sign;
        if(*p==',') p++; else break;
    }
    *out=arr; return i;
}

int main(int argc,char** argv){
    if(argc<2){ fprintf(stderr,"uso: %s <schedule|memory|disk|sync|shell> ...\n",argv[0]); return 1; }
    if(strcmp(argv[1],"schedule")==0){
        const char* algo=NULL; double quantum=1.0;
        struct Proc* procs=NULL; int np=0; int cap=0;
        int* prios=NULL; int npr=0;
        for(int i=2;i<argc;i++){
            if(strcmp(argv[i],"-a")==0||strcmp(argv[i],"--algo")==0){ algo=argv[++i]; }
            else if(strcmp(argv[i],"-q")==0||strcmp(argv[i],"--quantum")==0){ quantum=strtod(argv[++i],NULL); }
            else if(strcmp(argv[i],"-p")==0||strcmp(argv[i],"--process")==0){
                const char* s=argv[++i];
                char buf[128]; strncpy(buf,s,sizeof(buf)); buf[sizeof(buf)-1]='\0';
                char* pid=strtok(buf,":"); char* arr=strtok(NULL,":"); char* bur=strtok(NULL,":");
                if(!pid||!arr||!bur){ fprintf(stderr,"formato pid:arrival:burst\n"); return 1; }
                if(np==cap){ cap=cap?cap*2:8; procs=(struct Proc*)realloc(procs,cap*sizeof(struct Proc)); }
                strncpy(procs[np].pid,pid,sizeof(procs[np].pid)); procs[np].pid[sizeof(procs[np].pid)-1]='\0';
                procs[np].arrival=strtod(arr,NULL); procs[np].burst=strtod(bur,NULL); procs[np].priority=0; np++;
            } else if(strcmp(argv[i],"-P")==0||strcmp(argv[i],"--priority")==0){
                int left=argc-i-1; npr=left; prios=(int*)malloc(npr*sizeof(int));
                for(int j=0;j<left;j++) prios[j]=atoi(argv[i+1+j]); break;
            }
        }
        if(!algo||np==0){ fprintf(stderr,"requerido: -a ALGO y al menos -p\n"); return 1; }
        if(prios){ for(int i=0;i<np && i<npr;i++) procs[i].priority=prios[i]; free(prios); }
        run_scheduler(procs,np,algo,quantum);
        free(procs);
        return 0;
    } else if(strcmp(argv[1],"memory")==0){
        const char* algo=NULL; int frames=0; int* refs=NULL; int n=0;
        for(int i=2;i<argc;i++){
            if(strcmp(argv[i],"-a")==0||strcmp(argv[i],"--algo")==0){ algo=argv[++i]; }
            else if(strcmp(argv[i],"-f")==0||strcmp(argv[i],"--frames")==0){ frames=atoi(argv[++i]); }
            else if(strcmp(argv[i],"-r")==0||strcmp(argv[i],"--refs")==0){ n=parse_int_list(argv[++i],&refs); }
        }
        if(!algo||frames<=0||!refs){ fprintf(stderr,"requerido: -a ALGO -f FRAMES -r lista\n"); return 1; }
        run_memory(refs,n,frames,algo);
        free(refs);
        return 0;
    } else if(strcmp(argv[1],"disk")==0){
        const char* algo=NULL; int start=0; const char* direction="up"; int limit=199; int* req=NULL; int n=0;
        for(int i=2;i<argc;i++){
            if(strcmp(argv[i],"-a")==0||strcmp(argv[i],"--algo")==0){ algo=argv[++i]; }
            else if(strcmp(argv[i],"-s")==0||strcmp(argv[i],"--start")==0){ start=atoi(argv[++i]); }
            else if(strcmp(argv[i],"-d")==0||strcmp(argv[i],"--direction")==0){ direction=argv[++i]; }
            else if(strcmp(argv[i],"-l")==0||strcmp(argv[i],"--limit")==0){ limit=atoi(argv[++i]); }
            else if(strcmp(argv[i],"-r")==0||strcmp(argv[i],"--req")==0){ n=parse_int_list(argv[++i],&req); }
        }
        if(!algo||n==0){ fprintf(stderr,"requerido: -a ALGO -r lista -s start\n"); return 1; }
        run_disk(req,n,start,algo,direction,limit);
        free(req);
        return 0;
    } else if(strcmp(argv[1],"sync")==0){
        if(argc<3){ fprintf(stderr,"sync pc|dp\n"); return 1; }
        if(strcmp(argv[2],"pc")==0){
            int seconds=5, buffer=5, producers=1, consumers=1;
            for(int i=3;i<argc;i++){
                if(strcmp(argv[i],"-t")==0) seconds=atoi(argv[++i]);
                else if(strcmp(argv[i],"-b")==0) buffer=atoi(argv[++i]);
                else if(strcmp(argv[i],"-P")==0) producers=atoi(argv[++i]);
                else if(strcmp(argv[i],"-C")==0) consumers=atoi(argv[++i]);
            }
            run_pc(buffer,producers,consumers,seconds);
        } else if(strcmp(argv[2],"dp")==0){
            int seconds=5, n=5;
            for(int i=3;i<argc;i++){
                if(strcmp(argv[i],"-t")==0) seconds=atoi(argv[++i]);
                else if(strcmp(argv[i],"-n")==0) n=atoi(argv[++i]);
            }
            run_dp(n,seconds);
        } else { fprintf(stderr,"sync pc|dp\n"); return 1; }
        return 0;
    } else if(strcmp(argv[1],"shell")==0){
        run_shell(); return 0;
    } else {
        fprintf(stderr,"subcomando inválido\n"); return 1;
    }
}