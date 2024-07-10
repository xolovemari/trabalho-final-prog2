#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define true 1
#define false 0
#define MAX 100 // definicao do maximo de caracteres em cada tarefa

typedef int bool;
typedef char TIPOCHAVE[MAX]; // tipo chave = string

typedef struct
{
    TIPOCHAVE chave;
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

int tamanho(LISTA *l) // funcao padrao da implementacao de lista, percorre todos os elementos da lista, incrementendo um contador tam a cada elemento
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
    if (tamanho(l) == 0)
    {
        printf("Lista vazia\n");
    }

    PONT end = l->inicio;
    printf("Lista:\n");
    int i = 1;
    while (end != NULL)
    {
        printf("  %d - %s \n", i, end->reg.chave); // exibe a lista com i (numero da tarefa) - s (nome da tarefa)
        i++;
        end = end->prox;
    }
    printf("\n");
}

bool insere(LISTA *l, REGISTRO reg, int pos)
{
    if (pos < 0 || pos > tamanho(l)) // verifica se a posicao e valida para inserir o elemento
        return false;
    ELEMENTO *novo = (ELEMENTO *)malloc(sizeof(ELEMENTO)); // aloca memoria para o novo elemento
    if (novo == NULL)
        return false;
    novo->reg = reg;
    int i;
    ELEMENTO *p;
    if (pos == 0) // se a posicao for 0 insere o elemento no inicio da lista
    {
        novo->prox = l->inicio;
        l->inicio = novo;
    }
    else
    {
        p = l->inicio;
        for (i = 0; i < pos - 1; i++) // percorre a lista ate a posicao anterior a posicao
            p = p->prox;
        novo->prox = p->prox;
        p->prox = novo;
    }
    return true;
}

bool exclui(LISTA *l, int pos, FILE *historico)
{
    if (pos < 1 || pos > tamanho(l)) // verifica se a posicao e valida, se nao for retorna falso
        return false;
    int i;
    ELEMENTO *p;
    ELEMENTO *apagar;
    if (pos == 1) // se a posicao for 1, apaga o primeiro elemento
    {
        apagar = l->inicio;
        fprintf(historico, "Tarefa excluida: \"%s\"\n", apagar->reg.chave);
        l->inicio = apagar->prox;
    }
    else
    {
        p = l->inicio; // se nao, aponta para o incio da lisya e a percorre ate o elemento desejado
        for (i = 1; i < pos - 1; i++)
            p = p->prox;
        apagar = p->prox;
        fprintf(historico, "Tarefa excluida: \"%s\"\n", apagar->reg.chave); // imprime o elemento que foi apagado no historico
        p->prox = apagar->prox;                                             // atualiza o ponteiro para ser o proximo elemento do que esta sendo apagado, pulando-o
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
        free(apagar); // aponta-se para todos os elementos da lista e utiliza-se a funcao para liberar os elementos da memoria
    }
    l->inicio = NULL; // l->inicio eh definido como nulo para indicar que a lista esta vazia
}

void carregarLista(LISTA *lista, FILE *arq_tarefas)
{
    REGISTRO reg;
    while (fread(&reg, sizeof(REGISTRO), 1, arq_tarefas) == 1)
    {
        insere(lista, reg, tamanho(lista)); // insere os elementos dos arquivos binarios na lista, linha por linha
    }
}

void menu(LISTA *lista, REGISTRO palavra, FILE *historico);
bool modificaElemento(LISTA *lista, REGISTRO reg, int pos, FILE *historico);
bool modifica(LISTA *lista, REGISTRO palavra, int aux_pos, FILE *historico);
void ordenaPrazos(LISTA *lista);
void exibirTarefasPorPrazo(LISTA *lista, int prazo);

