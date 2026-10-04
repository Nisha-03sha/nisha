# ============================================================================
# RESTAURANT ORDER MANAGEMENT SYSTEM - MAKEFILE
# Data Structures Mini Project (C Backend)
# Compatible with Linux (gcc) and Windows (MSYS2 / MinGW-w64)
# ============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -D_POSIX_C_SOURCE=200809L
TARGET = server

# Detect Windows or POSIX environment
ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
    TARGET_BIN = $(TARGET).exe
    RM = del /Q /F
else
    LDFLAGS =
    TARGET_BIN = $(TARGET)
    RM = rm -f
endif

# Source and Object files
SRCS = main.c server.c queue.c linkedlist.c order.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	@echo "Build complete: $(TARGET_BIN)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET_BIN)
	./$(TARGET_BIN) 8080

clean:
	$(RM) $(OBJS) $(TARGET_BIN)
	@echo "Cleaned build artifacts."

.PHONY: all run clean
