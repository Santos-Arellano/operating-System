/*
 * Controlador de reservas
 * -----------------------
 * Servidor que:
 *  - Recibe el registro de agentes y mantiene un canal de respuesta a cada uno.
 *  - Procesa solicitudes de reserva, aceptando, reprogramando o negando según aforo.
 *  - Avanza el tiempo de simulación con un hilo de reloj y reporta entradas/salidas.
 *  - Emite un reporte final con estadísticas del día.
 * 
 * Comunicación vía FIFOs y concurrencia controlada con mutex.
 */
#include "comun.h"

// Información básica para responder a un agente
typedef struct {
    char nombre[MAX_NAME];
    char pipeRespuesta[MAX_PIPE_NAME];
    int  fd;       // descriptor abierto para escribir al agente
} AgenteInfo;

// Reserva confirmada en el sistema (dos horas consecutivas)
typedef struct {
    char familia[MAX_FAMILY];
    char agente[MAX_NAME];
    int  personas;
    int  horaInicio; // hora de inicio (ej: 10)
    int  horaFin;    // hora en la que ya NO está (ej: 12 si entra 10-11)
} Reserva;

// ==== Variables globales del controlador ====

// Parámetros de simulación
int horaIni = 7;
int horaFin = 19;
int segHoras = 2;
int aforoTotal = 50;
char pipePrincipal[MAX_PIPE_NAME];

// Estado del sistema
int horaActual = 7;
int ocupacion[MAX_HORAS];   // ocupación planificada por hora

#define MAX_RESERVAS 1000
Reserva reservas[MAX_RESERVAS];
int numReservas = 0;

#define MAX_AGENTES 32
AgenteInfo agentes[MAX_AGENTES];
int numAgentes = 0;

// Estadísticas simples
int solicitudesNegadas = 0;
int solicitudesAceptadas = 0;
int solicitudesReprogramadas = 0;

// Concurrencia
pthread_mutex_t mutexDatos = PTHREAD_MUTEX_INITIALIZER;
int fdPipePrincipal = -1;
int finSimulacion = 0;

// ==== Funciones auxiliares ====

// Imprime la configuración inicial del controlador
void imprimir_config() {
    printf("[Controlador] Configuración:\n");
    printf("  Hora inicio: %d\n", horaIni);
    printf("  Hora fin   : %d\n", horaFin);
    printf("  Seg/Hora   : %d\n", segHoras);
    printf("  Aforo total: %d\n", aforoTotal);
    printf("  FIFO       : %s\n", pipePrincipal);
}

// Busca agente por nombre
// Busca agente por nombre y retorna su índice o -1 si no existe
int buscar_agente(const char *nombre) {
    for (int i = 0; i < numAgentes; i++) {
        if (strcmp(agentes[i].nombre, nombre) == 0) {
            return i;
        }
    }
    return -1;
}

// Registra un agente y abre su FIFO de respuesta en escritura
int agregar_agente(const char *nombre, const char *pipeResp) {
    if (numAgentes >= MAX_AGENTES) {
        fprintf(stderr, "[Controlador] Máximo de agentes alcanzado\n");
        return -1;
    }
    strncpy(agentes[numAgentes].nombre, nombre, MAX_NAME);
    agentes[numAgentes].nombre[MAX_NAME-1] = '\0';
    strncpy(agentes[numAgentes].pipeRespuesta, pipeResp, MAX_PIPE_NAME);
    agentes[numAgentes].pipeRespuesta[MAX_PIPE_NAME-1] = '\0';

    // Abre FIFO del agente para escribir
    int fd = open(pipeResp, O_WRONLY);
    if (fd < 0) {
        perror("[Controlador] Error abriendo FIFO del agente");
        return -1;
    }
    agentes[numAgentes].fd = fd;
    numAgentes++;
    return numAgentes - 1;
}

// Envía un `Mensaje` al agente indicado por índice
void enviar_mensaje_agente(int idxAgente, const Mensaje *m) {
    if (idxAgente < 0 || idxAgente >= numAgentes) return;

    ssize_t n = write(agentes[idxAgente].fd, m, sizeof(Mensaje));
    if (n != sizeof(Mensaje)) {
        perror("[Controlador] Error escribiendo a agente");
    }
}

