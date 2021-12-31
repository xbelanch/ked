CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -pedantic -ggdb
LIBS := `pkg-config --cflags --libs sdl2` -lm

rogueban: rogueban.c v2.c
	$(CC) $(CFLAGS) -c rogueban.c v2.c
	$(CC) rogueban.o v2.o $(LIBS) -o rogueban