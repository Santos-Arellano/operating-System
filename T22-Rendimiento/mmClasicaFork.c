/* mmClasicaFork.c
 * Multiplicación de matrices clásica con procesos fork().
 * Uso: ./mm_fork N P
 *  - N: tamaño de matriz
 *  - P: número de procesos hijos
 * Imprime C si N<=4 y SIEMPRE la última línea es el tiempo total (microsegundos).
 * Nota: usa memoria compartida (mmap) para recoger C en el proceso padre.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

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

    // Reservas (A,B privadas, C compartida)
    double *A = (double*) aligned_alloc(64, sizeof(double)*N*N);
    double *B = (double*) aligned_alloc(64, sizeof(double)*N*N);
    double *C = (double*) mmap(NULL, sizeof(double)*N*N, PROT_READ|PROT_WRITE,
                               MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if (!A || !B || C == MAP_FAILED) {
        fprintf(stderr, "Fallo al reservar memoria\n");
        return 1;
    }
    srand(21);
    init_matrices(A, B, N);

    // Partición por filas
    int base = N / P;
    int rem  = N % P;
    int row  = 0;

    double t0 = now_us();
    for (int i = 0; i < P; ++i) {
        int take = base + (i < rem ? 1 : 0);
        int start = row;
        int end   = row + take;
        row += take;

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Hijo: calcula filas [start, end)
            for (int ii = start; ii < end; ++ii) {
                for (int j = 0; j < N; ++j) {
                    double suma = 0.0;
                    for (int k = 0; k < N; ++k) {
                        suma += A[ii*N + k] * B[k*N + j];
                    }
                    C[ii*N + j] = suma;
                }
            }
            _exit(0);
        }
    }

    for (int i = 0; i < P; ++i) {
        wait(NULL);
    }
    double t1 = now_us();

    print_matrix("C", C, N);
    printf("%.0f\n", (t1 - t0));

    munmap(C, sizeof(double)*N*N);
    free(A); free(B);
    return 0;
}
