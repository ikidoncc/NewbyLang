CC = gcc
CFLAGS = -Wall -g -Icompiler
SRC = compiler/lexer.c compiler/ast.c compiler/parser.c compiler/symtab.c compiler/semantic.c compiler/codegen.c compiler/main.c
OBJ = $(SRC:.c=.o)
TARGET = nbc

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) tests/hello.nb
	./hello

clean:
	rm -f $(OBJ) $(TARGET) hello output.asm output.o output tests/*.asm tests/*.o tests/hello tests/test_*
