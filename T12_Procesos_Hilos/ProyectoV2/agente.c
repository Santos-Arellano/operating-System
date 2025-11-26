#include "comun.h"

// Pequeña función para quitar salto de línea
void chomp(char *s) {
    size_t len = strlen(s);
    if (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
    }
}

// Imprime una respuesta de reserva para el usuario
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

void uso(const char *prog) {
    fprintf(stderr,
        "Uso: %s -s nombreAgente -a archivoSolicitudes -p pipePrincipal\n",
        prog);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    char nombreAgente[MAX_NAME] = "";
    char archivoSolicitudes[256] = "";
    char pipePrincipal[MAX_PIPE_NAME] = "";

    int opt;
    int flagS = 0, flagA = 0, flagP = 0;

    while ((opt = getopt(argc, argv, "s:a:p:")) != -1) {
        switch (opt) {
            case 's':
                strncpy(nombreAgente, optarg, MAX_NAME);
                flagS = 1;
                break;
            case 'a':
                strncpy(archivoSolicitudes, optarg, sizeof(archivoSolicitudes));
                flagA = 1;
                break;
            case 'p':
                strncpy(pipePrincipal, optarg, MAX_PIPE_NAME);
                flagP = 1;
                break;
            default:
                uso(argv[0]);
        }
    }

    if (!flagS || !flagA || !flagP) {
        uso(argv[0]);
    }

    // Construir nombre del FIFO de respuesta del agente
    char pipeRespuesta[MAX_PIPE_NAME];
    snprintf(pipeRespuesta, sizeof(pipeRespuesta), "fifo_%s", nombreAgente);

    // Crear FIFO de respuesta
    if (mkfifo(pipeRespuesta, 0666) < 0) {
        if (errno != EEXIST) {
            perror("[Agente] Error creando FIFO de respuesta");
            exit(EXIT_FAILURE);
        }
    }

    // Abrir FIFO principal para escribir (hacia el controlador)
    int fdPrincipal = open(pipePrincipal, O_WRONLY);
    if (fdPrincipal < 0) {
        perror("[Agente] Error abriendo FIFO principal para escritura");
        exit(EXIT_FAILURE);
    }

    // Enviar mensaje de registro
    Mensaje reg;
    memset(&reg, 0, sizeof(reg));
    reg.tipo = MSG_REGISTRO;
    strncpy(reg.nombreAgente, nombreAgente, MAX_NAME);
    strncpy(reg.pipeRespuesta, pipeRespuesta, MAX_PIPE_NAME);

    if (write(fdPrincipal, &reg, sizeof(reg)) != sizeof(reg)) {
        perror("[Agente] Error enviando mensaje de registro");
        exit(EXIT_FAILURE);
    }

    // Abrir FIFO de respuesta en lectura (bloquea hasta que el servidor abra escritura)
    int fdResp = open(pipeRespuesta, O_RDONLY);
    if (fdResp < 0) {
        perror("[Agente] Error abriendo FIFO de respuesta");
        exit(EXIT_FAILURE);
    }

    // Recibir la hora inicial de simulación
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

    // Abrir archivo de solicitudes
    FILE *f = fopen(archivoSolicitudes, "r");
    if (!f) {
        perror("[Agente] No se pudo abrir archivo de solicitudes");
        exit(EXIT_FAILURE);
    }

    char linea[256];
    int esPrimeraLinea = 1;

    while (fgets(linea, sizeof(linea), f)) {
        chomp(linea);
        if (strlen(linea) == 0) continue;

        // Saltar la cabecera si existe (Familia,HoraSolicitada,Personas)
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

        token = strtok(linea, ",");
        if (!token) continue;
        strncpy(familia, token, MAX_FAMILY);

        token = strtok(NULL, ",");
        if (!token) continue;
        horaSolicitada = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        personas = atoi(token);

        if (horaSolicitada < horaSimulacion) {
            printf("[Agente %s] Solicitud extemporánea en archivo "
                   "(hora %d < hora simulación %d). Enviando igual.\n",
                   nombreAgente, horaSolicitada, horaSimulacion);
        }

        Mensaje sol;
        memset(&sol, 0, sizeof(sol));
        sol.tipo = MSG_SOLICITUD;
        strncpy(sol.nombreAgente, nombreAgente, MAX_NAME);
        strncpy(sol.familia, familia, MAX_FAMILY);
        sol.horaSolicitada = horaSolicitada;
        sol.personas = personas;

        if (write(fdPrincipal, &sol, sizeof(sol)) != sizeof(sol)) {
            perror("[Agente] Error enviando solicitud");
            break;
        }

        // Esperar respuesta
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

    fclose(f);

    // Esperar posible mensaje de fin si no llegó todavía (opcional, simple)
    // Aquí podrías hacer otro read no bloqueante, pero para no complicar lo
    // dejamos así y terminamos.

    printf("[Agente %s] Termina.\n", nombreAgente);

    close(fdResp);
    close(fdPrincipal);
    unlink(pipeRespuesta);

    return 0;
}
