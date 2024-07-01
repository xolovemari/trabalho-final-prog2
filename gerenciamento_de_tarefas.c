#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0
#define MAX 100

typedef int bool;
typedef char TIPOCHAVE[MAX];

typedef struct
{
    TIPOCHAVE chave[MAX];
} REGISTRO;

typedef struct aux
{
    REGISTRO reg;
    struct aux *prox;
} ELEMENTO;

typedef ELEMENTO *PONT;

typedef struct
{
    PONT inicio;
} LISTA;

void inicializarLista(LISTA *l)
{
    l->inicio = NULL;
}

int tamanho(LISTA *l)
{
    PONT end = l->inicio;
    int tam = 0;
    while (end != NULL)
    {
        tam++;
        end = end->prox;
    }
    return tam;
}

void exibirLista(LISTA *l)
{
    PONT end = l->inicio;
    printf("Lista:\n");
    int i = 1;
    while (end != NULL)
    {
        printf("  %i - %s \n", i, end->reg.chave);
        i++;
        end = end->prox;
    }
    printf("\n");
}

bool insere(LISTA *l, REGISTRO reg, int pos)
{
    if (pos < 0 || pos > tamanho(l))
        return false;
    ELEMENTO *novo = (ELEMENTO *)malloc(sizeof(ELEMENTO));
    novo->reg = reg;
    int i;
    ELEMENTO *p;
    if (pos == 0)
    {
        novo->prox = l->inicio;
        l->inicio = novo;
    }
    else
    {
        p = l->inicio;
        for (i = 0; i < pos - 1; i++)
            p = p->prox;
        novo->prox = p->prox;
        p->prox = novo;
    }
    return true;
}

bool exclui(LISTA *l, int pos, FILE *historico)
{
    if (pos <= 0 || pos > tamanho(l))
        return false;
    int i;
    ELEMENTO *p;
    ELEMENTO *apagar;
    if (pos == 1)
    {
        apagar = l->inicio;
        fprintf(historico, "Tarefa excluida: \"%s\"\n", apagar->reg.chave);
        l->inicio = apagar->prox;
    }
    else
    {
        p = l->inicio;
        for (i = 1; i < pos - 1; i++)
            p = p->prox;
        apagar = p->prox;
        fprintf(historico, "Tarefa excluida: \"%s\"\n", apagar->reg.chave);
        p->prox = apagar->prox;
    }
    free(apagar);
    return true;
}

void reinicializarFila(LISTA *l)
{
    PONT end = l->inicio;
    while (end != NULL)
    {
        PONT apagar = end;
        end = end->prox;
        free(apagar);
    }
    l->inicio = NULL;
}

void menu(LISTA *lista, REGISTRO palavra, FILE *historico);
bool modificaElemento(LISTA *lista, REGISTRO reg, int pos, FILE *historico);
bool modifica(LISTA *lista, REGISTRO palavra, int aux_pos, FILE *historico);

//  “tarefas.bin” para salvar as tarefas e “historico.txt” para registrar quando tarefas são adicionadas ou concluídas.
// Remover, listar, excluir as tarefas pendentes

// Ao final de cada operação, o sistema atualizará o arquivo “tarefas.bin” com as mudanças mais recentes e
// registrará no arquivo “historico.txt” todas as adições ou remoções de tarefas realizadas.

// Antes de listar as tarefas, o sistema implementará a pesquisa binária para encontrar tarefas por prazo,
// considerando que a fila está ordenada e usará o algoritmo selection sort para ordenar as tarefas por prazo, facilitando a visualização do usuário.

// Após qualquer operação de gerenciamento, será possível verificar no arquivo “historico.txt” o
// registro detalhado das operações, oferecendo uma trilha de auditoria completa das atividades realizadas.

int main()
{
    LISTA lista;
    inicializarLista(&lista);
    REGISTRO palavra;
    FILE *arq_tarefas, *arq_historico;

    arq_tarefas = fopen("tarefas.bin", "ab+"); // abre um arquivo para adicao de dados, leitura e escrita, caso o arquivo nao existir, cria ele
    arq_historico = fopen("historico.txt", "w");

    menu(&lista, palavra, arq_historico);

    fwrite(&lista, sizeof(lista), 1, arq_tarefas);

    reinicializarFila(&lista);
    fclose(arq_tarefas);
    return 0;
}

