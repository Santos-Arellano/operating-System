/* mmClasicaOpenMP.c
 * Multiplicación de matrices (A x B = C) — versión clásica con OpenMP.
 * Uso: ./mm_omp N P
 *  - N: tamaño de la matriz (NxN)
 *  - P: número de hilos (OpenMP)
 * Salida: imprime (opcionalmente) C si N<=4 y SIEMPRE la última línea con el tiempo total en microsegundos.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

static inline double now_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1e6 + (double)tv.tv_usec;
}

static void init_matrices(double *A, double *B, int N) {
    for (int i = 0; i < N*N; ++i) {
        A[i] = (double)rand() / RAND_MAX * 4.0 + 1.0; // [1,5)
        B[i] = (double)rand() / RAND_MAX * 7.0 + 2.0; // [2,9)
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
    omp_set_num_threads(P);

    double *A = (double*) aligned_alloc(64, sizeof(double)*N*N);
    double *B = (double*) aligned_alloc(64, sizeof(double)*N*N);
    double *C = (double*) aligned_alloc(64, sizeof(double)*N*N);
    if (!A || !B || !C) {
        fprintf(stderr, "Fallo al reservar memoria\n");
        return 1;
    }

    srand(21);
    init_matrices(A, B, N);

    double t0 = now_us();

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double suma = 0.0;
            for (int k = 0; k < N; ++k) {
                suma += A[i*N + k] * B[k*N + j];
            }
            C[i*N + j] = suma;
        }
    }

    double t1 = now_us();

    print_matrix("C", C, N);
    printf("%.0f\n", (t1 - t0));

    free(A); free(B); free(C);
    return 0;
}
