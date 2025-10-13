CC=gcc
CFLAGS=-O2 -Wall -Wextra
TARGETS=central_signal sensor_temp_sig sensor_humedad_sig sensor_viento_sig sensor_lluvia_sig

all: $(TARGETS)

central_signal: central_signal.c
	$(CC) $(CFLAGS) $< -o $@

sensor_temp_sig: sensor_temp_sig.c
	$(CC) $(CFLAGS) $< -o $@

sensor_humedad_sig: sensor_humedad_sig.c
	$(CC) $(CFLAGS) $< -o $@

sensor_viento_sig: sensor_viento_sig.c
	$(CC) $(CFLAGS) $< -o $@

sensor_lluvia_sig: sensor_lluvia_sig.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)
