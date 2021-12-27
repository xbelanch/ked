CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -pedantic -ggdb
LIBS := `pkg-config --cflags --libs sdl2` -lm
rogueban: rogueban.c
	$(CC) $(CFLAGS) -o rogueban rogueban.c $(LIBS)