/* mmFilasOpenMP.c
 * Multiplicación de matrices optimizada por localidad — transponiendo B (filas x filas) con OpenMP.
 * Uso: ./mm_omp_trans N P
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

static void transpose(double *BT, const double *B, int N) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            BT[j*N + i] = B[i*N + j];
        }
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

    double *A  = (double*) aligned_alloc(64, sizeof(double)*N*N);
    double *B  = (double*) aligned_alloc(64, sizeof(double)*N*N);
    double *BT = (double*) aligned_alloc(64, sizeof(double)*N*N);
    double *C  = (double*) aligned_alloc(64, sizeof(double)*N*N);
    if (!A || !B || !BT || !C) {
        fprintf(stderr, "Fallo al reservar memoria\n");
        return 1;
    }

    srand(21);
    init_matrices(A, B, N);

    double t0 = now_us();

    // BT = B^T para acceder en contiguo
    transpose(BT, B, N);

    // C = A x B  ==  filas(A) · filas(BT)
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double suma = 0.0;
            const double *Ai  = &A[i*N];
            const double *BTj = &BT[j*N];
            for (int k = 0; k < N; ++k) {
                suma += Ai[k] * BTj[k];
            }
            C[i*N + j] = suma;
        }
    }

    double t1 = now_us();
    print_matrix("C", C, N);
    printf("%.0f\n", (t1 - t0));

    free(A); free(B); free(BT); free(C);
    return 0;
}
