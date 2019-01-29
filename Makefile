EXE = yate
SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.cc)
OBJ = $(SRC:$(SRC_DIR)/%.cc=$(SRC_DIR)/%.o)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
CPPFLAGS += -I $(SRC_DIR)
CFLAGS += -std=c++17 -g -Wall
LDLIBS = -lncurses -lstdc++fs

all: main

main: $(OBJ)
	mkdir -p bin
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o bin/$(EXE)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


.PHONY: all clean

clean:
	rm -f $(SRC_DIR)/*.o *~ core $(SRC_DIR)/*~
