# 🐻 Bearbox — Gerenciador de Tarefas em Terminal (TUI)

> Um gerenciador de tarefas minimalista, rápido e totalmente operável pelo teclado, feito em **C** usando **ncurses**.

## 📌 Sobre o Projeto

**Bearbox** é um gerenciador de tarefas em modo texto (TUI), desenvolvido para quem vive no terminal e quer produtividade sem distrações.  
Ele foi pensado para ser **simples, leve e direto**, sem dependências gráficas pesadas ou complexidade desnecessária.

Ideal para:
- Usuários de terminal / Linux
- Ambientes com poucos recursos
- Quem curte fluxo de trabalho estilo Vim / CLI

---

## ✨ Principais Características

- 🖥️ **Interface TUI (ncurses)** totalmente navegável pelo teclado  
- ⚡ **Leve e rápido**, consumo mínimo de recursos  
- 🧠 **Organização Kanban** em três colunas:
  - Nova
  - Andamento
  - Concluído
- 💾 **Persistência automática** 
- 🔢 **Sistema de IDs** para gerenciamento rápido  
- 📝 **Incrementos/Notas** para acompanhar evolução de tarefas
- 🎨 **Interface colorida** com cores por categoria
- 🪟 **Popups para input** 

---

## 🛠️ Tecnologias Utilizadas

- **Linguagem:** C  
- **Biblioteca:** ncurses  
- **Plataforma:** Linux  

---

## 🚀 Instalação

### Pré-requisitos

- GCC
- Biblioteca **ncurses**
- Sistema Linux ou Unix-like

### Instalação Rápida

```bash
# 1. Extraia o pacote
unzip bearbox.zip

# 2. Torne o instalador executável
chmod +x bbinstall.sh

# 3. Execute o instalador
./bbinstall.sh
```

## 📖 Como Usar

### Iniciar o Bearbox

```bash
bearbox
```

### Comandos Disponíveis

| Comando | Descrição |
|---------|-----------|
| `nt` | Adicionar nova tarefa |
| `rm <id>` | Remover tarefa |
| `mv <id>` | Mover tarefa para outra coluna |
| `ed <id>` | Editar descrição da tarefa |
| `in <id>` | Adicionar nota/incremento |
| `vi <id>` | Visualizar histórico de notas |
| `q` ou `sair` | Sair do programa |
| `help` ou `?` | Exibir ajuda |

### Indicadores

- `[N]` = Número de notas/incrementos na tarefa
- Exemplo: `<001> [3] Minha tarefa` = tarefa com 3 incrementos

## 📂 Estrutura de Dados

Os dados são automaticamente salvos em:
- **Linux/Mac**: `~/.local/share/bearbox/bearbox.dat`
- **Fallback**: `.bearbox/bearbox.dat` (diretório atual)

O diretório é criado automaticamente na primeira execução.

---

## 🎨 Esquema de Cores

- 🟢 **Verde** - Nova Tarefa
- 🟡 **Amarelo** - Em Andamento
- 🔵 **Azul** - Concluído
- 🔴 **Vermelho** - Erros e cabeçalho

---

## 🐛 Limitações Conhecidas

- Máximo de 256 caracteres por descrição/nota
- Máximo de 100 tarefas por categoria
- Redimensionamento de terminal pode afetar layout

## 🙏 Agradecimentos

Inspirado em ferramentas como Vim, Taskwarrior e Trello.  
Desenvolvido com ❤️ e ☕

---

⭐ **Se este projeto te ajudou, considere dar uma estrela!**
