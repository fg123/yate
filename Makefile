EXE = yate
SRC_DIR = src
SRC = $(shell find src/ -name "*.cc") src/syntax-lookup.cc
OBJ = $(SRC:%.cc=%.o)
HEADERS = $(shell find src/ -name "*.h")
CPPFLAGS += -I $(SRC_DIR) -I $(SRC_DIR)/prompts
CFLAGS += -g -std=c++11 -Wall -fmax-errors=5
LDLIBS = -L/usr/lib -lncurses -lstdc++fs -ltinfo

all: main

main: $(OBJ)
	mkdir -p bin
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o bin/$(EXE)

%.o: %.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

src/syntax-lookup.cc: utils/generate-syntax-lookup $(SRC_DIR)/syntax/*.cc
	utils/generate-syntax-lookup > src/syntax-lookup.cc

.PHONY: all clean release

clean:
	find . -name "*.o" -type f -delete
	rm -f *~ core $(SRC_DIR)/*~
