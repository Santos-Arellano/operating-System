//t21-concurrencia/ParteA_mutex_semaforos/climate_threads.c
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>

typedef struct {
    double temperatura;
    double humedad;
    double viento;
    double lluvia;
    struct timespec ts_temp;
    struct timespec ts_hum;
    struct timespec ts_viento;
    struct timespec ts_lluvia;
} clima_t;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static _Atomic int running = 1;
static clima_t shared = {0};

static void h_sigint(int sig){
    (void)sig;
    running = 0;
}

static long ms_now() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static void ts_to_str(const struct timespec* ts, char* buf, size_t n){
    struct tm tm;
    localtime_r(&ts->tv_sec, &tm);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", &tm);
}

static void write_file_atomic(){
    pthread_mutex_lock(&mtx);
    char t1[32], t2[32], t3[32], t4[32];
    ts_to_str(&shared.ts_temp, t1, sizeof t1);
    ts_to_str(&shared.ts_hum,  t2, sizeof t2);
    ts_to_str(&shared.ts_viento,t3, sizeof t3);
    ts_to_str(&shared.ts_lluvia,t4, sizeof t4);

    FILE* f = fopen("clima_actual.txt.tmp", "w");
    if(!f){ perror("fopen"); pthread_mutex_unlock(&mtx); return; }

    fprintf(f,
        "# Estado actual del clima (actualizado por seccion critica con mutex)\n"
        "Proceso PID: %d\n"
        "Temperatura (C): %.2f (ts=%s)\n"
        "Humedad (%%): %.2f (ts=%s)\n"
        "Viento (m/s): %.2f (ts=%s)\n"
        "Precipitacion (mm): %.2f (ts=%s)\n",
        getpid(),
        shared.temperatura, t1,
        shared.humedad, t2,
        shared.viento, t3,
        shared.lluvia, t4
    );
    fclose(f);
    rename("clima_actual.txt.tmp", "clima_actual.txt");
    pthread_mutex_unlock(&mtx);
}

typedef struct {
    const char* nombre;
    int base_s;
    int jitter_s;
    unsigned int seed;
} sensor_cfg_t;

static double rnd_range(unsigned int* seed, double min, double max){
    double r = rand_r(seed) / (double)RAND_MAX;
    return min + r*(max-min);
}

static int rnd_int_range(unsigned int* seed, int min, int max){
    if(max<=min) return min;
    return min + (rand_r(seed) % (max - min + 1));
}

static void* hilo_sensor(void* arg){
    sensor_cfg_t* cfg = (sensor_cfg_t*)arg;
    while(running){
        int delta = rnd_int_range(&cfg->seed, -cfg->jitter_s, cfg->jitter_s);
        int sleep_s = cfg->base_s + delta;
        if(sleep_s < 1) sleep_s = 1;
        for(int i=0;i<sleep_s && running;i++) sleep(1);
        if(!running) break;

        double valor = 0.0;
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);

        if(strcmp(cfg->nombre,"temperatura")==0){
            valor = rnd_range(&cfg->seed, -5.0, 40.0);
            pthread_mutex_lock(&mtx);
            shared.temperatura = valor;
            shared.ts_temp = now;
            pthread_mutex_unlock(&mtx);
        } else if(strcmp(cfg->nombre,"humedad")==0){
            valor = rnd_range(&cfg->seed, 0.0, 100.0);
            pthread_mutex_lock(&mtx);
            shared.humedad = valor;
            shared.ts_hum = now;
            pthread_mutex_unlock(&mtx);
        } else if(strcmp(cfg->nombre,"viento")==0){
            valor = rnd_range(&cfg->seed, 0.0, 30.0);
            pthread_mutex_lock(&mtx);
            shared.viento = valor;
            shared.ts_viento = now;
            pthread_mutex_unlock(&mtx);
        } else if(strcmp(cfg->nombre,"lluvia")==0){
            valor = rnd_range(&cfg->seed, 0.0, 50.0);
            pthread_mutex_lock(&mtx);
            shared.lluvia = valor;
            shared.ts_lluvia = now;
            pthread_mutex_unlock(&mtx);
        }

        printf("[%-11s] nuevo=%.2f a las %ld ms\n", cfg->nombre, valor, ms_now());
        fflush(stdout);

        write_file_atomic();
    }
    return NULL;
}

int main(void){
    signal(SIGINT, h_sigint);

    clock_gettime(CLOCK_REALTIME, &shared.ts_temp);
    shared.ts_hum = shared.ts_viento = shared.ts_lluvia = shared.ts_temp;

    // Escritura inicial del archivo para que exista desde el arranque
    write_file_atomic();

    sensor_cfg_t cfgs[4] = {
        { "temperatura", 8, 3, (unsigned)time(NULL) ^ 0xABC1 },
        { "humedad"    ,12, 2, (unsigned)time(NULL) ^ 0xABC2 },
        { "viento"     , 5, 2, (unsigned)time(NULL) ^ 0xABC3 },
        { "lluvia"     ,10, 5, (unsigned)time(NULL) ^ 0xABC4 },
    };

    pthread_t th[4];
    for(int i=0;i<4;i++){
        if(pthread_create(&th[i], NULL, hilo_sensor, &cfgs[i])!=0){
            perror("pthread_create");
            return 1;
        }
    }

    printf("Ejecutando... Ctrl+C para salir.\n");
    for(int i=0;i<4;i++) pthread_join(th[i], NULL);
    printf("Cerrado.\n");
    return 0;
}
