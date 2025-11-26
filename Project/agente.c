/*
 * Agente de reservas
 * ------------------
 * Cliente que:
 *  - Se registra ante el controlador enviando su FIFO de respuesta.
 *  - Lee solicitudes desde un archivo CSV y las envía al controlador.
 *  - Recibe respuestas y las imprime para el usuario.
 * 
 * Comunicación vía FIFOs (named pipes) y mensajes definidos en comun.h.
 */
#include "comun.h"

// Quita el salto de línea final (\n o \r) de una cadena leída con fgets
void chomp(char *s) {
    size_t len = strlen(s);
    if (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
    }
}

// Imprime de forma amigable una respuesta de reserva
void imprimir_respuesta(const Mensaje *m) {
    printf("[Agente %s] Respuesta para familia %s:\n",
           m->nombreAgente, m->familia);
    printf("  Codigo : %d\n", m->codigo);
    printf("  Mensaje: %s\n", m->texto);
    if (m->codigo == CODIGO_OK ||
        m->codigo == CODIGO_REPROGRAMADA ||
        m->codigo == CODIGO_EXTEMPORANEA_REPROGRAMADA) {
        printf("  Horas  : %d - %d\n", m->horaAsignada, m->horaAsignada2);
    }
    printf("--------------------------------------\n");
}

// Muestra uso de la línea de comandos y termina
void uso(const char *prog) {
    fprintf(stderr,
        "Uso: %s -s nombreAgente -a archivoSolicitudes -p pipePrincipal\n",
        prog);
    exit(EXIT_FAILURE);
}

