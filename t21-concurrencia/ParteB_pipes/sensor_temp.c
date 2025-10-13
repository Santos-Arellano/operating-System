//t21-concurrencia/ParteB_pipes/sensor_temp.c
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

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
    const char* fifo = "/tmp/fifo_temp";
    const char* sname = "TEMP";
    int base = 8;   // segundos base
    int jitter = 3; // +/-
    srand((unsigned)time(NULL) ^ getpid());

    if(mkfifo(fifo,0666)<0 && errno!=EEXIST){
        perror("mkfifo"); return 1;
    }

    fprintf(stderr, "[TEMP] Abriendo FIFO %s ...\\n", fifo);
    int fd = open(fifo, O_WRONLY);
    if(fd<0){ perror("open"); return 1; }

    fprintf(stderr, "[TEMP] Iniciando generacion indeterminada (base=%d, jitter=+/- %d)...\\n", base, jitter);
    char line[128], ts[64];
    while(1){
        int delta = (rand()%(2*jitter+1)) - jitter;
        int sleep_s = base + delta;
        if(sleep_s<1) sleep_s=1;
        sleep(sleep_s);
        double val = rnd_range(-5.0,40.0);
        now_str(ts,sizeof ts);
        int n = snprintf(line,sizeof line, "%s;%.2f;%s\\n", sname, val, ts);
        write(fd, line, n);
    }
    return 0;
}