int main()
{
    LISTA lista;
    inicializarLista(&lista);
    REGISTRO palavra;
    FILE *arq_tarefas, *arq_historico;

    arq_tarefas = fopen("tarefas.bin", "ab+");
    arq_historico = fopen("historico.txt", "a+");

    if (arq_tarefas == NULL || arq_historico == NULL)
    {
        printf("Erro ao abrir o arquivo de tarefas.\n");
        return 1;
    }

    carregarLista(&lista, arq_tarefas); // carregas a lista com os dados no arquivo

    menu(&lista, palavra, arq_historico); // abre o menu de opcoes

    // Reabrir o arquivo em modo "wb" para sobrescrever com a lista atualizada
    fclose(arq_tarefas);
    arq_tarefas = fopen("tarefas.bin", "wb");
    if (arq_tarefas == NULL)
    {
        printf("Erro ao abrir o arquivo de tarefas.\n");
        fclose(arq_historico);
        return 1;
    }

    PONT p = lista.inicio; // Salvar a lista atualizada no arquivo
    while (p != NULL)
    {
        fwrite(&p->reg, sizeof(REGISTRO), 1, arq_tarefas);
        p = p->prox;
    }

    reinicializarFila(&lista);
    fclose(arq_tarefas);
    fclose(arq_historico);
    return 0;
}

bool modificaElemento(LISTA *lista, REGISTRO reg, int pos, FILE *historico) // funcao que possibilita o usuario mudar alguma tarefa (opcional, nao pedido no trabalho)
{
    PONT p = lista->inicio;
    if (pos < 1 || pos > tamanho(lista)) // verifica se a o posicao do arquivo que deseja ser alterado eh valido
    {
        return false;
    }

    for (int i = 0; i < pos - 1 && p != NULL; i++) // itera o ponteiro ate a posicao do elemento
    {
        p = p->prox;
    }

    if (p == NULL)
        return false; // Verificação adicional

    fprintf(historico, "Tarefa modificada, de \"%s\", para \"%s\"\n", p->reg.chave, reg.chave); // imprime no arquivo de historico
    strcpy(p->reg.chave, reg.chave);                                                            // troca a tarefa pela nova desejada
    return true;
}

void opcoes()
{
    printf(" 1 - Escrever tarefas\n 2 - Modificar alguma tarefa \n 3 - Excluir uma tarefa\n 4 - Imprimir a Lista\n 5 - Exibir a tarefa dado um prazo\n 0 - Sair\n"); // opcoes da tarefa
}

bool ehDigito(char c)
{
    if (c >= '0' && c <= '9')
    {
        return true;
    }
    return false;
}

