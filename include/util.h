#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdbool.h>


bool intInArray(int s, int *arr, size_t len)
{
    size_t i;

    for (i = 0; i < len; ++i)
        if (s == arr[i])
            return true;
    return false;
}

/* returns 1 with a probability of prob/tot
 * else returns 0 */
int randint(int prob, int tot)
{
    int x = tot+1;
    while (x > tot)
        x = rand()/((RAND_MAX + 1u)/tot);
    return x < prob ? 1 : 0;
}

#endif /* end of include guard: UTIL_H */
