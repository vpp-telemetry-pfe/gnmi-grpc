CC=g++
CFLAGS=-Wall -Werror -O3
LDFLAGS=
SRC=src
INC=include
BUILD=build
TEST=test
MKDIR_P = mkdir -p
SERV_EXEC=helloworld
TEST_EXEC=test

all: server test

server: $(SRC)/main.cpp
	$(MKDIR_P) $(BUILD)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD)/$(SERV_EXEC) $^

test: $(TEST)/test.cpp
	$(MKDIR_P) $(TEST)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD)/$(TEST_EXEC) $^

clean:
	rm -rf $(BUILD)
