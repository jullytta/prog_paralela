#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

// Valores pequenos na matriz para garantir que nao vai ter
// overflow na multiplicacao de matrizes
#define DEFAULT_N 5
#define MAX_ELEMENT 100

void geraNumerosAleatorios(int n, float *nums){
  int i;
  for(i = 0; i < n; i++){
    nums[i] = rand()%MAX_ELEMENT;
  }
}

int main (int argc, char *argv[]) {

  int meu_ranque, num_procs, qtd_nums, meu_tamanho;
  int n = DEFAULT_N;
  float *nums;

  MPI_Offset offset;
  MPI_File arquivo;
  MPI_Aint extensao_no_arquivo;
  MPI_Datatype inttype, floattype;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  // Prepara para a criacao de numeros aleatorios
  // Semente deve ser diferente para cada processo para evitar
  // a geracao de numeros iguais.
  srand(time(NULL)+meu_ranque);

  // So precisamos escrever 2*n*n numeros aleatorios no arquivo,
  // e temos duas matrizes aleatorias.
  // Distribuimos a geracao desses numeros pelos processos.

  // Calcula quantos numeros serao gerados pelo processo

  qtd_nums = (2*n*n)/num_procs;
  meu_tamanho = qtd_nums*sizeof(float);

  #ifdef DEBUG_FLAG
  printf("Processo %d, qtd_nums %d, meu_tamanho %d\n",
     meu_ranque, qtd_nums, meu_tamanho);
  #endif

  // O ultimo processo fica com as sobras
  if(meu_ranque == num_procs-1){
    int sobra = (2*n*n)%num_procs;
    qtd_nums += sobra;
    meu_tamanho += sobra*sizeof(float);
  }

  // Aloca memoria
  nums = (float *) malloc(meu_tamanho);

  // Gera os numeros
  geraNumerosAleatorios(qtd_nums, nums);

  // Abre o arquivo
  MPI_File_open(MPI_COMM_WORLD, "matmult_entrada.dat",
     MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &arquivo);

  // Setando o tipo de arquivo para external32
  MPI_File_set_view(arquivo, 0, MPI_BYTE, MPI_BYTE,
     "external32", MPI_INFO_NULL);

  // Pegando o tamanho de inteiros em external32
  MPI_File_get_type_extent(arquivo, MPI_INT, &extensao_no_arquivo);
  MPI_Type_contiguous(1, MPI_INT, &inttype);
  MPI_Type_commit(&inttype);
  
  // Conta com o N na frente de tudo no offset
  offset = extensao_no_arquivo;

  // Processo raiz escreve a ordem das matrizes
  if(meu_ranque == 0){
    MPI_File_set_view(arquivo, 0, MPI_INT, inttype,
       "external32", MPI_INFO_NULL);
    MPI_File_write(arquivo, &n, 1, MPI_INT, MPI_STATUS_IGNORE);
  }

  // Pegando o tamanho de floats em external32
  MPI_File_get_type_extent(arquivo, MPI_FLOAT, &extensao_no_arquivo);
  MPI_Type_contiguous(1, MPI_FLOAT, &floattype);
  MPI_Type_commit(&floattype);

  // Atualizando offsets com os floats
  offset += extensao_no_arquivo*((2*n*n)/num_procs)*meu_ranque;

  // Cada processo faz sua visao do arquivo
  MPI_File_set_view(arquivo, offset, MPI_FLOAT, floattype,
     "external32", MPI_INFO_NULL);
  
  // Todos os processos escrevem seus numeros gerados
  MPI_File_write(arquivo, nums, qtd_nums,
     MPI_FLOAT, MPI_STATUS_IGNORE);

  MPI_File_close(&arquivo);

  MPI_Finalize();

  return 0;

}