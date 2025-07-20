# Makefile

# Compiler
CC = gcc

# Output binary name
TARGET = apm.exe

# Compiler flags
CFLAGS = -Wall -O2 -static 
# -I/mingw64/include

# Linker flags
# LDFLAGS = -L/mingw64/lib

# Libraries
LIBS = -lsqlcipher -lcrypto -lssl -lcrypt32 -lws2_32
# LIBS = -mwindows -municode -lsqlcipher -lcrypto -lssl -lcrypt32 -lws2_32

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

# Tests  
# SHA256
test-sha256: build/test-sha256

build/test-sha256: src/sha256.c tests/test-sha256.c
	$(CC) $(CFLAGS) -o $@ $^

run-sha256-test: test-sha256
	./build/test-sha256

# HMAC
test-hmac: build/test-hmac

build/test-hmac: src/sha256.c src/hmac.c tests/test-hmac.c
	$(CC) $(CFLAGS) -o $@ $^

run-hmac-test: test-hmac
	./build/test-hmac

#  PBKDF2
test-pbkdf2: build/test-pbkdf2

build/test-pbkdf2: src/sha256.c src/hmac.c src/pbkdf2.c tests/test-pbkdf2.c
	$(CC) $(CFLAGS) -o $@ $^

run-pbkdf2-test: test-pbkdf2
	./build/test-pbkdf2
