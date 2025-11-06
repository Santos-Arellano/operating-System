#!/usr/bin/env perl
# lanzador_santos.pl — automatiza la batería de experimentos
# Genera archivos .dat con 30 repeticiones por combinación (tam, hilos).
# Cada .dat contiene una columna con tiempos en microsegundos y al final
# líneas de resumen con promedio y desviación estándar.
#
# Uso: chmod +x lanzador_santos.pl && ./lanzador_santos.pl

use strict;
use warnings;
use POSIX qw(strftime);

# ---- Configurable ----
my @executables = ("./mm_omp", "./mm_pthreads", "./mm_fork", "./mm_omp_trans");
my @sizes       = (64, 128, 192, 256, 384, 512, 768, 1024);
my @threads     = (1, 2, 4, 8);         # Ajusta según núcleos disponibles
my $reps        = 30;                   # Repeticiones por punto
my $outdir      = "dat";                # Carpeta de salida
# ----------------------

mkdir $outdir unless -d $outdir;

sub mean {
    my ($arr) = @_;
    my $sum = 0; $sum += $_ for @$arr;
    return @$arr ? $sum / scalar(@$arr) : 0;
}

sub stdev {
    my ($arr) = @_;
    my $mu = mean($arr);
    my $sum = 0; $sum += ($_ - $mu) ** 2 for @$arr;
    return @$arr ? sqrt($sum / scalar(@$arr)) : 0;
}

my $ts = strftime("%Y%m%d-%H%M%S", localtime);
open my $log, ">", "$outdir/LOG-$ts.txt";

print $log "Inicio: $ts\n";
print $log "Execs: @executables\n";
print $log "Sizes: @sizes\n";
print $log "Threads: @threads\n";
print $log "Reps: $reps\n\n";

for my $exe (@executables) {
    for my $n (@sizes) {
        for my $th (@threads) {

            my $safe = $exe; $safe =~ s{^\./}{}g;  # sin ./ 
            my $fname = sprintf("%s/%s_N%04d_T%02d.dat", $outdir, $safe, $n, $th);
            open my $fh, ">", $fname or die "No puedo escribir $fname: $!";

            print $fh "# executable=$exe\n";
            print $fh "# size=$n\n";
            print $fh "# threads=$th\n";
            print $fh "# reps=$reps\n";
            print $fh "# time_unit=microseconds\n";
            print $fh "# -------------------------------\n";
            print ">> $exe  N=$n  TH=$th  reps=$reps\n";

            my @times;
            for my $r (1..$reps) {
                my $cmd = "$exe $n $th";
                my $out = `$cmd`;   # captura stdout
                my ($t) = ($out =~ /([0-9]+)\s*$/m);  # último entero
                if (defined $t) {
                    push @times, $t + 0;
                    print $fh "$t\n";
                } else {
                    print $fh "NaN\n";
                    print $log "[WARN] No pude parsear tiempo en rep $r para $cmd. Salida:\n$out\n";
                }
            }

            my $mu  = mean(\@times);
            my $sd  = stdev(\@times);
            my $cv  = ($mu > 0) ? ($sd/$mu) : 0;

            print $fh "# mean=$mu\n# stdev=$sd\n# cv=$cv\n";
            close $fh;

            printf $log "%-12s N=%-4d TH=%-2d  mean=%10.2f us  sd=%9.2f  cv=%6.3f\n",
                        $safe, $n, $th, $mu, $sd, $cv;
        }
    }
}

close $log;
print "Listo. Archivos .dat en '$outdir' y bitácora en $outdir/LOG-$ts.txt\n";
