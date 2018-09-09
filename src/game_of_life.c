#include <stdio.h>
#include <time.h> /* to seed rand */
#include <windows.h>
#include "tigr.h"
#include "parg.h"
#include "util.h" /* includes stdlib.h, stdbool.h */

#define TITLE "Conway's Game of Life"
#define HELP "Conway's Game of Life\nUsage: gol [-h] [-a STAY_ALIVE] [-b BE_BORN] "\
             "[-d DELAY (1...1000)] [-f FILENAME] [-s SIZE (0, 2, 4, 8)] [-r ALIVE_PROB (parts per 10,000)]"

const TPixel BLACK = {.r=0x00, .g=0x00, .b=0x00, .a=0xff};
const TPixel WHITE = {.r=0xff, .g=0xff, .b=0xff, .a=0xff};


typedef enum state {
    DEAD, ALIVE
} state;

typedef struct Cell {
    state isAlive;
    TPixel *pix;
    struct Cell *u, *ur, *r, *dr, *d, *dl, *l, *ul;
} Cell;

typedef struct Rule {
    /* use pointer to arrays, because arrays will be allocated dynamically */
    int **a, **b;
    size_t a_len, b_len;
} Rule;


static inline bool isWhite(TPixel *);

void initCells(Cell *, Tigr *, int);

bool setNextState(Cell *, Rule *);


int main(int argc, char **argv)
{
    srand(time(NULL));

    /* stay alive, be born, filename (default values) */
    char ac[8] = "23", bc[8] = "3", fn[100] = "initial.png";

    int w = 100, h = 100, delay = 100, size = 8;
    int prob_alive = 10000-3125, total = 10000;
    bool random = false;
    Tigr *init;


    /* parsing argv */
    struct parg_state ps;
    int c, err;
    parg_init(&ps);

    while ((c = parg_getopt(&ps, argc, argv, "ha:b:d:f:s:r:w:l:")) != -1) {
        switch (c) {
        case 'h':
            printf(HELP);
            return EXIT_SUCCESS;
        case 'a':
            strncpy_s(ac, 8, ps.optarg, 8);
            break;
        case 'b':
            strncpy_s(bc, 8, ps.optarg, 8);
            break;
        case 'd':
            delay = atoi(ps.optarg);
            if (!(1 <= delay && delay <= 1000)) {
                fputs("delay must be in [1, 1000] (milisecs) ...", stderr);
                return EXIT_FAILURE;
            }
            break;
        case 'f':
            err = strncpy_s(fn, 100, ps.optarg, 100);
            if (err) {
                fputs("failed to read filename from arguments...", stderr);
                return EXIT_FAILURE;
            }
            break;
        case 's':
            size = atoi(ps.optarg);
            if (!(size == 0 || size == 2 || size == 4 || size == 8)) {
                /* pixel size: auto, double, three times, four times */
                fputs("pixelsize must be in {0, 2, 4, 8} ...", stderr);
                return EXIT_FAILURE;
            }
            break;
        case 'r':
            random = true;
            prob_alive = 10000-atoi(ps.optarg);
            if (prob_alive >= 10000 || prob_alive < 0) {
                fputs("probability not understood...", stderr);
                return EXIT_FAILURE;
            }
            break;
        case 'w':
            w = atoi(ps.optarg);
            break;
        case 'l':
            h = atoi(ps.optarg);
            break;
        default: /* '?'*/
            printf(HELP);
            return EXIT_FAILURE;
        }
    }
    if (!w || !h) {
        fputs("width and height couldn't be read from arguments...", stderr);
        return EXIT_FAILURE;
    }

    /* set up rule */
    int i;

    size_t a_len = strlen(ac);
    int *a = malloc(a_len * sizeof(*a));
    for (i = 0; ac[i] != '\0'; ++i)
        a[i] = ac[i] -'0';

    size_t b_len = strlen(bc);
    int *b = malloc(b_len * sizeof(*b));
    for (i = 0; bc[i] != '\0'; ++i)
        b[i] = bc[i] -'0';

    Rule rule = { &a, &b, a_len, b_len };
    printf("Using Rule %s/%s\nInitial conditions: %s\n", ac, bc, random ? "random" : fn);


    /* read inital conditions and start display */
    if (random) {
        init = tigrBitmap(w, h);
        TPixel *data = init->pix, *end = init->pix + w*h;
        while (data != end)
            *data++ = randint(prob_alive, total) ? BLACK : WHITE;
        /* tigrSaveImage("rand.png", init); */
        /* return 0; */
    }
    else {
        init = tigrLoadImage(fn);
        if (!init) {
            fprintf(stderr, "Failed to open \"%s\". Terminating...", fn);
            return EXIT_FAILURE;
        }
        w = init->w, h = init->h;
    }


    Tigr *screen = tigrWindow(w, h, TITLE, 0);
    tigrBlit(screen, init, 0, 0, 0, 0, w, h);
    /* tigrFill(screen, 0, 0, init->w, init->h, WHITE); */

    /* alloc and init cells */
    int tot = w * h;
    Cell *cells = malloc(tot * sizeof(*cells));
    initCells(cells, screen, tot);

    /* mainloop */
    int iter = 0;
    bool changed;
    while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
        changed = false;
        for (Cell *cur = cells; cur != cells+tot; ++cur)
            changed = setNextState(cur, &rule) || changed;

        for (int i = 0; i < tot; ++i)
            screen->pix[i] = cells[i].isAlive ? BLACK : WHITE;

        tigrUpdate(screen);
        Sleep(delay);
        ++iter;
        printf("Iteration: %d\r", iter);

        if (!changed) {
            printf("\n\nA static configuration was reached after %d iterations\n\n", iter);
            system("PAUSE");
            break;
        }
    }

    free(a);
    free(b);
    free(cells);
    tigrFree(screen);
    tigrFree(init);

    return EXIT_SUCCESS;
}


static inline bool isWhite(TPixel *pix)
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

bool setNextState(Cell *cell, Rule *rule)
{
    bool changed = false;
    int alive = 0;
    /* it is requiered to ask for the color of the pixel and not for the
     * (during this iteration propably changed) state `isAlive` */
    alive += !isWhite(cell->u->pix) ? 1 : 0;
    alive += !isWhite(cell->ur->pix) ? 1 : 0;
    alive += !isWhite(cell->r->pix) ? 1 : 0;
    alive += !isWhite(cell->dr->pix) ? 1 : 0;
    alive += !isWhite(cell->d->pix) ? 1 : 0;
    alive += !isWhite(cell->dl->pix) ? 1 : 0;
    alive += !isWhite(cell->l->pix) ? 1 : 0;
    alive += !isWhite(cell->ul->pix) ? 1 : 0;

    if (cell->isAlive && !intInArray(alive, *(rule->a), rule->a_len)) {
        cell->isAlive = DEAD;
        changed = true;
    } else if (!cell->isAlive && intInArray(alive, *(rule->b), rule->b_len)) {
        cell->isAlive = ALIVE;
        changed = true;
    }

    return changed; 
}
