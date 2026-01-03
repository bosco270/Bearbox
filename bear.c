#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define MAX_TAREFAS 100
#define MAX_DESC 256
#define APP_NAME "bearbox"
#define DATA_FILE "bearbox.dat"

typedef enum {
    CAT_NOVA = 1,
    CAT_ANDAMENTO = 2,
    CAT_CONCLUIDA = 3
} Categoria;

typedef enum {
    COR_ERRO = 1,
    COR_NOVA = 2,
    COR_ANDAMENTO = 3,
    COR_CONCLUIDA = 4,
    COR_BEAR = 5,
    COR_BOX = 6
} CorPar;

typedef struct {
    int id;
    int categoria;
    char descricao[MAX_DESC];
    char incremento[MAX_DESC];
} Tarefa;

// ========== GERENCIAMENTO DE CAMINHO ==========

void obter_diretorio_dados(char *caminho, size_t tamanho) {
    const char *home = getenv("HOME");
    if (home) {
        snprintf(caminho, tamanho, "%s/.local/share/%s", home, APP_NAME);
    } else {
        snprintf(caminho, tamanho, ".%s", APP_NAME);
    }
}

int criar_diretorio_dados() {
    char caminho[512];
    obter_diretorio_dados(caminho, sizeof(caminho));
    
    struct stat st = {0};
    if (stat(caminho, &st) == -1) {
        if (mkdir(caminho, 0755) == -1) {
            return -1;
        }
    }
    return 0;
}

void obter_caminho_arquivo(char *caminho, size_t tamanho) {
    char dir[512];
    obter_diretorio_dados(dir, sizeof(dir));
    snprintf(caminho, tamanho, "%s/%s", dir, DATA_FILE);
}

// ========== FUNÇÕES DE ARQUIVO ==========

int obter_proximo_id() {
    char caminho[512];
    obter_caminho_arquivo(caminho, sizeof(caminho));
    
    FILE *file = fopen(caminho, "rb");
    int max_id = 0;
    Tarefa t;

    if (file) {
        while (fread(&t, sizeof(Tarefa), 1, file)) {
            if (t.id > max_id) max_id = t.id;
        }
        fclose(file);
    }
    return max_id + 1;
}

void adicionar_tarefa(const char *descricao, int categoria) {
    char caminho[512];
    obter_caminho_arquivo(caminho, sizeof(caminho));
    
    FILE *file = fopen(caminho, "ab");
    if (!file) return;

    Tarefa nova_tarefa = {
        .id = obter_proximo_id(),
        .categoria = categoria,
    };
    strncpy(nova_tarefa.descricao, descricao, MAX_DESC - 1);
    nova_tarefa.descricao[MAX_DESC - 1] = '\0';

    fwrite(&nova_tarefa, sizeof(Tarefa), 1, file);
    fclose(file);
}

int carregar_tarefas(Tarefa *tarefas, int categoria) {
    char caminho[512];
    obter_caminho_arquivo(caminho, sizeof(caminho));
    
    FILE *file = fopen(caminho, "rb");
    int count = 0;
    Tarefa t;

    if (file) {
        while (fread(&t, sizeof(Tarefa), 1, file) && count < MAX_TAREFAS) {
            if (t.categoria == categoria) tarefas[count++] = t;
        }
        fclose(file);
    }
    return count;
}

void remover_tarefa(int id) {
    char caminho[512], temp[520];
    obter_caminho_arquivo(caminho, sizeof(caminho));
    snprintf(temp, sizeof(temp), "%s.tmp", caminho);
    
    FILE *file = fopen(caminho, "rb");
    FILE *temp_file = fopen(temp, "wb");
    Tarefa t;

    if (file && temp_file) {
        while (fread(&t, sizeof(Tarefa), 1, file)) {
            if (t.id != id) fwrite(&t, sizeof(Tarefa), 1, temp_file);
        }
        fclose(file);
        fclose(temp_file);
        remove(caminho);
        rename(temp, caminho);
    } else {
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
    }
}

