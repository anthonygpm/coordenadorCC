// Grupo: Anthony Gabriel, Vinicius Gabriel, Lucas Teodosio.

/* 
Devemos fazer um programa que gere a oferta acadêmica de CC da UFAL. A entrada será o histórico de n alunos, e devemos a partir disso, 
alocar as matérias que serão ofertadas no período, junto com o professor que lecionará a matéria e sala que ela será ministrada.

Não é permitido que o um professor ou sala estejam alocados em mais de uma disciplina no mesmo dia e horários. - FEITO

A carga horária do professor e sua formação devem ser consideradas para alocá-lo a uma disciplina, 
por convenção todos os professores do curso estão habilitados a lecionar todas as disciplinas dos 4 primeiros períodos do curso. 

Sendo as disciplinas de enfase e dos 4 últimos períodos do curso alocados de acordo com o trabalho de mestrado e doutorado do professor. 

As salas laboratórios devem ser alocadas preferencialmente para as disciplinas que envolvam programação. 

É obrigatório que o professor esteja matriculado em pelo menos 1 disciplina.
*/


// O professor deve ter um limite máximo de disciplina por semestre planejado, igual a: máximo 3; - FEITO
// considere a possibilidade de solicitar um professor substituto para lecionar a disciplina; - FEITO
// os professores deve ser alocados no menor números de dias possíveis - FEITO
// as disciplinas obrigatórias devem ter maior prioridade - FEITO

