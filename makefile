# -----------------------------
# Simple Makefile for VCS Project
# -----------------------------

# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Source and Build Directories
SRC_DIR = src
BUILD_DIR = build

# List of all source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Create a list of object files for each source file
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Output executable name
TARGET = $(BUILD_DIR)/vcs

# Default rule
all: $(TARGET)

# Rule for linking object files into final executable
$(TARGET): $(OBJS)
	@echo Linking...
	$(CXX) $(OBJS) -o $(TARGET)
	@echo Build complete: $(TARGET)

# Rule for compiling each .cpp into .o file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo Compiling $<...
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build directory if it doesn’t exist
$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

# Clean command to remove compiled files (Windows-compatible)
clean:
	@if exist "$(BUILD_DIR)" ( \
		del /Q "$(BUILD_DIR)\*.o" 2>nul && \
		del /Q "$(BUILD_DIR)\vcs.exe" 2>nul && \
		rmdir /S /Q "$(BUILD_DIR)" 2>nul \
	) \
	else ( \
		echo No build directory found. \
	)
	@echo Cleaned build directory.
