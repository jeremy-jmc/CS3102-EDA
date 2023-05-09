# Variables
CXX = g++
CXXFLAGS = -Wall -std=c++11
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system
OUTPUT = space_filling
BUILDDIR = build

# Default target
all: $(BUILDDIR) $(OUTPUT)

# Create the build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile and link
$(OUTPUT): $(BUILDDIR)/main.o
	$(CXX) $(CXXFLAGS) $(BUILDDIR)/main.o -o $(OUTPUT) $(SFML_LIBS)

# Compile main.cpp
$(BUILDDIR)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o $(BUILDDIR)/main.o

# Clean up the build
.PHONY: clean
clean:
	rm -rf $(BUILDDIR) $(OUTPUT)

# Run the program
.PHONY: run
run: $(OUTPUT)
	./$(OUTPUT)
