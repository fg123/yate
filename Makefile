CXX = g++
EXE = yate
SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.cc)
OBJ = $(SRC:$(SRC_DIR)/%.cc=$(SRC_DIR)/%.o)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
$(info $(SRC))
$(info $(OBJ))
$(info $(HEADERS))
CPPFLAGS += -I $(SRC_DIR)
CFLAGS += -std=c++11 -g -Wall
LDLIBS = -lncurses

all: main


main: $(OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $(EXE)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


.PHONY: all clean

clean:
	rm -f $(SRC_DIR)/*.o *~ core $(SRC_DIR)/*~
