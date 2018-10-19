CC=g++
CFLAGS=-Wall -Werror -O3
LDFLAGS=
SRC=src
INC=include
BUILD=build
MKDIR_P = mkdir -p
EXEC=helloworld

all: $(SRC)/main.cpp
	$(MKDIR_P) build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD)/$(EXEC) $^

clean:
	rm -rf $(BUILD)