bool modificaElemento(LISTA *lista, REGISTRO reg, int pos, FILE *historico)
{
    PONT p = lista->inicio;
    if (pos > tamanho(lista) || pos < 1)
    {
        return false;
    }

    for (int i = 0; i < pos; i++)
    {
        p = p->prox;
    }

    fprintf(historico, "Tarefa modificada, de \"%s\", para \"%s\"\n", p->reg.chave, reg.chave);
    strcpy((char *)p->reg.chave, (char *)reg.chave);
    return true;
}

void menu(LISTA *lista, REGISTRO palavra, FILE *historico)
{
    int aux_pos;
    printf(" 1 - Escrever tarefas\n 2 - Modificar alguma tarefa \n 3 - Excluir uma tarefa\n 4 - Imprimir a Lista\n 0 - Sair\n");
    while (true)
    {
        scanf(" %[^\n]", palavra.chave);
        if (strcmp((char *)palavra.chave, "0") == 0)
            break;
        if (strcmp((char *)palavra.chave, "1") == 0)
        {
            puts("Digite as tarefas que deseja escrever, 0 para voltar");
            while (true)
            {
                scanf(" %[^\n]", palavra.chave);
                if (strcmp((char *)palavra.chave, "0") == 0)
                {
                    printf(" 1 - Escrever tarefas\n 2 - Modificar alguma tarefa \n 3 - Excluir uma tarefa\n 4 - Imprimir a Lista\n 0 - Sair\n");
                    break;
                }
                fprintf(historico, "Tarefa escrita: \"%s\"\n", palavra.chave);
                insere(lista, palavra, tamanho(lista));
            }
        }
        if (strcmp((char *)palavra.chave, "2") == 0)
        {
            puts("\nDe o numero do elemento deseja modificar, 0 para voltar");
            exibirLista(lista);
            scanf("%d", &aux_pos);
            if (aux_pos == 0)
            {
                printf(" 1 - Escrever tarefas\n 2 - Modificar alguma tarefa \n 3 - Excluir uma tarefa\n 4 - Imprimir a Lista\n 0 - Sair\n");
                break;
            }

            modifica(lista, palavra, aux_pos, historico);
        }
        if (strcmp((char *)palavra.chave, "3") == 0)
        {
            printf("De o numero da tarefa concluida, 0 para voltar\n");
            exibirLista(lista);
            scanf("%d", &aux_pos);
            if (aux_pos == 0)
            {
                printf(" 1 - Escrever tarefas\n 2 - Modificar alguma tarefa \n 3 - Excluir uma tarefa\n 4 - Imprimir a Lista\n 0 - Sair\n");
                continue;
            }
            if (exclui(lista, aux_pos, historico) == false)
            {
                printf("Posicaoo invalida. Tente novamente.\n");
            }
            printf(" 1 - Escrever tarefas\n 2 - Modificar alguma tarefa\n 3 - Excluir uma tarefa\n 4 - Imprimir a Lista\n 0 - Sair\n");
        }
        if (strcmp((char *)palavra.chave, "4") == 0)
        {
            if (tamanho(lista) != 0)
            {
                exibirLista(lista);
                printf(" 1 - Escrever tarefas\n 2 - Modificar alguma tarefa \n 3 - Excluir uma tarefa\n 4 - Imprimir a Lista\n 0 - Sair\n");
            }
            else
                printf("Lista vazia\n\n  1 - Escrever tarefas\n 2 - Modificar alguma tarefa \n 3 - Excluir uma tarefa\n 4 - Imprimir a Lista\n 0 - Sair\n");
        }
    }
}

bool modifica(LISTA *lista, REGISTRO palavra, int aux_pos, FILE *historico)
{
    puts("O que deseja inserir no lugar?");
    scanf(" %[^\n]", palavra.chave);
    modificaElemento(lista, palavra, aux_pos - 1, historico);
    menu(lista, palavra, historico);
}
