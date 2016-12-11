/*****************************************************************/
/* Multiplicacao de Matrizes Quadradas Paralela                  */
/* Entrada: n, ordem das matrizes, e duas matrizes A e B (n x n) */
/* Saida: matriz quadrada C, produto de A e B (nessa ordem)      */
/* Ultima revisao: 04/11/2016                                    */
/*                                                               */
/* Algoritmo                                                     */
/* 1. O processo raiz le a entrada e compartilha n e A com os    */
/* outros processos;                                             */
/* 2. Cada processo recebe, adicionalmente, uma parte das        */
/* colunas de B, em uma submatrix sub_B;                         */
/* 3. Cada processo calcula A x sub_B;                           */
/* 4. O processo raiz recolhe todos os resultados e monta a      */
/* matriz produto C.                                             */
/*****************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"


void Attach_debugger(int my_rank, int target, MPI_Comm comm);
void Create_Column_Type(MPI_Datatype *tipo_coluna, int n, int m);
void Malloc_matrix(float ***matrix, int n, int m);
void Matrix_mult(float **A, float **B, float **C,
   int l1, int c1l2, int c2);
void Print_matrix(float **matrix, int n, int m);
void Read_matrix(float **matrix, int n, int m);


int main (int argc, char *argv[]) {
    int i, j, n, p, meu_ranque, raiz = 0;
    double tempo_inicial, tempo_final;
    float **A;
    float **B;
    float **C;
    MPI_Datatype tipo_coluna, tipo_subcoluna;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    #ifdef GDB_FLAG
    Attach_debugger(meu_ranque, raiz, MPI_COMM_WORLD);
    #endif

    if(meu_ranque == raiz){
        scanf("%d", &n);

        Malloc_matrix(&A, n, n);
        Malloc_matrix(&B, n, n);
        Malloc_matrix(&C, n, n);

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

    // Agora os processos nao raiz ja podem alocar espaco para A
    if(meu_ranque != raiz)
        Malloc_matrix(&A, n, n);

    // Todos os processos terao sua copia de A
    MPI_Bcast(*A, n*n, MPI_FLOAT, raiz, MPI_COMM_WORLD);

    /*********************************************************/
    /* Cada processo calcula quantas colunas de B recebera   */
    /* Se n nao e' divisivel por p, o ultimo processo fica   */
    /* com o resto.                                          */
    /*********************************************************/
    int ndivido, nresto, meu_tamanho, meu_inicio;
    ndivido = n/p;
    nresto = n % p;
    meu_inicio = meu_ranque * ndivido;
    meu_tamanho = (meu_ranque < p -1) ? ndivido : ndivido + nresto;


    // Submatrizes para cada processo
    float **sub_B, **sub_C;
    Malloc_matrix(&sub_B, n, meu_tamanho);
    Malloc_matrix(&sub_C, n, meu_tamanho);


    /*********************************************************/
    /* Os vetores abaixo foram inicializados em preparacao   */
    /* para a comunicacao atraves de MPI_Scatterv e          */
    /* MPI_Gatherv.                                          */
    /*********************************************************/
    int *tamanhos, *deslocamentos;
    tamanhos = (int *) malloc(p*sizeof(int));
    deslocamentos = (int *) malloc(p*sizeof(int));
    for(i = 0; i < p-1; i++){
        tamanhos[i] = ndivido;
        deslocamentos[i] = ndivido*i;
    }
    tamanhos[p-1] = ndivido + nresto;
    deslocamentos[p-1] = ndivido*i;


    /*********************************************************/
    /* Fica aqui registrado que se fossemos usar um tipo     */
    /* derivado para linhas, ele poderia ser definido da     */
    /* seguinte forma:                                       */
    /* MPI_Datatype tipo_linha;                              */
    /* MPI_Type_contiguous(n, MPI_FLOAT, &tipo_linha);       */
    /* MPI_Type_commit(&tipo_linha);                         */
    /*********************************************************/
    Create_Column_Type(&tipo_coluna, n, n);

    /*********************************************************/
    /* Note que a submatriz de recepcao, sub_B, nem sempre   */
    /* tem as mesmas dimensoes que a matriz original, B. Por */
    /* esse motivo, o salto do tipo coluna correspondente    */
    /* para essas duas matrizes e' diferente, e precisamos   */
    /* de um tipo especifico para a coluna recebida.         */
    /*********************************************************/
    Create_Column_Type(&tipo_subcoluna, n, meu_tamanho);

    /***************** Observacao importante *****************/
    /* Apesar do sendbuf nao ser significativo fora da raiz, */
    /* nos ainda estamos lidando com C. Isso significa que   */
    /* em hipotese alguma deveriamos desreferenciar um       */
    /* ponteiro que nao tenha um endereco valido, pois isso  */
    /* seria acessar o valor de uma memoria que nao e' nossa,*/
    /* resultando em segmentation fault.                     */
    /*                                                       */
    /* A alternativa aqui foi fazer um if e desreferenciar   */
    /* apenas no processo raiz, utilizando NULL para outros  */
    /* processos.                                            */
    /*********************************************************/
    if(meu_ranque == raiz){        
        MPI_Scatterv(*B, tamanhos, deslocamentos,
                     tipo_coluna, *sub_B, meu_tamanho,
                     tipo_subcoluna, raiz, MPI_COMM_WORLD);
    }
    else {
        MPI_Scatterv(NULL, tamanhos, deslocamentos,
                     tipo_coluna, *sub_B, meu_tamanho,
                     tipo_subcoluna, raiz, MPI_COMM_WORLD);
    }

    #ifdef DEBUG_FLAG
    // Imprime colunas recebidas
    printf("Processo %d, sub matriz B:\n", meu_ranque);
    Print_matrix(sub_B, n, meu_tamanho);
    printf("\n");
    #endif

    Matrix_mult(A, sub_B, sub_C, n, n, meu_tamanho);

    #ifdef DEBUG_FLAG
    // Imprime sub matriz resultado
    printf("Processo %d, sub matriz C:\n", meu_ranque);
    Print_matrix(sub_C, n, meu_tamanho);
    printf("\n");
    #endif

    MPI_Gatherv(*sub_C, meu_tamanho, tipo_subcoluna,
                *C, tamanhos, deslocamentos,
                tipo_coluna, raiz, MPI_COMM_WORLD);

    if(meu_ranque == raiz){
        tempo_final = MPI_Wtime(); // Computacao concluida.
        // Compile com -DSTATS_FLAG para ter apenas o tempo
        // de execucao impresso.
        #ifndef STATS_FLAG
        Print_matrix(C, n, n);
        #else
        printf("%lf\t", tempo_final - tempo_inicial);
        #endif
    }

    // MPI_Type_free(&tipo_linha);
    MPI_Type_free(&tipo_coluna);
    MPI_Type_free(&tipo_subcoluna);
    MPI_Finalize();

    return 0;
}  /* main */


