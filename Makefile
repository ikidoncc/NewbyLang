CC = gcc
CFLAGS = -Wall -g -Isrc
SRC = src/lexer.c src/ast.c src/parser.c src/symtab.c src/codegen.c src/main.c
OBJ = $(SRC:.c=.o)
TARGET = nbc

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) hello.nb
	./hello

clean:
	rm -f $(OBJ) $(TARGET) hello output.asm output.o output

