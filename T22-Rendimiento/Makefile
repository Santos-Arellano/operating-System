# Makefile — Taller T22 (Santos)
CC       := gcc
CFLAGS   := -O3 -march=native -Wall -Wextra -std=c11
OMPFLAGS := -fopenmp
PTHFLAGS := -pthread

SRC_FORK      := mmClasicaFork.c
SRC_OMP       := mmClasicaOpenMP.c
SRC_PTHREADS  := mmClasicaPosix.c
SRC_OMP_TRANS := mmFilasOpenMP.c

BIN_FORK      := mm_fork
BIN_OMP       := mm_omp
BIN_PTHREADS  := mm_pthreads
BIN_OMP_TRANS := mm_omp_trans

.PHONY: all clean run_small

all: $(BIN_FORK) $(BIN_OMP) $(BIN_PTHREADS) $(BIN_OMP_TRANS)

$(BIN_FORK): $(SRC_FORK)
	$(CC) $(CFLAGS) $< -o $@

$(BIN_OMP): $(SRC_OMP)
	$(CC) $(CFLAGS) $(OMPFLAGS) $< -o $@

$(BIN_PTHREADS): $(SRC_PTHREADS)
	$(CC) $(CFLAGS) $(PTHFLAGS) $< -o $@

$(BIN_OMP_TRANS): $(SRC_OMP_TRANS)
	$(CC) $(CFLAGS) $(OMPFLAGS) $< -o $@

run_small: all
	./$(BIN_OMP) 4 1
	./$(BIN_PTHREADS) 4 1
	./$(BIN_FORK) 4 1
	./$(BIN_OMP_TRANS) 4 1

clean:
	rm -f $(BIN_FORK) $(BIN_OMP) $(BIN_PTHREADS) $(BIN_OMP_TRANS)
