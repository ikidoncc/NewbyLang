# Newby Language Roadmap

Este documento descreve as funcionalidades essenciais que o **Newby** precisa suportar antes da implementação do sistema oficial de `library`. O objetivo é garantir que a linguagem seja capaz de lidar com recursos de baixo nível e modularidade.

---

## 1. Interoperabilidade e Baixo Nível (FFI)
Permitir que a linguagem interaja com o sistema operacional e bibliotecas C existentes.
- [x] **Palavra-chave `extern`**: Declarar funções externas sem corpo (ex: `extern func printf(string fmt);`).
- [x] **Chamadas de Sistema (`syscall`)**: Suporte nativo para invocar interrupções do kernel.
- [x] **Linking Externo**: Atualizar o compilador para aceitar bibliotecas externas (ex: `-lc`) no passo de linkagem.

## 2. Sistema de Tipos Avançado
Necessário para manipular estruturas de dados complexas usadas em I/O e rede.
- [x] **Ponteiros (`*T`)**: Capacidade de referenciar e manipular endereços de memória diretamente.
- [x] **Estruturas (`struct`)**: Definição de tipos de dados compostos.
- [x] **Alocação Dinâmica**: Implementação de `malloc` e `free` via runtime ou FFI com a `libc`.

## 3. Modularidade e Organização
Evitar conflitos de nomes e permitir a reutilização de código entre arquivos.
- [x] **Namespaces**: Sistema para isolar símbolos (ex: `IO.print` vs `Log.print`).
- [x] **Comando `import`**: Lógica de pré-processamento para carregar outros arquivos `.nb`.
- [x] **Visibilidade (`pub`)**: Controle de quais funções e variáveis são expostas para fora de um arquivo.

## 4. Pipeline de Compilação Modular
Preparar o `nbc` para trabalhar com múltiplos módulos de forma eficiente.
- [ ] **Compilação Separada**: Gerar arquivos `.o` sem incluir o ponto de entrada `_start`.
- [ ] **Busca Automática**: O compilador deve procurar arquivos na pasta `library/` por padrão.

---

## Ordem Sugerida de Implementação

1.  **FFI (`extern`)**: Permite usar funções do C imediatamente para expandir capacidades.
2.  **Ponteiros**: Essencial para passar buffers para as funções `extern`.
3.  **Namespaces & Import**: Base para o sistema de bibliotecas.
4.  **Structs**: Permite abstrações mais ricas.

---
*Este roadmap é um documento vivo e deve ser atualizado conforme o progresso do compilador.*
