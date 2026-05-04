# Newby Compiler (nbc)

Este é um projeto educacional do **Newby**, um compilador minimalista escrito em **C puro**, que converte código-fonte `.nb` para executáveis binários nativos x86-64 Linux.

O foco deste projeto é didático, demonstrando as etapas fundamentais da construção de um compilador real, desde a análise léxica até a geração de código binário.

## 🚀 Funcionalidades Suportadas

- Declaração de variáveis do tipo `int` (64 bits).
- Expressões matemáticas básicas (Adição `+`).
- Comando `print()` para exibir valores inteiros no console.
- Gerenciamento de variáveis na **Stack** (pilha).
- Geração de executáveis Linux ELF64 de passo único.

## 🏗️ Arquitetura do Sistema

O compilador `nbc` segue o pipeline clássico:

1.  **Lexer (`src/lexer.c`)**: Transforma o código fonte em tokens.
2.  **Parser (`src/parser.c`)**: Valida a sintaxe e constrói a árvore sintática (AST).
3.  **Codegen (`src/codegen.c`)**: Gera instruções Assembly x86-64 temporárias e invoca o assembler/linker para criar o executável final.

## 🛠️ Requisitos

- **GCC** (Para compilar o `nbc`)
- **NASM** (Assembler usado internamente pelo `nbc`)
- **LD** (Linker usado internamente pelo `nbc`)
- **Make**

## 💻 Como Compilar e Usar

### 1. Compilar o Newby Compiler (`nbc`)
Execute:
```bash
make
```

### 2. Compilar um Código Newby
O compilador gera automaticamente um executável com o mesmo nome do arquivo de entrada (sem a extensão `.nb`):
```bash
./nbc hello.nb
```

### 3. Executar o Binário Gerado
```bash
./hello
```

### 4. Atalho de Desenvolvimento
Para compilar o `nbc`, compilar `hello.nb` e executar o resultado em um único passo:
```bash
make run
```

## 📝 Exemplo de Código Newby (`hello.nb`)

```c
int x = 3;
int y = x + 5;
print(y);
print(x);
```

---
Criado para fins educacionais. Sinta-se à vontade para explorar e modificar o Newby!
