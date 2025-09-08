#!/bin/bash

# Script de Pruebas Automatizado - Taller T12 Procesos e Hilos
# Grupo 7: Santos Alejandro Arellano Olarte, Jeison Camilo Alfonso Moreno, Jose Villaroel

echo "========================================"
echo "  TALLER T12 - PROCESOS E HILOS"
echo "  GRUPO 7 - PRUEBAS AUTOMATIZADAS"
echo "========================================"
echo ""

# Función para pausar entre pruebas
pausar() {
    echo ""
    echo "Presiona ENTER para continuar con la siguiente prueba..."
    read -r
    clear
}

# Verificar que los ejecutables existen
echo "Verificando ejecutables..."
if [ ! -f "./matrizsum" ] || [ ! -f "./ejecutor" ] || [ ! -f "./matrizproc" ]; then
    echo "Error: Algunos ejecutables no existen. Compilando..."
    gcc -o matrizsum "Parte 1: matrizsum.c" -Wall
    gcc -o ejecutor "Parte 1: ejecutor.c" -Wall
    gcc -o matrizproc "Parte 2: matrizproc.c" -pthread -Wall
    chmod +x matrizsum ejecutor matrizproc
fi

echo "Ejecutables verificados ✓"
echo ""

# PARTE 1: PRUEBAS DE ERRORES
echo "=========================================="
echo "PARTE 1: PRUEBAS DE VALIDACIÓN DE ERRORES"
echo "=========================================="

echo ""
echo "--- CAPTURA 1: matrizsum sin argumentos ---"
echo "Comando: ./matrizsum"
./matrizsum
pausar

echo "--- CAPTURA 2: matrizsum con un argumento ---"
echo "Comando: ./matrizsum 8"
./matrizsum 8
pausar

echo "--- CAPTURA 3: matrizsum n no divide m ---"
echo "Comando: ./matrizsum 10 3"
./matrizsum 10 3
pausar

echo "--- CAPTURA 8: ejecutor sin argumentos ---"
echo "Comando: ./ejecutor"
./ejecutor
pausar

echo "--- CAPTURA 9: ejecutor con un argumento ---"
echo "Comando: ./ejecutor 8"
./ejecutor 8
pausar

echo "--- CAPTURA 13: matrizproc sin argumentos ---"
echo "Comando: ./matrizproc"
./matrizproc
pausar

echo "--- CAPTURA 14: matrizproc con un argumento ---"
echo "Comando: ./matrizproc 10"
./matrizproc 10
pausar

echo "--- CAPTURA 15: matrizproc operación inválida ---"
echo "Comando: ./matrizproc 10 multiplicar"
./matrizproc 10 multiplicar
pausar

# PARTE 2: PRUEBAS EXITOSAS DE MATRIZSUM
echo "======================================"
echo "PARTE 2: PRUEBAS EXITOSAS - MATRIZSUM"
echo "======================================"

echo ""
echo "--- CAPTURA 4: Matriz 4x4 con 2 procesos ---"
echo "Comando: ./matrizsum 4 2"
./matrizsum 4 2
pausar

echo "--- CAPTURA 5: Matriz 8x8 con 4 procesos ---"
echo "Comando: ./matrizsum 8 4"
./matrizsum 8 4
pausar

echo "--- CAPTURA 6: Matriz 12x12 con 6 procesos ---"
echo "Comando: ./matrizsum 12 6"
./matrizsum 12 6
pausar

echo "--- CAPTURA 7: Matriz 16x16 con 8 procesos ---"
echo "Comando: ./matrizsum 16 8"
./matrizsum 16 8
pausar

# PARTE 3: PRUEBAS DE EJECUTOR
echo "=================================="
echo "PARTE 3: PRUEBAS DE EJECUTOR (exec)"
echo "=================================="

echo ""
echo "--- CAPTURA 10: Ejecutor 6x6 con 3 procesos ---"
echo "Comando: ./ejecutor 6 3"
./ejecutor 6 3
pausar

echo "--- CAPTURA 11: Ejecutor 8x8 con 2 procesos ---"
echo "Comando: ./ejecutor 8 2"
./ejecutor 8 2
pausar

echo "--- CAPTURA 12: Ejecutor 12x12 con 4 procesos ---"
echo "Comando: ./ejecutor 12 4"
./ejecutor 12 4
pausar

