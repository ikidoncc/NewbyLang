CC = gcc
CFLAGS = -Wall -g -Icompiler
SRC = compiler/lexer.c compiler/ast.c compiler/parser.c compiler/symtab.c compiler/semantic.c compiler/codegen.c compiler/main.c
OBJ = $(SRC:.c=.o)
TARGET = nbc

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) tests/hello.nby
	./hello

clean:
	rm -f $(OBJ) $(TARGET) hello output.asm output.o output tests/*.asm tests/*.o tests/hello tests/test_array tests/test_bool tests/test_ffi tests/test_func tests/test_string tests/test_while tests/test_array_mini tests/test_types
