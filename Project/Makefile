CC=gcc
CFLAGS=-O2 -std=c11 -Wall -Wextra
LDFLAGS=
PTHREAD=-lpthread

BIN=osproj
SRC=main.c scheduler.c memory.c disk.c sync.c shell.c

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC) $(PTHREAD) $(LDFLAGS)

clean:
	rm -f $(BIN)