#!/usr/bin/perl
#**************************************************************
#          		Pontificia Universidad Javeriana
#     Materia: Sistemas Operativos
#     Tema: Taller de Evaluación de Rendimiento
#     Fichero: Script de automatización de ejecución por lotes
#****************************************************************/

use strict;
use warnings;

my $Path = `pwd`;
chomp($Path);

# Binarios esperados en este directorio (generados por Makefile)
my @BINARIOS = (
    "mmClasicaOpenMP",
    "mmFilasOpenMP",
    "mmClasicaPosix",
    "mmClasicaFork"
);

# Configuración de la batería
my @Size_Matriz = (8, 32, 64);  # evita impresión de matrices con N>=9
my @Num_Hilos   = (1, 2, 4, 8);
my $Repeticiones = 5;

# Directorio de salida para .dat
my $DAT_DIR = "$Path/dat";
mkdir $DAT_DIR unless -d $DAT_DIR;

foreach my $bin (@BINARIOS) {
    my $bin_path = "$Path/$bin";
    if (! -x $bin_path) {
        print STDERR "[WARN] No se encontró ejecutable: $bin_path\n";
        next;
    }

    foreach my $size (@Size_Matriz) {
        foreach my $hilo (@Num_Hilos) {
            my $file = "$DAT_DIR/${bin}-${size}-Hilos-${hilo}.dat";
            for (my $i = 0; $i < $Repeticiones; $i++) {
                system("$bin_path $size $hilo >> $file");
            }
        }
    }
}