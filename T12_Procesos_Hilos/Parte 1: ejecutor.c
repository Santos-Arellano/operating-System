#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Validación de argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <tamaño_matriz> <numero_procesos>\n", argv[0]);
        fprintf(stderr, "Este programa ejecutará matrizsum con los argumentos proporcionados\n");
        fprintf(stderr, "Ejemplo: %s 8 4\n", argv[0]);
        return 1;
    }
    
    printf("Ejecutando matrizsum con argumentos: m=%s, n=%s\n\n", argv[1], argv[2]);
    
    // Crear un proceso hijo para ejecutar matrizsum
    pid_t pid = fork();
    
    if (pid < 0) {
        fprintf(stderr, "Error al crear el proceso\n");
        return 1;
    }
    else if (pid == 0) {
        // Proceso hijo: ejecutar matrizsum usando execl
        execl("./matrizsum", "matrizsum", argv[1], argv[2], NULL);
        
        // Si llegamos aquí, hubo un error en exec
        perror("Error ejecutando matrizsum");
        fprintf(stderr, "Asegúrate de que matrizsum esté compilado y en el directorio actual\n");
        exit(1);
    }
    else {
        // Proceso padre: esperar a que termine el hijo
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("\nEl programa matrizsum terminó con código de salida: %d\n", 
                   WEXITSTATUS(status));
        }
        else {
            printf("\nEl programa matrizsum terminó de forma anormal\n");
        }
    }
    
    return 0;
}