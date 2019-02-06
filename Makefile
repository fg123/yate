EXE = yate
SRC_DIR = src
SRC = $(shell find . -name "*.cc")
OBJ = $(SRC:%.cc=%.o)
HEADERS = $(shell find . -name "*.h")
CPPFLAGS += -I $(SRC_DIR) -I $(SRC_DIR)/prompts
CFLAGS += -std=c++17 -g -Wall
LDLIBS = -L/usr/lib -lncurses -lstdc++fs

all: main

main: $(OBJ)
	mkdir -p bin
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o bin/$(EXE)

%.o: %.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


.PHONY: all clean

clean:
	find . -name "*.o" -type f -delete
	rm -f *~ core $(SRC_DIR)/*~
