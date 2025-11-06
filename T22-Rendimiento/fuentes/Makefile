GCC = gcc
CFLAGS = -lm
FOPENMP = -fopenmp -O3
POSIX = -lpthread

PROGRAMAS = mmClasicaOpenMP 

All: $(PROGRAMAS)

mmClasicaFork:
	$(GCC) $(CLFAGS) $@.c -o $@ 

mmClasicaOpenMP:
	$(GCC) $(CLFAGS) $@.c -o $@ $(FOPENMP)

clean:
	$(RM) $(PROGRAMAS) 