int mover_tarefa(int id, int nova_categoria) {
    char caminho[512];
    obter_caminho_arquivo(caminho, sizeof(caminho));
    
    FILE *file = fopen(caminho, "rb+");
    if (!file) return -1;

    Tarefa t;
    int encontrou = 0;

    while (fread(&t, sizeof(Tarefa), 1, file)) {
        if (t.id == id) {
            t.categoria = nova_categoria;
            fseek(file, -sizeof(Tarefa), SEEK_CUR);
            fwrite(&t, sizeof(Tarefa), 1, file);
            encontrou = 1;
            break;
        }
    }
    fclose(file);
    return encontrou ? 0 : -1;
}

Tarefa* buscar_tarefa(int id) {
    char caminho[512];
    obter_caminho_arquivo(caminho, sizeof(caminho));
    
    FILE *file = fopen(caminho, "rb");
    if (!file) return NULL;

    Tarefa *t = malloc(sizeof(Tarefa));
    if (!t) {
        fclose(file);
        return NULL;
    }

    while (fread(t, sizeof(Tarefa), 1, file)) {
        if (t->id == id) {
            fclose(file);
            return t;
        }
    }

    free(t);
    fclose(file);
    return NULL;
}

// ========== INTERFACE ==========

void linhasuperior() {
    attron(COLOR_PAIR(COR_ERRO));
    for (int i = 0; i < COLS; i++) mvaddch(0, i, ' ');
    mvprintw(0, 3, "BEARBOX beta");
    mvprintw(0, COLS - 16, "©JoaoBSJR");
    attroff(COLOR_PAIR(COR_ERRO));
    refresh();
}

void desenhar_titulo(WINDOW *janela, const char *titulo, int cor) {
    wattron(janela, COLOR_PAIR(cor));
    mvwprintw(janela, 0, 8, " %s ", titulo);
    wattroff(janela, COLOR_PAIR(cor));
    wrefresh(janela);
}

void atualizar_janela(WINDOW *janela, int categoria, int cor) {
    Tarefa tarefas[MAX_TAREFAS];
    int count = carregar_tarefas(tarefas, categoria);

    wclear(janela);
    box(janela, 0, 0);
    wattron(janela, COLOR_PAIR(cor));

    int max_y = getmaxy(janela);
    int linha = 2;

    for (int i = 0; i < count && linha < (max_y - 2); i++) {
        // Conta quantos incrementos a tarefa tem
        int num_incrementos = 0;
        if (strlen(tarefas[i].incremento) > 0) {
            num_incrementos = 1; // Pelo menos 1
            for (int j = 0; tarefas[i].incremento[j] != '\0'; j++) {
                if (tarefas[i].incremento[j] == '\n') {
                    num_incrementos++;
                }
            }
        }
        
        // Monta o marcador de incrementos
        if (num_incrementos > 0) {
            mvwprintw(janela, linha, 1, "<%03d> [%d] %s", 
                      tarefas[i].id, num_incrementos, tarefas[i].descricao);
        } else {
            mvwprintw(janela, linha, 1, "<%03d> %s", 
                      tarefas[i].id, tarefas[i].descricao);
        }
        linha += 2;
    }

    wattroff(janela, COLOR_PAIR(cor));
    wrefresh(janela);
}