// Añade reserva al arreglo global
// Registra la reserva confirmada en el arreglo global
void registrar_reserva(const char *familia, const char *agente, int personas,
                       int horaInicio, int horaFin) {
    if (numReservas >= MAX_RESERVAS) {
        fprintf(stderr, "[Controlador] Máximo de reservas alcanzado\n");
        return;
    }
    Reserva *r = &reservas[numReservas++];
    strncpy(r->familia, familia, MAX_FAMILY);
    r->familia[MAX_FAMILY-1] = '\0';
    strncpy(r->agente, agente, MAX_NAME);
    r->agente[MAX_NAME-1] = '\0';
    r->personas = personas;
    r->horaInicio = horaInicio;
    r->horaFin = horaFin;
}

// Intenta ubicar 2 horas seguidas a partir de una hora de inicio dada
// Retorna 1 si se encontró, y pone en *hInicio la hora encontrada
// Busca dos horas consecutivas con cupo desde `desde` hasta `horaFin`
int buscar_horas_consecutivas(int personas, int desde, int *hInicio) {
    for (int h = desde; h + 1 < horaFin; h++) {
        if (ocupacion[h] + personas <= aforoTotal &&
            ocupacion[h + 1] + personas <= aforoTotal) {
            *hInicio = h;
            return 1;
        }
    }
    return 0;
}

// Lógica para procesar una solicitud concreta
// Procesa una solicitud: valida, decide aceptación/reprogramación/negación y responde
void procesar_solicitud(Mensaje *m) {
    int idxAgente = buscar_agente(m->nombreAgente);
    if (idxAgente < 0) {
        fprintf(stderr, "[Controlador] Solicitud de agente desconocido: %s\n",
                m->nombreAgente);
        return;
    }

    Mensaje resp;
    memset(&resp, 0, sizeof(resp));
    resp.tipo = MSG_RESPUESTA;
    strncpy(resp.familia, m->familia, MAX_FAMILY);
    strncpy(resp.nombreAgente, m->nombreAgente, MAX_NAME);

    int w = m->horaSolicitada;
    int x = m->personas;

    printf("[Controlador] Solicitud de %s: Familia=%s, Hora=%d, Personas=%d\n",
           m->nombreAgente, m->familia, w, x);

    // Si el grupo nunca cabe por aforo total
    if (x > aforoTotal) {
        // Nunca caben
        resp.codigo = CODIGO_NEGADA_VOLVER_OTRO_DIA;
        snprintf(resp.texto, MAX_TEXTO,
                 "Reserva negada: grupo muy grande para el aforo.");
        solicitudesNegadas++;
        enviar_mensaje_agente(idxAgente, &resp);
        return;
    }

    resp.familia[MAX_FAMILY-1] = '\0';
    resp.nombreAgente[MAX_NAME-1] = '\0';

    // Personas debe ser > 0
    if (x <= 0) {
        resp.codigo = CODIGO_NEGADA_VOLVER_OTRO_DIA;
        snprintf(resp.texto, MAX_TEXTO,
                 "Reserva negada: cantidad de personas inválida.");
        solicitudesNegadas++;
        enviar_mensaje_agente(idxAgente, &resp);
        return;
    }

    // Hora solicitada debe estar dentro del horario y permitir dos horas consecutivas
    if (w < horaIni || w + 1 > horaFin) {
        resp.codigo = CODIGO_NEGADA_VOLVER_OTRO_DIA;
        snprintf(resp.texto, MAX_TEXTO,
                 "Reserva negada: hora fuera del horario del parque.");
        solicitudesNegadas++;
        enviar_mensaje_agente(idxAgente, &resp);
        return;
    }

    int esExtemporanea = (w < horaActual);

    int hAsignada = -1;

    // Acepta en la hora solicitada si hay cupo en w y w+1
    if (!esExtemporanea &&
        w >= horaIni && w + 1 <= horaFin &&
        ocupacion[w] + x <= aforoTotal &&
        ocupacion[w + 1] + x <= aforoTotal) {

        // Se acepta en la misma hora solicitada
        hAsignada = w;
        resp.codigo = CODIGO_OK;
        snprintf(resp.texto, MAX_TEXTO,
                 "Reserva aceptada en la hora solicitada.");

    } else {
        // Reprograma: busca otra franja a partir de la horaActual
        int desde = horaActual;
        int ok = buscar_horas_consecutivas(x, desde, &hAsignada);
        if (!ok) {
            resp.codigo = CODIGO_NEGADA_VOLVER_OTRO_DIA;
            snprintf(resp.texto, MAX_TEXTO,
                     "No hay cupo disponible, favor reservar otro día.");
            solicitudesNegadas++;
            enviar_mensaje_agente(idxAgente, &resp);
            return;
        }

        if (esExtemporanea) {
            resp.codigo = CODIGO_EXTEMPORANEA_REPROGRAMADA;
            snprintf(resp.texto, MAX_TEXTO,
                     "Solicitud extemporánea, reprogramada a otra hora.");
        } else {
            resp.codigo = CODIGO_REPROGRAMADA;
            snprintf(resp.texto, MAX_TEXTO,
                     "Reserva reprogramada a otra hora con cupo.");
        }
        solicitudesReprogramadas++;
    }

    // Actualiza ocupación y registra la reserva
    ocupacion[hAsignada] += x;
    ocupacion[hAsignada + 1] += x;

    registrar_reserva(m->familia, m->nombreAgente, x,
                      hAsignada, hAsignada + 2);

    resp.horaAsignada  = hAsignada;
    resp.horaAsignada2 = hAsignada + 1;
    solicitudesAceptadas++;

    enviar_mensaje_agente(idxAgente, &resp);
}

