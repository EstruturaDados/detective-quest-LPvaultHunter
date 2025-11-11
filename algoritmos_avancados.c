#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR 128
#define HASH_SIZE 31   // número primo pequeno pra hash simples

/* ---------------------------
   Estruturas de dados
   ---------------------------*/

// Nó da árvore de cômodos (mapa da mansão)
typedef struct Sala {
    char nome[MAX_STR];
    struct Sala *esq;
    struct Sala *dir;
} Sala;

// Nó da BST de pistas (armazenadas em ordem alfabética)
typedef struct PistaBST {
    char pista[MAX_STR];
    struct PistaBST *esq;
    struct PistaBST *dir;
} PistaBST;

// Entrada da tabela hash (separate chaining)
typedef struct HashItem {
    char chave[MAX_STR];      // a pista
    char suspeito[MAX_STR];   // suspeito associado
    struct HashItem *prox;
} HashItem;

/* ---------------------------
   Protótipos (requisitados nos requisitos)
   ---------------------------*/

/**
 * criarSala() – cria dinamicamente um cômodo.
 * Retorna ponteiro para Sala alocada com nome preenchido.
 */
Sala* criarSala(const char* nome);

/**
 * explorarSalas() – navega pela árvore e ativa o sistema de pistas.
 * Navegação interativa: 'e' (esquerda), 'd' (direita), 's' (sair).
 * Quando visita uma sala, exibe pista (se existir) e insere na BST e na hash (se coletada).
 */
void explorarSalas(Sala *raiz, PistaBST **bstColetadas, HashItem *hashTable[]);

/**
 * inserirPista() / adicionarPista() – insere a pista coletada na árvore de pistas (BST).
 * evita inserir duplicatas.
 */
PistaBST* inserirPista(PistaBST *raiz, const char *pista);

/**
 * inserirNaHash() – insere associação pista/suspeito na tabela hash.
 * (aqui usamos para inicializar e também para caso queiramos inserir dinâmicamente).
 */
void inserirNaHash(HashItem *hashTable[], const char *pista, const char *suspeito);

/**
 * encontrarSuspeito() – consulta o suspeito correspondente a uma pista na tabela hash.
 * Retorna ponteiro para string de suspeito ou NULL se não existir.
 */
const char* encontrarSuspeito(HashItem *hashTable[], const char *pista);

/**
 * verificarSuspeitoFinal() – conduz à fase de julgamento final.
 * Conta quantas pistas coletadas apontam para o suspeito acusado; pede acusação ao jogador.
 */
void verificarSuspeitoFinal(PistaBST *bstColetadas, HashItem *hashTable[]);

/* ---------------------------
   Funções utilitárias e internas
   ---------------------------*/
unsigned long hash_djb2(const char *str);
HashItem* criarHashItem(const char *chave, const char *suspeito);
void listarPistasInOrder(PistaBST *raiz);
int existePista(PistaBST *raiz, const char *pista);
int contarPistasParaSuspeito(PistaBST *raiz, HashItem *hashTable[], const char *suspeito);
void liberarBST(PistaBST *raiz);
void liberarHash(HashItem *table[]);
void liberarSalas(Sala *raiz);

/* ---------------------------
   Implementação
   ---------------------------*/

