CFLAGS += -Wall -Wpedantic
LINK_FLAGS += -lGL -lGLEW -lSDL2 -lGLU
CC ?= gcc
BIN_NAME ?= 02
SRCS = main.c shader.c deps/*.c

all:
	$(CC) $(SRCS) $(LINK_FLAGS) $(CFLAGS) -o $(BIN_NAME)

debug: CFLAGS += -O0 -g -DDEBUG
debug: all

clean:
	rm -f ./*.o
	rm -f $(BIN_NAME)
