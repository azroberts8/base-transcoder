CC      = cc
CFLAGS  = -Wall -Wextra -std=c11

SRC     = src/main.c src/alphabet.c src/transcode.c
BIN     = build/transcode

.PHONY: build test clean

build: $(BIN)

$(BIN): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^

test: $(BIN)
	python3 tests/test_hello_binary_emoji.py

clean:
	rm -rf build
