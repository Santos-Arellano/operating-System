/*
 * comun.h — Definiciones compartidas
 * -----------------------------------
 * Contiene includes estándar, constantes de protocolo y la estructura
 * de mensaje utilizada por controlador y agente.
 */
#ifndef COMUN_H
#define COMUN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>

// Tamaños máximos de cadenas
#define MAX_NAME       64
#define MAX_FAMILY     64
#define MAX_PIPE_NAME  128
#define MAX_TEXTO      256

// Rango de horas del parque (por simplicidad)
#define HORA_MIN 7
#define HORA_MAX 19
#define MAX_HORAS 24

// Tipos de mensaje
#define MSG_REGISTRO      1   // Registro del agente
#define MSG_HORA_INICIAL  2   // Respuesta con hora actual al registrarse
#define MSG_SOLICITUD     3   // Solicitud de reserva
#define MSG_RESPUESTA     4   // Respuesta a solicitud
#define MSG_FIN           5   // Fin de la simulación

// Códigos de respuesta para MSG_RESPUESTA
#define CODIGO_OK                        0  // Aceptada en hora solicitada
#define CODIGO_REPROGRAMADA              1  // Reprogramada a otra hora
#define CODIGO_EXTEMPORANEA_REPROGRAMADA 2  // Extemporánea, reprogramada
#define CODIGO_NEGADA_VOLVER_OTRO_DIA    3  // No hay cupo / inválida

// Mensaje genérico para todos los intercambios
typedef struct {
    int  tipo;                          // Uno de los MSG_*
    char nombreAgente[MAX_NAME];        // Agente que envía/recibe
    char pipeRespuesta[MAX_PIPE_NAME];  // FIFO del agente (en registro)
    char familia[MAX_FAMILY];           // Nombre de la familia
    int  horaSolicitada;                // W o la horaActual enviada
    int  personas;                      // X
    int  codigo;                        // CODIGO_* (en respuesta)
    int  horaAsignada;                  // Hora de entrada (si aplica)
    int  horaAsignada2;                 // Hora siguiente (horaAsignada+1)
    char texto[MAX_TEXTO];              // Mensaje descriptivo
} Mensaje;

#endif // COMUN_H
