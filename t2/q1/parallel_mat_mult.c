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

void Create_Column_Type(MPI_Datatype *tipo_coluna, int n, int m);
void Read_matrix(float **matrix, int n, int m);
void Matrix_mult(float **A, float **B, float **C, int l1, int c1, int l2, int c2);
void Print_matrix(float **matrix, int n, int m);
void Malloc_matrix(float ***matrix, int n, int m);
void Attach_debugger(int meu_ranque, int processo_objetivo);

int main (int argc, char *argv[]) {
    int i, j, n, p, meu_ranque, raiz = 0;
    double tempo_inicial, tempo_final;
    float **A;
    float **B;
    float **C;
    MPI_Datatype tipo_linha, tipo_coluna, tipo_subcoluna;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    #ifdef GDB_FLAG
    Attach_debugger(meu_ranque, raiz);
    #endif

    if(meu_ranque == raiz){
        // Le a ordem das matrizes
        scanf("%d", &n);

        // Aloca memoria antes de mais nada
        Malloc_matrix(&A, n, n);
        Malloc_matrix(&B, n, n);
        Malloc_matrix(&C, n, n);

        // Le as matrizes
        Read_matrix(A, n, n);
        Read_matrix(B, n, n);
    }

    #ifdef DEBUG_FLAG
    if(meu_ranque == 0){
        printf("Matrix A:\n");
        Print_matrix(A, n, n);
        printf("\n");
        
        printf("Matrix B:\n");
        Print_matrix(B, n, n);
        printf("\n");
    }
    #endif

    // Desconsidera a leitura da entrada no calculo do tempo gasto.
    tempo_inicial = MPI_Wtime();
    
    // Todos os processos precisam saber qual a ordem das matrizes
    MPI_Bcast(&n, 1, MPI_INT, raiz, MPI_COMM_WORLD);

    // Cria o tipo linha, que e' continuo
    MPI_Type_contiguous(n, MPI_FLOAT, &tipo_linha);
    MPI_Type_commit(&tipo_linha);
    
    Create_Column_Type(&tipo_coluna, n, n);

    // Cada processo calcula quantas linhas e colunas recebera
    int ndivido, nresto, meu_tamanho, meu_inicio;
    ndivido = n/p;
    nresto = n % p;
    meu_inicio = meu_ranque * ndivido;
    // O ultimo processo pega as sobras
    meu_tamanho = (meu_ranque < p -1) ? ndivido : ndivido + nresto;

    // Submatrizes que receberao parte das matrizes originais
    float **sub_A, **sub_B, **sub_C;
    Malloc_matrix(&sub_A, meu_tamanho, n);
    Malloc_matrix(&sub_B, n, meu_tamanho);
    Malloc_matrix(&sub_C, meu_tamanho, meu_tamanho);

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

    // Cria o tipo para a coluna da sub matriz
    // Importante ter um tipo diferente porque a sub matriz tem um
    // numero menor de colunas, portanto o salto tambem e' menor
    Create_Column_Type(&tipo_subcoluna, n, meu_tamanho);

    MPI_Scatterv(*A, tamanhos, deslocamentos,
                 tipo_linha, *sub_A, meu_tamanho,
                 tipo_linha, raiz, MPI_COMM_WORLD);

// Uma parte do programa que eu nao consigo apagar
// TODO(jullytta): remover esse bloco de codigo sem gerar seg fault
// Resultados da investigacao com GDB:
// Sem essa parte do codigo, o endereco de B fica como 0xffffff...,
// ou seja, invalido, para o processo 1.\
// Por algum motivo estranho, isso causa seg fault?
// Achei que os outros processos nao precisavam
// alocar espaco para A e B
    if(meu_ranque == 1)
        Malloc_matrix(&B, n, n);

    MPI_Scatterv(*B, tamanhos, deslocamentos,
                 tipo_coluna, *sub_B, meu_tamanho,
                 tipo_subcoluna, raiz, MPI_COMM_WORLD);

    // Verifica se as colunas e linhas foram recebidas com sucesso
    #ifdef DEBUG_FLAG
    // Imprime linhas recebidas
    printf("Processo %d, sub matriz A:\n", meu_ranque);
    Print_matrix(sub_A, meu_tamanho, n);
    printf("\n");

    // Imprime colunas recebidas
    printf("Processo %d, sub matriz B:\n", meu_ranque);
    Print_matrix(sub_B, n, meu_tamanho);
    printf("\n");
    #endif

    Matrix_mult(sub_A, sub_B, sub_C, meu_tamanho, n, n, meu_tamanho);

    #ifdef DEBUG_FLAG
    // Imprime sub matriz resultado
    printf("Processo %d, sub matriz C:\n", meu_ranque);
    Print_matrix(sub_C, meu_tamanho, meu_tamanho);
    printf("\n");
    #endif

    // TODO(jullytta): juntar todas as sub_C na C resultado!

    if(meu_ranque == raiz){
        tempo_final = MPI_Wtime(); // Computacao concluida.
        #ifndef STATS_FLAG
        // Print_matrix(C, n, n);
        #else
        printf("%lf\t", tempo_final - tempo_inicial);
        #endif
    }

    MPI_Type_free(&tipo_linha);
    MPI_Type_free(&tipo_coluna);
    MPI_Type_free(&tipo_subcoluna);
    MPI_Finalize();

    return 0;
}  /* main */

