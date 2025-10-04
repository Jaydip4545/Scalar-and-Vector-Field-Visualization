# --- Compiler ---
CXX = g++
CXXFLAGS = -std=c++11 -Wall -I./src/

# --- Directories ---
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# --- Source Files ---
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# --- Detect platform ---
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    TARGET = $(BIN_DIR)/Visualizer
    LIBS = -lglfw -lGLEW -lGL -ldl
    EXE_EXT =
else ifeq ($(OS),Windows_NT)
    TARGET = $(BIN_DIR)/Visualizer.exe
    LIBS = -lglfw3 -lglew32 -lopengl32 -lgdi32
    EXE_EXT = .exe
else
    $(error Unsupported OS)
endif

# --- Default target ---
all: $(TARGET)

# --- Linking ---
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ -o $@ $(LIBS)
	@echo "Linking complete. Executable is at $(TARGET)"

# --- Compiling ---
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Compiled $<"

# --- Clean ---
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned build directories."

# --- Run ---
run: all
ifeq ($(EXE_EXT),.exe)
	./$(TARGET)
else
	./$(TARGET)
endif

.PHONY: all clean run
