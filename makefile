# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -I./src/

# Libraries
LIBS = -lglfw -lGLEW -lGL -ldl

# --- Directories ---
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
TARGET = $(BIN_DIR)/Visualizer

# --- Source Files ---
# Automatically find all .cpp files in the src directory
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
# Create a list of object files in the obj directory
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# --- Rules ---

# Default rule: build everything
all: $(TARGET)

# Rule to link the program
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ -o $@ $(LIBS)
	@echo "Linking complete. Executable is at $(TARGET)"

# Rule to compile .cpp files into .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Compiled $<"

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned build directories."

# A phony target to run the program
run: all
	./$(TARGET)

.PHONY: all clean run