// ==== Hilo de recepción de mensajes del FIFO principal ====

// Hilo que recibe mensajes del FIFO principal y los despacha
void *hilo_recepcion(void *arg) {
    (void)arg;
    Mensaje m;

    while (!finSimulacion) {
        ssize_t n = read(fdPipePrincipal, &m, sizeof(Mensaje));
        if (n == 0) {
            // No hay escritores; pequeño sleep para no quemar CPU
            usleep(100000);
            continue;
        } else if (n < 0) {
            if (errno == EINTR) continue;
            perror("[Controlador] Error al leer FIFO principal");
            break;
        }
        if (n != sizeof(Mensaje)) {
            fprintf(stderr, "[Controlador] Mensaje incompleto recibido\n");
            continue;
        }

        pthread_mutex_lock(&mutexDatos);

        if (m.tipo == MSG_REGISTRO) {
            printf("[Controlador] Registro de agente: %s, FIFO resp: %s\n",
                   m.nombreAgente, m.pipeRespuesta);
            int idx = agregar_agente(m.nombreAgente, m.pipeRespuesta);
            if (idx >= 0) {
                Mensaje resp;
                memset(&resp, 0, sizeof(resp));
                resp.tipo = MSG_HORA_INICIAL;
                resp.horaSolicitada = horaActual;  // hora actual de simulación
                snprintf(resp.texto, MAX_TEXTO,
                         "Bienvenido, la hora actual de simulación es %d",
                         horaActual);
                enviar_mensaje_agente(idx, &resp);
            }

        } else if (m.tipo == MSG_SOLICITUD) {
            procesar_solicitud(&m);
        } else {
            // Otros tipos se pueden manejar aquí
            printf("[Controlador] Mensaje desconocido tipo=%d\n", m.tipo);
        }

        pthread_mutex_unlock(&mutexDatos);
    }

    return NULL;
}

// ==== Hilo de reloj ====

// Hilo de reloj: avanza hora de simulación y reporta entradas/salidas
void *hilo_reloj(void *arg) {
    (void)arg;
    while (1) {
        sleep(segHoras);

        pthread_mutex_lock(&mutexDatos);

        if (horaActual >= horaFin) {
            pthread_mutex_unlock(&mutexDatos);
            break;
        }

        horaActual++;
        printf("\n[Controlador] >>> Avanza la hora a %d\n", horaActual);

        // Mostrar quién entra o sale en esta hora
        printf("[Controlador] Familias que entran a las %d:\n", horaActual);
        for (int i = 0; i < numReservas; i++) {
            if (reservas[i].horaInicio == horaActual) {
                printf("   - %s (%d personas) [Agente %s]\n",
                       reservas[i].familia, reservas[i].personas,
                       reservas[i].agente);
            }
        }

        printf("[Controlador] Familias que salen a las %d:\n", horaActual);
        for (int i = 0; i < numReservas; i++) {
            if (reservas[i].horaFin == horaActual) {
                printf("   - %s (%d personas) [Agente %s]\n",
                       reservas[i].familia, reservas[i].personas,
                       reservas[i].agente);
            }
        }

        pthread_mutex_unlock(&mutexDatos);
    }

    return NULL;
}

// ==== Reporte final simple ====

