/* serial_mat_mult.c -- Multiplica duas matrizes quadradas 
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

main() {
    int      n;
    float  **A;
    float  **B;
    float  **C;

    void Read_matrix(char* prompt, float **A, int n);
    void Serial_matrix_mult(float **A, float **B, float **C, int n);
    void Print_matrix(char* title, float **C, int n);

    #ifdef DEBUG_FLAG
    printf("Qual a ordem das matrizes?\n");
    #endif
    scanf("%d", &n);

    int i;
    A = (float **)malloc(n * sizeof(float*));
    for(i = 0; i < n; i++) A[i] = (float *)malloc(n*sizeof(float));
    B = (float **)malloc(n * sizeof(float*));
    for(i = 0; i < n; i++) B[i] = (float *)malloc(n*sizeof(float));
    C = (float **)malloc(n * sizeof(float*));
    for(i = 0; i < n; i++) C[i] = (float *)malloc(n*sizeof(float));

    Read_matrix("Entre A", A, n);
    #ifdef DEBUG_FLAG
    Print_matrix("A = ", A, n);
    #endif
    Read_matrix("Entre B", B, n);
    #ifdef DEBUG_FLAG
    Print_matrix("B = ", B, n);
    #endif
    Serial_matrix_mult(A, B, C, n);
    Print_matrix("O produto é", C, n);

}  /* main */


/*****************************************************************/
void Read_matrix(
        char*    prompt  /* in  */,
        float   **A      /* out */,
        int      n      /* in  */) {
    int i, j;

    #ifdef DEBUG_FLAG
    printf("%s\n", prompt);
    #endif

    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            scanf("%f", &A[i][j]);
}  /* Read_matrix */


/*****************************************************************/
/* MATRIX_T é um array bi-dimensional de floats  */
void Serial_matrix_mult(
        float  **A  /* in  */,
        float  **B  /* in  */,
        float  **C  /* out */,
        int        n  /* in  */) {

    int i, j, k;

    #ifdef DEBUG_FLAG
    void Print_matrix(char* title, float  **C, int n);

    Print_matrix("Matriz  A = ", A, n);
    Print_matrix("Matriz B = ", B, n);
    #endif

    for (i = 0; i < n; i++)    
        for (j = 0; j < n; j++) {
            C[i][j] = 0.0;
            for (k = 0; k < n; k++)
                C[i][j] = C[i][j] + A[i][k]*B[k][j];
            #ifdef DEBUG_FLAG
            printf("i = %d, j = %d, c_ij = %f\n", i, j, C[i][j]);
            #endif
        }
}  /* Multiplicação Serial */


/*****************************************************************/
void Print_matrix(
        char*    title  /* in  */,
        float  **C      /* out */,
        int      n      /* in  */) {
    int i, j;

    #ifdef DEBUG_FLAG
    printf("%s\n", title);
    #endif
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            printf("%4.1f\t\t", C[i][j]);
        printf("\n");
    }
}  /* Read_matrix */