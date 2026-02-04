CC = gcc
CFLAGS = -Wall -pthread
BIN_DIR = bin
SRC_CMD = src/commands

TARGETS = server_bin client_bin $(BIN_DIR)/hola $(BIN_DIR)/juego $(BIN_DIR)/v21

all: $(BIN_DIR) $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

server_bin: src/server/main.c
	$(CC) $(CFLAGS) -o server_bin src/server/main.c

client_bin: src/client/main.c
	$(CC) $(CFLAGS) -o client_bin src/client/main.c

$(BIN_DIR)/hola: $(SRC_CMD)/hola.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/hola $(SRC_CMD)/hola.c

$(BIN_DIR)/juego: $(SRC_CMD)/juego.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/juego $(SRC_CMD)/juego.c

$(BIN_DIR)/v21: $(SRC_CMD)/v21.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/v21 $(SRC_CMD)/v21.c

clean:
	rm -rf server_bin client_bin $(BIN_DIR)