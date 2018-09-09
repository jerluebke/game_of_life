Conway's Game of Life
===
Written for Windows.  
Build with  

    make gol

For the graphical representation I am using Richard Mitton's [TIGR](https://bitbucket.org/rmitton/tigr/src).  
For parsing commandline arguments I am using jibsen's [parg](https://github.com/jibsen/parg).  

The field is designed as a torus. You can load your own initial conditions by
supplying it as a `.png`-bitmap with `-f FILENAME`.  

    Usage: gol [-h] [-a STAY_ALIVE] [-b BE_BORN] [-d DELAY (1...1000)] [-f FILENAME]
        [-s SIZE (0, 2, 4, 8)] [-r ALIVE_PROB (parts per 10,000)]

