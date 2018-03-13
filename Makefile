GCC = gcc
INCLUDE_PATH = .
LIB_PATH = .
LIBS = -lpthread
SRC=$(wildcard *.c)

all: pq-test
pq-test: $(SRC)
	$(GCC) $(LIBS) -o $@ $^

clean:
	rm -rf *.o pq-test
	