/*****************************************************************/
/* Essa funcao 'trava' o processo indicado por processo_objetivo */
/* para possibilitar a conexao deste processo com o debugger.    */
/* Modifique manualmente a variavel attached para continuar.     */
/*****************************************************************/
void Attach_debugger(int my_rank, int target, MPI_Comm comm){
    int attached = 0;

    if(my_rank == target){
        while(!attached){
            sleep(1);
        }
    }
    
    MPI_Barrier(comm);

} /* Attach_debugger */


/*****************************************************************/
/* Cria um tipo derivado, tipo_coluna, que equivale a uma coluna */
/* em uma matriz n x m.                                          */
/* A matriz deve ser continua na memoria para esse tipo derivado */
/* funcionar adequadamente, e deve ser constituida por floats.   */
/*****************************************************************/
void Create_Column_Type(MPI_Datatype *tipo_coluna, int n, int m){
    MPI_Aint lim_inf, extensao;
    // Tipo intermediario
    MPI_Datatype coluna;
    
    MPI_Type_get_extent(MPI_FLOAT, &lim_inf, &extensao);

    // n elementos em cada coluna
    // colunas sao blocos de largura 1
    // cada bloco comeca m elementos depois do inicio do outro
    MPI_Type_vector(n, 1, m, MPI_FLOAT, &coluna);
    MPI_Type_commit(&coluna);

    // Necessario para usar comunicacao coletiva, como
    // MPI_Scatter e MPI_Gather
    MPI_Type_create_resized(coluna, lim_inf, extensao, tipo_coluna);
    MPI_Type_commit(tipo_coluna);
}


/*****************************************************************/
/* A alocacao aqui tem um pouquinho de gambiarra para garantir   */
/* que a matriz seja continua na memoria (necessario para usar   */
/* os tipos derivados).                                          */
/*****************************************************************/
void Malloc_matrix(float ***matrix, int n, int m){
    int i;
    *matrix = malloc(n*sizeof(float*));
    (*matrix)[0] = malloc(n*m*sizeof(float));
    for(i = 1; i < n; i++)
        (*matrix)[i] = &((*matrix)[0][i*m]);
} /* Malloc_matrix */


/*****************************************************************/
/* Multiplica duas matrizes, A e B, sendo A uma matriz l1 x c1 e */
/* B uma matriz l2 x c2. O resultado e' uma matriz C, l1 x c2.   */
/* Parte do principio que a memoria para C ja foi devidamente    */
/* alocada.                                       */
/*****************************************************************/
void Matrix_mult(float **A, float **B, float **C, int l1, int c1l2, int c2){
    int i, j, k;

    for (i = 0; i < l1; i++){
        for (j = 0; j < c2; j++) {
            C[i][j] = 0.0;
            for (k = 0; k < c1l2; k++){
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
void Read_matrix(float **matrix, int n, int m) {
    int i, j;

    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            scanf("%f", &matrix[i][j]);
}  /* Read_matrix */
