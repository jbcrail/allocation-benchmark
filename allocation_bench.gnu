set terminal svg size 1024,768 dynamic enhanced fname 'arial' fsize 10 name "memory_allocation" butt solid
set key bmargin center horizontal Left reverse noenhanced autotitle columnheader nobox
set output 'allocation_bench.svg'
set title "Memory Allocation"
set auto x
set ylabel "ns/op"
set grid y
set logscale y
set style data histogram
set style histogram cluster gap 1 title offset 0,-0.5
set style fill solid border -1
set boxwidth 0.9
set xtics nomirror rotate by -45 font ",8" offset -1
plot newhistogram "", 'allocation_bench.csv' using 2:xtic(1) t col, '' u 3 t col, '' u 4 t col
