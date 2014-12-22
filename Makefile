ECHO=/bin/echo
CC=gcc
CFLAGS=-g -std=gnu99 -Wall -Werror
OBJ=allocation_bench.o
CSV=allocation_bench.csv
PLOT=allocation_bench.gnu

uname_S := $(shell sh -c 'uname -s 2>/dev/null || $(ECHO) not')

ifeq ($(uname_S),$(filter $(uname_S),Linux FreeBSD))
	LIBS+=-lrt
endif

ITERATIONS=250
DATA=$(addprefix bench_,1K 2K 4K 8K 16K 32K 64K 128K 256K 512K 1M 2M 4M 8M 16M 32M 64M)
PROGRAM=allocation_bench

.SECONDARY:

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROGRAM): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

all: $(PROGRAM)

random_%:
	-@dd if=/dev/urandom bs=$(shell $(ECHO) $(@:random_%=%) | tr A-Z a-z) count=1 >$@ 2>&1

bench_%: random_%
	@$(ECHO) -n " $*"
	@$(ECHO) -n $* >>$(CSV)
	@$(ECHO) -n " " >>$(CSV)
	@$(ECHO) -n $(shell ./$(PROGRAM) 1 $(ITERATIONS) $< | sed -e 's/^.*:[ ]*//' -e 's/ .*//') >>$(CSV)
	@$(ECHO) -n " " >>$(CSV)
	@$(ECHO) -n $(shell ./$(PROGRAM) 2 $(ITERATIONS) $< | sed -e 's/^.*:[ ]*//' -e 's/ .*//') >>$(CSV)
	@$(ECHO) -n " " >>$(CSV)
	@$(ECHO) $(shell ./$(PROGRAM) 3 $(ITERATIONS) $< | sed -e 's/^.*:[ ]*//' -e 's/ .*//') >>$(CSV)

.PHONY: pre_bench
pre_bench: all
	@$(ECHO) -n "benchmarks:"
	@$(ECHO) "bytes malloc malloc/memset calloc" >$(CSV)

bench: pre_bench $(DATA)
	@$(ECHO)

svg: bench $(PLOT)
	gnuplot $(PLOT)

clean:
	rm -rf *.dSYM
	rm -f *.svg random_* $(CSV)
	rm -f $(PROGRAM) $(OBJ)
