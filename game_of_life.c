#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include "tigr.h"

#define TITLE "Conway's Game of Life"

const TPixel BLACK = {.r=0x00, .g=0x00, .b=0x00, .a=0xff};
const TPixel WHITE = {.r=0xff, .g=0xff, .b=0xff, .a=0xff};
int BE_BORN[] = {3, '\0'};
int STAY_ALIVE[] = {2, 3, '\0'};


typedef enum state {
    DEAD, ALIVE
} state;

typedef struct Cell {
    state isAlive;
    TPixel *pix;
    struct Cell *u, *ur, *r, *dr, *d, *dl, *l, *ul;
} Cell;


bool isWhite(TPixel *);

void initCells(Cell *, Tigr *, int);

void setNextState(Cell *) /*, int *, int *, int *, int *) */;

bool intInArray(int, int *, int *);


int main(int argc, char **argv)
{
    Tigr *init = tigrLoadImage("initial.png");
    if (!init) {
        fputs("Failed to open \"inital.png\". Terminating...", stderr);
        return -1;
    }

    Tigr *screen = tigrWindow(init->w, init->h, TITLE, 0);
    tigrBlit(screen, init, 0, 0, 0, 0, init->w, init->h);

    int tot = init->w * init->h;
    Cell *cells = malloc(tot * sizeof(Cell));
    initCells(cells, screen, tot);

    while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
        for (Cell *cur = cells; cur != cells+tot; ++cur)
            setNextState(cur) /* , &STAY_ALIVE[0], &STAY_ALIVE[2], &BE_BORN[0], &BE_BORN[1]) */;

        for (int i = 0; i < tot; ++i)
            screen->pix[i] = cells[i].isAlive ? BLACK : WHITE;

        tigrUpdate(screen);
        Sleep(500);
    }

    free(cells);
    tigrFree(init);
    return 0;
}


bool isWhite(TPixel *pix)
{
    return (pix->r==0xff && pix->g==0xff && pix->b==0xff && pix->a==0xff) ? true : false;
}

void initCells(Cell *cells, Tigr *screen, int tot)
{
    /* width and height of cells */
    int w = screen->w, h = screen->h;
    int i;

    /* iterate over all cells and set neighbours */
    for (i = 0; i < tot; ++i) {
        /* set neighbours */
        cells[i].u = &cells[i-w];
        cells[i].ur = &cells[i-w+1];
        cells[i].r = &cells[i+1];
        cells[i].dr = &cells[i+w+1];
        cells[i].d = &cells[i+w];
        cells[i].dl = &cells[i+w-1];
        cells[i].l = &cells[i-1];
        cells[i].ul = &cells[i-w-1];

        /* set pixel reference */
        cells[i].pix = &screen->pix[i];

        /* init current state */
        cells[i].isAlive = isWhite(cells[i].pix) ? DEAD : ALIVE;
    }

    /* overwrite special cases */
    /* upmost and downmost rows */
    for (i = 0; i < w; ++i) {
        cells[i].u = &cells[i+w*(h-1)];
        cells[i+w*(h-1)].d = &cells[i];
        cells[i].ur = &cells[i+w*(h-1)+1];
        cells[i].ul = &cells[i+w*(h-1)-1];
        cells[i+w*(h-1)].dr = &cells[i+1];
        cells[i+w*(h-1)].dl = &cells[i-1];
    }

    /* leftmost and rightmost columns*/
    for (i = 0; i < tot; i+=w) {
        cells[i].l = &cells[i+w-1];
        cells[i+w-1].r = &cells[i];
        cells[i].ul = &cells[i-1];
        cells[i].dl = &cells[i+2*w-1];
        cells[i+w-1].ur = &cells[i-w];
        cells[i+w-1].dr = &cells[i+w];
    }

    /* upper left and lower right corner*/
    cells[0].ul = &cells[tot-1];
    cells[tot-1].dr = &cells[0];
    /* upper right and lower left corner */
    cells[w-1].ur = &cells[w*(h-1)];
    cells[w*(h-1)].dl = &cells[w-1];
}

void setNextState(Cell *cell) /*, int *alive_start, int *alive_end,
        int *born_start, int *born_end) */
{
    int alive = 0;
    alive += cell->u->isAlive ? 1 : 0;
    alive += cell->ur->isAlive ? 1 : 0;
    alive += cell->r->isAlive ? 1 : 0;
    alive += cell->dr->isAlive ? 1 : 0;
    alive += cell->d->isAlive ? 1 : 0;
    alive += cell->dl->isAlive ? 1 : 0;
    alive += cell->l->isAlive ? 1 : 0;
    alive += cell->ul->isAlive ? 1 : 0;

    if (cell->isAlive && !(alive == 2 || alive == 3))
        cell->isAlive = DEAD;
    else if (!cell->isAlive && alive == 3)
        cell->isAlive = ALIVE;
}

inline bool intInArray(int i, int *start, int *end)
{
    bool found = false;
    for (; start != end; ++start)
        found = (i == *start);
    return found;
}
