# Taller T12 - Procesos e Hilos en Sistemas Operativos

## 📋 Descripción General

Este taller implementa tres programas en C para demostrar el uso y funcionamiento de procesos e hilos en Linux. El objetivo es comprender las diferencias entre estos dos mecanismos de concurrencia y su aplicación práctica en el procesamiento paralelo de matrices.

## 🎯 Objetivos del Taller

- Comprender el funcionamiento de procesos en Linux mediante `fork()`
- Implementar comunicación entre procesos padre e hijos
- Utilizar llamadas al sistema `exec` para ejecutar programas
- Trabajar con hilos POSIX usando `pthread`
- Comparar el rendimiento y características entre procesos e hilos

## 📁 Estructura del Proyecto

```
T12_Procesos_Hilos/
│
├── Parte 1: matrizsum.c     # Programa principal con procesos (Parte 1)
├── Parte 1: ejecutor.c      # Programa que usa exec() (Parte 1)
├── Parte 2: matrizproc.c    # Programa con hilos pthread (Parte 2)
├── README.md                # Este archivo
├── GUIA_PRUEBAS.md         # Guía detallada de pruebas
├── INSTRUCCIONES_CAPTURAS.md # Instrucciones para capturas
├── ejecutar_pruebas.sh      # Script automático de pruebas
├── pruebas_rapidas.sh       # Script de verificación rápida
├── matrizsum                # Ejecutable de matrizsum
├── ejecutor                 # Ejecutable de ejecutor
├── matrizproc               # Ejecutable de matrizproc
└── T12 Procesos e Hilos Sistemas Operativos.pdf  # Documento del taller
```

## 🛠️ Requisitos del Sistema

- **Sistema Operativo**: Linux (Ubuntu, Debian, Fedora, etc.)
- **Compilador**: GCC (GNU Compiler Collection)
- **Librerías**: pthread (normalmente incluida en Linux)
- **Herramientas**: Terminal/Shell de Linux

### Instalación de Requisitos (si es necesario)

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential

# Fedora/RHEL
sudo dnf install gcc make

# Arch Linux
sudo pacman -S base-devel
```

## 📚 Programas Implementados

### 1. matrizsum - Procesamiento con Procesos

**Descripción**: Programa que crea múltiples procesos hijos para sumar las filas de una matriz cuadrada.

**Características**:
- Recibe dos argumentos: tamaño de matriz (m) y número de procesos (n)
- Valida que n sea divisor de m
- Cada proceso hijo procesa m/n filas
- Muestra el PID de cada proceso y sus resultados

**Uso**:
```bash
./matrizsum <tamaño_matriz> <numero_procesos>
```

**Ejemplo**:
```bash
./matrizsum 8 4   # Matriz 8x8 con 4 procesos
```

### 2. ejecutor - Llamada con exec()

**Descripción**: Programa que ejecuta `matrizsum` utilizando las llamadas al sistema de la familia `exec`.

**Características**:
- Crea un proceso hijo
- Usa `execl()` para ejecutar matrizsum
- Espera la finalización del programa ejecutado

**Uso**:
```bash
./ejecutor <tamaño_matriz> <numero_procesos>
```

**Ejemplo**:
```bash
./ejecutor 6 3   # Ejecuta matrizsum con matriz 6x6 y 3 procesos
```

### 3. matrizproc - Procesamiento con Hilos

**Descripción**: Programa que usa hilos POSIX para procesar filas de una matriz, calculando suma o máximo.

**Características**:
- Recibe tamaño de matriz y tipo de operación
- Crea p hilos (p = número_de_grupo + 2)
- Soporta operaciones: "sumar" y "max"
- Distribuye equitativamente las filas entre hilos

**Uso**:
```bash
./matrizproc <tamaño_matriz> <operacion>
```

**Ejemplos**:
```bash
./matrizproc 10 sumar   # Suma elementos de cada fila
./matrizproc 10 max     # Encuentra máximo de cada fila
```

## 🔨 Compilación

### Compilación Manual

```bash
# Compilar matrizsum
gcc -o matrizsum "Parte 1: matrizsum.c" -Wall

# Compilar ejecutor
gcc -o ejecutor "Parte 1: ejecutor.c" -Wall

# Compilar matrizproc (requiere pthread)
gcc -o matrizproc "Parte 2: matrizproc.c" -pthread -Wall
```

### Compilación con Makefile

Crea un archivo `Makefile` con el siguiente contenido:

```makefile
CC = gcc
CFLAGS = -Wall -g
PTHREAD_FLAG = -pthread

all: matrizsum ejecutor matrizproc

matrizsum: "Parte 1: matrizsum.c"
	$(CC) $(CFLAGS) -o matrizsum "Parte 1: matrizsum.c"

