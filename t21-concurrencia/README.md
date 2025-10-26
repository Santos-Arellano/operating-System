# Taller T21 – Concurrencia (Mutex, Semáforos, Pipes, Señales)

## 👥 Autores

- Grupo: 7
- Integrantes:
  - Santos Alejandro Arellano Olarte
  - Jeison Camilo Alfonso Moreno
  - Jose Villaroel

  
Este repositorio contiene **tres implementaciones** del sistema de sensores climáticos solicitado:

1. **Parte A — Programa multihilo con exclusión mutua (mutex):**  
   Un binario que lanza 4 hilos (temperatura, humedad, viento, precipitación), cada uno con su periodo de muestreo. Todos **escriben de forma segura** en el mismo archivo `clima_actual.txt` mediante sección crítica protegida con `pthread_mutex_t`.

2. **Parte B — Múltiples programas + FIFOs (pipes con nombre):**  
   Cuatro ejecutables (uno por sensor) **escriben por su FIFO** hacia un proceso **central** que integra y actualiza `clima_actual.txt`. Los sensores generan datos **a intervalos propios (indeterminados)**.

3. **Parte C — Múltiples programas + FIFOs + Señales (SIGUSR1):**  
   Cuatro sensores inician, **registran su PID**, y **esperan SIGUSR1**. El proceso **central** envía `SIGUSR1` a todos **cada 5 s**, y los sensores responden escribiendo su dato en el FIFO correspondiente. El central integra y actualiza `clima_actual.txt`.

> Probado en Linux con `gcc`, `make` y `pthread`.

---
 
## Requisitos

- Linux (nativo o VM).
- `gcc` y `make`.
- Permisos para crear FIFOs en `/tmp`.

---

## Estructura

```
t21-concurrencia/
├─ README.md
├─ Informe_T21_para_Google_Docs.md
├─ ParteA_mutex_semaforos/
│  ├─ Makefile
│  └─ climate_threads.c
├─ ParteB_pipes/
│  ├─ Makefile
│  ├─ central.c
│  ├─ sensor_temp.c
│  ├─ sensor_humedad.c
│  ├─ sensor_viento.c
│  ├─ sensor_lluvia.c
│  └─ run.sh
└─ ParteC_pipes_senales/
   ├─ Makefile
   ├─ central_signal.c
   ├─ sensor_temp_sig.c
   ├─ sensor_humedad_sig.c
   ├─ sensor_viento_sig.c
   ├─ sensor_lluvia_sig.c
   └─ run.sh
```

---

## Compilación

Desde la carpeta raíz:

```bash
# Parte A
cd ParteA_mutex_semaforos && make && cd ..

# Parte B
cd ParteB_pipes && make && cd ..

# Parte C
cd ParteC_pipes_senales && make && cd ..
```

---

## Ejecución

### Parte A (multihilo con mutex)

```bash
cd ParteA_mutex_semaforos
./climate_threads
# Detener con Ctrl+C
```

- Archivo de salida: `clima_actual.txt` (en el directorio de ejecución).

---

### Parte B (procesos + FIFOs)

```bash
cd ParteB_pipes
./run.sh
# run.sh crea los FIFOs y lanza el central y los 4 sensores con sus periodos.
# Detener con: Ctrl+C en la ventana donde corre 'central' y luego:
pkill -f sensor_
```

- FIFOs en `/tmp/fifo_{temp,humedad,viento,lluvia}`.
- Archivo de salida: `clima_actual.txt` (en el directorio de ejecución de `central`).

---

### Parte C (procesos + FIFOs + señales)

```bash
cd ParteC_pipes_senales
./run.sh
# run.sh crea los FIFOs, inicia sensores (que escriben su PID) y luego el central.
# El central envía SIGUSR1 a todos cada 5 s y espera sus lecturas.
# Detener con Ctrl+C en el central y luego:
pkill -f _sig
```

- PIDs guardados en `/tmp/sensor_*.pid`.
- Archivo de salida: `clima_actual.txt` (en el directorio de ejecución de `central_signal`).

---

## Notas y consejos

- Si un FIFO ya existe, `mkfifo` lo dejará; no es error.
- Para “ver” el archivo de clima en vivo:
  ```bash
  tail -f clima_actual.txt
  ```
- Si un sensor inicia antes que el central, puede bloquear al abrir el FIFO. Usa los `run.sh` que arrancan en el orden correcto.
- Sanitiza procesos colgados:
  ```bash
  pkill -f sensor_
  pkill -f central_signal
  pkill -f central
  ```

---

## Limpieza

```bash
# Limpiar binarios
cd ParteA_mutex_semaforos && make clean && cd ..
cd ParteB_pipes && make clean && cd ..
cd ParteC_pipes_senales && make clean && cd ..

# Eliminar FIFOs
rm -f /tmp/fifo_temp /tmp/fifo_humedad /tmp/fifo_viento /tmp/FIFO_LLUVIA
rm -f /tmp/sensor_*.pid
```

---


## Licencia

MIT — para fines académicos.
