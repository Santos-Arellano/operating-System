# ProyectoV2 — Simulación de reservas con IPC (FIFOs) y hilos

Sistema distribuido simple compuesto por un `controlador` y múltiples `agentes` que interactúan mediante FIFOs (named pipes). Los agentes registran solicitudes de reserva (familia, hora solicitada, personas) y el controlador acepta, reprograma o niega según el aforo y el estado de la simulación. El tiempo avanza mediante un hilo de reloj.

## Requisitos
- Compilador C (`gcc`) con soporte C11.
- POSIX: `pthread`, `mkfifo`, `open/read/write`.
- macOS o Linux.

## Estructura
- `controlador.c`: servidor de simulación; gestiona registros, solicitudes, reloj y reporte final.
- `agente.c`: cliente que lee CSV y envía solicitudes; recibe respuestas.
- `comun.h`: contrato común (struct `Mensaje`, constantes `MSG_*`, `CODIGO_*`).
- `solicitudes1.csv`, `solicitudes2.csv`, `solicitudes3.csv`: ejemplos.
- `Makefile`: reglas de compilación.

## Compilación
```bash
cd ProyectoV2
make
```
Genera `./controlador` y `./agente` con `-std=c11 -Wall -Wextra -pthread`.

## Ejecución
1) Inicia el controlador en un terminal:
```bash
./controlador -i 7 -f 19 -s 2 -t 50 -p pipe_central
```
- `-i`: hora inicio (ej. 7)
- `-f`: hora fin (ej. 19)
- `-s`: segundos por hora simulada
- `-t`: aforo total
- `-p`: nombre del FIFO principal

2) Inicia uno o varios agentes en otros terminales:
```bash
./agente -s Agente1 -a solicitudes1.csv -p pipe_central
./agente -s Agente2 -a solicitudes2.csv -p pipe_central
./agente -s Agente3 -a solicitudes3.csv -p pipe_central
```
- `-s`: nombre del agente (genera FIFO `fifo_<Agente>`)
- `-a`: archivo CSV de solicitudes
- `-p`: FIFO principal del controlador

## Flujo de Mensajes
- Registro:
  - Agente envía `MSG_REGISTRO` con su FIFO de respuesta.
  - Controlador responde `MSG_HORA_INICIAL` con `horaActual` y mensaje de bienvenida.
- Solicitud:
  - Agente envía `MSG_SOLICITUD` por cada línea del CSV.
  - Controlador responde `MSG_RESPUESTA` con `codigo` y, si aplica, `horaAsignada` y `horaAsignada2`.
- Fin:
  - Al terminar la simulación, controlador envía `MSG_FIN` a todos los agentes.

## Lógica de Reservas
- Acepta en la hora solicitada si existen 2 horas consecutivas con cupo (`w` y `w+1`).
- Si no, reprograma a la primera franja disponible a partir de `horaActual`.
- Si no hay cupo en el día, niega.
- Valida entradas: niega si `personas <= 0` o la hora está fuera de `[horaIni, horaFin)`.

### Códigos de respuesta (`CODIGO_*`)
- `CODIGO_OK`: aceptada en la hora solicitada.
- `CODIGO_REPROGRAMADA`: reprogramada a otra hora con cupo.
- `CODIGO_EXTEMPORANEA_REPROGRAMADA`: solicitada antes de `horaActual`, reprogramada.
- `CODIGO_NEGADA_VOLVER_OTRO_DIA`: no hay cupo o parámetros inválidos.

## Formato de CSV
Cada línea: `Familia,HoraSolicitada,Personas`
Ejemplo (`solicitudes1.csv`):
```
Zuluaga,8,10
Dominguez,8,4
Rojas,10,10
```
El agente ignora una cabecera si encuentra `Familia` en la primera línea.

## Ejemplo de Sesión
Controlador:
```bash
./controlador -i 7 -f 19 -s 1 -t 20 -p pipe_central
[Controlador] Configuración:
  Hora inicio: 7
  Hora fin   : 19
  Seg/Hora   : 1
  Aforo total: 20
  FIFO       : pipe_central
```
Agente:
```bash
./agente -s Agente1 -a solicitudes1.csv -p pipe_central
[Agente Agente1] Registrado. Hora actual de simulación: 7
Mensaje del controlador: Bienvenido, la hora actual de simulación es 7
[Agente Agente1] Respuesta para familia Zuluaga:
  Codigo : 0
  Mensaje: Reserva aceptada en la hora solicitada.
  Horas  : 8 - 9
...
```

## Concurrencia y Estado
- `controlador` usa dos hilos:
  - Recepción (`read` sobre FIFO principal), protegido con `mutexDatos`.
  - Reloj (`sleep(segHoras)`), actualiza `horaActual` y reporta entradas/salidas.
- Estado compartido protegido: `ocupacion[h]`, reservas y agentes.
- Truco `fdDummy` para mantener abierto el FIFO principal y evitar `read==0`.

## Reporte Final
Al final del día, imprime horas pico/valle y totales:
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
Elimina binarios y FIFOs `fifo_*` y `pipe_*`.

## Pruebas Sugeridas
- Extemporáneas: solicitudes con `horaSolicitada < horaActual`.
- Límite de aforo: ajustar `-t` para forzar reprogramaciones/negaciones.
- Múltiples agentes: enviar cargas concurrentes y verificar consistencia.
- Borde de horario: pedir en `horaFin-1` (aceptar) y `horaFin` (negar).

## Problemas frecuentes
- `Error abriendo FIFO principal`: asegúrate de que el controlador esté ejecutándose y que el nombre de FIFO coincida.
- `Mensaje incompleto recibido`: revisa que las versiones de `comun.h` sean idénticas en controlador/agente.
- `Permisos`: si los FIFOs no se crean, valida permisos del directorio y elimina FIFOs previos con `make clean`.

## Alineación con el enunciado
- Comunicación por FIFOs con mensajes tipados (`MSG_*`).
- Gestión de aforo y ventanas de tiempo consecutivas.
- Simulación de tiempo por hilo separado.
- Reportes finales y trazabilidad por consola.