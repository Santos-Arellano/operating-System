CC=gcc
CFLAGS=-O2 -std=c11 -Wall -Wextra
LDFLAGS=-pthread

all: controlador agente

controlador: controlador.c comun.h
	$(CC) $(CFLAGS) controlador.c -o controlador $(LDFLAGS)

agente: agente.c comun.h
	$(CC) $(CFLAGS) agente.c -o agente $(LDFLAGS)

clean:
	rm -f controlador agente fifo_* pipe_*