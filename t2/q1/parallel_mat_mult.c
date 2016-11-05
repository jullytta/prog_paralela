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

#define MAX_ORDER 10

typedef float MATRIX_T[MAX_ORDER][MAX_ORDER];

void Read_matrix(char* prompt, MATRIX_T A, int n);
void Parallel_matrix_mult(MATRIX_T A, MATRIX_T B, MATRIX_T C, int n);
void Print_matrix(char* title, MATRIX_T C, int n);

int main (int argc, char *argv[]) {
    int       n;
    double tempo_inicial, tempo_final;
    MATRIX_T  A;
    MATRIX_T  B;
    MATRIX_T  C;

    MPI_Init(&argc, &argv);

    scanf("%d", &n);

    Read_matrix("Entre A", A, n);
    Read_matrix("Entre B", B, n);
    
    // Desconsidera a leitura da entrada no calculo do tempo gasto.
    tempo_inicial = MPI_Wtime();
    
    Parallel_matrix_mult(A, B, C, n);

    tempo_final = MPI_Wtime(); // Computacao concluida.

    #ifndef STATS_FLAG
    Print_matrix("O produto é", C, n);
    #else
    printf("%lf\t", tempo_final - tempo_inicial);
    #endif

    MPI_Finalize();

    return 0;
}  /* main */


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

    int i, j, k;

    for (i = 0; i < n; i++)    
        for (j = 0; j < n; j++) {
            C[i][j] = 0.0;
            for (k = 0; k < n; k++)
                C[i][j] = C[i][j] + A[i][k]*B[k][j];
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