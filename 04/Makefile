CFLAGS += -Wall -Wpedantic
LINK_FLAGS += -lGL -lGLEW -lSDL2 -lGLU -lm
CC ?= gcc
BIN_NAME ?= 04
SRCS = main.c shader.c camera.c deps/*.c

all:
	$(CC) $(SRCS) $(LINK_FLAGS) $(CFLAGS) -o $(BIN_NAME)

debug: CFLAGS += -O0 -g -DDEBUG
debug: all

clean:
	rm -f ./*.o
	rm -f $(BIN_NAME)