void exibir_help() {
    curs_set(0);
    int largura = 70, altura = 15;
    WINDOW *help_win = newwin(altura, largura, (LINES - altura) / 2, (COLS - largura) / 2);
    box(help_win, 0, 0);
    
    wattron(help_win, COLOR_PAIR(COR_ERRO));
    mvwprintw(help_win, 1, 2, "AJUDA - BEARBOX");
    wattroff(help_win, COLOR_PAIR(COR_ERRO));
    
    mvwprintw(help_win, 3, 2, "Comandos:");
    mvwprintw(help_win, 4, 2, " nt - Nova tarefa");
    mvwprintw(help_win, 5, 2, " ta - Tarefa em andamento");
    mvwprintw(help_win, 6, 2, " tc - Tarefa concluida");
    mvwprintw(help_win, 7, 2, " rm <id> - Remover");
    mvwprintw(help_win, 8, 2, " mv <id> - Mover");
    mvwprintw(help_win, 9, 2, " ed <id> - Editar");
    mvwprintw(help_win, 10, 2, " in <id> - Incrementar");
    mvwprintw(help_win, 11, 2, " vi <id> - Ver incrementos");
    mvwprintw(help_win, 12, 2, " q/sair - Sair");
    mvwprintw(help_win, 13, 2, " [N] = Numero de incrementos");
    
    wattron(help_win, COLOR_PAIR(COR_BEAR));
    mvwprintw(help_win, 3, 52, "O___O");
    mvwprintw(help_win, 4, 52, "(°_°)");
    mvwprintw(help_win, 5, 52, "| _ |");
    mvwprintw(help_win, 6, 51, "/||_||\\");
    wattroff(help_win, COLOR_PAIR(COR_BEAR));
    
    wattron(help_win, COLOR_PAIR(COR_BOX));
    mvwprintw(help_win, 7, 50, "|*******|");
    mvwprintw(help_win, 8, 50, "|  box  |");
    mvwprintw(help_win, 9, 50, "|_______|");
    wattroff(help_win, COLOR_PAIR(COR_BOX));
    
    wattron(help_win, COLOR_PAIR(COR_ERRO));
    mvwprintw(help_win, altura - 2, 2, "Pressione qualquer tecla");
    wattroff(help_win, COLOR_PAIR(COR_ERRO));

    wrefresh(help_win);
    getch();
    delwin(help_win);
}

void mostrar_erro(const char *msg) {
    int largura = 50, altura = 5;
    WINDOW *err_win = newwin(altura, largura, (LINES - altura) / 2, (COLS - largura) / 2);
    box(err_win, 0, 0);
    
    wattron(err_win, COLOR_PAIR(COR_ERRO));
    mvwprintw(err_win, 0, 4, " ERRO ");
    mvwprintw(err_win, 2, 2, "%s", msg);
    mvwprintw(err_win, altura - 2, 2, "Pressione qualquer tecla...");
    wattroff(err_win, COLOR_PAIR(COR_ERRO));
    
    wrefresh(err_win);
    getch();
    delwin(err_win);
}

void adicionar_tarefa_ui(int categoria) {
    const char *titulos[] = {"", "Nova Tarefa", "Em Andamento", "Concluida"};
    int cores[] = {0, COR_NOVA, COR_ANDAMENTO, COR_CONCLUIDA};
    
    int largura = 60, altura = 6;
    WINDOW *win = newwin(altura, largura, (LINES - altura) / 2, (COLS - largura) / 2);
    box(win, 0, 0);

    wattron(win, COLOR_PAIR(cores[categoria]));
    mvwprintw(win, 0, 4, " %s ", titulos[categoria]);
    wattroff(win, COLOR_PAIR(cores[categoria]));
    
    mvwprintw(win, 2, 2, "Descricao: ");
    wrefresh(win);

    echo();
    curs_set(1);
    char buffer[MAX_DESC];
    mvwgetnstr(win, 2, 13, buffer, sizeof(buffer) - 1);
    noecho();
    curs_set(0);

    if (strlen(buffer) > 0) {
        adicionar_tarefa(buffer, categoria);
        wattron(win, COLOR_PAIR(COR_NOVA));
        mvwprintw(win, altura - 2, 2, "Tarefa adicionada!");
        wattroff(win, COLOR_PAIR(COR_NOVA));
        wrefresh(win);
        getch();
    }

    delwin(win);
}

