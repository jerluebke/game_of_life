#include <stdio.h>
#include <string.h>
#include <time.h>
#include "tigr.h"
#include "parg.h"
#include "util.h"

#define HELP "Random Black-White Bitmap Generator\nUsage: randpng [-h] "\
    "[-p PROB (parts per 10,000)] [-w WIDTH] [-l HEIGHT] [-f FILENAME]"


const TPixel BLACK = {.r=0x00, .g=0x00, .b=0x00, .a=0xff};
const TPixel WHITE = {.r=0xff, .g=0xff, .b=0xff, .a=0xff};


int main(int argc, char **argv)
{
    int w = 100, h = 100, prob = 10000-3125, total = 10000;
    char fn[100] = "rand.png";
    Tigr *bmp;

    struct parg_state ps;
    int c, err;
    parg_init(&ps);

    while ((c = parg_getopt(&ps, argc, argv, "hp:w:l:f:")) != -1) {
        switch (c) {
        case 'h':
            printf(HELP);
            return 0;
        case 'p':
            prob = 10000-atoi(ps.optarg);
            if (prob == 10000) {
                fputs("probability not understood...", stderr);
                return -1;
            }
            break;
        case 'w':
            w = atoi(ps.optarg);
            break;
        case 'l':
            h = atoi(ps.optarg);
            break;
        case 'f':
            err = strncpy_s(fn, 100, ps.optarg, 100);
            if (err) {
                fputs("failed to read filename from arguments...", stderr);
                return -1;
            }
            break;
        case '?':
            if (ps.optopt == 'p')
                break;  /* using default */
        default:    /* FALLTHRU! */
            printf(HELP);
            return -1;
        }
    }
    if (!w || !h) {
        fputs("failed to read width or height from arguments...", stderr);
        return -1;
    }

    bmp = tigrBitmap(w, h);
    TPixel *data = bmp->pix, *end = bmp->pix + w*h;
    while (data != end)
        *data++ = randint(prob, total) ? BLACK : WHITE;
    tigrSaveImage(fn, bmp);
    tigrFree(bmp);

    return 0;
}
