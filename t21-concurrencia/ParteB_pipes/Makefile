CC=gcc
CFLAGS=-O2 -Wall -Wextra
TARGETS=central sensor_temp sensor_humedad sensor_viento sensor_lluvia

all: $(TARGETS)

central: central.c
	$(CC) $(CFLAGS) $< -o $@

sensor_temp: sensor_temp.c
	$(CC) $(CFLAGS) $< -o $@

sensor_humedad: sensor_humedad.c
	$(CC) $(CFLAGS) $< -o $@

sensor_viento: sensor_viento.c
	$(CC) $(CFLAGS) $< -o $@

sensor_lluvia: sensor_lluvia.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)