void remover_tarefa_ui(int id) {
    Tarefa *t = buscar_tarefa(id);
    if (!t) {
        mostrar_erro("Tarefa nao encontrada!");
        return;
    }

    int largura = 60, altura = 7;
    WINDOW *win = newwin(altura, largura, (LINES - altura) / 2, (COLS - largura) / 2);
    box(win, 0, 0);
    
    wattron(win, COLOR_PAIR(t->categoria + 1));
    mvwprintw(win, 0, 4, " Remover Tarefa ");
    wattroff(win, COLOR_PAIR(t->categoria + 1));
    
    mvwprintw(win, 2, 2, "<%03d> %s", t->id, t->descricao);
    mvwprintw(win, 4, 2, "Confirmar (s/N): ");
    wrefresh(win);

    echo();
    curs_set(1);
    char resp[10];
    mvwgetnstr(win, 4, 19, resp, sizeof(resp) - 1);
    noecho();
    curs_set(0);

    if (resp[0] == 's' || resp[0] == 'S') {
        remover_tarefa(id);
        wattron(win, COLOR_PAIR(COR_NOVA));
        mvwprintw(win, altura - 2, 2, "Tarefa removida!");
        wattroff(win, COLOR_PAIR(COR_NOVA));
    } else {
        mvwprintw(win, altura - 2, 2, "Cancelado.");
    }
    
    wrefresh(win);
    getch();
    delwin(win);
    free(t);
}

void mover_tarefa_ui(int id) {
    Tarefa *t = buscar_tarefa(id);
    if (!t) {
        mostrar_erro("Tarefa nao encontrada!");
        return;
    }

    int largura = 60, altura = 9;
    WINDOW *win = newwin(altura, largura, (LINES - altura) / 2, (COLS - largura) / 2);
    box(win, 0, 0);
    
    wattron(win, COLOR_PAIR(t->categoria + 1));
    mvwprintw(win, 0, 4, " Mover Tarefa ");
    wattroff(win, COLOR_PAIR(t->categoria + 1));
    
    mvwprintw(win, 2, 2, "<%03d> %s", t->id, t->descricao);
    mvwprintw(win, 4, 2, "[1] Nova  [2] Andamento  [3] Concluida");
    mvwprintw(win, 6, 2, "Escolha: ");
    wrefresh(win);

    echo();
    curs_set(1);
    char escolha[10];
    mvwgetnstr(win, 6, 11, escolha, sizeof(escolha) - 1);
    noecho();
    curs_set(0);

    int nova_cat = atoi(escolha);
    if (nova_cat >= 1 && nova_cat <= 3) {
        mover_tarefa(id, nova_cat);
        wattron(win, COLOR_PAIR(COR_NOVA));
        mvwprintw(win, altura - 2, 2, "Tarefa movida!");
        wattroff(win, COLOR_PAIR(COR_NOVA));
    } else {
        wattron(win, COLOR_PAIR(COR_ERRO));
        mvwprintw(win, altura - 2, 2, "Opcao invalida!");
        wattroff(win, COLOR_PAIR(COR_ERRO));
    }
    
    wrefresh(win);
    getch();
    delwin(win);
    free(t);
}

void editar_tarefa(int id) {
    Tarefa *t = buscar_tarefa(id);
    if (!t) {
        mostrar_erro("Tarefa nao encontrada!");
        return;
    }

    int largura = 60, altura = 6;
    WINDOW *win = newwin(altura, largura, (LINES - altura) / 2, (COLS - largura) / 2);
    box(win, 0, 0);

    wattron(win, COLOR_PAIR(t->categoria + 1));
    mvwprintw(win, 0, 4, " Editar <%03d> ", t->id);
    wattroff(win, COLOR_PAIR(t->categoria + 1));
    
    mvwprintw(win, 2, 2, "Descricao: ");
    wrefresh(win);

    echo();
    curs_set(1);
    char novo_texto[MAX_DESC];
    mvwgetnstr(win, 2, 13, novo_texto, sizeof(novo_texto) - 1);
    noecho();
    curs_set(0);

    if (novo_texto[0] != '\0') {
        strncpy(t->descricao, novo_texto, sizeof(t->descricao) - 1);
        t->descricao[sizeof(t->descricao) - 1] = '\0';
        
        char caminho[512];
        obter_caminho_arquivo(caminho, sizeof(caminho));
        
        FILE *file = fopen(caminho, "rb+");
        if (file) {
            Tarefa temp;
            while (fread(&temp, sizeof(Tarefa), 1, file)) {
                if (temp.id == id) {
                    fseek(file, -sizeof(Tarefa), SEEK_CUR);
                    fwrite(t, sizeof(Tarefa), 1, file);
                    break;
                }
            }
            fclose(file);
        }
        
        wattron(win, COLOR_PAIR(COR_NOVA));
        mvwprintw(win, altura - 2, 2, "Atualizado!");
        wattroff(win, COLOR_PAIR(COR_NOVA));
    } else {
        mvwprintw(win, altura - 2, 2, "Cancelado.");
    }
    
    wrefresh(win);
    getch();
    delwin(win);
    free(t);
}

