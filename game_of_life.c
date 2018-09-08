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
    state current;
    TPixel *pix;
    struct Cell *u, *ur, *r, *dr, *d, *dl, *l, *ul;
} Cell;


bool isWhite(TPixel *);

void blackWhite(Tigr *);

void initCells(Cell *, Tigr *, int);

void nextGen(Cell *, Cell *);

void setNextState(Cell *) /*, int *, int *, int *, int *) */;

bool intInArray(int, int *, int *);


int main(int argc, char **argv)
{
    Tigr *init = tigrLoadImage("initial.png");
    if (!init) {
        fputs("Failed to open \"inital.png\". Terminating...", stderr);
        return -1;
    }

    blackWhite(init);
    Tigr *screen = tigrWindow(init->w, init->h, TITLE, 0);
    tigrBlit(screen, init, 0, 0, 0, 0, init->w, init->h);

    int tot = init->w * init->h;
    Cell *cells = malloc(tot * sizeof(Cell));
    initCells(cells, screen, tot);

    tigrUpdate(screen);
    Sleep(1000);

    while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
        int i = 0;
        for (Cell *cur = cells; cur != cells+tot; ++cur, ++i) {
            setNextState(cur) /* , &STAY_ALIVE[0], &STAY_ALIVE[2], &BE_BORN[0], &BE_BORN[1]) */;
            /* fprintf(stderr, "%d (%d, %d)\n", i, i%screen->w, i/screen->h); */
            /* if (!i%screen->w) */
            /*     fputc('\n', stderr); */
        }

        for (int i = 0; i < tot; ++i)
            screen->pix[i] = cells[i].current == ALIVE ? BLACK : WHITE;

        /* fputs("\n\n", stderr); */

        tigrUpdate(screen);
        Sleep(1000);
    }

    free(cells);
    tigrFree(init);
    return 0;
}


bool isWhite(TPixel *pix)
{
    return (pix->r==0xff && pix->g==0xff && pix->b==0xff && pix->a==0xff) ? true : false;
}

void blackWhite(Tigr *input)
{
    TPixel *data = input->pix;
    TPixel *end = data + input->w*input->h;

    for (; data != end; ++data) {
        if (!isWhite(data))
            *data = BLACK;
    }
}

void initCells(Cell *cells, Tigr *screen, int tot)
{
    /* width and height of cells */
    int w = screen->w, h = screen->h;

    /* iterate over all cells and set neighbours */
    for (int i = 0; i < tot; ++i) {
        /* set references to neighbouring cells NULL */
        cells[i].u = NULL, cells[i].ur = NULL, cells[i].r = NULL, cells[i].dr = NULL,
            cells[i].d = NULL, cells[i].dl = NULL, cells[i].l = NULL, cells[i].ul = NULL;

        /* special cases */
        if (i < w) {    /* upmost and downmost rows */
            cells[i].u = &cells[i+w*(h-1)];
            cells[i+w*(h-1)].d = &cells[i];
            cells[i].ur = &cells[i+w*(h-1)+1];
            cells[i].ul = &cells[i+w*(h-1)-1];
            cells[i+w*(h-1)].dr = &cells[i+1];
            cells[i+w*(h-1)].dl = &cells[i-1];
        }
        if (!i%w) {     /* leftmost and rightmost columns*/
            cells[i].l = &cells[i+w-1];
            cells[i+w-1].r = &cells[i];
            cells[i].ul = &cells[i-1];
            cells[i].dl = &cells[i+2*w-1];
            cells[i+w-1].ur = &cells[i-w];
            cells[i+w-1].dr = &cells[i+w];
        }
        if (i == 0) {   /* upper left and lower right */
            cells[i].ul = &cells[tot-1];
            cells[tot-1].dr = &cells[i];
        }
        if (i == w-1) { /* upper right and lower left */
            cells[i].ur = &cells[w*(h-1)];
            cells[w*(h-1)].dl = &cells[i];
        }

        /* set remaining neighbours */
        if (!cells[i].u)
            cells[i].u = &cells[i-w];
        if (!cells[i].ur)
            cells[i].ur = &cells[i-w+1];
        if (!cells[i].r)
            cells[i].r = &cells[i+1];
        if (!cells[i].dr)
            cells[i].dr = &cells[i+w+1];
        if (!cells[i].d)
            cells[i].d = &cells[i+w];
        if (!cells[i].dl)
            cells[i].dl = &cells[i+w-1];
        if (!cells[i].l)
            cells[i].l = &cells[i-1];
        if (!cells[i].ul)
            cells[i].ul = &cells[i-w-1];

        /* set pixel reference */
        cells[i].pix = &screen->pix[i];

        /* init current state */
        cells[i].current = isWhite(cells[i].pix) ? DEAD : ALIVE;
    }
}

void setNextState(Cell *cell) /*, int *alive_start, int *alive_end,
        int *born_start, int *born_end) */
{
    int alive = 0;
    alive = !isWhite(cell->u->pix) ? alive+1 : alive;
    alive = !isWhite(cell->ur->pix) ? alive+1 : alive;
    alive = !isWhite(cell->r->pix) ? alive+1 : alive;
    alive = !isWhite(cell->dr->pix) ? alive+1 : alive;
    alive = !isWhite(cell->d->pix) ? alive+1 : alive;
    alive = !isWhite(cell->dl->pix) ? alive+1 : alive;
    alive = !isWhite(cell->l->pix) ? alive+1 : alive;
    alive = !isWhite(cell->ul->pix) ? alive+1 : alive;

    /* if (alive) */
    /*     fprintf(stderr, "alive neighbours = %d\n", alive); */

    if (!isWhite(cell->pix) && !(alive == 2 || alive == 3)) {
        cell->current = DEAD;
    }
    else if (isWhite(cell->pix) && alive == 3) {
        cell->current = ALIVE;
    }
}

inline bool intInArray(int i, int *start, int *end)
{
    bool found = false;
    for (; start != end; ++start)
        found = (i == *start);
    return found;
}
