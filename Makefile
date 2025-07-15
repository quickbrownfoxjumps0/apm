# Makefile

# Compiler
CC = gcc

# Output binary name
TARGET = apm.exe

# Compiler flags
CFLAGS = -Wall -O2 -static -mwindows -municode

# Libraries
# LIBS = -lssl -lcrypto -lsqlcipher

# Source files
SRCS = $(wildcard src/*.c)

# Object files
OBJS = $(patsubst src/%, $(BUILD_DIR)/%, $(SRCS:.c=.o))

# Build output dir
BUILD_DIR = build

# How to compile object files
$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Default rule
all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Clean
clean:
	rm -rf $(BUILD_DIR)

