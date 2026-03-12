CC      = cc
CFLAGS  = -Wall -Wextra -std=c11

SRC     = src/main.c
BIN     = build/transcode

.PHONY: build clean

build: $(BIN)

$(BIN): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf build
