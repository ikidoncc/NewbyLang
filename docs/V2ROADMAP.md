# 🧩 Newby Language — Tasks Roadmap (v2 Implementation Plan)

Este documento lista as tarefas práticas para implementação da **Nova fase do ecossistema Newby**, assumindo que a linguagem base já está completa.

---

# 🚀 EPIC 0 — Toolchain Unificado (CORE DO ECOSSISTEMA)

## 🎯 Objetivo

Transformar o Newby em uma plataforma utilizável via CLI única.

---

## 🧰 CLI principal (`nby`)

* [ ] Criar CLI base (`nby`)
* [ ] Implementar parser de comandos (`build`, `run`, `fmt`, `check`, `test`)
* [ ] Conectar CLI ao compilador interno (`nbyc`)
* [ ] Sistema de help (`nby --help`)
* [ ] Sistema de versionamento (`nby --version`)

---

## ⚙️ Integração com compilador

* [ ] Abstrair execução do `nbyc`
* [ ] Pipeline: CLI → Compiler → Linker → Binary
* [ ] Captura de erros estruturados
* [ ] Suporte a modo debug vs release

---

# 🚀 EPIC 1 — Build System (CRÍTICO)

## 🎯 Objetivo

Transformar compilação em sistema inteligente e incremental.

---

## 🔗 Pipeline de build

* [ ] Implementar grafo de dependências entre arquivos
* [ ] Ordenação topológica de módulos
* [ ] Build incremental (cache de objetos `.o`)
* [ ] Rebuild seletivo baseado em hash de arquivos

---

## ⚙️ Linker automático

* [ ] Automatizar linking de múltiplos `.o`
* [ ] Resolver símbolos externos (`extern`)
* [ ] Gerar executável final automaticamente
* [ ] Integrar linker do sistema (ld/gcc)

---

# 🚀 EPIC 2 — Package Manager (`nby pm`)

## 🎯 Objetivo

Criar sistema de dependências do ecossistema.

---

## 📦 Core do package manager

* [ ] Criar comando `nby pm`
* [ ] Implementar `add/remove/install`
* [ ] Sistema de versões semânticas
* [ ] Resolver dependências transitivas

---

## 🧾 Lockfile

* [ ] Criar `nby.lock`
* [ ] Garantir builds determinísticos
* [ ] Hash de dependências

---

## 🌐 Registry (base futura)

* [ ] Definir formato de pacotes
* [ ] Estrutura de publicação (`nby pm publish`)
* [ ] Metadata de pacotes

---

# 🚀 EPIC 3 — Testing System

## 🎯 Objetivo

Garantir qualidade do ecossistema.

---

* [ ] Criar comando `nby test`
* [ ] Descoberta automática de testes
* [ ] Execução paralela
* [ ] Relatórios estruturados
* [ ] Integração com build system

---

# 🚀 EPIC 4 — Language Server (LSP)

## 🎯 Objetivo

Integração completa com IDEs.

---

* [ ] Criar `nby lsp`
* [ ] Autocomplete de tipos
* [ ] Go-to-definition
* [ ] Diagnósticos em tempo real
* [ ] Highlight semântico

---

# 🚀 EPIC 5 — Debugger

## 🎯 Objetivo

Ferramenta de inspeção de execução.

---

* [ ] Criar `nby debug`
* [ ] Breakpoints
* [ ] Step execution
* [ ] Stack trace detalhado
* [ ] Inspect de memória

---

# 🚀 EPIC 6 — Formatter & Linter

## 🎯 Objetivo

Padronização do código.

---

## Formatter (`nby fmt`)

* [ ] AST-based formatting
* [ ] Regras de estilo consistentes
* [ ] Auto-fix de indentação

---

## Linter (`nby check`)

* [ ] Detectar variáveis não usadas
* [ ] Detectar tipos inválidos
* [ ] Detectar unsafe mal utilizado
* [ ] Sugestões de melhoria

---

# 🚀 EPIC 7 — Projeto e Estrutura Padrão

## 🎯 Objetivo

Padronizar ecossistema de projetos.

---

* [ ] Criar `nby init`
* [ ] Gerar estrutura padrão de projeto
* [ ] Definir `nby.toml`
* [ ] Suporte a múltiplos módulos (`src/`, `library/`, `tests/`)

---

# 🚀 EPIC 8 — UX do Desenvolvedor

## 🎯 Objetivo

Melhorar experiência de uso da linguagem.

---

* [ ] Mensagens de erro estruturadas
* [ ] Erros com contexto de código
* [ ] Sugestões automáticas
* [ ] Stack trace legível
* [ ] Output colorido no CLI

---

# 🚀 EPIC 9 — Infraestrutura do Compilador

## 🎯 Objetivo

Preparar evolução futura da linguagem.

---

* [ ] Separar frontend/backend do compilador
* [ ] Abstração de geração de código
* [ ] Preparar backend LLVM (opcional futuro)
* [ ] Suporte a IR intermediária

---

# 🧭 PRIORIDADES GLOBAIS

## 🔥 Alta prioridade (MVP do ecossistema)

* CLI (`nby`)
* build system
* linker automático
* package manager básico

---

## ⚙️ Média prioridade

* LSP
* formatter
* linter
* test system

---

## 🚀 Baixa prioridade (infra avançada)

* debugger
* registry completo
* IR intermediária
* plugin system

---

# 💡 VISÃO FINAL

O Newby deixa de ser apenas um compilador e passa a ser:

> 🔥 um ecossistema completo de desenvolvimento nativo com toolchain unificada
