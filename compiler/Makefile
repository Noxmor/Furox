# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g

# Directories
SRC_DIR = src
OBJ_DIR = bin-int
BIN_DIR = bin

# Find all the .c files in the src directory
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Create a list of .o files corresponding to the .c files, placing them in the $(OBJ_DIR) directory
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Name of the final executable
EXEC = $(BIN_DIR)/furoxc

# Ensure that all neccessary directories exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

all: $(EXEC)

# Rule to create the final executable
$(EXEC): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(EXEC)

# Rule to create the object files from source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
