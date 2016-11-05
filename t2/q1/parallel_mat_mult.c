/* parallel_mat_mult.c -- Multiplica duas matrizes quadradas 
*
* Entrada: 
*    n: Ordem das matrizes 
*    A,B: Matrizes de entrada
* Saída:
*    C: Matriz produto
*
*/
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MAX_ORDER 10

typedef float MATRIX_T[MAX_ORDER][MAX_ORDER];

void Create_Matrix_Types(MPI_Datatype *tipo_linha, MPI_Datatype *tipo_coluna, int n);
void Read_matrix(char* prompt, MATRIX_T A, int n);
void Parallel_matrix_mult(MATRIX_T A, MATRIX_T B, MATRIX_T C, int n);
void Print_matrix(char* title, MATRIX_T C, int n);

int main (int argc, char *argv[]) {
    int i, j, n, p, meu_ranque, raiz = 0;
    double tempo_inicial, tempo_final;
    MATRIX_T  A;
    MATRIX_T  B;
    MATRIX_T  C;
    MPI_Datatype tipo_linha, tipo_coluna;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if(meu_ranque == raiz){
        scanf("%d", &n);
        Read_matrix("Entre A", A, n);
        Read_matrix("Entre B", B, n);
    }

    #ifdef DEBUG_FLAG
    if(meu_ranque == 0){
        printf("Matrix A:\n");
        Print_matrix("", A, n);
        printf("\n");
        
        printf("Matrix B:\n");
        Print_matrix("", B, n);
        printf("\n");
    }
    #endif

    // Desconsidera a leitura da entrada no calculo do tempo gasto.
    tempo_inicial = MPI_Wtime();
    
    // Todos os processos precisam saber qual a ordem das matrizes
    MPI_Bcast(&n, 1, MPI_INT, raiz, MPI_COMM_WORLD);

    // Tipos especiais devem ser inicializados
    Create_Matrix_Types(&tipo_linha, &tipo_coluna, n);

    // Cada processo calcula quantas linhas e colunas recebera
    int ndivido, nresto, meu_tamanho, meu_inicio;
    ndivido = n/p;
    nresto = n % p;

    // meu_inicio marca qual linha/coluna e' a primeira para cada
    // processo. Por exemplo, com 4 processos e matrizes 8x8,
    // as linhas destinadas ao processo 1 sao A2 e A3
    // e as colunas B2 e B3.
    // Assim, o inicio do processo 1 eh 2.
    meu_inicio = meu_ranque * ndivido;

    // Quantidade de linhas/colunas que cada processo e' responsavel
    // O ultimo processo pega as sobras tambem
    meu_tamanho = (meu_ranque < p -1) ? ndivido : ndivido + nresto;

    // Em seguida, cada processo aloca memoria para receber
    float **minhas_linhas = (float **) malloc(meu_tamanho*sizeof(float*));
    for(i = 0; i < meu_tamanho; i++)
        minhas_linhas[i] = (float *) malloc(n*sizeof(float));

    float **minhas_colunas = (float **) malloc(n*sizeof(float*));
    for(j = 0; j < n; j++)
        minhas_colunas[j] = (float *) malloc(meu_tamanho*sizeof(float));

    // Todos comecam com minhas_linhas e minhas_colunas sendo
    // matrizes cheias de zeros
    for(i = 0; i < meu_tamanho; i++)
        for(j = 0; j < n; j++)
            minhas_linhas[i][j] = 0;

    for(i = 0; i < n; i++)
        for(j = 0; j < meu_tamanho; j++)
            minhas_colunas[i][j] = 0;

    // Preparacao para o envio:
    // Os vetores criados abaixo sao necessarios para utilizar
    // o MPI_Scatterv. Usamos esse scatter porque n nao e'
    // sempre divisivel por p. O ultimo processo fica
    // com o trabalho extra.

    // Esse vetor guarda as quantidades para cada processo
    int *tamanhos;
    tamanhos = (int *) malloc(p*sizeof(int));
    for(i = 0; i < p-1; i++){
        tamanhos[i] = ndivido;
    }
    tamanhos[p-1] = ndivido + nresto;

    // Esse vetor guarda onde a parte de cada processo comeca
    int *deslocamentos;
    deslocamentos = (int *) malloc(p*sizeof(int));
    for(i = 0; i < p; i++){
        deslocamentos[i] = ndivido*i;
    }

    #ifdef DEBUG_FLAG
    if(meu_ranque == raiz){
        printf("Tamanhos:\n");
        for(i = 0; i < p; i++){
            printf("%d\t", tamanhos[i]);
        }
        printf("\n");
        printf("Deslocamentos:\n");
        for(i = 0; i < p; i++){
            printf("%d\t", deslocamentos[i]);
        }
        printf("\n");
        printf("\n");
    }
    #endif

    // TODO(jullytta): trocar recebimento por minhas_colunas
    MATRIX_T recebimento;
    // O processo raiz distribui as matrizes A e B
    MPI_Scatterv(&B, tamanhos, deslocamentos,
                 tipo_coluna, &recebimento, meu_tamanho,
                 tipo_coluna,
                 raiz, MPI_COMM_WORLD);

    #ifdef DEBUG_FLAG
    // Verifica se as colunas e linhas foram recebidas com sucesso
    // Imprime linhas recebidas
    printf("Processo %d, minhas linhas:\n", meu_ranque);
    for(i = 0; i < meu_tamanho; i++){
        for(j = 0; j < n; j++){
            printf("%4.1f\t\t", minhas_linhas[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    // Imprime colunas recebidas
    printf("Processo %d, minhas colunas:\n", meu_ranque);
    for(i = 0; i < n; i++){
        for(j = 0; j < meu_tamanho; j++){
            printf("%4.1f\t\t", recebimento[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    #endif

    // Parallel_matrix_mult(A, B, C, n);

    if(meu_ranque == raiz){
        tempo_final = MPI_Wtime(); // Computacao concluida.
        #ifndef STATS_FLAG
        // Print_matrix("O produto é", C, n);
        #else
        printf("%lf\t", tempo_final - tempo_inicial);
        #endif
    }

    MPI_Finalize();

    return 0;
}  /* main */

/*****************************************************************/
void Create_Matrix_Types(MPI_Datatype *tipo_linha, MPI_Datatype *tipo_coluna, int n){
    MPI_Datatype linha, coluna;
    MPI_Aint lim_inf, extensao;
    
    // Matrizes sao formadas por floats
    MPI_Type_get_extent(MPI_FLOAT, &lim_inf, &extensao);

    // Cria o tipo linha
    // n linhas, linhas sao blocos de largura n
    // cada bloco comeca logo depois do outro (1)
    MPI_Type_vector(n, n, 1, MPI_FLOAT, &linha);
    MPI_Type_commit(&linha);
    MPI_Type_create_resized(linha, lim_inf, extensao, tipo_linha);
    MPI_Type_commit(tipo_linha);

    // Cria o tipo coluna
    // n colunas, colunas sao blocos de largura 1
    // cada bloco comeca n elementos depois do outro
    MPI_Type_vector(n, 1, n, MPI_FLOAT, &coluna);
    MPI_Type_commit(&coluna);
    MPI_Type_create_resized(coluna, lim_inf, extensao, tipo_coluna);
    MPI_Type_commit(tipo_coluna);
}


/*****************************************************************/
void Read_matrix(
        char*    prompt  /* in  */,
        MATRIX_T  A      /* out */,
        int      n      /* in  */) {
    int i, j;

    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            scanf("%f", &A[i][j]);
}  /* Read_matrix */


/*****************************************************************/
/* MATRIX_T é um array bi-dimensional de floats  */
void Parallel_matrix_mult(
        MATRIX_T  A  /* in  */,
        MATRIX_T  B  /* in  */,
        MATRIX_T  C  /* out */,
        int        n  /* in  */) {

    int i, j, k, p, meu_ranque, raiz = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // TODO(jullytta): Aqui, cada processo faz apenas a sua parte
    for (i = 0; i < n; i++){
        for (j = 0; j < n; j++) {
            C[i][j] = 0.0;
            for (k = 0; k < n; k++){
                C[i][j] = C[i][j] + A[i][k]*B[k][j];
            }
        }
    }
}  /* Multiplicação Paralela */


/*****************************************************************/
void Print_matrix(
        char*    title  /* in  */,
        MATRIX_T  C      /* out */,
        int      n      /* in  */) {
    int i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            printf("%4.1f\t\t", C[i][j]);
        printf("\n");
    }
}  /* Read_matrix */