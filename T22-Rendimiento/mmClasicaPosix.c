/* mmClasicaPosix.c
 * Multiplicación de matrices clásica con Pthreads (Posix).
 * Uso: ./mm_pthreads N P
 *  - N: tamaño de matriz
 *  - P: número de hilos
 * Imprime C si N<=4 y SIEMPRE la última línea es el tiempo total (microsegundos).
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    const double *A;
    const double *B;
    double *C;
    int N;
    int row_start;
    int row_end;
} task_t;

static inline double now_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1e6 + (double)tv.tv_usec;
}

static void init_matrices(double *A, double *B, int N) {
    for (int i = 0; i < N*N; ++i) {
        A[i] = (double)rand() / RAND_MAX * 4.0 + 1.0;
        B[i] = (double)rand() / RAND_MAX * 7.0 + 2.0;
    }
}

static void* worker(void *arg) {
    task_t *t = (task_t*)arg;
    const int N = t->N;
    for (int i = t->row_start; i < t->row_end; ++i) {
        for (int j = 0; j < N; ++j) {
            double suma = 0.0;
            for (int k = 0; k < N; ++k) {
                suma += t->A[i*N + k] * t->B[k*N + j];
            }
            t->C[i*N + j] = suma;
        }
    }
    return NULL;
}

static void print_matrix(const char* name, const double *M, int N) {
    if (N > 4) return;
    printf("%s =\n", name);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%8.2f ", M[i*N + j]);
        }
        printf("\n");
    }
    printf("**-----------------------------**\n");
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s N P\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    int P = atoi(argv[2]);
    if (N <= 0 || P <= 0) {
        fprintf(stderr, "Parámetros inválidos\n");
        return 1;
    }

    double *A = (double*) aligned_alloc(64, sizeof(double)*N*N);
    double *B = (double*) aligned_alloc(64, sizeof(double)*N*N);
    double *C = (double*) aligned_alloc(64, sizeof(double)*N*N);
    if (!A || !B || !C) {
        fprintf(stderr, "Fallo al reservar memoria\n");
        return 1;
    }

    srand(21);
    init_matrices(A, B, N);

    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)*P);
    task_t    *tasks   = (task_t*)   malloc(sizeof(task_t)*P);
    if (!threads || !tasks) {
        fprintf(stderr, "Fallo al reservar hilos\n");
        return 1;
    }

    // Reparto por filas
    int base = N / P;
    int rem  = N % P;
    int row = 0;
    for (int i = 0; i < P; ++i) {
        int take = base + (i < rem ? 1 : 0);
        tasks[i].A = A; tasks[i].B = B; tasks[i].C = C; tasks[i].N = N;
        tasks[i].row_start = row;
        tasks[i].row_end   = row + take;
        row += take;
    }

    double t0 = now_us();
    for (int i = 0; i < P; ++i) {
        pthread_create(&threads[i], NULL, worker, &tasks[i]);
    }
    for (int i = 0; i < P; ++i) {
        pthread_join(threads[i], NULL);
    }
    double t1 = now_us();

    print_matrix("C", C, N);
    printf("%.0f\n", (t1 - t0));

    free(A); free(B); free(C);
    free(threads); free(tasks);
    return 0;
}
