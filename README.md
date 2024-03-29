OVERVIEW
========

This is the source code of several heuristic search algorithms and test
domains used for the experiments described in the AAAI-10 paper ``Searching
Without a Heuristic: Efficient Use of Abstraction'' by Bradford Larsen, Ethan
Burns, Wheeler Ruml, and Robert Holte.

The test domains include the fifteen puzzle, the macro fifteen puzzle, the
glued fifteen puzzle, and the fourteen pancake puzzle.  The algorithms include
A*, Hierarchical A*, IDA*, Hierarchical IDA*, and Switchback.  Instances for
these domains are found in the `testdata` subdirectory.


BUILDING INSTRUCTIONS
---------------------

The code is written in C++ and has been built succcessfully on several flavors
of Linux, as well as Mac OS X 10.6 and 10.7.

On a UNIX-like system with the GCC toolchain and an internet connection, you
will hopefully have good luck simply typing `make` in the top-level directory.
This will produce a binary named `search` in the top-level directory.


USAGE
-----

To run the code one must specify the search domain, algorithm, and input.  For
example, to solve instance number 55 of the fifteen puzzle:

    $ ./search tiles astar testdata/korf100/55
    ######## The Instance ########
    Initial state:
    13  8       14      3
    9   1       0       7
    15  5       4       10
    12  2       6       11


    Goal state:
    0   1       2       3
    4   5       6       7
    8   9       10      11
    12  13      14      15


    Initial Manhattan distance heuristic estimate: 29


    ######## Search Results ########
    found a solution:
    0   1       2       3
    4   5       6       7
    8   9       10      11
    12  13      14      15

    f: 41
    g: 41
    h: 0

    expanded: 151978 (180926/s)
    generated: 306960 (365428/s)
    time: 0.84 s
    137049 nodes in open at end of search
    289028 nodes in closed at end of search

Run the program without arguments for usage instructions.


CONTACT INFORMATION
-------------------

Contact Bradford Larsen at brad.larsen@gmail.com with questions or comments.
