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
#include "mpi.h"

#define MAX_ORDER 10

typedef float MATRIX_T[MAX_ORDER][MAX_ORDER];

void Create_Matrix_Types(MPI_Datatype *tipo_linha, MPI_Datatype *tipo_coluna, int n);
void Read_matrix(char* prompt, MATRIX_T A, int n);
void Parallel_matrix_mult(MATRIX_T A, MATRIX_T B, MATRIX_T C, int n);
void Print_matrix(char* title, MATRIX_T C, int n);

int main (int argc, char *argv[]) {
    int n, p, meu_ranque, raiz = 0;
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

    // Desconsidera a leitura da entrada no calculo do tempo gasto.
    tempo_inicial = MPI_Wtime();
    
    // Todos os processos precisam saber qual a ordem das matrizes
    MPI_Bcast(&n, 1, MPI_INT, raiz, MPI_COMM_WORLD);

    #ifdef DEBUG_FLAG
    printf("Processo %d recebeu n = %d\n", meu_ranque, n);
    #endif

    // Tipos especiais devem ser inicializados
    Create_Matrix_Types(&tipo_linha, &tipo_coluna, n);

    Parallel_matrix_mult(A, B, C, n);

    if(meu_ranque == raiz){
        tempo_final = MPI_Wtime(); // Computacao concluida.
        #ifndef STATS_FLAG
        Print_matrix("O produto é", C, n);
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