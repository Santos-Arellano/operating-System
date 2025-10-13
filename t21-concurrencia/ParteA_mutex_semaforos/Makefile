CC=gcc
CFLAGS=-O2 -Wall -Wextra -pthread
TARGET=climate_threads

all: $(TARGET)

$(TARGET): climate_threads.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET)
