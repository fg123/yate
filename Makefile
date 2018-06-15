CXX=g++
CXXFLAGS= -std=c++11
SRC=src
CFLAGS= -I $(SRC) -lncurses

_DEPS = $(SRC)/*.h
DEPS = $(patsubst %,$(SRC)/%,$(_DEPS))

_OBJ = main.o yate.o tab-set.o pane-set.o
OBJ = $(patsubst %,$(SRC)/%,$(_OBJ))


$(SRC)/%.o: $(SRC)/%.c $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o yate $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(SRC)/*.o *~ core $(SRC)/*~
