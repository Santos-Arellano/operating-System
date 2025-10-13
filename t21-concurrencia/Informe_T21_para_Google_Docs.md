# T21 – Mutex – Semáforos – Pipes – Señales
**Curso:** Sistemas Operativos  
**Integrantes:** [Nombres y códigos]  
**Fecha:** [dd/mmm/aaaa]

---

## 1. Objetivo
Implementar y comparar tres arquitecturas de concurrencia para un sistema de sensores climáticos:  
1) multihilo con exclusión mutua; 2) procesos + FIFOs; 3) procesos + FIFOs + señales (SIGUSR1).

## 2. Entorno de trabajo
- **SO:** Linux (nativo/VM).
- **Compilador:** `gcc`, `make`.
- **Librerías:** `pthread`, `signal.h`, `poll`.
- **Repositorio local:** Estructura provista en este entregable.

> **Captura 1:** Comprobación de versión de `gcc` y arquitectura.  
> **PEGA AQUÍ**

## 3. Diseño de la solución
### 3.1. Parte A — Multihilo (mutex)
- 4 hilos: temperatura, humedad, viento, precipitación.
- Periodos *indeterminados*: p. ej., viento 5±2 s; humedad 12±2 s; etc.
- Sección crítica con `pthread_mutex_t` para **reescribir atómicamente** `clima_actual.txt` con los valores más recientes.

> **Captura 2:** Ejecución de `./climate_threads` mostrando logs.  
> **PEGA AQUÍ**

### 3.2. Parte B — Procesos + FIFOs
- 4 procesos de sensor escriben en **FIFOs** independientes en `/tmp`.
- Un proceso **central** multiplexer con `poll()` integra entradas y actualiza `clima_actual.txt`.
- Formato de línea: `SENSOR;VALOR;TIMESTAMP`.

> **Captura 3:** `ls -l /tmp/fifo_*` y ejecución con `./run.sh`.  
> **PEGA AQUÍ**

### 3.3. Parte C — Procesos + FIFOs + Señales
- Sensores **registran su PID** en `/tmp/sensor_*.pid` y **esperan SIGUSR1**.
- El **central** envía `SIGUSR1` **cada 5 s** y espera respuestas (con timeout) por los FIFOs.
- Manejo de terminación con `SIGINT`/`SIGTERM`.

> **Captura 4:** Envío de señales (logs del central) y recepción (logs de sensores).  
> **PEGA AQUÍ**

## 4. Compilación y ejecución (paso a paso)
```bash
# Parte A
cd ParteA_mutex_semaforos && make && ./climate_threads

# Parte B
cd ParteB_pipes && make && ./run.sh

# Parte C
cd ParteC_pipes_senales && make && ./run.sh
```

> **Captura 5:** `tail -f clima_actual.txt` mostrando actualizaciones.  
> **PEGA AQUÍ**

## 5. Pruebas y resultados
- Verifica que **todas** las variables se actualicen dentro de ventanas esperadas.
- **Parte B:** desconecta/reconecta un sensor y observa resiliencia del central.
- **Parte C:** interrumpe temporalmente un sensor y evalúa si el central detecta “respuesta ausente” en el período.

> **Captura 6:** Prueba de resiliencia en Parte B.  
> **PEGA AQUÍ**

> **Captura 7:** Prueba de petición-periódica con SIGUSR1 en Parte C.  
> **PEGA AQUÍ**

## 6. Análisis comparativo
| Criterio | Parte A (mutex) | Parte B (FIFOs) | Parte C (FIFOs + señales) |
|---|---|---|---|
| Facilidad de implementar | Alta | Media | Media |
| Overhead de IPC | Bajo (memoria compartida implícita) | Medio | Medio |
| Acoplamiento temporal | Bajo | Bajo | **Alto** (petición-respuesta) |
| Control del central | Bajo | Medio | **Alto** |
| Robustez ante fallos de un sensor | Media | Alta (central no cae) | Alta |
| Complejidad de despliegue | Baja | Media | Media/Alta |

## 7. Conclusiones
- La versión con **mutex** es la más simple para un único binario y bajo overhead.
- La versión con **FIFOs** separa responsabilidades y favorece **aislamiento** y **observabilidad** por proceso.
- La versión con **señales** añade **control explícito** del central (pull), útil cuando las lecturas deben estar **sincronizadas**.

## 8. Referencias
- Enunciado del taller (PDF adjunto).
- Manuales de Linux: `man 7 signal`, `man 2 poll`, `man 3 pthread_mutex_lock`.

## 9. Anexos
- Fragmentos de código clave.
- Comandos de compilación y ejecución (ya incluidos).
```
[Espacio reservado para pegar más capturas y tablas de resultados]
```
