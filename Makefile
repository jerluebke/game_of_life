CFLAGS = -I./include
LDFLAGS = -ld3d9

gol : ./src/game_of_life.c ./src/tigr.c ./src/parg.c
	gcc $^ -O3 -o $@ $(CFLAGS) $(LDFLAGS)