unsigned long hash_djb2(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

HashItem* criarHashItem(const char *chave, const char *suspeito) {
    HashItem *it = (HashItem*) malloc(sizeof(HashItem));
    if (!it) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strncpy(it->chave, chave, MAX_STR-1); it->chave[MAX_STR-1] = '\0';
    strncpy(it->suspeito, suspeito, MAX_STR-1); it->suspeito[MAX_STR-1] = '\0';
    it->prox = NULL;
    return it;
}

void inserirNaHash(HashItem *hashTable[], const char *pista, const char *suspeito) {
    unsigned long h = hash_djb2(pista) % HASH_SIZE;
    HashItem *novo = criarHashItem(pista, suspeito);

    // Inserção no início da lista encadeada (separate chaining)
    novo->prox = hashTable[h];
    hashTable[h] = novo;
}

const char* encontrarSuspeito(HashItem *hashTable[], const char *pista) {
    unsigned long h = hash_djb2(pista) % HASH_SIZE;
    HashItem *curr = hashTable[h];
    while (curr) {
        if (strcmp(curr->chave, pista) == 0) return curr->suspeito;
        curr = curr->prox;
    }
    return NULL;
}

/* criarSala
   Cria e retorna uma Sala recém-alocada com o nome dado.
*/
Sala* criarSala(const char* nome) {
    Sala *s = (Sala*) malloc(sizeof(Sala));
    if (!s) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strncpy(s->nome, nome, MAX_STR-1);
    s->nome[MAX_STR-1] = '\0';
    s->esq = s->dir = NULL;
    return s;
}

/* inserirPista / adicionarPista
   Insere uma pista (string) na BST de pistas coletadas em ordem alfabética.
   Evita inserir duplicatas (compara strings).
*/
PistaBST* inserirPista(PistaBST *raiz, const char *pista) {
    if (!pista || strlen(pista) == 0) return raiz;
    if (raiz == NULL) {
        PistaBST *novo = (PistaBST*) malloc(sizeof(PistaBST));
        if (!novo) { perror("malloc"); exit(EXIT_FAILURE); }
        strncpy(novo->pista, pista, MAX_STR-1); novo->pista[MAX_STR-1] = '\0';
        novo->esq = novo->dir = NULL;
        printf("Pista coletada: \"%s\"\n", pista);
        return novo;
    }
    int cmp = strcmp(pista, raiz->pista);
    if (cmp == 0) {
        printf("Você já coletou a pista \"%s\" antes. Não será adicionada novamente.\n", pista);
        return raiz; // duplicata
    } else if (cmp < 0) {
        raiz->esq = inserirPista(raiz->esq, pista);
    } else {
        raiz->dir = inserirPista(raiz->dir, pista);
    }
    return raiz;
}

void listarPistasInOrder(PistaBST *raiz) {
    if (!raiz) return;
    listarPistasInOrder(raiz->esq);
    printf(" - %s\n", raiz->pista);
    listarPistasInOrder(raiz->dir);
}

int existePista(PistaBST *raiz, const char *pista) {
    if (!raiz) return 0;
    int cmp = strcmp(pista, raiz->pista);
    if (cmp == 0) return 1;
    else if (cmp < 0) return existePista(raiz->esq, pista);
    else return existePista(raiz->dir, pista);
}

/* contarPistasParaSuspeito
   Percorre a BST e conta quantas pistas estão associadas ao suspeito informado.
*/
int contarPistasParaSuspeito(PistaBST *raiz, HashItem *hashTable[], const char *suspeito) {
    if (!raiz) return 0;
    int count = 0;
    const char *s = encontrarSuspeito(hashTable, raiz->pista);
    if (s && strcmp(s, suspeito) == 0) count = 1;
    return count + contarPistasParaSuspeito(raiz->esq, hashTable, suspeito)
                 + contarPistasParaSuspeito(raiz->dir, hashTable, suspeito);
}

/* explorarSalas
   Percorre interativamente a árvore de salas, começando na raiz passada.
   Ao visitar cada sala, identifica (por lógica) se há uma pista e a coleta.
   Entrada: raiz da mansão, ponteiro para ponteiro da BST de pistas coletadas.
*/
void explorarSalas(Sala *raiz, PistaBST **bstColetadas, HashItem *hashTable[]) {
    Sala *atual = raiz;
    char escolha[8];

    printf("\n--- Exploração da Mansão iniciada ---\n");
    printf("Comandos: 'e' = esquerda, 'd' = direita, 's' = sair da exploração\n");

    while (atual) {
        printf("\nVocê está na sala: %s\n", atual->nome);

        // Regras lógicas que associam cada sala a uma pista (definidas estaticamente)
        // Se a regra retorna uma string não vazia, existe uma pista nessa sala.
        char pistaEncontrada[MAX_STR] = "";
        if (strcmp(atual->nome, "Hall") == 0) {
            strncpy(pistaEncontrada, "pegada de bota molhada", MAX_STR-1);
        } else if (strcmp(atual->nome, "Biblioteca") == 0) {
            strncpy(pistaEncontrada, "fio de seda vermelho", MAX_STR-1);
        } else if (strcmp(atual->nome, "Sala de Jantar") == 0) {
            strncpy(pistaEncontrada, "marca de copo com monograma", MAX_STR-1);
        } else if (strcmp(atual->nome, "Cozinha") == 0) {
            strncpy(pistaEncontrada, "lâmina com resquício de sangue", MAX_STR-1);
        } else if (strcmp(atual->nome, "Quarto Principal") == 0) {
            strncpy(pistaEncontrada, "brinco de pérola quebrado", MAX_STR-1);
        } else if (strcmp(atual->nome, "Escritório") == 0) {
            strncpy(pistaEncontrada, "bilhete rasgado com iniciais C.R.", MAX_STR-1);
        } else if (strcmp(atual->nome, "Jardim") == 0) {
            strncpy(pistaEncontrada, "sementes pisoteadas", MAX_STR-1);
        } else {
            pistaEncontrada[0] = '\0'; // sem pista
        }

        if (strlen(pistaEncontrada) > 0) {
            printf("Você encontrou uma pista: \"%s\"\n", pistaEncontrada);
            // Se ainda não coletada, insere na BST
            if (!existePista(*bstColetadas, pistaEncontrada)) {
                *bstColetadas = inserirPista(*bstColetadas, pistaEncontrada);
            } else {
                printf("(Essa pista já fazia parte das suas evidências coletadas.)\n");
            }
        } else {
            printf("Não há pistas aparentes nesta sala.\n");
        }

        // Pergunta ação do jogador
        printf("\nEscolha o próximo passo (e/d/s): ");
        if (!fgets(escolha, sizeof(escolha), stdin)) break;
        // remove espaços e newline
        char c = '\0';
        for (int i = 0; escolha[i]; ++i) {
            if (!isspace((unsigned char)escolha[i])) { c = tolower((unsigned char)escolha[i]); break; }
        }
        if (c == 's') {
            printf("Você optou por encerrar a exploração.\n");
            break;
        } else if (c == 'e') {
            if (atual->esq) {
                atual = atual->esq;
            } else {
                printf("Não há sala à esquerda. Permaneça onde está.\n");
            }
        } else if (c == 'd') {
            if (atual->dir) {
                atual = atual->dir;
            } else {
                printf("Não há sala à direita. Permaneça onde está.\n");
            }
        } else {
            printf("Comando inválido. Use 'e', 'd' ou 's'.\n");
        }
    }

    printf("\n--- Exploração encerrada ---\n");
}

/* verificarSuspeitoFinal
   Lista pistas coletadas, solicita acusação e verifica se pelo menos 2 pistas apontam
   para o suspeito acusado. Exibe veredito.
*/
void verificarSuspeitoFinal(PistaBST *bstColetadas, HashItem *hashTable[]) {
    if (!bstColetadas) {
        printf("\nVocê não coletou pistas suficientes para realizar uma acusação.\n");
        return;
    }

    printf("\nPistas coletadas (ordem alfabética):\n");
    listarPistasInOrder(bstColetadas);

    char acusado[MAX_STR];
    printf("\nDigite o nome do suspeito que você deseja acusar: ");
    if (!fgets(acusado, sizeof(acusado), stdin)) {
        printf("Erro de leitura. Saindo.\n");
        return;
    }
    // remove newline
    size_t ln = strlen(acusado);
    if (ln > 0 && acusado[ln - 1] == '\n')
        acusado[ln - 1] = '\0';

    // contagem
    int count = contarPistasParaSuspeito(bstColetadas, hashTable, acusado);

    printf("\nPistas que vinculam \"%s\": %d\n", acusado, count);

    if (count >= 2) {
        printf("\nVEREDICTO: Acusação sustentada! Há evidências suficientes para culpar %s.\n", acusado);
    } else {
        printf("\nVEREDICTO: Acusação FRACA. Apenas %d pista(s) suportam a acusação — são necessárias pelo menos 2.\n", count);
    }
}


/* Funções de liberação de memória */
void liberarBST(PistaBST *raiz) {
    if (!raiz) return;
    liberarBST(raiz->esq);
    liberarBST(raiz->dir);
    free(raiz);
}

void liberarHash(HashItem *table[]) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashItem *curr = table[i];
        while (curr) {
            HashItem *tmp = curr;
            curr = curr->prox;
            free(tmp);
        }
        table[i] = NULL;
    }
}

