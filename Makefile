CC = gcc
CFLAGS = -Wall -g -Isrc
SRC = src/lexer.c src/ast.c src/parser.c src/symtab.c src/codegen.c src/main.c
OBJ = $(SRC:.c=.o)
TARGET = transpiler

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) hello.cj
	nasm -f elf64 output.asm -o output.o
	ld output.o -o output
	./output

clean:
	rm -f $(OBJ) $(TARGET) output.asm output.o output
