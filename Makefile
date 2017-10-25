.PHONY : all clean run new

MPIROOT = /usr

CFLAGS+= -Wall -g $(INCL)
CC=gcc
MPICC=  $(MPIROOT)/bin/mpic++
INCL= -I$(MPIROOT)/include
SRCS= consola.cpp main.cpp nodo.cpp HashMap.cpp base.cpp
BIN= dist_hashmap

all: dist_hashmap

$(BIN): $(SRCS)
	$(MPICC) $(CFLAGS) -o $(BIN) $(SRCS)
test-1-run:
	for i in $$(seq 2 10); do echo Corriendo con np=$$i; cat test-1.txt | mpiexec -np $$i ./dist_hashmap; for j in $$(seq 1 $$((i-1)) ); do cat rank$${j}.log | grep corpus>/dev/null; if [ "$$?" != "0" ]; then echo Error: el nodo $$j no ha procesado ningun archivo;	exit 1;	fi; done; done

test-2-run:
	awk -f corpus.awk corpus | sort -nk 2 | tail -n 1 >corpus-max
	cat corpus-max
	for i in 0 1 2 3 4; do sed -n "$$((i * 500 + 1)),$$(((i + 1) * 500))p" corpus >corpus-"$$i"; done
	for i in 1 2 3 4 5; do cat test-2.txt | mpiexec -np $$((i + 1)) ./dist_hashmap | sort | diff -u - corpus-max; done
	rm -f corpus-max corpus-[0-4]
test-3-run:
	cat test-3.txt | mpiexec -np 6 ./dist_hashmap | diff -u - test-3-resultado.txt
test-4-run:
	awk -f corpus.awk corpus50 | sort >corpus-post
	cat test-4.txt | mpiexec -np 6 ./dist_hashmap | sort | diff -u - corpus-post
	rm -f corpus-post
test-5-run:
	for i in 0 1 2 3 4; do sed -n "$$((i * 2 + 1)),$$(((i + 1) * 2))p" corpus >corpus-"$$i"; done
	cat 5-resultado.txt | sort > test-5-resultado.txt
	for i in 1; do cat test-5.txt | mpiexec -np $$((i + 1)) ./dist_hashmap | sort | diff -u - test-5-resultado.txt; done
clean:
	rm -f $(BIN)
	rm -f *.log

new: clean all
