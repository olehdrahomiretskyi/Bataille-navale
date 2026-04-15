TARGET = bin/battleship
CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -I. $(shell pkg-config --cflags sdl2) -lm
LDFLAGS= $(shell pkg-config --libs sdl2) -lSDL2_ttf
SRC    = src/main.c src/logic.c src/render.c

all:
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