// Imprime estadísticas finales del día
void reporte_final() {
    printf("\n========== REPORTE FINAL ==========\n");

    // horas pico/valle usando ocupación planificada
    int pico = -1, valle = 1e9;
    int horaPico = -1, horaValle = -1;

    for (int h = horaIni; h < horaFin; h++) {
        if (ocupacion[h] > pico) {
            pico = ocupacion[h];
            horaPico = h;
        }
        if (ocupacion[h] < valle) {
            valle = ocupacion[h];
            horaValle = h;
        }
    }

    printf("Hora pico  : %d con %d personas\n", horaPico, pico);
    printf("Hora valle : %d con %d personas\n", horaValle, valle);
    printf("Solicitudes aceptadas    : %d\n", solicitudesAceptadas);
    printf("Solicitudes reprogramadas: %d\n", solicitudesReprogramadas);
    printf("Solicitudes negadas      : %d\n", solicitudesNegadas);
    printf("===================================\n");
}

// ==== Parsing de argumentos ====

// Muestra uso de la línea de comandos y termina
void uso(const char *prog) {
    fprintf(stderr,
        "Uso: %s -i horaIni -f horaFin -s segHoras -t aforo -p pipeRecibe\n",
        prog);
    exit(EXIT_FAILURE);
}

// Punto de entrada del controlador: parsea CLI, crea FIFO y lanza hilos
int main(int argc, char *argv[]) {
    int opt;
    int flagI = 0, flagF = 0, flagS = 0, flagT = 0, flagP = 0;

    while ((opt = getopt(argc, argv, "i:f:s:t:p:")) != -1) {
        switch (opt) {
            case 'i':
                horaIni = atoi(optarg);
                flagI = 1;
                break;
            case 'f':
                horaFin = atoi(optarg);
                flagF = 1;
                break;
            case 's':
                segHoras = atoi(optarg);
                flagS = 1;
                break;
            case 't':
                aforoTotal = atoi(optarg);
                flagT = 1;
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

    if (!flagI || !flagF || !flagS || !flagT || !flagP) {
        uso(argv[0]);
    }

    if (horaIni < HORA_MIN || horaFin > HORA_MAX || horaIni >= horaFin) {
        fprintf(stderr, "Horas inválidas. Deben estar entre %d y %d\n",
                HORA_MIN, HORA_MAX);
        exit(EXIT_FAILURE);
    }

    if (segHoras <= 0 || aforoTotal <= 0) {
        fprintf(stderr, "segHoras y aforo deben ser > 0\n");
        exit(EXIT_FAILURE);
    }

    horaActual = horaIni;
    memset(ocupacion, 0, sizeof(ocupacion));

    imprimir_config();

    // Crear FIFO principal
    // Crea FIFO principal (si ya existe no falla)
    if (mkfifo(pipePrincipal, 0666) < 0) {
        if (errno != EEXIST) {
            perror("[Controlador] Error creando FIFO principal");
            exit(EXIT_FAILURE);
        }
    }

    // Abre FIFO principal en lectura
    fdPipePrincipal = open(pipePrincipal, O_RDONLY);
    if (fdPipePrincipal < 0) {
        perror("[Controlador] Error abriendo FIFO principal para lectura");
        exit(EXIT_FAILURE);
    }

    // Truco para que read() no devuelva 0 cuando no haya escritores
    // Descriptor dummy en escritura para evitar EOF en read cuando no hay escritores
    int fdDummy = open(pipePrincipal, O_WRONLY);
    if (fdDummy < 0) {
        perror("[Controlador] Error abriendo FIFO principal dummy");
        // No es fatal
    }

    // Crea hilos de recepción y reloj
    pthread_t thReloj, thRecepcion;
    pthread_create(&thRecepcion, NULL, hilo_recepcion, NULL);
    pthread_create(&thReloj, NULL, hilo_reloj, NULL);

    // Esperar a que termine el reloj (fin de simulación)
    // Espera fin del reloj (fin de simulación)
    pthread_join(thReloj, NULL);

    // Marcar fin y enviar MSG_FIN a los agentes
    pthread_mutex_lock(&mutexDatos);
    finSimulacion = 1;
    for (int i = 0; i < numAgentes; i++) {
        Mensaje fin;
        memset(&fin, 0, sizeof(fin));
        fin.tipo = MSG_FIN;
        snprintf(fin.texto, MAX_TEXTO, "Fin de la simulación.");
        enviar_mensaje_agente(i, &fin);
    }
    pthread_mutex_unlock(&mutexDatos);

    // Cancelar hilo de recepción (simple para terminar)
    // Termina el hilo de recepción y genera reporte
    pthread_cancel(thRecepcion);
    pthread_join(thRecepcion, NULL);

    reporte_final();

    // Cerrar y limpiar
    close(fdPipePrincipal);
    if (fdDummy >= 0) close(fdDummy);
    for (int i = 0; i < numAgentes; i++) {
        close(agentes[i].fd);
    }
    unlink(pipePrincipal);

    return 0;
}