void incrementar_tarefa(int id) {
    Tarefa *t = buscar_tarefa(id);
    if (!t) {
        mostrar_erro("Tarefa nao encontrada!");
        return;
    }

    int largura = 60, altura = 6;
    WINDOW *win = newwin(altura, largura, (LINES - altura) / 2, (COLS - largura) / 2);
    box(win, 0, 0);

    wattron(win, COLOR_PAIR(t->categoria + 1));
    mvwprintw(win, 0, 4, " Incremento <%03d> ", t->id);
    wattroff(win, COLOR_PAIR(t->categoria + 1));
    
    mvwprintw(win, 2, 2, "Texto: ");
    wrefresh(win);

    echo();
    curs_set(1);
    char buffer[256] = {0};
    mvwgetnstr(win, 2, 9, buffer, sizeof(buffer) - 1);
    noecho();
    curs_set(0);

    if (strlen(buffer) > 0) {
        size_t existente_len = strlen(t->incremento);
        size_t max_len = sizeof(t->incremento) - 1;

        if (existente_len + strlen(buffer) + 2 < max_len) {
            if (existente_len > 0) strncat(t->incremento, "\n", max_len - existente_len);
            strncat(t->incremento, buffer, max_len - strlen(t->incremento));

            char caminho[512];
            obter_caminho_arquivo(caminho, sizeof(caminho));
            
            FILE *file = fopen(caminho, "rb+");
            if (file) {
                Tarefa temp;
                while (fread(&temp, sizeof(Tarefa), 1, file)) {
                    if (temp.id == id) {
                        fseek(file, -sizeof(Tarefa), SEEK_CUR);
                        fwrite(t, sizeof(Tarefa), 1, file);
                        break;
                    }
                }
                fclose(file);
            }

            wattron(win, COLOR_PAIR(COR_NOVA));
            mvwprintw(win, altura - 2, 2, "Salvo!");
            wattroff(win, COLOR_PAIR(COR_NOVA));
        } else {
            wattron(win, COLOR_PAIR(COR_ERRO));
            mvwprintw(win, altura - 2, 2, "Limite excedido!");
            wattroff(win, COLOR_PAIR(COR_ERRO));
        }
    } else {
        mvwprintw(win, altura - 2, 2, "Cancelado.");
    }

    wrefresh(win);
    getch();
    delwin(win);
    free(t);
}

void visualizar_incrementos(int id) {
    curs_set(0);
    Tarefa *t = buscar_tarefa(id);
    if (!t) {
        mostrar_erro("Tarefa nao encontrada!");
        return;
    }

    int largura = 70, altura = 20;
    WINDOW *win = newwin(altura, largura, (LINES - altura) / 2, (COLS - largura) / 2);
    box(win, 0, 0);

    wattron(win, COLOR_PAIR(t->categoria + 1));
    mvwprintw(win, 0, 2, " <%03d> %s ", t->id, t->descricao);
    mvwprintw(win, 2, 2, "Historico:");
    wattroff(win, COLOR_PAIR(t->categoria + 1));

    if (strlen(t->incremento) == 0) {
        mvwprintw(win, 4, 2, "(Sem incrementos)");
    } else {
        char copia[MAX_DESC];
        strncpy(copia, t->incremento, sizeof(copia));
        
        char *inc = strtok(copia, "\n");
        int linha = 4;
        while (inc && linha < altura - 2) {
            mvwprintw(win, linha++, 2, "* %s", inc);
            inc = strtok(NULL, "\n");
        }
    }

    wattron(win, COLOR_PAIR(t->categoria + 1));
    mvwprintw(win, altura - 2, 2, "Pressione qualquer tecla");
    wattroff(win, COLOR_PAIR(t->categoria + 1));
    wrefresh(win);

    getch();
    delwin(win);
    free(t);
}

