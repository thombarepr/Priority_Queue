GCC = gcc
INCLUDE_PATH = -I.
LIBS_PATH = -L.
LIBS = -lpthread

TEST_APP=thread
SRC=$(TEST_APP).c \
    pq.c

all: $(TEST_APP) 
$(TEST_APP): $(SRC)
	$(GCC) $(LIBS_PATH) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(TEST_APP)
	
