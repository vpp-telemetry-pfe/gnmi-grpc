CC=g++
CFLAGS=-Wall -Werror -O3
LDFLAGS=
SRC=src
INC=include
BUILD=build
EXEC=helloworld

all: $(SRC)/main.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD)/$(EXEC) $^

clean:
	rm $(BUILD)/*
