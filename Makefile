CC := gcc
CFLAGS := -O3 -std=c99 -Wall

ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

TARGET := disasm_floats$(EXE)

.PHONY: all

all: $(TARGET)

clean:
	rm -f disasm_floats disasm_floats.exe

$(TARGET): disasm_floats.c
	$(CC) $(CFLAGS) -o $@ $^
