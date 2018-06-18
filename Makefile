CXX = g++
EXE = yate
SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.cc)
OBJ = $(SRC:$(SRC_DIR)/%.cc=$(SRC_DIR)/%.o)
$(info $(SRC))
$(info $(OBJ))
CPPFLAGS += -I $(SRC_DIR)
CFLAGS += -std=c++11 -g -Wall
LDLIBS = -lncurses

all: main


main: $(OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $(EXE)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc $(wildcard $(SRC)/*.h)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


.PHONY: all clean

clean:
	rm -f $(SRC_DIR)/*.o *~ core $(SRC_DIR)/*~
