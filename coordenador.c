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


// O professor deve ter um limite máximo de disciplina por semestre planejado, igual a: máximo 3; - FEITO
// considere a possibilidade de solicitar um professor substituto para lecionar a disciplina;
// os professores deve ser alocados no menor números de dias possíveis
// as disciplinas obrigatórias devem ter maior prioridade - FEITO

/*
Todas as disciplinas devem ser ofertadas, exceto:
o número de alunos a serem matriculados na mesma for inferior a 10 e nenhum deles esteja em no prazo máximo de conclusão de curso;
que o número de alunos a serem matriculados na mesma é igual a 0;
que não existir professor capacitado a ministrar a referida disciplina (o que acontece hoje em dia com a eletiva de FPGA);
*/

/*
PROBLEMA - nos professores, é melhor adicionar uma lista encadeada das matérias que ele pode lecionar, já que no momento, 
está sendo limitado a ele apenas 3 matérias possíveis de lecionar.

PROBLEMA - a lista de alunos está muito aleatória, com matérias meio desconexas.

PROBLEMA - na contagem, ele está adicionando perfeitamente para as matérias obrigatórias, sendo que, para as metérias eletivas,
ele está adicionando apenas naquelas que o aluno reprovou/trancou, e não nas eletivas seguintes.

TODO - adicionar que se não houver professor disponível para a matéria, recomendar solicitar um professor substituto.
TODO - adicionar o sistema de dias e horários, para não se chocarem.
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

// Estrutura de aluno
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
    int disciplinas_alocadas;
} Professor;

// Estrutura de necessidade de disciplina
typedef struct {
    char codigo[20];
    char nome[100];
    int periodo;
    int obrigatoria;
    int necessidade;
    // Novos campos (não usados na lógica atual, mas lidos)
    int pre_requisitos;
    int ch_disc;
    int completa_arquivo;  // Renomeado para não conflitar com o campo 'completa' do aluno
    char horario[20];
    char dia_aula[20];
} NecessidadeDisciplina;

// Estrutura de oferta
typedef struct {
    char codigo_disciplina[20];
    char nome_disciplina[100];
    char nome_professor[100];
    char nome_sala[50];
    int alunos_interessados;
} Oferta;

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
    fgets(line, LINE_BUFFER, fp); // Ignora cabeçalho

    while (fgets(line, LINE_BUFFER, fp) && count < max) {
        char *saveptr;
        char *token = strtok_r(line, ";\n", &saveptr);
        if (!token) continue;

        // Nome do professor
        strcpy(profs[count].nome_professor, token);
        profs[count].num_disciplinas = 0;
        profs[count].disciplinas_alocadas = 0;

        // Processa os tokens restantes
        char *last_token = NULL;
        while ((token = strtok_r(NULL, ";\n", &saveptr)) != NULL) {
            // Verifica se é numérico (carga horária)
            char *endptr;
            long val = strtol(token, &endptr, 10);
            
            if (*endptr == '\0') { // É numérico (carga horária)
                profs[count].carga_horaria = (int)val;
                break;
            } 
            else { // É disciplina
                if (profs[count].num_disciplinas < MAX_DISCIPLINAS) {
                    strcpy(profs[count].disciplinas[profs[count].num_disciplinas], token);
                    profs[count].num_disciplinas++;
                }
            }
            last_token = token;
        }
        count++;
    }

    fclose(fp);
    return count;
}

int read_materias(const char *filename, NecessidadeDisciplina materias[], int max) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
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

        // Pre-requisitos (não usado)
        token = strtok(NULL, ";\n");

        // Carga horária (não usado)
        token = strtok(NULL, ";\n");
        if (token) materias[count].ch_disc = atoi(token);

        // Obrigatoria
        token = strtok(NULL, ";\n");
        if (!token) continue;
        materias[count].obrigatoria = (atoi(token) == 1) ? 1 : 0;

        // Completa (não usado na lógica)
        token = strtok(NULL, ";\n");
        if (token) materias[count].completa_arquivo = atoi(token);

        // Horario
        token = strtok(NULL, ";\n");
        if (token) strcpy(materias[count].horario, token);

        // Dia de aula
        token = strtok(NULL, ";\n");
        if (token) strcpy(materias[count].dia_aula, token);

        materias[count].necessidade = 0;
        count++;
    }
    fclose(fp);
    return count;
}

void calcular_prioridade(NecessidadeDisciplina materias[], int n_materias, 
                        Aluno alunos[], int n_alunos) {
    for (int i = 0; i < n_alunos; i++) {
        Disciplina *d = alunos[i].disciplinas_cursadas;
        while (d != NULL) {
            if (d->completa == 0) {  // Usa o status do aluno, não do arquivo
                for (int j = 0; j < n_materias; j++) {
                    if (strcmp(materias[j].codigo, d->codigo_disciplina) == 0 &&
                        materias[j].periodo <= alunos[i].periodo) { // Usa o período da disciplina 
                        if (materias[j].obrigatoria == 1) {
                            materias[j].necessidade += 2; // peso 2 para disciplina obrigatória
                        } else {
                            materias[j].necessidade++; // peso 1 para disciplina eletiva
                        }
                        break;
                    }
                }
            }
            d = d->prox;
        }
    }
}

int professor_pode(Professor *p, const char *codigo_disciplina) {
    for (int i = 0; i < p->num_disciplinas; i++) {
        if (strcmp(p->disciplinas[i], codigo_disciplina) == 0) return 1;
    }
    return 0;
}

int alocar_disciplinas(NecessidadeDisciplina materias[], int n_materias, 
                      Professor profs[], int n_profs, Sala salas[], int n_salas, 
                      Oferta ofertas[], int max_ofertas) {
    int count = 0;
    for (int i = 0; i < n_materias && count < max_ofertas; i++) {
        if (materias[i].necessidade == 0) continue;

        int prof_index = -1, sala_index = -1;

        // Encontra professor disponível
        for (int j = 0; j < n_profs; j++) {
            if (professor_pode(&profs[j], materias[i].codigo) && 
                profs[j].carga_horaria > 0 &&
                profs[j].disciplinas_alocadas < 3) {  // NOVA CONDIÇÃO
                prof_index = j;
                break;
            }
        }

        // Encontra sala
        for (int j = 0; j < n_salas; j++) {
            if (salas[j].capacidade >= materias[i].necessidade) {
                sala_index = j;
                break;
            }
        }

        if (prof_index != -1 && sala_index != -1) {
            // Preenche a oferta
            strcpy(ofertas[count].codigo_disciplina, materias[i].codigo);
            strcpy(ofertas[count].nome_disciplina, materias[i].nome);
            strcpy(ofertas[count].nome_professor, profs[prof_index].nome_professor);
            strcpy(ofertas[count].nome_sala, salas[sala_index].nome_sala);
            ofertas[count].alunos_interessados = materias[i].necessidade;

            // Atualiza informações
            salas[sala_index].capacidade -= materias[i].necessidade;
            profs[prof_index].carga_horaria--;
            profs[prof_index].disciplinas_alocadas++;  // INCREMENTA A ALOCAÇÃO

            count++;
        }
    }
    return count;
}

void ordenar_materias_por_necessidade(NecessidadeDisciplina materias[], int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = i+1; j < n; j++) {
            if (materias[j].necessidade > materias[i].necessidade) {
                NecessidadeDisciplina temp = materias[i];
                materias[i] = materias[j];
                materias[j] = temp;
            }
        }
    }
}

void imprimir_ofertas(Oferta ofertas[], int n) {
    printf("\n--- Disciplinas Ofertadas no Semestre ---\n");
    for (int i = 0; i < n; i++) {
        printf("Disciplina: %s (%s)\n", ofertas[i].nome_disciplina, ofertas[i].codigo_disciplina);
        printf("  Professor: %s\n", ofertas[i].nome_professor);
        printf("  Sala: %s\n", ofertas[i].nome_sala);
        printf("  Alunos interessados: %d\n", ofertas[i].alunos_interessados);
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

int main() {
    Aluno alunos[MAX_ALUNOS];
    Sala salas[MAX_SALAS];
    Professor profs[MAX_PROFS];
    NecessidadeDisciplina materias[MAX_DISCIPLINAS];
    Oferta ofertas[MAX_DISCIPLINAS];

    int n_alunos = read_alunos("alunos.csv", alunos, MAX_ALUNOS);
    if(n_alunos == 0) {
        printf("Nenhum aluno carregado!\n");
        return 1;
    }

    int n_salas = read_salas("salas.csv", salas, MAX_SALAS);
    if (n_salas == 0) {
        printf("Erro: Nenhuma sala carregada!\n");
        return 1;
    }

    int n_profs = read_professores("professores.csv", profs, MAX_PROFS);
    if (n_profs == 0) {
        printf("Erro: Nenhum professor carregado!\n");
        return 1;
    }

    int n_materias = read_materias("materias.csv", materias, MAX_DISCIPLINAS);
    if (n_materias == 0) {
        printf("Erro: Nenhuma disciplina carregada!\n");
        return 1;
    }

    
    
    calcular_prioridade(materias, n_materias, alunos, n_alunos);
    
    // for (int i = 0; i < n_materias; i++) {
    //     printf("Disciplina: %s (%s) | Obrigatória: %d | Necessidade: %d\n",
    //         materias[i].nome, materias[i].codigo, materias[i].obrigatoria, materias[i].necessidade);
    // }
    
    ordenar_materias_por_necessidade(materias, n_materias);

    int n_ofertas = alocar_disciplinas(materias, n_materias, profs, n_profs, salas, n_salas, ofertas, MAX_DISCIPLINAS);

    imprimir_ofertas(ofertas, n_ofertas);

    liberar_alunos(alunos, n_alunos);
    return 0;
}
