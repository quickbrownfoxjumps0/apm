# Makefile

# Compiler
CC = gcc

# Output binary name
TARGET = apm.exe

# Compiler flags
CFLAGS = -Wall -O2 -static

# Libraries to link
# LIBS = -lsqlcipher -lcrypto -lssl -lcrypt32 -lws2_32 -lcomctl32 -mwindows 
LIBS = -lsqlcipher -lcrypto -lssl -lcrypt32 -lws2_32 -lcomctl32 -lbcrypt -ladvapi32 -mwindows

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
SRCS = $(shell find $(SRC_DIR) -name '*.c')

# Object files go to build/
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(subst $(SRC_DIR)/,,$(SRCS)))

# Default target
all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile each .c to .o inside build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create final executable
$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)

# === Tests ===

# SHA256
test-sha256: build/test-sha256

build/test-sha256: src/crypto/sha256.c tests/test-sha256.c
	$(CC) $(CFLAGS) -o $@ $^

run-sha256-test: test-sha256
	./build/test-sha256

# HMAC
test-hmac: build/test-hmac

build/test-hmac: src/crypto/sha256.c src/crypto/hmac.c tests/test-hmac.c
	$(CC) $(CFLAGS) -o $@ $^

run-hmac-test: test-hmac
	./build/test-hmac

# PBKDF2
test-pbkdf2: build/test-pbkdf2

build/test-pbkdf2: src/crypto/sha256.c src/crypto/hmac.c src/crypto/pbkdf2.c tests/test-pbkdf2.c
	$(CC) $(CFLAGS) -o $@ $^

run-pbkdf2-test: test-pbkdf2
	./build/test-pbkdf2

