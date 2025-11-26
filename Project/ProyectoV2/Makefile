all: controlador agente

controlador: controlador.c
	gcc controlador.c -o controlador -pthread

agente: agente.c
	gcc agente.c -o agente -pthread

clean:
	rm -f controlador agente fifo_* pipe_*