// Punto de entrada del agente
int main(int argc, char *argv[]) {
    char nombreAgente[MAX_NAME] = "";
    char archivoSolicitudes[256] = "";
    char pipePrincipal[MAX_PIPE_NAME] = "";

    int opt;
    int flagS = 0, flagA = 0, flagP = 0;

    // Parseo de argumentos: nombre del agente, CSV y FIFO principal
    while ((opt = getopt(argc, argv, "s:a:p:")) != -1) {
        switch (opt) {
            case 's':
                strncpy(nombreAgente, optarg, MAX_NAME);
                nombreAgente[MAX_NAME-1] = '\0';
                flagS = 1;
                break;
            case 'a':
                strncpy(archivoSolicitudes, optarg, sizeof(archivoSolicitudes));
                archivoSolicitudes[sizeof(archivoSolicitudes)-1] = '\0';
                flagA = 1;
                break;
            case 'p':
                strncpy(pipePrincipal, optarg, MAX_PIPE_NAME);
                pipePrincipal[MAX_PIPE_NAME-1] = '\0';
                flagP = 1;
                break;
            default:
                uso(argv[0]);
        }
    }

    if (!flagS || !flagA || !flagP) {
        uso(argv[0]);
    }

    // Construye nombre del FIFO de respuesta del agente (p.ej. fifo_Agente1)
    char pipeRespuesta[MAX_PIPE_NAME];
    snprintf(pipeRespuesta, sizeof(pipeRespuesta), "fifo_%s", nombreAgente);

    // Crea FIFO de respuesta; si ya existe, lo reutiliza
    if (mkfifo(pipeRespuesta, 0666) < 0) {
        if (errno != EEXIST) {
            perror("[Agente] Error creando FIFO de respuesta");
            exit(EXIT_FAILURE);
        }
    }

    // Abre FIFO principal para escribir (hacia el controlador)
    int fdPrincipal = open(pipePrincipal, O_WRONLY);
    if (fdPrincipal < 0) {
        perror("[Agente] Error abriendo FIFO principal para escritura");
        exit(EXIT_FAILURE);
    }

    // Envía mensaje de registro para que el controlador conozca este agente
    Mensaje reg;
    memset(&reg, 0, sizeof(reg));
    reg.tipo = MSG_REGISTRO;
    strncpy(reg.nombreAgente, nombreAgente, MAX_NAME);
    reg.nombreAgente[MAX_NAME-1] = '\0';
    strncpy(reg.pipeRespuesta, pipeRespuesta, MAX_PIPE_NAME);
    reg.pipeRespuesta[MAX_PIPE_NAME-1] = '\0';

    if (write(fdPrincipal, &reg, sizeof(reg)) != sizeof(reg)) {
        perror("[Agente] Error enviando mensaje de registro");
        exit(EXIT_FAILURE);
    }

    // Abre FIFO de respuesta en lectura (bloquea hasta que el controlador abra escritura)
    int fdResp = open(pipeRespuesta, O_RDONLY);
    if (fdResp < 0) {
        perror("[Agente] Error abriendo FIFO de respuesta");
        exit(EXIT_FAILURE);
    }

    // Recibe la hora inicial de simulación y mensaje de bienvenida
    Mensaje m;
    ssize_t n = read(fdResp, &m, sizeof(m));
    if (n != sizeof(m) || m.tipo != MSG_HORA_INICIAL) {
        fprintf(stderr, "[Agente] Error recibiendo hora inicial\n");
        exit(EXIT_FAILURE);
    }

    int horaSimulacion = m.horaSolicitada;
    printf("[Agente %s] Registrado. Hora actual de simulación: %d\n",
           nombreAgente, horaSimulacion);
    printf("Mensaje del controlador: %s\n", m.texto);

    // Abre CSV de solicitudes (Familia,HoraSolicitada,Personas)
    FILE *f = fopen(archivoSolicitudes, "r");
    if (!f) {
        perror("[Agente] No se pudo abrir archivo de solicitudes");
        exit(EXIT_FAILURE);
    }

    char linea[256];
    int esPrimeraLinea = 1;

    // Procesa cada línea del CSV y envía una solicitud al controlador
    while (fgets(linea, sizeof(linea), f)) {
        chomp(linea);
        if (strlen(linea) == 0) continue;

        // Salta cabecera si existe (contiene "Familia")
        if (esPrimeraLinea) {
            esPrimeraLinea = 0;
            if (strstr(linea, "Familia") != NULL) {
                continue;
            }
        }

        char *token;
        char familia[MAX_FAMILY];
        int horaSolicitada;
        int personas;

        // Familia
        token = strtok(linea, ",");
        if (!token) continue;
        strncpy(familia, token, MAX_FAMILY);
        familia[MAX_FAMILY-1] = '\0';

        // Hora solicitada (entero)
        token = strtok(NULL, ",");
        if (!token) continue;
        horaSolicitada = atoi(token);

        // Personas (entero > 0)
        token = strtok(NULL, ",");
        if (!token) continue;
        personas = atoi(token);
        if (personas <= 0) {
            printf("[Agente %s] Solicitud inválida: personas <= 0, se omite.\n", nombreAgente);
            continue;
        }

        if (horaSolicitada < horaSimulacion) {
            printf("[Agente %s] Solicitud extemporánea en archivo "
                   "(hora %d < hora simulación %d). Enviando igual.\n",
                   nombreAgente, horaSolicitada, horaSimulacion);
        }

        // Construye y envía el mensaje de solicitud
        Mensaje sol;
        memset(&sol, 0, sizeof(sol));
        sol.tipo = MSG_SOLICITUD;
        strncpy(sol.nombreAgente, nombreAgente, MAX_NAME);
        sol.nombreAgente[MAX_NAME-1] = '\0';
        strncpy(sol.familia, familia, MAX_FAMILY);
        sol.familia[MAX_FAMILY-1] = '\0';
        sol.horaSolicitada = horaSolicitada;
        sol.personas = personas;

        if (write(fdPrincipal, &sol, sizeof(sol)) != sizeof(sol)) {
            perror("[Agente] Error enviando solicitud");
            break;
        }

        // Espera respuesta del controlador y la imprime
        Mensaje resp;
        n = read(fdResp, &resp, sizeof(resp));
        if (n != sizeof(resp)) {
            perror("[Agente] Error leyendo respuesta");
            break;
        }

        if (resp.tipo == MSG_RESPUESTA) {
            imprimir_respuesta(&resp);
        } else if (resp.tipo == MSG_FIN) {
            printf("[Agente %s] Recibido fin de simulación.\n", nombreAgente);
            break;
        } else {
            printf("[Agente %s] Mensaje inesperado tipo=%d\n",
                   nombreAgente, resp.tipo);
        }

        // Esperar 2 segundos antes de enviar la siguiente
        sleep(2);
    }

    // Cierra archivo y termina protocolo
    fclose(f);

    // Esperar posible mensaje de fin si no llegó todavía (opcional, simple)
    // Aquí podrías hacer otro read no bloqueante, pero para no complicar lo
    // dejamos así y terminamos.

    // Mensaje final de salida
    printf("[Agente %s] Termina.\n", nombreAgente);

    close(fdResp);
    close(fdPrincipal);
    // Cierra y elimina FIFO de respuesta de este agente
    unlink(pipeRespuesta);

    return 0;
}