void menu(LISTA *lista, REGISTRO palavra, FILE *historico)
{
    int aux_pos, prazo;
    opcoes();    // opcoes da tarefa
    while (true) // loop de opcoes (mostra as opcoes enquanto o usuario quiser)
    {
        scanf(" %[^\n]", palavra.chave);     // le todos os dados de entrada menos a linha nova (enter)
        if (strcmp(palavra.chave, "0") == 0) // se o usuario entrar com "0", ele sai do menu
            break;
        if (strcmp(palavra.chave, "1") == 0) // se o usuario entrar com "1", ele entra no loop de escrever as tarefas
        {
            puts("Digite respectivamente, o prazo (em dias) e o nome da(s) tarefa(s) que deseja registrar, 0 para voltar");
            while (true) // loop de escrever as tarefas
            {
                scanf(" %[^\n]", palavra.chave); // le a tarefa a ser escrita, dada pelo usuario

                if (strcmp(palavra.chave, "0") == 0) // se a tarefa for igual a 0, volta para o menu
                {
                    opcoes(); // opcoes do menu
                    break;
                }

                if (ehDigito(palavra.chave[0]) == 0)
                {
                    printf("Ordem de entrada errada\n");
                    opcoes();
                    break;
                }

                fprintf(historico, "Tarefa escrita: \"%s\"\n", palavra.chave); // coloca a tarefa escrita no historico
                insere(lista, palavra, tamanho(lista));                        // insere a tarefa na lista
            }
        }
        if (strcmp(palavra.chave, "2") == 0) // se o usuario entrar com "2", entra na opcao de modificar algum elemento
        {
            puts("\nDe o numero do elemento deseja modificar, 0 para voltar");
            ordenaPrazos(lista);
            exibirLista(lista); // imprime a lista para o usuario ter alguma referencia
            scanf("%d", &aux_pos);
            if (aux_pos == 0) // se a posicao do elemento dado for 0, volta para o menu
            {
                opcoes();
                continue;
            }
            if (modifica(lista, palavra, aux_pos, historico))
            {
                printf("Tarefa modificada com sucesso \n\n");
            }
            else
            {
                printf("Posição inválida\n");
            }
            opcoes();
        }
        if (strcmp(palavra.chave, "3") == 0) // se o usuario entrar com "3", entra na opcao de excluir as tarefas ja concluidas
        {
            printf("De o numero da tarefa concluida, 0 para voltar\n");
            ordenaPrazos(lista);
            exibirLista(lista);
            scanf("%d", &aux_pos);
            if (aux_pos == 0) // se a entrada for "0", volta para o menu
            {
                opcoes();
                continue;
            }
            if (exclui(lista, aux_pos, historico) == false) // chama a funcao exclui, se ele retornar false entra no if
            {
                printf("Posicao invalida.\n"); // se a posicao da tarefa for invalida, imprime isso
            }
            else
                printf("Tarefa excluida com sucesso \n\n");

            opcoes();
        }
        if (strcmp(palavra.chave, "4") == 0) // se o usuario entrar com "4", imprime a lista de tarefas
        {
            if (tamanho(lista) != 0)
            {
                ordenaPrazos(lista);
                exibirLista(lista);
                opcoes();
            }
            else // se a lista for vazia imprime isso e "volta" ao menu
                opcoes();
        }
        if (strcmp(palavra.chave, "5") == 0) // se o usuario entrar com "5", exibe tarefas de determinado prazo
        {
            ordenaPrazos(lista);
            printf("De o prazo que voce quer procurar, 0 para voltar\n");
            scanf("%d", &prazo);
            if (prazo != 0)
            {
                exibirTarefasPorPrazo(lista, prazo);
            }
            opcoes();
        }
    }
}

bool modifica(LISTA *lista, REGISTRO palavra, int aux_pos, FILE *historico) // funcao de modificar um elemento da lista
{
    puts("O que deseja inserir no lugar?");
    scanf(" %[^\n]", palavra.chave);                             // pede ao usuario o que ele deseja substituir
    return modificaElemento(lista, palavra, aux_pos, historico); // chama a funcao modificaElemento
}

int extrairPrazo(const char *tarefa)
{
    int prazo;
    sscanf(tarefa, "%d", &prazo); // extrai o prazo da string
    return prazo;
}

void ordenaPrazos(LISTA *lista)
{
    PONT i, p, posMenor;
    REGISTRO aux;

    for (i = lista->inicio; i != NULL && i->prox != NULL; i = i->prox) // Percorre a lista do início ao penúltimo elemento
    {
        posMenor = i; // Assume que o menor elemento inicialmente é o próprio i

        for (p = i->prox; p != NULL; p = p->prox)
        {
            if (extrairPrazo(p->reg.chave) < extrairPrazo(posMenor->reg.chave)) // Compara os prazos dos registros para determinar o menor
                posMenor = p;                                                   // Atualiza posMenor se encontrar um registro com prazo menor
        }

        if (posMenor != i) // Se posMenor for diferente de i, troca os registros entre i e posMenor
        {
            aux = i->reg;
            i->reg = posMenor->reg;
            posMenor->reg = aux;
        }
    }
}

void exibirTarefasPorPrazo(LISTA *lista, int prazo)
{
    PONT p = lista->inicio;
    bool encontrado = false;

    printf("Tarefas com prazo de %d dias:\n", prazo);
    while (p != NULL)
    {
        int tarefa_prazo;
        sscanf(p->reg.chave, "%d", &tarefa_prazo); // extrai o prazo da string
        if (tarefa_prazo == prazo)
        {
            printf("  %s\n", p->reg.chave);
            encontrado = true;
        }
        p = p->prox;
    }

    if (!encontrado)
    {
        printf("Nenhuma tarefa encontrada com prazo de %d dias.\n", prazo);
    }
    printf("\n");
}
