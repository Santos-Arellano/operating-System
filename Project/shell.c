#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

static char* trim(char* s){ while(*s==' '||*s=='\t'||*s=='\n') s++; char* e=s+strlen(s); while(e>s && (e[-1]==' '||e[-1]=='\t'||e[-1]=='\n')) *--e='\0'; return s; }

static int split(char* s,char** out,int max){ int n=0; char* tok=strtok(s," \t"); while(tok&&n<max){ out[n++]=tok; tok=strtok(NULL," \t"); } return n; }

static int parse_pipeline(char* line,char** cmds,int max){ int n=0; char* p=line; while(n<max){ char* q=strchr(p,'|'); if(!q){ cmds[n++]=p; break; } *q='\0'; cmds[n++]=p; p=q+1; } return n; }

static int is_bg(char* s){ size_t len=strlen(s); if(len>0 && s[len-1]=='&'){ s[len-1]='\0'; return 1; } return 0; }

static void run_command(char** argv,int bg,int in_fd,int out_fd){
    if(argv[0]==NULL) _exit(0);
    if(strcmp(argv[0],"cd")==0){ if(argv[1]) chdir(argv[1]); return; }
    if(strcmp(argv[0],"exit")==0){ exit(0); }
    pid_t pid=fork();
    if(pid==0){
        if(in_fd!=-1){ dup2(in_fd,0); }
        if(out_fd!=-1){ dup2(out_fd,1); }
        execvp(argv[0],argv);
        perror("exec"); _exit(127);
    } else {
        if(!bg){ int st; waitpid(pid,&st,0); }
    }
}

static void parse_redirs(char** argv,int* in_fd,int* out_fd){
    *in_fd=-1; *out_fd=-1;
    for(int i=0; argv[i]; i++){
        if(strcmp(argv[i],"<")==0 && argv[i+1]){ int fd=open(argv[i+1],O_RDONLY); if(fd>=0) *in_fd=fd; argv[i]=NULL; }
        else if(strcmp(argv[i],">")==0 && argv[i+1]){ int fd=open(argv[i+1],O_CREAT|O_WRONLY|O_TRUNC,0644); if(fd>=0) *out_fd=fd; argv[i]=NULL; }
        else if(strcmp(argv[i],">>")==0 && argv[i+1]){ int fd=open(argv[i+1],O_CREAT|O_WRONLY|O_APPEND,0644); if(fd>=0) *out_fd=fd; argv[i]=NULL; }
    }
}

void run_shell(void){
    char* line=NULL; size_t cap=0;
    while(1){
        printf("$ "); fflush(stdout);
        ssize_t r=getline(&line,&cap,stdin); if(r<=0) break; char* l=trim(line); if(*l=='\0') continue;
        int bg=is_bg(l);
        char* cmds[32]; int ncmd=parse_pipeline(l,cmds,32);
        int in_fd=-1, out_fd=-1;
        if(ncmd==1){
            char* argv[128]; char* s=strdup(cmds[0]); int argc=split(s,argv,127); argv[argc]=NULL; parse_redirs(argv,&in_fd,&out_fd);
            run_command(argv,bg,in_fd,out_fd); if(in_fd!=-1) close(in_fd); if(out_fd!=-1) close(out_fd); free(s);
        } else {
            int prev_in=-1; int pipefd[2];
            for(int ci=0; ci<ncmd; ci++){
                char* argv[128]; char* s=strdup(cmds[ci]); int argc=split(s,argv,127); argv[argc]=NULL; int local_in=-1, local_out=-1; parse_redirs(argv,&local_in,&local_out);
                if(ci<ncmd-1){ pipe(pipefd); }
                int in_use=(prev_in!=-1?prev_in:local_in);
                int out_use=(ci<ncmd-1?pipefd[1]:local_out);
                pid_t pid=fork();
                if(pid==0){
                    if(in_use!=-1) dup2(in_use,0);
                    if(out_use!=-1) dup2(out_use,1);
                    if(ci<ncmd-1){ close(pipefd[0]); }
                    execvp(argv[0],argv); perror("exec"); _exit(127);
                } else {
                    if(prev_in!=-1) close(prev_in);
                    if(local_in!=-1 && in_use==local_in) close(local_in);
                    if(ci<ncmd-1){ close(pipefd[1]); prev_in=pipefd[0]; }
                    else { if(out_use!=-1) close(out_use); }
                    if(!bg){ int st; waitpid(pid,&st,0); }
                }
                free(s);
            }
            if(prev_in!=-1) close(prev_in);
        }
    }
    free(line);
}