/*****************************************************************/
void Create_Column_Type(MPI_Datatype *tipo_coluna, int n, int m){
    MPI_Datatype coluna;
    MPI_Aint lim_inf, extensao;
    
    // Matrizes sao formadas por floats
    MPI_Type_get_extent(MPI_FLOAT, &lim_inf, &extensao);

    // Cria o tipo coluna
    // n elementos na coluna, colunas sao blocos de largura 1
    // cada bloco comeca m elementos depois do outro
    MPI_Type_vector(n, 1, m, MPI_FLOAT, &coluna);
    MPI_Type_commit(&coluna);
    MPI_Type_create_resized(coluna, lim_inf, extensao, tipo_coluna);
    MPI_Type_commit(tipo_coluna);

}


/*****************************************************************/
void Read_matrix(float **matrix, int n, int m) {
    int i, j;

    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            scanf("%f", &matrix[i][j]);
}  /* Read_matrix */


/*****************************************************************/
/* MATRIX_T é um array bi-dimensional de floats  */
void Matrix_mult(float **A, float **B, float **C, int l1, int c1, int l2, int c2){
    int i, j, k;

    for (i = 0; i < l1; i++){
        for (j = 0; j < c2; j++) {
            C[i][j] = 0.0;
            for (k = 0; k < c1; k++){
                C[i][j] = C[i][j] + A[i][k]*B[k][j];
            }
        }
    }
}  /* Multiplicação Paralela */


/*****************************************************************/
void Print_matrix(float **matrix, int n, int m) {
    int i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++)
            printf("%4.1f\t\t", matrix[i][j]);
        printf("\n");
    }
}  /* Print_matrix */

/*****************************************************************/
/* A alocacao aqui tem um pouquinho de gambiarra para garantir   */
/* que a matriz seja continua na memoria (necessario para usar   */
/* os tipos derivados)                                           */
void Malloc_matrix(float ***matrix, int n, int m){
    int i;
    *matrix = malloc(n*sizeof(float*));
    (*matrix)[0] = malloc(n*m*sizeof(float));
    for(i = 1; i < n; i++)
        (*matrix)[i] = &((*matrix)[0][i*m]);
} /* Malloc_matrix */

/*****************************************************************/
/* Essa funcao 'trava' o processo indicado por processo_objetivo */
/* para possibilitar a conexao deste processo com o debugger     */
/* Modifique manualmente a variavel attached para continuar      */
void Attach_debugger(int meu_ranque, int processo_objetivo){
    int attached = 0;

    if(meu_ranque == processo_objetivo){
        while(!attached){
            sleep(1);
        }
    }
} /* Attach_debugger */