/*
Todas as disciplinas devem ser ofertadas, exceto:
o número de alunos a serem matriculados na mesma for inferior a 10 e nenhum deles esteja em no prazo máximo de conclusão de curso;
que o número de alunos a serem matriculados na mesma é igual a 0;
que não existir professor capacitado a ministrar a referida disciplina (o que acontece hoje em dia com a eletiva de FPGA);
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ALUNOS 1000
#define MAX_SALAS 200
#define MAX_PROFS 200
#define MAX_DISCIPLINAS 50
#define MAX_AULAS 500
#define LINE_BUFFER 256

// Estrutura para aula alocada (horário/dia)
typedef struct {
    char professor[100];
    char sala[50];
    char dia[4];
    char periodo;  // 'M' ou 'T'
    int inicio;
    int fim;
} AulaAlocada;

// Estrutura de disciplina (lista encadeada)
typedef struct Disciplina {
    char codigo_disciplina[20];
    char nome_disciplina[100];
    float media;
    int completa;
    struct Disciplina *prox;
} Disciplina;

// Estrutura de aluno
typedef struct {
    char nome_aluno[100];
    int periodo;
    char enfase;
    Disciplina *disciplinas_cursadas;
} Aluno;

// Estrutura de sala
typedef struct {
    char nome_sala[50];
    int capacidade;
} Sala;

// Estrutura para disciplinas que professor leciona
typedef struct DisciplinaProfessor {
    char codigo[20];
    struct DisciplinaProfessor *prox;
} DisciplinaProfessor;

// Estrutura de professor
typedef struct {
    char nome_professor[100];
    DisciplinaProfessor *disciplinas; // Lista encadeada
    int carga_horaria;
    int disciplinas_alocadas;
} Professor;

// Estrutura de necessidade de disciplina
typedef struct {
    char codigo[20];
    char nome[100];
    int periodo;
    int obrigatoria;
    double necessidade;
    int pre_requisitos;
    int ch_disc;
    int completa_arquivo;
    char horario[20];
    char dia_aula[20];
    char enfase[20];
} NecessidadeDisciplina;

// Estrutura de oferta
typedef struct {
    char codigo_disciplina[20];
    char nome_disciplina[100];
    char nome_professor[100];
    char nome_sala[50];
    double nivel_interesse;
    char horario[20];       
    char dias_aula[50];
} Oferta;

// Função auxiliar para decompor dias da semana
void decompor_dias(const char *str_dias, char dias[][4], int *num_dias) {
    *num_dias = 0;
    int len = strlen(str_dias);
    if (len == 0) return;
    
    char buffer[50];
    strcpy(buffer, str_dias);
    
    char *token = strtok(buffer, ",");
    while (token != NULL && *num_dias < 7) {
        strncpy(dias[*num_dias], token, 3);
        dias[*num_dias][3] = '\0';
        (*num_dias)++;
        token = strtok(NULL, ",");
    }
}

// Função auxiliar para extrair informações do horário
int extrair_horario(const char *horario_str, char *periodo, int *inicio, int *fim) {
    if (strlen(horario_str) != 4) {
        return -1;
    }
    *periodo = horario_str[0];
    if (*periodo != 'M' && *periodo != 'T') {
        return -1;
    }
    if (!isdigit(horario_str[1])) {
        return -1;
    }
    *inicio = horario_str[1] - '0';
    if (horario_str[2] != *periodo) {
        return -1;
    }
    if (!isdigit(horario_str[3])) {
        return -1;
    }
    *fim = horario_str[3] - '0';
    return 0;
}

int read_alunos(const char *filename, Aluno alunos[], int max) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo de alunos");
        return 0;
    }

    char line[LINE_BUFFER];
    int count = 0;
    fgets(line, LINE_BUFFER, fp); // Ignora cabeçalho

    while (fgets(line, LINE_BUFFER, fp)) {
        char *token = strtok(line, ";,\t");
        if (!token) continue;

        char nome_aluno[100];
        int periodo;
        char codigo[20], nome_disciplina[100];
        float media;

        strcpy(nome_aluno, token);
        token = strtok(NULL, ";\t");
        if (!token) continue;
        periodo = atoi(token);
        token = strtok(NULL, ";\t");
        if (!token) continue;
        strcpy(codigo, token);
        token = strtok(NULL, ";\t");
        if (!token) continue;
        strcpy(nome_disciplina, token);
        token = strtok(NULL, ";\t\n");
        if (!token) continue;
        media = atof(token);
        token = strtok(NULL, ";\t");
        if (!token) continue;
        alunos[count].enfase = token[0];

        // Verifica se o aluno já existe
        int i;
        for (i = 0; i < count; i++) {
            if (strcmp(alunos[i].nome_aluno, nome_aluno) == 0 && alunos[i].periodo == periodo) {
                break;
            }
        }

        if (i == count) {
            // Novo aluno
            if (count >= max) {
                fprintf(stderr, "Limite de alunos excedido.\n");
                break;
            }
            strcpy(alunos[count].nome_aluno, nome_aluno);
            alunos[count].periodo = periodo;
            alunos[count].disciplinas_cursadas = NULL;
            i = count;
            count++;
        }

        // Cria nova disciplina e insere no início da lista
        Disciplina *nova = (Disciplina *)malloc(sizeof(Disciplina));
        if (!nova) {
            perror("Erro de alocação de memória");
            exit(1);
        }
        strcpy(nova->codigo_disciplina, codigo);
        strcpy(nova->nome_disciplina, nome_disciplina);
        nova->media = media;
        nova->completa = media >= 7 ? 1 : 0;
        nova->prox = alunos[i].disciplinas_cursadas;
        alunos[i].disciplinas_cursadas = nova;
    }

    fclose(fp);
    return count;
}

int read_salas(const char *filename, Sala salas[], int max) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo de salas");
        return 0;
    }
    char line[LINE_BUFFER];
    int count = 0;
    fgets(line, LINE_BUFFER, fp);
    while (fgets(line, LINE_BUFFER, fp) && count < max) {
        char *token = strtok(line, ";,\t");
        if (!token) continue;
        strcpy(salas[count].nome_sala, token);
        token = strtok(NULL, ";,\t\n");
        salas[count].capacidade = atoi(token);
        count++;
    }
    fclose(fp);
    return count;
}

int read_professores(const char *filename, Professor profs[], int max) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo de professores");
        return 0;
    }
    char line[LINE_BUFFER];
    int count = 0;

    // pula cabeçalho
    fgets(line, LINE_BUFFER, fp);

    while (fgets(line, LINE_BUFFER, fp) && count < max) {
        char *save_semicolon;
        char *save_comma;
        char *token = strtok_r(line, ";\n", &save_semicolon);
        if (!token) continue;

        // 1) Nome do professor
        strncpy(profs[count].nome_professor, token, sizeof(profs[count].nome_professor)-1);
        profs[count].nome_professor[sizeof(profs[count].nome_professor)-1] = '\0';

        // inicializa lista
        profs[count].disciplinas = NULL;
        profs[count].disciplinas_alocadas = 0;

        // 2) Campo “Disciplinas” (códigos separados por vírgula)
        token = strtok_r(NULL, ";\n", &save_semicolon);
        if (token) {
            // usamos um buffer para não corromper save_semicolon
            char buffer[LINE_BUFFER];
            strncpy(buffer, token, LINE_BUFFER-1);
            buffer[LINE_BUFFER-1] = '\0';

            // agora separe por vírgula com strtok_r
            char *disc = strtok_r(buffer, ",", &save_comma);
            while (disc) {
                DisciplinaProfessor *no = malloc(sizeof(*no));
                if (!no) { perror("malloc"); exit(1); }
                strncpy(no->codigo, disc, sizeof(no->codigo)-1);
                no->codigo[sizeof(no->codigo)-1] = '\0';
                no->prox = profs[count].disciplinas;
                profs[count].disciplinas = no;

                disc = strtok_r(NULL, ",", &save_comma);
            }
        }

        // 3) Carga horária
        token = strtok_r(NULL, ";\n", &save_semicolon);
        profs[count].carga_horaria = token ? atoi(token) : 0;

        count++;
    }

    fclose(fp);
    return count;
}

int read_materias(const char *filename, NecessidadeDisciplina materias[], int max) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo de materias");
        return 0;
    }
    char line[LINE_BUFFER];
    int count = 0;
    fgets(line, LINE_BUFFER, fp); // Ignora cabeçalho

    while (fgets(line, LINE_BUFFER, fp) && count < max) {
        char *token = strtok(line, ";\n");
        if (!token) continue;

        // Codigo
        strcpy(materias[count].codigo, token);

        // Nome
        token = strtok(NULL, ";\n");
        if (!token) continue;
        strcpy(materias[count].nome, token);

        // Periodo
        token = strtok(NULL, ";\n");
        if (!token) continue;
        materias[count].periodo = atoi(token);

        // Pre-requisitos (ignorado)
        token = strtok(NULL, ";\n");
        
        // Carga horária
        token = strtok(NULL, ";\n");
        if (token) materias[count].ch_disc = atoi(token);
        else materias[count].ch_disc = 72; // Default

        // Obrigatoria
        token = strtok(NULL, ";\n");
        if (!token) continue;
        materias[count].obrigatoria = (atoi(token) == 1) ? 1 : 0;

        // Completa (ignorado)
        token = strtok(NULL, ";\n");
        materias[count].completa_arquivo = 0;

        // Horario
        token = strtok(NULL, ";\n");
        if (token) strcpy(materias[count].horario, token);
        else strcpy(materias[count].horario, "T34"); // Default

        // Dia de aula
        token = strtok(NULL, ";\n");
        if (token) strcpy(materias[count].dia_aula, token);
        else strcpy(materias[count].dia_aula, "SEG,QUA"); // Default

        token = strtok(NULL, ";\n");
        if (token) strcpy(materias[count].enfase, token);
        else strcpy(materias[count].enfase, "");

        materias[count].necessidade = 0;
        count++;
    }
    fclose(fp);
    return count;
}

void calcular_prioridade(NecessidadeDisciplina materias[], int n_materias, 
                        Aluno alunos[], int n_alunos) {
    // Primeiro, passa pelas disciplinas cursadas
    for (int i = 0; i < n_alunos; i++) {
        Disciplina *d = alunos[i].disciplinas_cursadas;
        while (d != NULL) {
            if (d->completa == 0) {  // Usa o status do aluno, não do arquivo
                for (int j = 0; j < n_materias; j++) {
                    if (strcmp(materias[j].codigo, d->codigo_disciplina) == 0 &&
                        materias[j].periodo <= alunos[i].periodo) { // Usa o período da disciplina 
                        if (materias[j].obrigatoria == 1) {
                            materias[j].necessidade += 3; // peso 3 para disciplina obrigatória
                        } else {
                            materias[j].necessidade += 0.5; // peso 0.5 para disciplina eletiva
                        }
                        break;
                    }
                }
            }
            d = d->prox;
        }
    }

    // Depois, adicionava mais 1 nas matérias da ênfase de interesse
    for (int i = 0; i < n_alunos; i++) {
        if (alunos[i].enfase != '0') {  // aluno tem alguma ênfase
            for (int j = 0; j < n_materias; j++) {
                if (strcmp(materias[j].enfase, "0") != 0) { // matéria tem ênfase específica
                    for (int c = 0; c < 5; c += 2) {
                        int k;
                        for (k = 0; k <= MAX_DISCIPLINAS; k++) {
                            if (strcmp(materias[j].codigo, alunos[i].disciplinas_cursadas[k].codigo_disciplina) == 0) break;
                        }
                        if (materias[j].enfase[c] == alunos[i].enfase && alunos[i].disciplinas_cursadas[k].completa != 1) {
                            materias[j].necessidade += 0.5;
                            break;
                        }
                    }
                }
            }
        }
    }
}

int professor_pode(Professor *p, const char *codigo_disciplina) {

    DisciplinaProfessor *d = p->disciplinas;

    int pode = 0;
    while (d) {
        if (strcmp(d->codigo, codigo_disciplina) == 0) {
            pode = 1;
            // não dá break aqui, para listar todas as disciplinas
        }
        d = d->prox;
    }

    if (pode) {
        return 1;
    } else {
        return 0;
    }
}

void formatar_horario_legivel(const char *horario_str, char *saida, int tamanho_saida) {
    char periodo;
    int inicio, fim;
    if (extrair_horario(horario_str, &periodo, &inicio, &fim) == 0) {
        const char *periodo_nome = (periodo == 'M') ? "Manhã" : "Tarde";
        snprintf(saida, tamanho_saida, "%s: %dª à %dª aula", periodo_nome, inicio, fim);
    } else {
        snprintf(saida, tamanho_saida, "Horário inválido");
    }
}

int conflito(AulaAlocada *a, AulaAlocada *b) {
    if (strcmp(a->dia, b->dia) != 0) return 0;
    if (a->periodo != b->periodo) return 0;
    if (a->inicio < b->fim && a->fim > b->inicio) {
        return 1;
    }
    return 0;
}

// Função para verificar conflitos em uma sala
int sem_conflito_sala(char *nome_sala, char dias[][4], int num_dias, char periodo, int inicio, int fim, AulaAlocada *aulas, int n_aulas) {
    for (int d = 0; d < num_dias; d++) {
        AulaAlocada tmp;
        strcpy(tmp.sala, nome_sala);
        strcpy(tmp.dia, dias[d]);
        tmp.periodo = periodo;
        tmp.inicio = inicio;
        tmp.fim = fim;

        for (int i = 0; i < n_aulas; i++) {
            if (strcmp(aulas[i].sala, nome_sala) == 0) {
                if (conflito(&aulas[i], &tmp)) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int alocar_disciplinas(NecessidadeDisciplina materias[], int n_materias, 
                      Professor profs[], int n_profs, Sala salas[], int n_salas, 
                      Oferta ofertas[], int max_ofertas) {
    int count = 0;
    AulaAlocada aulas_alocadas[MAX_AULAS];
    int n_aulas_alocadas = 0;

    for (int i = 0; i < n_materias && count < max_ofertas; i++) {
        if (materias[i].necessidade == 0) continue;

        // Extrair horário da matéria
        char periodo;
        int inicio, fim;
        if (extrair_horario(materias[i].horario, &periodo, &inicio, &fim) != 0) {
            continue;
        }

        // Decompor dias da matéria
        char dias[7][4];
        int num_dias;
        decompor_dias(materias[i].dia_aula, dias, &num_dias);

        // Tentar encontrar professor disponível
        int prof_index = -1;
        for (int j = 0; j < n_profs; j++) {
            if (professor_pode(&profs[j], materias[i].codigo) && profs[j].disciplinas_alocadas == 0) {
                prof_index = j;
                profs[j].disciplinas_alocadas++;
                break;
            }
        }
        /* se der como resultado um professor substituto, ele procura um que já foi alocado em outra matéria, 
        fazendo com que todos sejam obrigatoriamente alocados naquelas matérias que são capazes de lecionar e no menor número de dias possível */
        if (prof_index == -1) {
            for (int j = 0; j < n_profs; j++) {
                if (professor_pode(&profs[j], materias[i].codigo) && profs[j].disciplinas_alocadas <= 3) {
                    prof_index = j;
                    break;
                }
            }
        }

        // Tentar encontrar sala sem conflito
        int sala_index = -1;
        for (int k = 0; k < n_salas; k++) {
            if (salas[k].capacidade < materias[i].necessidade) {
                continue;
            }
            if (sem_conflito_sala(salas[k].nome_sala, dias, num_dias, periodo, inicio, fim, aulas_alocadas, n_aulas_alocadas)) {
                sala_index = k;
                break;
            }
        }

        // Criar oferta
        strcpy(ofertas[count].codigo_disciplina, materias[i].codigo);
        strcpy(ofertas[count].nome_disciplina, materias[i].nome);
        if (prof_index == -1) {
            strcpy(ofertas[count].nome_professor, "Professor substituto");
        } else {
            strcpy(ofertas[count].nome_professor, profs[prof_index].nome_professor);
        }
        if (sala_index == -1) {
            strcpy(ofertas[count].nome_sala, "A matéria deverá ser dividida entre duas salas");
        } else {
            strcpy(ofertas[count].nome_sala, salas[sala_index].nome_sala);
        }
        strcpy(ofertas[count].horario, materias[i].horario);
        strcpy(ofertas[count].dias_aula, materias[i].dia_aula);
        ofertas[count].nivel_interesse = materias[i].necessidade;

        // Registrar aulas alocadas
        for (int d = 0; d < num_dias; d++) {
            strcpy(aulas_alocadas[n_aulas_alocadas].sala, salas[sala_index].nome_sala);
            strcpy(aulas_alocadas[n_aulas_alocadas].dia, dias[d]);
            aulas_alocadas[n_aulas_alocadas].periodo = periodo;
            aulas_alocadas[n_aulas_alocadas].inicio = inicio;
            aulas_alocadas[n_aulas_alocadas].fim = fim;
            n_aulas_alocadas++;
        }
        count++;
    }
    return count;
}