# PARTE 4: PRUEBAS DE MATRIZPROC - SUMAR
echo "=========================================="
echo "PARTE 4: PRUEBAS DE MATRIZPROC - OPERACIÓN SUMAR"
echo "Configurado para GRUPO 7 (9 hilos)"
echo "=========================================="

echo ""
echo "--- CAPTURA 16: Matriz 4x4 sumar ---"
echo "Comando: ./matrizproc 4 sumar"
./matrizproc 4 sumar
pausar

echo "--- CAPTURA 17: Matriz 10x10 sumar ---"
echo "Comando: ./matrizproc 10 sumar"
./matrizproc 10 sumar
pausar

echo "--- CAPTURA 18: Matriz 15x15 sumar ---"
echo "Comando: ./matrizproc 15 sumar"
./matrizproc 15 sumar
pausar

# PARTE 5: PRUEBAS DE MATRIZPROC - MAX
echo "========================================"
echo "PARTE 5: PRUEBAS DE MATRIZPROC - OPERACIÓN MAX"
echo "Configurado para GRUPO 7 (9 hilos)"
echo "========================================"

echo ""
echo "--- CAPTURA 19: Matriz 4x4 max ---"
echo "Comando: ./matrizproc 4 max"
./matrizproc 4 max
pausar

echo "--- CAPTURA 20: Matriz 10x10 max ---"
echo "Comando: ./matrizproc 10 max"
./matrizproc 10 max
pausar

echo "--- CAPTURA 21: Matriz 20x20 max ---"
echo "Comando: ./matrizproc 20 max"
./matrizproc 20 max
pausar

# PARTE 6: PRUEBAS DE RENDIMIENTO
echo "=================================="
echo "PARTE 6: PRUEBAS DE RENDIMIENTO"
echo "=================================="

echo ""
echo "--- CAPTURA 22: Tiempo de ejecución matrizsum ---"
echo "Comando: time ./matrizsum 100 10"
time ./matrizsum 100 10
pausar

echo "--- CAPTURA 23: Tiempo de ejecución matrizproc ---"
echo "Comando: time ./matrizproc 100 sumar"
time ./matrizproc 100 sumar
pausar

echo "--- CAPTURA 24: Matriz grande - procesos ---"
echo "Comando: ./matrizsum 50 10"
./matrizsum 50 10
pausar

echo "--- CAPTURA 25: Matriz grande - hilos ---"
echo "Comando: ./matrizproc 50 sumar"
./matrizproc 50 sumar
pausar

# PARTE 7: CASOS ESPECIALES
echo "=============================="
echo "PARTE 7: CASOS ESPECIALES"
echo "=============================="

echo ""
echo "--- CAPTURA 26: Matrices 1x1 ---"
echo "Comando: ./matrizsum 1 1"
./matrizsum 1 1
echo ""
echo "Comando: ./matrizproc 1 sumar"
./matrizproc 1 sumar
pausar

echo "--- CAPTURA 27: Matrices 2x2 ---"
echo "Comando: ./matrizsum 2 1"
./matrizsum 2 1
echo ""
echo "Comando: ./matrizproc 2 max"
./matrizproc 2 max
pausar

echo "--- CAPTURA 28: Verificación de PIDs diferentes ---"
echo "Comando: ./matrizsum 6 3 | grep 'PID='"
./matrizsum 6 3 | grep "PID="
pausar

echo "--- CAPTURA 29: Verificación de IDs de hilos ---"
echo "Comando: ./matrizproc 8 sumar | grep 'Hilo'"
./matrizproc 8 sumar | grep "Hilo"
pausar

echo "========================================"
echo "  ¡TODAS LAS PRUEBAS COMPLETADAS!"
echo "========================================"
echo ""
echo "Resumen de capturas realizadas:"
echo "- Parte 1: Validación de errores (8 capturas)"
echo "- Parte 2: matrizsum exitoso (4 capturas)"
echo "- Parte 3: ejecutor (3 capturas)"
echo "- Parte 4: matrizproc sumar (3 capturas)"
echo "- Parte 5: matrizproc max (3 capturas)"
echo "- Parte 6: rendimiento (4 capturas)"
echo "- Parte 7: casos especiales (4 capturas)"
echo ""
echo "Total: 29 capturas para el informe"
echo ""
echo "Configuración utilizada:"
echo "- Grupo: 7"
echo "- Hilos en matrizproc: 9 (7 + 2)"
echo "- Integrantes: Santos Alejandro Arellano Olarte, Jeison Camilo Alfonso Moreno, Jose Villaroel"
echo ""
echo "¡Listo para generar el informe!"