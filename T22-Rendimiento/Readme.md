# T22 — Evaluación de Rendimiento (README Completo)

> **Objetivo:** comparar el rendimiento de 4 variantes de multiplicación de matrices cuadradas `N×N` (fork, OpenMP clásica, OpenMP con transpuesta, Pthreads) midiendo **tiempo en microsegundos**, y derivando **speedup** y **eficiencia** bajo distintos tamaños `N` y niveles de paralelismo `P`.

---

## Índice
1. [Requisitos](#requisitos)
2. [Estructura del proyecto](#estructura-del-proyecto)
3. [Compilación](#compilación)
4. [Uso de cada ejecutable](#uso-de-cada-ejecutable)
5. [Prueba rápida (sanity check)](#prueba-rápida-sanity-check)
6. [Batería de experimentos automatizada](#batería-de-experimentos-automatizada)
7. [Formato de los `.dat`](#formato-de-los-dat)
8. [Procesamiento: CSV + gráficas](#procesamiento-csv--gráficas)
9. [Diseño experimental recomendado](#diseño-experimental-recomendado)
10. [Interpretación de resultados](#interpretación-de-resultados)
11. [Buenas prácticas y reproducibilidad](#buenas-prácticas-y-reproducibilidad)
12. [Solución de problemas (FAQ)](#solución-de-problemas-faq)
13. [Qué entregar](#qué-entregar)
14. [Checklist de entrega](#checklist-de-entrega)
15. [Créditos](#créditos)

---

## Requisitos

- **SO:** Linux (nativo o WSL/VM).  
- **Compilador:** `gcc` con soporte OpenMP y Pthreads.  
  - Verificar: `gcc --version` y `echo | cpp -fopenmp -dM | grep -i openmp`.
- **Herramientas:** `make`, `perl`, `python3`.  
- **Python (para gráficas):** `pandas`, `matplotlib`  
  - Instalar: `pip install pandas matplotlib`

> **Nota:** En macOS, usa Homebrew (`brew install gcc gnuplot python`), y si hay problemas con `aligned_alloc`, cambia por `posix_memalign` (ver FAQ).

---

## Estructura del proyecto

```
.
├── mmClasicaOpenMP.c      # OpenMP clásica
├── mmFilasOpenMP.c        # OpenMP con B transpuesta (filas × filas)
├── mmClasicaPosix.c       # Pthreads
├── mmClasicaFork.c        # fork() con mmap para C compartida
├── Makefile               # build
├── lanzador_santos.pl     # batería de experimentos (genera .dat + LOG)
├── scripts/
│   └── plot_results.py    # procesa .dat → summary.csv + PNGs
└── README.md              # este documento
```

**Binarios generados por `make`:**

- `mm_omp`       (OpenMP clásica)  
- `mm_omp_trans` (OpenMP con transpuesta)  
- `mm_pthreads`  (Pthreads)  
- `mm_fork`      (Procesos `fork()` + `mmap` para C)

---

## Compilación

Compila todo con optimizaciones y flags habilitados:

```bash
make
```
Limpia binarios:
```bash
make clean
```

> Si prefieres un Makefile alterno con más comentarios: `Makefile.santos`.

---

## Uso de cada ejecutable

Todos los programas siguen la misma interfaz:
```
./<binario> N P
```

- `N` = tamaño de la matriz (cuadrada)  
- `P` = hilos/procesos (según el binario)  
- **Salida:** si `N <= 4`, imprime la matriz `C` (sanity); **siempre** la **última línea** es el **tiempo total en microsegundos**.

Ejemplos:
```bash
./mm_omp 256 4
./mm_omp_trans 512 8
./mm_pthreads 384 2
./mm_fork 256 4
```

---

## Prueba rápida (sanity check)

```bash
make run_small
# Ejecuta N=4, P=1 sobre todos los binarios
# Verás impresión de C y, al final, un número (tiempo en µs)
```

---

## Batería de experimentos automatizada

El script Perl `lanzador_santos.pl` recorre combinaciones de `N` y `P`, ejecuta **30 repeticiones** por punto y guarda resultados en `./dat/*.dat` más un log con promedios.

```bash
chmod +x lanzador_santos.pl
./lanzador_santos.pl
# dat/LOG-YYYYMMDD-HHMMSS.txt — resumen por punto
# dat/mm_omp_N01024_T08.dat  — crudos + mean/stdev/cv
```

**Personalización:** edita en el script los arreglos:
```perl
my @executables = ("./mm_omp", "./mm_pthreads", "./mm_fork", "./mm_omp_trans");
my @sizes       = (64, 128, 192, 256, 384, 512, 768, 1024);
my @threads     = (1, 2, 4, 8);
my $reps        = 30;
```

---

## Formato de los `.dat`

Cada `.dat` corresponde a una combinación `(ejecutable, N, P)` e incluye:

```
# executable=./mm_omp
# size=256
# threads=4
# reps=30
# time_unit=microseconds
# -------------------------------
<tiempo_rep_1>
<tiempo_rep_2>
...
<tiempo_rep_30>
# mean=<promedio_us>
# stdev=<desviacion_us>
# cv=<coef_var>
```

> La **última línea** de cada ejecución del binario es un número (tiempo en µs), que es lo que el Perl captura.

---

## Procesamiento: CSV + gráficas

Script: `scripts/plot_results.py`  
Salida: `out/summary.csv` y 3 gráficas por ejecutable:

- `out/time_<exec>.png` — Tiempo vs `N` (una curva por `P`).  
- `out/speedup_<exec>.png` — Speedup vs `P` (una curva por `N`).  
- `out/eff_<exec>.png` — Eficiencia vs `P` (una curva por `N`).

Ejecutar:
```bash
pip install pandas matplotlib
python3 scripts/plot_results.py
```

**Columnas de `summary.csv`:**
- `exec` — nombre del ejecutable.
- `N` — tamaño de matriz.
- `T` — hilos/procesos.
- `mean_us`, `stdev_us`, `cv` — media, desviación, coeficiente de variación.
- `reps` — número de repeticiones (esperado: 30).
- `speedup` — \( S_p = T_1/T_p \) (por `exec` y `N`).
- `efficiency` — \( E_p = S_p/p \).

> El script calcula `speedup` y `eficiencia` tomando como `T1` el tiempo promedio del caso `T=1` para esa pareja `(exec, N)`.

---

## Diseño experimental recomendado

- **Tamaños (`N`)**: `64, 128, 192, 256, 384, 512, 768, 1024`  
  - Cubren rangos pequeño→mediano→grande (distintas cachés / presión de memoria).
- **Paralelismo (`P`)**: `1, 2, 4, 8` (ajusta a núcleos reales de tu máquina).  
- **Repeticiones**: `30` por punto (estabilidad estadística).  
- **Sistemas**: si debes comparar dos Linux distintos (nativo y VM), corre la misma batería en ambos y reporta diferencias.

**Consejos prácticos:**
- Cierra apps pesadas y fija plan de energía en “alto rendimiento”.
- Evita que otros procesos “ruidosos” contaminen las medidas.
- Repite mediciones si el `cv` es alto (> 5–10%).

---

## Interpretación de resultados

- **OpenMP (clásica) vs. OpenMP (transpuesta):** la versión con transpuesta suele mejorar localidad y reducir fallos de caché; esperas menor tiempo en matrices medianas/grandes.  
- **Pthreads vs. OpenMP:** ambos deben escalar parecido si el reparto por filas es similar; difiere overhead de creación y afinidad.  
- **fork():** suele tener mayor overhead por procesos, pero en cargas grandes puede acercarse si la memoria no es cuello de botella; aquí usamos `mmap` compartida para recolectar `C`.  
- **Speedup y eficiencia:** aumentan con `P` hasta saturar el ancho de banda de memoria; la eficiencia decae cuando el problema es “pequeño” o el paralelismo supera a los núcleos físicos.  

**Claves para discutir en el informe:**
- Dónde se observa mejor escalamiento (rango de `N`).
- Punto a partir del cual el speedup “se aplana” por memoria.
- Variabilidad entre sistemas (hipervisor/VM vs. nativo).
- Efecto real de la transpuesta frente a la clásica.

---

## Buenas prácticas y reproducibilidad

- **Semilla fija:** ya fijada (`srand(21)`) para datos reproducibles.  
- **Especificar HW/SW:** documenta CPU, núcleos/hilos, RAM, SO, kernel y GCC.  
  - Comandos útiles:
    ```bash
    lscpu
    free -h
    uname -a
    gcc --version
    ```
- **Bitácora:** guarda `dat/LOG-*.txt`, `out/summary.csv` y PNGs junto al informe.  
- **Afinidad (opcional):** puedes fijar afinidad / `GOMP_CPU_AFFINITY` si quieres mayor control.

---

## Solución de problemas (FAQ)

**1) “gcc: error: unrecognized command line option ‘-fopenmp’”**  
Instala `libgomp` / toolchain con OpenMP (p. ej., `sudo apt install gcc libgomp1`). En macOS usa `brew install gcc` y compila con `gcc-XX` de Homebrew.

**2) “undefined reference to aligned_alloc” (macOS/antiguo glibc)**  
Reemplaza `aligned_alloc` por `posix_memalign`:
```c
double *A; if (posix_memalign((void**)&A, 64, sizeof(double)*N*N)) A=NULL;
```
Aplica igual para `B`, `C`, `BT`.

**3) “Killed” o `segfault` con N grande**  
No hay RAM suficiente: usa tamaños menores o reduce paralelismo. Cada matriz `N×N` de `double` consume `8*N*N` bytes (3–4 matrices en memoria).

**4) Resultados muy variables**  
Cierra apps, repite la batería, verifica gobernador de CPU (ondemand/performance), desactiva turbo en evaluación estricta, incrementa `reps` a 50–100.

**5) En `fork()` no veo `C`**  
En esta entrega **sí** se comparte `C` por `mmap`. Si cambias a `malloc`, cada proceso tendrá su copia y el padre no verá los resultados.

---

## Qué entregar

- **PDF** con:
  - Descripción breve de cada variante y cómo se mide el tiempo.
  - Especificación HW/SW de los sistemas probados.
  - Diseño experimental (tabla de `N`, `P`, `reps`).
  - Resultados (tablas/gráficas) + análisis (speedup, eficiencia).
  - Conclusiones y recomendaciones.
- **ZIP** con:
  - Código (`.c`), `Makefile`, `lanzador_santos.pl`, `scripts/plot_results.py`.
  - Carpeta `dat/` (opcional por tamaño) y `out/` con CSV/PNGs.

---

## Checklist de entrega

- [ ] `make` compila los 4 binarios sin warnings críticos.  
- [ ] `make run_small` funciona y muestra tiempo final (µs).  
- [ ] `./lanzador_santos.pl` generó `dat/*.dat` y un `LOG-*.txt`.  
- [ ] `python3 scripts/plot_results.py` generó `out/summary.csv` y PNGs.  
- [ ] Informe PDF con: metodología, HW/SW, diseño, resultados, análisis, conclusiones.  
- [ ] ZIP con fuentes + scripts + (opcional) datos y gráficos.  

---

## Créditos

Autor: **Santos Arellano**  
Ayuda de soporte: guías y scripts de automatización creados para el taller T22.