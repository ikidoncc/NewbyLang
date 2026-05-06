# 🚀 Newby Language Roadmap (Unified — Systems + Multiparadigm)

Este documento define as funcionalidades essenciais do **Newby** antes da estabilização do sistema de bibliotecas (`library`) e do ecossistema oficial.

---

## 🧭 Prioridades Imediatas (Próxima Fase)
1.  **OOP Leve**: Implementar métodos em Structs (`p.metodo()`).
2.  **Expressividade**: Enums reais e Pattern Matching avançado.
3.  **Linker Inteligente**: Automação do linking de múltiplos arquivos `.o`.

---

# 1. ⚙️ Interoperabilidade e Baixo Nível (FFI)

- [x] `extern` para funções C (sem corpo)
- [x] `syscall` nativo (acesso direto ao kernel)
- [x] linking externo (`-lc`, etc.)
- [ ] ABI compatível com C (padronização de calling convention)

---

# 2. 🧠 Sistema de Tipos (CORE DA LINGUAGEM)

## ✔ Tipos básicos
- [x] int, float, bool, char, string
- [x] ponteiros (`*T`)

## 🧱 Tipos compostos
- [x] structs (básico implementado)
- [ ] arrays fixos e dinâmicos

## 🔥 Sistema de tipos moderno
- [x] enums (tagged unions)
- [x] pattern matching (`match`)
- [ ] generics (`fn<T>`)
- [ ] inferência de tipos
- [ ] traits / interfaces (polimorfismo seguro)

---

# 3. 🧬 Sistema de Memória Híbrido (ESTILO RUST)

- [ ] ownership (um dono por valor)
- [ ] move semantics
- [ ] borrowing (referências `&T` e `&mut T`)
- [ ] lifetimes e regras de aliasing
- [ ] modo `safe` padrão e blocos `unsafe`

---

# 4. 🧩 Modularidade e Organização

- [x] namespaces (`IO.print`, `Log.print`)
- [x] `import` de arquivos `.nb`
- [x] visibilidade (`pub`, `private`)
- [ ] resolução de módulos por path completo

---

# 5. ⚡ Multiparadigma (CORE DA LINGUAGEM)

## Procedural
- funções livres
- controle explícito de fluxo

## Funcional
- [ ] funções como first-class values
- [ ] closures / lambdas
- [ ] imutabilidade opcional

## OOP leve
- [ ] structs com métodos -- **ALTA PRIORIDADE**
- [ ] encapsulamento via `pub/private`
- [ ] traits como base de polimorfismo

---

# 6. ⚡ Sistema de Erros Moderno

- [ ] `Option<T>`
- [ ] `Result<T, E>`
- [ ] operador `?` (propagação de erro)

---

# 7. 📦 Sistema de Pacotes

- [ ] package manager (`nypm`)
- [ ] lockfile (`nb.lock`)

---

## 🔥 Pipeline de Compilação Modular (CRÍTICO)

## 🔥 Compilação modular
- [x] geração de `.o` sem `_start`
- [x] separação entre módulos compiláveis
- [x] linking final automatizado de múltiplos objetos (Intelligent Linker)

## 🔥 Organização de bibliotecas
- [x] busca automática em `library/`
- [ ] build incremental (recompilar só o necessário)

---

# 🛠 Tooling (ECOSSISTEMA)

- [ ] linter (`nb check`)
- [ ] formatter (`nb fmt`)
- [ ] build tool (`nb build`)

---

# 💡 Visão final
O **Newby** evolui para uma linguagem de sistemas moderna, multiparadigma, com segurança de memória e um ecossistema modular escalável.
