#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Validación de argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso correcto: %s <tamaño_matriz> <numero_procesos>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s 8 4\n", argv[0]);
        return 1;
    }
    
    // Conversión y validación de argumentos
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    
    if (m <= 0 || n <= 0) {
        fprintf(stderr, "Error: Los argumentos deben ser números positivos\n");
        return 1;
    }
    
    // Validar que n sea divisor de m
    if (m % n != 0) {
        fprintf(stderr, "Error: n (%d) debe ser divisor de m (%d)\n", n, m);
        fprintf(stderr, "Uso correcto: El número de procesos debe dividir exactamente el tamaño de la matriz\n");
        return 1;
    }
    
    // Declarar e inicializar la matriz con valores de ejemplo
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
        // Inicializar con valores de ejemplo
        for (int j = 0; j < m; j++) {
            matriz[i][j] = i + j + 1;
        }
    }
    
    // Matriz inicializada (%dx%d) - sin mostrar para optimizar rendimiento
    printf("Matriz inicializada (%dx%d) - Procesando con %d procesos...\n", m, m, n);
    
    // Calcular filas por proceso
    int filas_por_proceso = m / n;
    
    // Crear n procesos hijos
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            fprintf(stderr, "Error al crear el proceso hijo %d\n", i);
            return 1;
        }
        else if (pid == 0) {
            // Código del proceso hijo
            int inicio_fila = i * filas_por_proceso;
            int fin_fila = inicio_fila + filas_por_proceso;
            
            // Procesar las filas asignadas sin mostrar detalles
            for (int fila = inicio_fila; fila < fin_fila; fila++) {
                int suma = 0;
                // Calcular suma sin mostrar elementos
                for (int col = 0; col < m; col++) {
                    suma += matriz[fila][col];
                }
            }
            
            // Liberar memoria en el hijo antes de terminar
            for (int j = 0; j < m; j++) {
                free(matriz[j]);
            }
            free(matriz);
            
            exit(0); // Terminar el proceso hijo
        }
    }
    
    // Proceso padre espera a todos los hijos
    for (int i = 0; i < n; i++) {
        int status;
        pid_t pid_hijo = wait(&status);
        if (pid_hijo < 0) {
            fprintf(stderr, "Error esperando al proceso hijo\n");
        }
    }
    
    printf("\nProceso padre (PID=%d): Todos los hijos han terminado\n", getpid());
    
    // Liberar memoria en el padre
    for (int i = 0; i < m; i++) {
        free(matriz[i]);
    }
    free(matriz);
    
    return 0;
}