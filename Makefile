SRC_DIR = src/
BUILD_DIR = build/

SOURCES = $(wildcard $(SRC_DIR)*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)%.c=$(BUILD_DIR)%.o)

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g

.phony: all run clean

$(BUILD_DIR)%.o: $(SRC_DIR)%.c
	@mkdir -p build
	@echo "Compiling $@"
	@$(CC) $(CFLAGS) -c $< -o $@

run: $(OBJECTS)
	@$(CC) $(OBJECTS) -o build/main
	@chmod +x build/main
	@build/main

clean:
	@echo "Cleaning..."
	@rm -rf build