int comparar_necessidade(const void *a, const void *b) {
    const NecessidadeDisciplina *m1 = (const NecessidadeDisciplina *)a;
    const NecessidadeDisciplina *m2 = (const NecessidadeDisciplina *)b;
    return m2->necessidade - m1->necessidade; // Ordem decrescente
}

void ordenar_materias_por_necessidade(NecessidadeDisciplina materias[], int n) {
    qsort(materias, n, sizeof(NecessidadeDisciplina), comparar_necessidade);
}

void imprimir_ofertas(Oferta ofertas[], int n) {
    printf("\n--- Disciplinas Ofertadas no Semestre ---\n");
    for (int i = 0; i < n; i++) {
        if (ofertas[i].nivel_interesse < 10) continue;
        printf("Disciplina: %s (%s)\n", ofertas[i].nome_disciplina, ofertas[i].codigo_disciplina);
        printf("  Professor: %s\n", ofertas[i].nome_professor);
        printf("  Sala: %s\n", ofertas[i].nome_sala);
        
        // Formata e exibe o horário
        char horario_legivel[50];
        formatar_horario_legivel(ofertas[i].horario, horario_legivel, sizeof(horario_legivel));
        printf("  Horário: %s\n", horario_legivel);
        
        // Exibe os dias decompondo a string
        char dias[7][4];
        int num_dias;
        decompor_dias(ofertas[i].dias_aula, dias, &num_dias);
        printf("  Dias: ");
        for (int d = 0; d < num_dias; d++) {
            printf("%s", dias[d]);
            if (d < num_dias-1) printf(", ");
        }
        printf("\n");
        
        printf("  Nível de interesse: %.0f\n", ofertas[i].nivel_interesse);
        printf("\n");
    }
}

