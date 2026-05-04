# Mini-C Transpiler (x86-64 NASM)

Este é um projeto educacional de um transpiler minimalista escrito em **C puro**, que converte uma linguagem simplificada (inspirada em C) para **Assembly x86-64 (Sintaxe NASM)** para Linux.

O foco deste projeto é didático, demonstrando as etapas fundamentais da construção de um compilador sem a complexidade de otimizações ou bibliotecas externas.

## 🚀 Funcionalidades Suportadas

- Declaração de variáveis do tipo `int` (64 bits).
- Expressões matemáticas básicas (Adição `+`).
- Comando `print()` para exibir valores inteiros no console.
- Gerenciamento de variáveis na **Stack** (pilha).
- Saída em Assembly compatível com **NASM** e chamadas de sistema (**syscalls**) do Linux.

## 🏗️ Arquitetura do Sistema

O transpiler está dividido em módulos claros, seguindo o pipeline clássico de compilação:

1.  **Lexer (`src/lexer.c`)**: Transforma o código fonte em uma sequência de tokens significativos.
2.  **Parser (`src/parser.c`)**: Utiliza a técnica de **Descida Recursiva** para validar a sintaxe e construir a árvore.
3.  **AST (`src/ast.c`)**: Representa o programa de forma estruturada em uma **Abstract Syntax Tree**.
4.  **Symbol Table (`src/symtab.c`)**: Mapeia os nomes das variáveis para seus respectivos offsets na pilha (`rbp - offset`).
5.  **Codegen (`src/codegen.c`)**: Percorre a AST e emite instruções Assembly x86-64. Inclui um helper em Assembly para converter números em strings (print).

## 🛠️ Requisitos

- **GCC** (Compilador C)
- **NASM** (Assembler)
- **LD** (Linker do Linux)
- **Make**

## 💻 Como Compilar e Usar

### 1. Compilar o Transpiler
No diretório raiz do projeto, execute:
```bash
make
```

### 2. Transpilar um Código
O transpiler aceita o código fonte como uma string no argumento:
```bash
./transpiler "int x = 10; int y = x + 5; print(y);"
```
Isso gerará um arquivo chamado `output.asm`.

### 3. Gerar o Executável Final
Use o NASM para montar e o LD para linkar:
```bash
nasm -f elf64 output.asm -o output.o
ld output.o -o output
```

### 4. Executar
```bash
./output
# Resultado esperado: 15
```

## 📝 Exemplo de Código Suportado

```c
int a = 100;
int b = 200;
int c = a + b;
print(c);
```

## 🧠 Explicação Técnica: Por que Stack?

Diferente de variáveis globais que ficam na seção `.data` ou `.bss`, este transpiler armazena variáveis locais na **Pilha (Stack)**.
- Usamos o registrador `RBP` como base da moldura de pilha (*stack frame*).
- Cada nova variável `int` reserva 8 bytes.
- Isso é fundamental para que, no futuro, você possa adicionar **funções e recursão**, onde cada chamada de função precisa de seu próprio espaço isolado na memória.

## 🔍 Como Evoluir Este Projeto?

- **Operadores:** Adicione `-`, `*` e `/` no Lexer e no `gen_expression` no Codegen.
- **Controle de Fluxo:** Implemente `if` e `while` gerando labels e instruções de salto (`jmp`, `je`, `jne`).
- **Input:** Implemente um comando `read()` usando a syscall `read` (ID 0).

---
Criado para fins educacionais. Sinta-se à vontade para explorar e modificar o código!
