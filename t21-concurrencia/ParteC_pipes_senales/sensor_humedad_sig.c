//t21-concurrencia/ParteC_pipes_senales/sensor_humedad_sig.c
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

static volatile sig_atomic_t got_usr1 = 0;
static volatile sig_atomic_t running  = 1;

static void h_usr1(int s){ (void)s; got_usr1 = 1; }
static void h_term(int s){ (void)s; running = 0; }

static double rnd_range(double min, double max){
    double r = rand() / (double)RAND_MAX;
    return min + r*(max-min);
}

static void now_str(char* buf, size_t n){
    time_t t=time(NULL); struct tm tm;
    localtime_r(&t,&tm);
    strftime(buf,n,"%Y-%m-%d %H:%M:%S",&tm);
}

int main(void){
    const char* fifo = "/tmp/fifo_humedad";
    const char* sname = "HUM";
    const char* pidfile = "/tmp/sensor_HUM.pid";
    srand((unsigned)time(NULL) ^ getpid());

    FILE* pf = fopen(pidfile,"w");
    if(pf){ fprintf(pf,"%d\n", getpid()); fclose(pf); }

    struct sigaction sa1 = {.sa_handler=h_usr1};
    sigemptyset(&sa1.sa_mask); sa1.sa_flags=0;
    sigaction(SIGUSR1,&sa1,NULL);
    struct sigaction sat = {.sa_handler=h_term};
    sigemptyset(&sat.sa_mask); sat.sa_flags=0;
    sigaction(SIGINT,&sat,NULL);
    sigaction(SIGTERM,&sat,NULL);

    if(mkfifo(fifo,0666)<0 && errno!=EEXIST){
        perror("mkfifo"); return 1;
    }
    int fd = open(fifo, O_WRONLY);
    if(fd<0){ perror("open fifo"); return 1; }

    fprintf(stderr, "[%s] PID=%d esperando SIGUSR1...\n", sname, getpid());
    char line[128], ts[64];
    while(running){
        pause();
        if(!running) break;
        if(got_usr1){
            got_usr1 = 0;
            double val = rnd_range(0.0,100.0);
            now_str(ts,sizeof ts);
            int n = snprintf(line,sizeof line, "%s;%.2f;%s;%d\n", sname, val, ts, getpid());
            write(fd, line, n);
        }
    }
    close(fd);
    unlink(pidfile);
    return 0;
}
