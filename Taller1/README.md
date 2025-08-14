# Taller 1 - Sistemas Operativos

Este repositorio contiene los programas desarrollados para el Taller 1 del curso de Sistemas Operativos. Los programas están escritos en C y demuestran diferentes conceptos fundamentales de sistemas operativos como procesos, manejo de archivos, y comparación de valores.


#Grupo 7

## 📁 Estructura del Proyecto

```
operating-System/
├── README.md
└── Taller1/
    ├── Docs/
    │   ├── dormilon.c
    │   ├── ListaArreglos.tar.gz
    │   ├── T11 - unix-cocalc.pdf
    │   └── taller.txt
    ├── dormilon.c
    ├── lector.c
    ├── lector2.c
    ├── menor.c
    └── Taller1.pdf
```

## 🚀 Programas Incluidos

### 1. `menor.c` - Comparador de Números

**Descripción:** Programa que compara dos números enteros y determina cuál es menor, mayor o si son iguales.

**Funcionalidad:**
- Recibe dos argumentos de línea de comandos (números enteros)
- Compara los valores y muestra el resultado
- Maneja errores de entrada (número incorrecto de argumentos)

**Compilación:**
```bash
gcc -Wall -Wextra -O2 -std=c11 -o menor menor.c
```

**Ejecución:**
```bash
./menor <numero1> <numero2>
```

**Ejemplos:**
```bash
./menor 5 10
# Salida: 5 es menor que 10

./menor 15 3
# Salida: 15 es mayor que 3

./menor 7 7
# Salida: 7 es igual a 7
```

### 2. `lector.c` - Lector de Archivos Básico

**Descripción:** Programa que lee los primeros 10 bytes de un archivo especificado.

**Funcionalidad:**
- Abre un archivo en modo solo lectura
- Lee los primeros 10 bytes
- Muestra el descriptor de archivo y los bytes leídos
- Cierra el archivo y libera memoria

**Compilación:**
```bash
gcc -Wall -Wextra -O2 -std=c11 -o lector lector.c
```

**Ejecución:**
```bash
./lector <nombre_archivo>
```

**Ejemplo:**
```bash
./lector archivo.txt
# Salida: se llamo a read(3, c, 10). Devolvio que 10 bytes fueron leidos.
#        Esos bytes son los siguientes: [contenido]
```

### 3. `lector2.c` - Lector de Archivos Mejorado

**Descripción:** Versión mejorada del lector con mejor manejo de errores.

**Funcionalidad:**
- Incluye verificación de errores al abrir archivos
- Manejo de errores en la operación de lectura
- Mensajes de error más descriptivos usando `perror()`
- Misma funcionalidad básica que `lector.c` pero más robusto

**Compilación:**
```bash
gcc -Wall -Wextra -O2 -std=c11 -o lector2 lector2.c
```

**Ejecución:**
```bash
./lector2 <nombre_archivo>
```

**Ejemplo:**
```bash
./lector2 archivo.txt
# Salida: Se llamó a read(3, c, 10). Devolvió que 10 bytes fueron leídos.
#        Esos bytes son los siguientes: [contenido]
```

### 4. `dormilon.c` - Proceso Zombie

**Descripción:** Programa que crea un proceso zombie para demostrar conceptos de gestión de procesos.

**Funcionalidad:**
- Crea un proceso hijo usando `fork()`
- El proceso padre entra en un bucle infinito durmiendo
- El proceso hijo termina inmediatamente con código de salida 4
- Demuestra el concepto de procesos zombie

**Compilación:**
```bash
gcc -Wall -Wextra -O2 -std=c11 -o dormilon dormilon.c
```

**Ejecución:**
```bash
./dormilon
```

**Nota:** Este programa creará un proceso zombie. Para verlo, ejecuta en otra terminal:
```bash
ps aux | grep dormilon
```

## 🛠️ Requisitos del Sistema

### Para macOS:
- **Compilador GCC:** Instalar Command Line Tools de Xcode
```bash
xcode-select --install
```

### Para Linux:
- **Compilador GCC:** Generalmente ya instalado
```bash
# Si no está instalado:
sudo apt-get install build-essential  # Ubuntu/Debian
sudo yum install gcc                  # CentOS/RHEL
```

### Para Windows:
- **MinGW-w64** o **Cygwin**
- **WSL (Windows Subsystem for Linux)** - Recomendado

## 📋 Compilación y Ejecución Masiva

Para compilar todos los programas de una vez:

```bash
# Navegar al directorio del proyecto
cd Taller1

# Compilar todos los programas
gcc -Wall -Wextra -O2 -std=c11 -o menor menor.c
gcc -Wall -Wextra -O2 -std=c11 -o lector lector.c
gcc -Wall -Wextra -O2 -std=c11 -o lector2 lector2.c
gcc -Wall -Wextra -O2 -std=c11 -o dormilon dormilon.c
```

## 🔍 Conceptos Demostrados

### `menor.c`
- **Argumentos de línea de comandos** (`argc`, `argv`)
- **Conversión de strings a enteros** (`atoi()`)
- **Estructuras de control** (if-else)
- **Manejo de errores básico**

