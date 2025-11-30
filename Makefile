# Compiler and flags
CC := gcc
CFLAGS := -Wall -std=c99 -Ibuild/external/raylib/src 
LDFLAGS := -lSDL2 -lm -ldl -lpthread -lGL -lrt -lX11

# Paths
SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin
RAYLIB_DIR := build/external/raylib
RAYLIB_LIB := $(RAYLIB_DIR)/src/libraylib.a

# Sources and objects
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Output executable
TARGET := $(BIN_DIR)/my_game
WEB_TARGET := $(BIN_DIR)/index.html

.PHONY: all clean directories web

# Default target: desktop
all: directories $(TARGET)

# Desktop build
$(TARGET): $(OBJS) 
	$(CC) $^ -o $@ $(RAYLIB_LIB) $(LDFLAGS)

# Compile .c to .o in build/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

# Web build using emcc
web: directories
	emcc src/main.c src/rope.c build/external/raylib/src/*.c \
		-Ibuild/external/raylib/src \
		-DPLATFORM_WEB \
		-s USE_GLFW=3 \
		-s FULL_ES2=1 \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s WASM=1 \
		-s USE_WEBGL2=1 \
		-o bin/index.html

# Create build and bin dirs if missing
directories:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/*
