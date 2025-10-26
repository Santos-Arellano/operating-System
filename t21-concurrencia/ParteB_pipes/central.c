//t21-concurrencia/ParteB_pipes/central.c
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <time.h>

#define FIFO_TEMP     "/tmp/fifo_temp"
#define FIFO_HUMEDAD  "/tmp/fifo_humedad"
#define FIFO_VIENTO   "/tmp/fifo_viento"
#define FIFO_LLUVIA   "/tmp/FIFO_LLUVIA"

typedef struct {
    double temp, hum, vien, lluv;
    struct timespec ts_temp, ts_hum, ts_vien, ts_lluv;
} clima_t;

static void mkfifo_if_needed(const char* path){
    if(mkfifo(path, 0666) < 0){
        if(errno != EEXIST){
            perror("mkfifo");
            exit(1);
        }
    }
}

static void ts_to_str(const struct timespec* ts, char* buf, size_t n){
    struct tm tm;
    localtime_r(&ts->tv_sec, &tm);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", &tm);
}

static void write_file(const clima_t* c){
    FILE* f = fopen("clima_actual.txt.tmp","w");
    if(!f){ perror("fopen"); return; }
    char t1[32],t2[32],t3[32],t4[32];
    ts_to_str(&c->ts_temp,t1,sizeof t1);
    ts_to_str(&c->ts_hum ,t2,sizeof t2);
    ts_to_str(&c->ts_vien,t3,sizeof t3);
    ts_to_str(&c->ts_lluv,t4,sizeof t4);
    fprintf(f,
        "# Estado actual del clima (central con FIFOs)\n"
        "Temperatura (C): %.2f (ts=%s)\n"
        "Humedad (%%): %.2f (ts=%s)\n"
        "Viento (m/s): %.2f (ts=%s)\n"
        "Precipitacion (mm): %.2f (ts=%s)\n",
        c->temp,t1,c->hum,t2,c->vien,t3,c->lluv,t4
    );
    fclose(f);
    rename("clima_actual.txt.tmp","clima_actual.txt");
}

int main(void){
    mkfifo_if_needed(FIFO_TEMP);
    mkfifo_if_needed(FIFO_HUMEDAD);
    mkfifo_if_needed(FIFO_VIENTO);
    mkfifo_if_needed(FIFO_LLUVIA);

    int fds[4];
    const char* names[4] = { "TEMP","HUM","VIENTO","LLUVIA" };
    const char* fifos[4] = { FIFO_TEMP, FIFO_HUMEDAD, FIFO_VIENTO, FIFO_LLUVIA };

    struct pollfd pfds[4];
    int dummy_w[4];

    for(int i=0;i<4;i++){
        fds[i] = open(fifos[i], O_RDONLY | O_NONBLOCK);
        if(fds[i]<0){ perror("open rd"); return 1; }
        dummy_w[i] = open(fifos[i], O_WRONLY | O_NONBLOCK);
        pfds[i].fd = fds[i];
        pfds[i].events = POLLIN;
    }

    clima_t c = {0};
    clock_gettime(CLOCK_REALTIME, &c.ts_temp);
    c.ts_hum = c.ts_vien = c.ts_lluv = c.ts_temp;

    char buf[256];
    printf("[central] Esperando datos en FIFOs...\n");
    while(1){
        int r = poll(pfds, 4, -1);
        if(r<0){ perror("poll"); break; }
        for(int i=0;i<4;i++){
            if(pfds[i].revents & POLLIN){
                ssize_t n = read(pfds[i].fd, buf, sizeof(buf)-1);
                if(n>0){
                    buf[n]=0;
                    char sensor[32], ts[64];
                    double val=0;
                    if(sscanf(buf, "%31[^;];%lf;%63[^\n]", sensor, &val, ts)==3){
                        struct timespec now; clock_gettime(CLOCK_REALTIME,&now);
                        if(strcmp(sensor,"TEMP")==0){ c.temp=val; c.ts_temp=now; }
                        else if(strcmp(sensor,"HUM")==0){ c.hum=val; c.ts_hum=now; }
                        else if(strcmp(sensor,"VIENTO")==0){ c.vien=val; c.ts_vien=now; }
                        else if(strcmp(sensor,"LLUVIA")==0){ c.lluv=val; c.ts_lluv=now; }
                        printf("[central] %s=%.2f (%s)\n", sensor, val, ts);
                        write_file(&c);
                    }
                }
            }
        }
    }

    for(int i=0;i<4;i++){ close(fds[i]); close(dummy_w[i]); }
    return 0;
}
