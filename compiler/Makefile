# Compiler settings
CC = gcc

CFLAGS = -Wall -Wextra -I./include/ -DFRX_ENABLE_ASSERTS
DEBUG_CFLAGS = -g -O0 -DFRX_DEBUG
RELEASE_CFLAGS = -O3

# Directories
SRC_DIR = src

OBJ_DIR = bin-int
DEBUG_OBJ_DIR = $(OBJ_DIR)/debug
RELEASE_OBJ_DIR = $(OBJ_DIR)/release

BIN_DIR = bin
DEBUG_BIN_DIR = $(BIN_DIR)/debug
RELEASE_BIN_DIR = $(BIN_DIR)/release

# Find all the .c files in the src directory
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Create a list of .o files corresponding to the .c files, placing them in their obj directory
DEBUG_OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(DEBUG_OBJ_DIR)/%.o)
RELEASE_OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(RELEASE_OBJ_DIR)/%.o)

# Name of the final executable
DEBUG_EXEC = $(DEBUG_BIN_DIR)/furoxc
RELEASE_EXEC = $(RELEASE_BIN_DIR)/furoxc

# Ensure that all neccessary directories exist
$(shell mkdir -p $(DEBUG_OBJ_DIR) $(RELEASE_OBJ_DIR) $(DEBUG_BIN_DIR) $(RELEASE_BIN_DIR))

all: $(DEBUG_EXEC) $(RELEASE_EXEC)

# Rule to create the final executable
$(DEBUG_EXEC): $(DEBUG_OBJ_FILES)
	$(CC) $(DEBUG_OBJ_FILES) -o $(DEBUG_EXEC)

$(RELEASE_EXEC): $(RELEASE_OBJ_FILES)
	$(CC) $(RELEASE_OBJ_FILES) -o $(RELEASE_EXEC)

# Rule to create the object files from source files
$(DEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_OBJ_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

$(RELEASE_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_OBJ_DIR)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all debug release clean