void liberar_alunos(Aluno alunos[], int n) {
    for (int i = 0; i < n; i++) {
        Disciplina *d = alunos[i].disciplinas_cursadas;
        while (d) {
            Disciplina *tmp = d;
            d = d->prox;
            free(tmp);
        }
    }
}

void liberar_professores(Professor profs[], int n) {
    for (int i = 0; i < n; i++) {
        DisciplinaProfessor *d = profs[i].disciplinas;
        while (d) {
            DisciplinaProfessor *tmp = d;
            d = d->prox;
            free(tmp);
        }
    }
}

int main() {
    Aluno alunos[MAX_ALUNOS];
    Sala salas[MAX_SALAS];
    Professor profs[MAX_PROFS];
    NecessidadeDisciplina materias[MAX_DISCIPLINAS];
    Oferta ofertas[MAX_DISCIPLINAS];

    int n_alunos = read_alunos("alunos.csv", alunos, MAX_ALUNOS);
    printf("Alunos carregados: %d\n", n_alunos);
    
    int n_salas = read_salas("salas.csv", salas, MAX_SALAS);
    printf("Salas carregadas: %d\n", n_salas);
    
    int n_profs = read_professores("professores.csv", profs, MAX_PROFS);
    printf("Professores carregados: %d\n", n_profs);
    
    int n_materias = read_materias("materias.csv", materias, MAX_DISCIPLINAS);
    printf("Disciplinas carregadas: %d\n", n_materias);

    calcular_prioridade(materias, n_materias, alunos, n_alunos);
    ordenar_materias_por_necessidade(materias, n_materias);

    int n_ofertas = alocar_disciplinas(materias, n_materias, profs, n_profs, salas, n_salas, ofertas, MAX_DISCIPLINAS);
    printf("\nTotal de ofertas alocadas: %d\n", n_ofertas);

    imprimir_ofertas(ofertas, n_ofertas);

    liberar_alunos(alunos, n_alunos);
    liberar_professores(profs, n_profs);
    
    return 0;
}