ejecutor: "Parte 1: ejecutor.c"
	$(CC) $(CFLAGS) -o ejecutor "Parte 1: ejecutor.c"

matrizproc: "Parte 2: matrizproc.c"
	$(CC) $(CFLAGS) $(PTHREAD_FLAG) -o matrizproc "Parte 2: matrizproc.c"

clean:
	rm -f matrizsum ejecutor matrizproc

run-test:
	@echo "=== Prueba matrizsum ==="
	./matrizsum 8 4
	@echo "\n=== Prueba ejecutor ==="
	./ejecutor 6 3
	@echo "\n=== Prueba matrizproc (sumar) ==="
	./matrizproc 10 sumar
	@echo "\n=== Prueba matrizproc (max) ==="
	./matrizproc 10 max

.PHONY: all clean run-test
```

Luego ejecuta:
```bash
make all        # Compila todos los programas
make run-test   # Ejecuta pruebas básicas
make clean      # Limpia los ejecutables
```

## ⚙️ Configuración Importante

### ✅ CONFIGURACIÓN ACTUAL - matrizproc.c

El archivo `"Parte 2: matrizproc.c"` está **CORRECTAMENTE CONFIGURADO** para el **Grupo 7**:

```c
int p = 9;  // Configurado: p = 7 + 2 = 9 hilos
```

**Configuración por Grupo**:
- Grupo 1: `int p = 3;` (3 hilos)
- Grupo 2: `int p = 4;` (4 hilos)
- Grupo 3: `int p = 5;` (5 hilos)
- Grupo 4: `int p = 6;` (6 hilos)
- Grupo 5: `int p = 7;` (7 hilos)
- Grupo 6: `int p = 8;` (8 hilos)
- **Grupo 7: `int p = 9;` (9 hilos)** ✅ **ACTUAL**

## 🧪 Casos de Prueba

### Test 1: Validación de Argumentos
```bash
# Debe mostrar error - falta argumentos
./matrizsum 8

# Debe mostrar error - n no divide a m
./matrizsum 10 3

# Debe funcionar correctamente
./matrizsum 12 4
```

### Test 2: Diferentes Tamaños de Matriz
```bash
# Matriz pequeña
./matrizsum 4 2
./matrizproc 4 sumar

# Matriz mediana
./matrizsum 16 4
./matrizproc 16 max

# Matriz grande
./matrizsum 100 10
./matrizproc 100 sumar
```

### Test 3: Verificación de exec()
```bash
# Debe ejecutar matrizsum correctamente
./ejecutor 8 2
./ejecutor 12 6
```

## 📊 Salida Esperada

### matrizsum - Ejemplo de Salida
```
Matriz original (8x8):
  1   2   3   4   5   6   7   8 
  2   3   4   5   6   7   8   9 
  3   4   5   6   7   8   9  10 
  4   5   6   7   8   9  10  11 
  5   6   7   8   9  10  11  12 
  6   7   8   9  10  11  12  13 
  7   8   9  10  11  12  13  14 
  8   9  10  11  12  13  14  15 

Proceso hijo PID=12345 procesando filas 0 a 1
  Fila 0: 1 2 3 4 5 6 7 8 -> Suma = 36 (PID: 12345)
  Fila 1: 2 3 4 5 6 7 8 9 -> Suma = 44 (PID: 12345)
...
```

### matrizproc - Ejemplo de Salida (sumar)
```
Matriz original (4x4):
  1   2   0   0 
  1   1   3   4 
  2   1   1   0 
  0   0   0   3 

Procesando con 5 hilos, operación: sumar
----------------------------------------
Hilo 0 procesando filas 0 a 0
Fila 0:   1   2   0   0  -> Suma: 3
Hilo 1 procesando filas 1 a 1
Fila 1:   1   1   3   4  -> Suma: 9
...
```

## 🐛 Solución de Problemas Comunes

### Problema 1: "Permission denied" al ejecutar
```bash
# Solución: Dar permisos de ejecución
chmod +x matrizsum ejecutor matrizproc
```

### Problema 2: "undefined reference to pthread_create"
```bash
# Solución: Asegurar que se incluye -pthread al compilar
gcc -o matrizproc matrizproc.c -pthread
```

### Problema 3: "Segmentation fault"
- Verificar que los argumentos sean números positivos
- Asegurar que n sea divisor de m en matrizsum
- Verificar la asignación de memoria

### Problema 4: ejecutor no encuentra matrizsum
```bash
# Asegurar que matrizsum está en el directorio actual
ls -la matrizsum

