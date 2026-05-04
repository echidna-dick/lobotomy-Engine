CC = gcc
SRC = src/main.c src/card.c src/deck.c src/hand.c
OUT = game.exe
INCLUDES = -Iinclude -Isrc
LIBS = -Llib -lraylib -lopengl32 -lgdi32 -lwinmm

all:
	$(CC) $(SRC) $(INCLUDES) $(LIBS) -o $(OUT)