void liberarSalas(Sala *raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esq);
    liberarSalas(raiz->dir);
    free(raiz);
}

/* ---------------------------
   main: monta mansão, popula hash, inicia exploração
   ---------------------------*/
int main(void) {
    setbuf(stdout, NULL);

    // 1) Montagem manual do mapa da mansão (binária)
    // Exemplo de árvore:
    //              Hall
    //            /      \
    //     Biblioteca   Sala de Jantar
    //      /     \       /       \
    //  Escritório Quarto  Cozinha  Jardim
    //    (esq)   (dir)
    Sala *hall = criarSala("Hall");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *salaJantar = criarSala("Sala de Jantar");
    Sala *escritorio = criarSala("Escritório");
    Sala *quarto = criarSala("Quarto Principal");
    Sala *cozinha = criarSala("Cozinha");
    Sala *jardim = criarSala("Jardim");

    hall->esq = biblioteca;
    hall->dir = salaJantar;
    biblioteca->esq = escritorio;
    biblioteca->dir = quarto;
    salaJantar->esq = cozinha;
    salaJantar->dir = jardim;

    // 2) Inicializar tabela hash (NULLs)
    HashItem *hashTable[HASH_SIZE];
    for (int i = 0; i < HASH_SIZE; ++i) hashTable[i] = NULL;

    // 3) Definir associação pista -> suspeito (regras estáticas)
    // Esses pares representam "verdade conhecida" no jogo
    inserirNaHash(hashTable, "pegada de bota molhada", "Joao");
    inserirNaHash(hashTable, "fio de seda vermelho", "Maria");
    inserirNaHash(hashTable, "marca de copo com monograma", "Carlos");
    inserirNaHash(hashTable, "lâmina com resquício de sangue", "Carlos");
    inserirNaHash(hashTable, "brinco de pérola quebrado", "Maria");
    inserirNaHash(hashTable, "bilhete rasgado com iniciais C.R.", "Carlos");
    inserirNaHash(hashTable, "sementes pisoteadas", "Joao");

    // 4) BST vazia para pistas coletadas
    PistaBST *pistasColetadas = NULL;

    // 5) Explicar breve instrução ao jogador
    printf("Bem-vindo(a) a Detective Quest (Capítulo Final)!\n");
    printf("Explore a mansão e colete pistas. Ao finalizar, acuse o suspeito.\n");

    // 6) Exploração interativa (começa no hall)
    explorarSalas(hall, &pistasColetadas, hashTable);

    // 7) Fase de julgamento
    verificarSuspeitoFinal(pistasColetadas, hashTable);

    // 8) Limpeza
    liberarBST(pistasColetadas);
    liberarHash(hashTable);
    liberarSalas(hall);

    printf("\nObrigado por jogar. Até a próxima investigação!\n");
    return 0;

    //LPvaultHunter ass
}
