// Grupo: Anthony Gabriel, Vinicius Gabriel, Lucas Teodosio.

/* 
Devemos fazer um programa que gere a oferta acadêmica de CC da UFAL. A entrada será o histórico de n alunos, e devemos a partir disso, 
alocar as matérias que serão ofertadas no período, junto com o professor que lecionará a matéria e sala que ela será ministrada.

Não é permitido que o um professor ou sala estejam alocados em mais de uma disciplina no mesmo dia e horários. 

A carga horária do professor e sua formação devem ser consideradas para alocá-lo a uma disciplina, 
por convenção todos os professores do curso estão habilitados a lecionar todas as disciplinas dos 4 primeiros períodos do curso. 

Sendo as disciplinas de enfase e dos 4 últimos períodos do curso alocados de acordo com o trabalho de mestrado e doutorado do professor. 

As salas laboratórios devem ser alocadas preferencialmente para as disciplinas que envolvam programação. 

É obrigatório que o professor esteja matriculado em pelo menos 1 disciplina.
*/


// O professor deve ter um limite máximo de disciplina por semestre planejado, igual a: máximo 3;
// considere a possibilidade de solicitar um professor substituto para lecionar a disciplina;
// os professores deve ser alocados no menor números de dias possíveis
// as disciplinas obrigatórias devem ter maior prioridade

/*
Todas as disciplinas devem ser ofertadas, exceto:
o número de alunos a serem matriculados na mesma for inferior a 10 e nenhum deles esteja em no prazo máximo de conclusão de curso;
que o número de alunos a serem matriculados na mesma é igual a 0;
que não existir professor capacitado a ministrar a referida disciplina (o que acontece hoje em dia com a eletiva de FPGA);
*/

/*
TODO a partir do período do aluno e matérias pagas, incrementar um contador nas matérias do período que o aluno vai cursar 
ou precisa cursar (que está atrasada)

após isso organizá-las de forma descrecente (maior prioriade até a menor) e ir alocando-as - como se fosse em uma pilha
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ALUNOS 1000
#define MAX_SALAS 200
#define MAX_PROFS 200
#define MAX_DISCIPLINAS 50
#define LINE_BUFFER 256

// Estrutura de disciplina (lista encadeada)
typedef struct Disciplina {
    char codigo_disciplina[20];
    char nome_disciplina[100];
    float media;
    int completa;
    struct Disciplina *prox;
} Disciplina;

// Estrutura de aluno com lista de disciplinas
typedef struct {
    char nome_aluno[100];
    int periodo;
    Disciplina *disciplinas_cursadas;
} Aluno;

// Estrutura de sala
typedef struct {
    char nome_sala[50];
    int capacidade;
} Sala;

// Estrutura de professor
typedef struct {
    char nome_professor[100];
    char disciplinas[MAX_DISCIPLINAS][20];
    int num_disciplinas;
    int carga_horaria;
} Professor;

// Função para leitura dos alunos com lista dinâmica de disciplinas
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
        token = strtok(NULL, ";,\t");
        if (!token) continue;
        periodo = atoi(token);
        token = strtok(NULL, ";,\t");
        if (!token) continue;
        strcpy(codigo, token);
        token = strtok(NULL, ";,\t");
        if (!token) continue;
        strcpy(nome_disciplina, token);
        token = strtok(NULL, ";,\t\n");
        if (!token) continue;
        media = atof(token);

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

// Função para leitura de salas
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

// Função para leitura de professores
int read_professores(const char *filename, Professor profs[], int max) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo de professores");
        return 0;
    }
    char line[LINE_BUFFER];
    int count = 0;
    fgets(line, LINE_BUFFER, fp);
    while (fgets(line, LINE_BUFFER, fp) && count < max) {
        char *token = strtok(line, ";,\t");
        if (!token) continue;
        strcpy(profs[count].nome_professor, token);
        profs[count].num_disciplinas = 0;
        while ((token = strtok(NULL, ";,\t")) != NULL && profs[count].num_disciplinas < MAX_DISCIPLINAS) {
            char *endptr;
            long val = strtol(token, &endptr, 10);
            if (*endptr == '\0') {
                profs[count].carga_horaria = (int)val;
                break;
            } else {
                strcpy(profs[count].disciplinas[profs[count].num_disciplinas++], token);
            }
        }
        count++;
    }
    fclose(fp);
    return count;
}

// Função para imprimir os dados dos alunos
void imprimir_alunos(Aluno alunos[], int n) {
    for (int i = 0; i < n; i++) {
        printf("Aluno: %s (Período %d)\n", alunos[i].nome_aluno, alunos[i].periodo);
        Disciplina *d = alunos[i].disciplinas_cursadas;
        while (d != NULL) {
            printf("  - %s (%s): %.2f, ", d->nome_disciplina, d->codigo_disciplina, d->media);
            if (d->completa == 1) printf("Aprovado\n");
            else if (d->media == -1) printf("Trancou\n");
            else printf("Reprovado\n");
            d = d->prox;
        }
    }
}

// Função para liberar a memória alocada das listas de disciplinas
void liberar_alunos(Aluno alunos[], int n) {
    for (int i = 0; i < n; i++) {
        Disciplina *d = alunos[i].disciplinas_cursadas;
        while (d != NULL) {
            Disciplina *tmp = d;
            d = d->prox;
            free(tmp);
        }
    }
}

// Função principal
int main() {
    Aluno alunos[MAX_ALUNOS];
    Sala salas[MAX_SALAS];
    Professor profs[MAX_PROFS];

    int n_alunos = read_alunos("alunos.csv", alunos, MAX_ALUNOS);
    int n_salas = read_salas("salas.csv", salas, MAX_SALAS);
    int n_profs = read_professores("professores.csv", profs, MAX_PROFS);

    printf("Total de alunos: %d\n", n_alunos);
    printf("Total de salas: %d\n", n_salas);
    printf("Total de professores: %d\n", n_profs);

    imprimir_alunos(alunos, n_alunos);

    liberar_alunos(alunos, n_alunos);

    return 0;
}
