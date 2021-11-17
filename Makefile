CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -pedantic -ggdb
SDL2 := `pkg-config --cflags --libs sdl2`
rogueban: rogueban.c
	$(CC) $(CFLAGS) -c rogueban.c
	$(CC) $(SDL2) rogueban.o -o rogueban
	./rogueban
clean:
	rm *.o