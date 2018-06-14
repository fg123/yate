CC=g++
SRC=src
CFLAGS=-I $(SRC) -lncurses 

_DEPS = $(SRC)/*.h
DEPS = $(patsubst %,$(SRC)/%,$(_DEPS))

_OBJ = main.o
OBJ = $(patsubst %,$(SRC)/%,$(_OBJ))


$(SRC)/%.o: $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o yate $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(SRC)/*.o *~ core $(SRC)/*~ 
