# Simulación de reservas — IPC por FIFOs y hilos (Proyecto)

Sistema compuesto por un `controlador` y múltiples `agentes` que interactúan mediante FIFOs (named pipes). Los agentes registran solicitudes de reserva (familia, hora solicitada, personas) y el controlador acepta, reprograma o niega según aforo y estado de la simulación. El tiempo avanza mediante un hilo de reloj.

**Novedades**
- Lecturas/escrituras robustas de estructuras en `agente` (manejo de `EINTR` y I/O parcial).
- Guía de ejecución de los 3 casos de prueba del enunciado.
- Ajuste de rutas: usar este directorio `Project` con `Makefile`.

## Requisitos
- `gcc` con C11 y `pthread`.
- POSIX: `mkfifo`, `open/read/write`.
- macOS o Linux.

## Estructura
- `controlador.c`: servidor de simulación; gestiona registros, solicitudes, reloj y reporte final.
- `agente.c`: cliente que lee CSV y envía solicitudes; recibe respuestas.
- `comun.h`: contrato común (`Mensaje`, `MSG_*`, `CODIGO_*`).
- `solicitudes1.csv`, `solicitudes2.csv`, `solicitudes3.csv`: ejemplos de carga.
- `Makefile`: reglas de compilación en este directorio.

## Compilación
```bash
cd Project
make
```
Genera `./controlador` y `./agente` con `-std=c11 -Wall -Wextra -pthread`.

## Ejecución básica
- Controlador (terminal A):
```bash
./controlador -i 7 -f 19 -s 2 -t 50 -p pipe_central
```
- Agentes (terminales B/C/D):
```bash
./agente -s Agente1 -a solicitudes1.csv -p pipe_central
./agente -s Agente2 -a solicitudes2.csv -p pipe_central
./agente -s Agente3 -a solicitudes3.csv -p pipe_central
```

## Casos de prueba
**Caso 1 — Ejecuciones aisladas y manejo de argumentos**
- Controlador sin argumentos: `./controlador` → imprime uso.
- Agente sin argumentos: `./agente` → imprime uso.
- Ejecución válida aislada:
```bash
./controlador -i 7 -f 10 -s 1 -t 50 -p pipe_case1
./agente -s Agente1 -a solicitudes1.csv -p pipe_case1
```

**Caso 2 — Funcionamiento normal 1 agente vs controlador**
- Ejemplo para ver aceptaciones y negativas:
```bash
./controlador -i 7 -f 12 -s 2 -t 20 -p pipe_case2
./agente -s Agente3 -a solicitudes3.csv -p pipe_case2
```
- Esperado: `FamiliaX,8,25` suele negarse por aforo; `FamiliaY,10,15` se acepta o reprograma según ocupación.

**Caso 3 — Múltiples agentes en concurrencia**
```bash
./controlador -i 7 -f 19 -s 2 -t 20 -p pipe_case3
./agente -s Agente1 -a solicitudes1.csv -p pipe_case3
./agente -s Agente2 -a solicitudes2.csv -p pipe_case3
./agente -s Agente3 -a solicitudes3.csv -p pipe_case3
```
- Observa registros, respuestas variadas (OK/reprogramada/negada), avances de hora y reporte final.

## Flujo de mensajes
- Registro: `MSG_REGISTRO` y respuesta `MSG_HORA_INICIAL` con la `horaActual`.
- Solicitud: `MSG_SOLICITUD` y `MSG_RESPUESTA` con `codigo`, `horaAsignada` y `horaAsignada2` cuando aplica.
- Fin: `MSG_FIN` emitido a todos los agentes al terminar la simulación.

## Lógica de reservas
- Acepta si hay 2 horas consecutivas con cupo.
- Reprograma desde `horaActual` a la primera franja disponible.
- Niega si no hay cupo en el día o parámetros inválidos.

### Códigos de respuesta
- `CODIGO_OK`: aceptada en la hora solicitada.
- `CODIGO_REPROGRAMADA`: movida a otra hora con cupo.
- `CODIGO_EXTEMPORANEA_REPROGRAMADA`: solicitada antes de `horaActual`, reprogramada.
- `CODIGO_NEGADA_VOLVER_OTRO_DIA`: no hay cupo o datos inválidos.

## Formato de CSV
Cada línea: `Familia,HoraSolicitada,Personas`
Ejemplo (`solicitudes1.csv`):
```
Zuluaga,8,10
Dominguez,8,4
Rojas,10,10
```
El agente ignora una cabecera si la primera línea contiene `Familia`.

## Concurrencia y estado
- Dos hilos en el controlador: recepción y reloj.
- Estado compartido protegido con `mutexDatos` (ocupación, reservas, agentes).
- Descriptor dummy en escritura para evitar `read==0` cuando no haya escritores en el FIFO principal.
- En el agente, lecturas/escrituras de `Mensaje` se realizan en bucles para evitar I/O parcial y manejar `EINTR`.

## Reporte final
Ejemplo de salida:
```
========== REPORTE FINAL ==========
Hora pico  : <h> con <X> personas
Hora valle : <h> con <Y> personas
Solicitudes aceptadas    : A
Solicitudes reprogramadas: R
Solicitudes negadas      : N
===================================
```

## Limpieza
```bash
make clean
```
Elimina binarios y FIFOs `fifo_*` y `pipe_*` en el directorio.

## Troubleshooting
- `Error abriendo FIFO principal`: verifica que el controlador está corriendo y que el nombre del FIFO coincide.
- `Mensaje incompleto recibido`: usa versiones idénticas de `comun.h` en controlador y agente; las lecturas/escrituras del agente son completas.
- `zsh: terminated`: evita relanzar comandos en el mismo terminal ocupado; usa un terminal por proceso.
- Permisos: si no se crean FIFOs, revisa permisos del directorio y ejecuta `make clean` para limpiar FIFOs previos.

## Alineación con el enunciado (PDF)
- Comunicación por FIFOs con mensajes tipados (`MSG_*`).
- Gestión de aforo y ventanas de tiempo consecutivas.
- Simulación de tiempo por hilo separado y reporte final.
- Cobertura de los tres casos de prueba (aislado, 1 agente, múltiples agentes).
