#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

// Estructura para pasar datos a los hilos
typedef struct {
    int **matriz;
    int m;                  // Tamaño de la matriz
    int inicio_fila;        // Primera fila a procesar
    int fin_fila;          // Última fila a procesar (exclusiva)
    char operacion[10];    // "sumar" o "max"
    int thread_id;         // ID del hilo
} ThreadData;

// Función que ejecutará cada hilo
void* procesar_filas(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    printf("Hilo %d procesando filas %d a %d\n", 
           data->thread_id, data->inicio_fila, data->fin_fila - 1);
    
    for (int i = data->inicio_fila; i < data->fin_fila; i++) {
        printf("Fila %d: ", i);
        
        // Mostrar los valores de la fila
        for (int j = 0; j < data->m; j++) {
            printf("%3d ", data->matriz[i][j]);
        }
        
        // Realizar la operación solicitada
        if (strcmp(data->operacion, "sumar") == 0) {
            int suma = 0;
            for (int j = 0; j < data->m; j++) {
                suma += data->matriz[i][j];
            }
            printf(" -> Suma: %d\n", suma);
        }
        else if (strcmp(data->operacion, "max") == 0) {
            int max = data->matriz[i][0];
            for (int j = 1; j < data->m; j++) {
                if (data->matriz[i][j] > max) {
                    max = data->matriz[i][j];
                }
            }
            printf(" -> Máximo: %d\n", max);
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    // CONFIGURACIÓN PARA GRUPO 7
    int p = 9;  // Grupo 7: p = 7 + 2 = 9
    
    // Validación de argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <tamaño_matriz> <operacion>\n", argv[0]);
        fprintf(stderr, "Operaciones válidas: sumar, max\n");
        fprintf(stderr, "Ejemplo: %s 10 sumar\n", argv[0]);
        return 1;
    }
    
    // Obtener y validar el tamaño de la matriz
    int m = atoi(argv[1]);
    if (m <= 0) {
        fprintf(stderr, "Error: El tamaño de la matriz debe ser un número positivo\n");
        return 1;
    }
    
    // Validar la operación
    char* operacion = argv[2];
    if (strcmp(operacion, "sumar") != 0 && strcmp(operacion, "max") != 0) {
        fprintf(stderr, "Error: Operación inválida '%s'\n", operacion);
        fprintf(stderr, "Operaciones válidas: sumar, max\n");
        return 1;
    }
    
    // Semilla para números aleatorios
    srand(time(NULL));
    
    // Crear la matriz dinámica
    int **matriz = (int **)malloc(m * sizeof(int *));
    if (matriz == NULL) {
        fprintf(stderr, "Error al asignar memoria para la matriz\n");
        return 1;
    }
    
    for (int i = 0; i < m; i++) {
        matriz[i] = (int *)malloc(m * sizeof(int));
        if (matriz[i] == NULL) {
            fprintf(stderr, "Error al asignar memoria para la fila %d\n", i);
            return 1;
        }
        // Llenar con valores aleatorios (0-9)
        for (int j = 0; j < m; j++) {
            matriz[i][j] = rand() % 10;
        }
    }
    
    // Mostrar la matriz original
    printf("Matriz original (%dx%d):\n", m, m);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            printf("%3d ", matriz[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    
    printf("Procesando con %d hilos, operación: %s\n", p, operacion);
    printf("----------------------------------------\n");
    
    // Crear arreglos para hilos y datos
    pthread_t threads[p];
    ThreadData thread_data[p];
    
    // Calcular distribución equitativa de filas
    int filas_base = m / p;
    int filas_extra = m % p;
    int fila_actual = 0;
    
    // Crear y lanzar los hilos
    for (int i = 0; i < p; i++) {
        thread_data[i].matriz = matriz;
        thread_data[i].m = m;
        thread_data[i].inicio_fila = fila_actual;
        
        // Distribuir las filas extra entre los primeros hilos
        int filas_este_hilo = filas_base;
        if (i < filas_extra) {
            filas_este_hilo++;
        }
        
        thread_data[i].fin_fila = fila_actual + filas_este_hilo;
        strcpy(thread_data[i].operacion, operacion);
        thread_data[i].thread_id = i;
        
        // Crear el hilo
        if (pthread_create(&threads[i], NULL, procesar_filas, &thread_data[i]) != 0) {
            fprintf(stderr, "Error al crear el hilo %d\n", i);
            return 1;
        }
        
        fila_actual = thread_data[i].fin_fila;
    }
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < p; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error al esperar el hilo %d\n", i);
        }
    }
    
    printf("----------------------------------------\n");
    printf("Procesamiento completado con %d hilos\n", p);
    
    // Liberar memoria
    for (int i = 0; i < m; i++) {
        free(matriz[i]);
    }
    free(matriz);
    
    return 0;
}