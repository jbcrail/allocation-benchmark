# allocation-benchmark

A simple benchmark of memory allocation performance intended to compare
several allocation/initialization strategies, including malloc,
malloc/memset, and calloc. A SVG chart of the results can be generated.

This is not intended as a production-quality benchmark. It was built to
explore if malloc/memset is slower than calloc.

To benchmark:

    make bench

To benchmark more than the default iterations (250):

    make bench ITERATIONS=1000

To generate a SVG chart:

    make svg