// ========== MAIN ==========

int main() {
    criar_diretorio_dados();

    initscr();
    cbreak();
    noecho();

    if (!has_colors()) {
        printw("Terminal sem suporte a cores");
        getch();
        endwin();
        return -1;
    }

    start_color();
    init_pair(COR_ERRO, COLOR_WHITE, COLOR_RED);
    init_pair(COR_NOVA, COLOR_WHITE, COLOR_GREEN);
    init_pair(COR_ANDAMENTO, COLOR_WHITE, COLOR_YELLOW);
    init_pair(COR_CONCLUIDA, COLOR_WHITE, COLOR_BLUE);
    init_pair(COR_BEAR, COLOR_BLACK, COLOR_WHITE);
    init_pair(COR_BOX, COLOR_BLACK, 130);

    char comando[256];
    int id;

    while (1) {
        int ymax, xmax;
        getmaxyx(stdscr, ymax, xmax);

        int largura_esq = xmax / 3;
        int largura_centro = xmax - 2 * largura_esq;

        WINDOW *janela_esq = newwin(ymax - 4, largura_esq, 4, 1);
        WINDOW *janela_centro = newwin(ymax - 4, largura_centro, 4, largura_esq + 1);
        WINDOW *janela_dir = newwin(ymax - 4, largura_esq - 1, 4, largura_esq + largura_centro + 1);

        linhasuperior();
        curs_set(1);

        atualizar_janela(janela_esq, CAT_NOVA, COR_NOVA);
        atualizar_janela(janela_centro, CAT_ANDAMENTO, COR_ANDAMENTO);
        atualizar_janela(janela_dir, CAT_CONCLUIDA, COR_CONCLUIDA);

        desenhar_titulo(janela_esq, "NOVA", COR_NOVA);
        desenhar_titulo(janela_centro, "ANDAMENTO", COR_ANDAMENTO);
        desenhar_titulo(janela_dir, "CONCLUIDO", COR_CONCLUIDA);

        WINDOW *console_win = newwin(3, xmax - 1, 1, 1);
        box(console_win, 0, 0);
        mvwprintw(console_win, 1, 1, ">:");
        wrefresh(console_win);

        echo();
        mvwgetnstr(console_win, 1, 3, comando, sizeof(comando) - 1);
        noecho();
        delwin(console_win);

        if (strcmp(comando, "q") == 0 || strcmp(comando, "sair") == 0) {
            break;
        } else if (strcmp(comando, "nt") == 0) {
            adicionar_tarefa_ui(CAT_NOVA);
        } else if (strcmp(comando, "ta") == 0) {
            adicionar_tarefa_ui(CAT_ANDAMENTO);
        } else if (strcmp(comando, "tc") == 0) {
            adicionar_tarefa_ui(CAT_CONCLUIDA);
        } else if (strncmp(comando, "rm ", 3) == 0) {
            if (sscanf(comando, "rm %d", &id) == 1) {
                remover_tarefa_ui(id);
            }
        } else if (strncmp(comando, "mv ", 3) == 0) {
            if (sscanf(comando, "mv %d", &id) == 1) {
                mover_tarefa_ui(id);
            }
        } else if (strcmp(comando, "help") == 0 || strcmp(comando, "?") == 0) {
            exibir_help();
        } else if (strncmp(comando, "ed ", 3) == 0) {
            if (sscanf(comando, "ed %d", &id) == 1) {
                editar_tarefa(id);
            }
        } else if (strncmp(comando, "in ", 3) == 0) {
            if (sscanf(comando, "in %d", &id) == 1) {
                incrementar_tarefa(id);
            }
        } else if (strncmp(comando, "vi ", 3) == 0) {
            if (sscanf(comando, "vi %d", &id) == 1) {
                visualizar_incrementos(id);
            }
        } else if (strlen(comando) > 0) {
            mostrar_erro("Comando desconhecido! Digite 'help'");
        }

        refresh();
    }

    endwin();
    return 0;
}