# Si es necesario, usar ruta completa en ejecutor.c
execl("/ruta/completa/matrizsum", "matrizsum", argv[1], argv[2], NULL);
```

## 📈 Análisis de Rendimiento

### Comparación Procesos vs Hilos

| Aspecto | Procesos (fork) | Hilos (pthread) |
|---------|-----------------|-----------------|
| **Creación** | Más costosa | Más rápida |
| **Memoria** | Espacio separado | Espacio compartido |
| **Comunicación** | IPC necesario | Variables compartidas |
| **Overhead** | Mayor | Menor |
| **Aislamiento** | Completo | Ninguno |
| **Seguridad** | Mayor | Menor |

### Métricas de Tiempo (Opcional)

Para medir el tiempo de ejecución:
```bash
time ./matrizsum 1000 10
time ./matrizproc 1000 sumar
```

## 📝 Estructura del Informe

El informe PDF debe incluir:

1. **Portada**
   - Nombre del curso
   - Número del taller
   - Integrantes del grupo
   - Fecha

2. **Introducción** (1 página)
   - Objetivos del taller
   - Conceptos de procesos e hilos

3. **Desarrollo Parte 1** (3-4 páginas)
   - Explicación de matrizsum.c
   - Explicación de ejecutor.c
   - Diagramas de flujo
   - Capturas de pantalla

4. **Desarrollo Parte 2** (2-3 páginas)
   - Explicación de matrizproc.c
   - Análisis de distribución de trabajo
   - Capturas de pantalla

5. **Resultados y Análisis** (2 páginas)
   - Comparación procesos vs hilos
   - Análisis de eficiencia
   - Casos de prueba ejecutados

6. **Conclusiones** (1 página)
   - Aprendizajes obtenidos
   - Ventajas y desventajas observadas

7. **Anexos**
   - Código fuente completo
   - Evidencias adicionales



## 🚀 Scripts de Prueba Automatizados

### Scripts Disponibles

#### 1. `ejecutar_pruebas.sh` - Pruebas Completas con Capturas
```bash
./ejecutar_pruebas.sh
```
- Ejecuta las **29 capturas necesarias** para el informe
- Pausa entre cada prueba para tomar screenshots
- Incluye validación de errores, casos exitosos y pruebas de rendimiento
- Muestra el comando antes de ejecutarlo

#### 2. `pruebas_rapidas.sh` - Verificación Rápida
```bash
./pruebas_rapidas.sh
```
- Verificación rápida sin pausas
- Ideal para comprobar que todo funciona correctamente
- Ejecuta casos básicos de todos los programas

### Archivos de Documentación

- **`GUIA_PRUEBAS.md`**: Guía detallada con todos los casos de prueba
- **`INSTRUCCIONES_CAPTURAS.md`**: Instrucciones paso a paso para las capturas

### Ejecución Recomendada

1. **Verificación inicial**:
   ```bash
   ./pruebas_rapidas.sh
   ```

2. **Capturas para el informe**:
   ```bash
   ./ejecutar_pruebas.sh
   ```

3. **Comandos individuales** (consultar `INSTRUCCIONES_CAPTURAS.md`)

## 📦 Entrega Final

### Archivos Requeridos
```
taller_t12_grupo_7.zip/
├── Parte 1: matrizsum.c
├── Parte 1: ejecutor.c
├── Parte 2: matrizproc.c
├── matrizsum (ejecutable)
├── ejecutor (ejecutable)
├── matrizproc (ejecutable)
├── ejecutar_pruebas.sh
├── pruebas_rapidas.sh
├── GUIA_PRUEBAS.md
├── INSTRUCCIONES_CAPTURAS.md
├── informe.pdf
└── README.md
```

### Comando para Empaquetar
```bash
# Crear directorio de entrega
mkdir entrega_final
cp "Parte 1: matrizsum.c" "Parte 1: ejecutor.c" "Parte 2: matrizproc.c" entrega_final/
cp *.md *.sh informe.pdf entrega_final/
cp matrizsum ejecutor matrizproc entrega_final/

# Comprimir
zip -r taller_t12_grupo_7.zip entrega_final/
# o
tar -czf taller_t12_grupo_7.tar.gz entrega_final/
```

## 👥 Autores

- Grupo: 7
- Integrantes:
  - Santos Alejandro Arellano Olarte
  - Jeison Camilo Alfonso Moreno
  - Jose Villaroel

## 📅 Fecha de Entrega

[Fecha según lo indicado en la plataforma]

## 📚 Referencias

- Manual de fork(): `man 2 fork`
- Manual de exec(): `man 3 exec`
- Manual de pthread: `man 7 pthreads`
- Documentación de GCC: https://gcc.gnu.org/onlinedocs/
- POSIX Threads Programming: https://computing.llnl.gov/tutorials/pthreads/

## 📄 Licencia

Este proyecto es parte del curso de Sistemas Operativos y es solo para fines educativos.

