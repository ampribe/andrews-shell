CXX = clang++
CXXFLAGS = -Wall -g -I$(SRC_DIR)
GTEST_FLAGS = -lgtest -lgtest_main -pthread

SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
TEST_FILES = $(wildcard $(TEST_DIR)/*.cpp)

SRC_OBJ = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJ = $(TEST_FILES:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

TEST_BINS = $(TEST_FILES:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/%)

# Default target
all: $(BUILD_DIR) $(TEST_BINS)

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

# Link test executables
$(BUILD_DIR)/%: $(OBJ_DIR)/%.o $(SRC_OBJ)
	$(CXX) $^ -o $@ $(GTEST_FLAGS)

# Run all tests
test: $(TEST_BINS)
	for test in $(TEST_BINS) ; do ./$$test ; done

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
