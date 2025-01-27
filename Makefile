CXX = clang++
CXXFLAGS = -Wall -g -I$(SRC_DIR)
GTEST_FLAGS = -lgtest -lgtest_main -pthread

SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Separate main source file from other source files
MAIN_SRC = $(SRC_DIR)/ash.cpp
LIB_SRC = $(filter-out $(MAIN_SRC),$(wildcard $(SRC_DIR)/*.cpp))
TEST_FILES = $(wildcard $(TEST_DIR)/*.cpp)

# Generate object files for library sources (excluding main)
LIB_OBJ = $(LIB_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
MAIN_OBJ = $(MAIN_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJ = $(TEST_FILES:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

TEST_BINS = $(TEST_FILES:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/%)

# Main executable
MAIN = $(BUILD_DIR)/ash

# Default target - build everything
all: $(BUILD_DIR) $(MAIN) $(TEST_BINS)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test files
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build main executable
$(MAIN): $(MAIN_OBJ) $(LIB_OBJ)
	$(CXX) $^ -o $@

# Link test executables (using only library objects, not main)
$(BUILD_DIR)/%: $(OBJ_DIR)/%.o $(LIB_OBJ)
	$(CXX) $^ -o $@ $(GTEST_FLAGS)

# Run all tests
test: $(TEST_BINS)
	for test in $(TEST_BINS) ; do ./$$test ; done

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
