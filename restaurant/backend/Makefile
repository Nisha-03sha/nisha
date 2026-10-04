# ============================================================================
# BASIL & EMBER - MAKEFILE (C Backend)
# Compatible with Linux (gcc) and Windows (MSYS2 / MinGW-w64)
# ============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -D_POSIX_C_SOURCE=200809L
TARGET = server

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
    TARGET_BIN = $(TARGET).exe
    RM = del /Q /F
else
    LDFLAGS =
    TARGET_BIN = $(TARGET)
    RM = rm -f
endif

SRCS = main.c server.c queue.c linkedlist.c order.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET_BIN)

.PHONY: all clean