### `lector.c` y `lector2.c`
- **Sistema de archivos Unix** (descriptores de archivo)
- **Llamadas al sistema** (`open()`, `read()`, `close()`)
- **Gestión de memoria dinámica** (`calloc()`, `free()`)
- **Manejo de errores** (`perror()`)

### `dormilon.c`
- **Creación de procesos** (`fork()`)
- **Estados de procesos** (zombie)
- **Códigos de salida** (`exit()`)
- **Suspensión de procesos** (`sleep()`)

## ⚠️ Notas Importantes

1. **Proceso Zombie:** `dormilon.c` crea un proceso zombie intencionalmente para fines educativos. En un entorno de producción, esto debería evitarse.

2. **Manejo de Errores:** `lector2.c` es la versión recomendada para uso en producción debido a su mejor manejo de errores.

3. **Permisos:** Asegúrate de que los archivos que intentes leer con `lector` o `lector2` tengan permisos de lectura.

4. **Compilación:** Se recomienda usar las flags de compilación mostradas para detectar warnings y optimizar el código.

## 📚 Recursos Adicionales

- **Documentación del Taller:** `Taller1.pdf`
- **Archivos de ejemplo:** `Docs/ListaArreglos.tar.gz`
- **Código de referencia:** `Docs/dormilon.c`

## 🤝 Contribuciones

Este es un proyecto educativo. Si encuentras errores o tienes sugerencias de mejora, no dudes en crear un issue o pull request.

---

**Desarrollado para el curso de Sistemas Operativos**  
*Universidad Javeriana*

## 🚀 GUÍA DE EJECUCIÓN PASO A PASO (macOS)

### Paso 1: Navegar al directorio del proyecto
```bash
cd /Users/santosa/Documents/GitHub/operating-System/Taller1
```

### Paso 2: Compilar todos los programas
```bash
# Compilar menor.c
gcc -Wall -Wextra -O2 -std=c11 -o menor menor.c

# Compilar lector.c
gcc -Wall -Wextra -O2 -std=c11 -o lector lector.c

# Compilar lector2.c
gcc -Wall -Wextra -O2 -std=c11 -o lector2 lector2.c

# Compilar dormilon.c
gcc -Wall -Wextra -O2 -std=c11 -o dormilon dormilon.c
```

### Paso 3: Crear un archivo de prueba para los lectores
```bash
echo "Hola mundo, este es un archivo de prueba para los programas lector." > archivo_prueba.txt
```

### Paso 4: Ejecutar todos los programas

#### 4.1 Ejecutar menor.c (comparador de números)
```bash
./menor 5 10
# Salida esperada: 5 es menor que 10

./menor 15 3
# Salida esperada: 15 es mayor que 3

./menor 7 7
# Salida esperada: 7 es igual a 7

# Probar con argumentos incorrectos
./menor 5
# Salida esperada: Uso de ./menor argumentos
```

#### 4.2 Ejecutar lector.c (lector básico)
```bash
./lector archivo_prueba.txt
# Salida esperada: se llamo a read(3, c, 10). Devolvio que 10 bytes fueron leidos.
#                  Esos bytes son los siguientes: Hola mundo
```

#### 4.3 Ejecutar lector2.c (lector mejorado)
```bash
./lector2 archivo_prueba.txt
# Salida esperada: Se llamó a read(3, c, 10). Devolvió que 10 bytes fueron leídos.
#                  Esos bytes son los siguientes: Hola mundo

# Probar con archivo inexistente
./lector2 archivo_inexistente.txt
# Salida esperada: Error al abrir el archivo: No such file or directory
```

#### 4.4 Ejecutar dormilon.c (proceso zombie)
```bash
# En una terminal:
./dormilon

# En otra terminal (nueva ventana o pestaña), verificar el proceso zombie:
ps aux | grep dormilon
# Deberías ver el proceso zombie en la lista

# Para terminar el proceso dormilon:
# En la primera terminal, presiona Ctrl+C
```

### Paso 5: Verificar que todos los ejecutables se crearon
```bash
ls -la *.c *.out
# Deberías ver: dormilon, lector, lector2, menor
```

### Paso 6: Limpiar archivos temporales (opcional)
```bash
# Eliminar archivo de prueba
rm archivo_prueba.txt

# Eliminar ejecutables (si quieres limpiar)
# rm menor lector lector2 dormilon
```

## 🔧 Solución de Problemas

### Si gcc no está instalado:
```bash
xcode-select --install
```

### Si hay problemas de permisos:
```bash
chmod +x menor lector lector2 dormilon
```

### Si quieres ver los warnings durante la compilación:
```bash
gcc -Wall -Wextra -std=c11 -o menor menor.c
```

### Para verificar que los programas funcionan correctamente:
```bash
# Verificar menor.c
./menor 1 2 && echo "✓ menor.c funciona correctamente"

# Verificar lector.c
echo "test" > test.txt && ./lector test.txt && echo "✓ lector.c funciona correctamente"

# Verificar lector2.c
./lector2 test.txt && echo "✓ lector2.c funciona correctamente"

# Limpiar archivo de prueba
rm test.txt
```

