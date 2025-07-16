CXX = g++
CC  = $(CXX)

# Generate dependencies in *.d files
DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

# Define preprocessor, compiler, and linker flags
CPPFLAGS =  -I.
CXXFLAGS =  -O3 -Wall -Wextra -pedantic-errors -Wold-style-cast 
CXXFLAGS += -std=c++17 -march=native
CXXFLAGS += -g
CXXFLAGS += $(DEPFLAGS)
LDFLAGS =   -g 

# Remove -Werror to prevent warnings from stopping the build
# If you still want strict checks but no build break, use:
# CXXFLAGS += -Wno-error

# Libraries for GUI (raylib and pthread)
LIBS_GUI = -lraylib -lpthread

# Targets
PROGS = othello othello_gui

all: $(PROGS)

# Compile othello (main.cpp)
othello: main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile othello_gui (gui.cpp)
othello_gui: gui.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS_GUI)

# Run programs
run_othello: othello
	./othello

run_gui: othello_gui
	./othello_gui

# Phony targets
.PHONY: all test clean distclean run_othello run_gui

# Standard clean
clean:
	rm -f *.o $(PROGS)

distclean: clean
	rm -f *.d

# Automatically generate dependencies
SRC = $(wildcard *.cpp)
-include $(SRC:.cpp=.d)




# If you want to compile and run the programs in one line, use:
# g++ -O3 -std=c++17 -march=native gui.cpp -o othello_gui -lraylib -lpthread && ./othello_gui
# g++ -O3 -std=c++17 -march=native main.cpp -o othello && ./othello