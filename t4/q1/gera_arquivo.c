// Esse programa escreve DEFAULT_N, seguido de DEFAULT_N pares
// ordenados em um arquivo chamado "xydata", utilizando rotinas
// MPI I/O. Esses pares ordenados sao gerados aleatoriamente,
// utilizando uma reta conhecida, mas adicionando um pequeno erro
// ao valor de y, para que sirvam como entrada para um programa
// que estima a reta usando minimos quadrados.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

#define DEFAULT_N 5
#define MAX_RAND_X 1000
#define MAX_RAND_M 100
#define MAX_RAND_B 1000

// Parametros gerados aleatoriamente
void geraParametros(double *m, double *b){
  *m = rand()%MAX_RAND_M;
  *b = rand()%MAX_RAND_B;
}

void geraVetorEscrita(int n, double m, double b,
   double *vet_escrita){
  int i;
  for(i = 0; i < n; i += 2){
    // Gera x aleatoriamente
    double x = rand()%MAX_RAND_X;

    // Encontra o y correto
    double y = x*m + b;

    // Adiciona um erro aleatorio 'a coordenada y
    // para justicar a utilizacao de minimos quadrados
    // para encontrar a equacao
    y += rand()/(double)RAND_MAX;

    #ifdef DEBUG_FLAG
    printf("x = %.2f\t\ty = %.2f\n", x, y);
    #endif

    // Guarda os valores no vetor de escrita
    vet_escrita[i] = x;
    vet_escrita[i+1] = y;
  }
}

int main (int argc, char *argv[]) {

  int meu_ranque, num_procs, meu_tamanho, num_doubles;
  int i, n = DEFAULT_N;
  double m, b;
  double *vet_escrita;

  MPI_Offset offset;
  MPI_File arquivo;
  MPI_Status estado;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  // Prepara para a criacao de numeros aleatorios
  // Semente deve ser diferente para cada processo para evitar
  // a geracao de numeros iguais.
  srand(time(NULL)+meu_ranque);

  // Processo zero gera aleatoriamente parametros para a reta
  // e compartilha esses valores com os outros processos
  // Garante que todos os processos tem o mesmo m e o mesmo b
  if(meu_ranque == 0)
    geraParametros(&m, &b);

  MPI_Bcast(&m, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&b, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  #ifdef DEBUG_FLAG
  if(meu_ranque == 0)
    printf("Parametros: n = %d\tm = %.2f\tb = %.2f\n", n, m, b);
  #endif

  // Calcula quantos pares (x, y) sao escritos pelo processo
  num_doubles = n/num_procs;

  // Cada par tem dois doubles, logo:
  num_doubles *= 2;

  // Tamanho ocupado pelos doubles, para cada processo
  meu_tamanho = num_doubles*sizeof(double);

  // Calcula offsets, considerando que N sera escrito no inicio
  // do arquivo
  offset = sizeof(int) + meu_ranque*meu_tamanho;

  // O ultimo processo fica com as sobras
  if(meu_ranque == num_procs-1){
    int sobra = (n%num_procs)*2;
    num_doubles += sobra;
    meu_tamanho += sobra*sizeof(double);
  }

  #ifdef DEBUG_FLAG
  printf("Processo %d escreve %d pares, ocupando %d.\n",
     meu_ranque, num_doubles/2, meu_tamanho);
  #endif

  // Aloca memoria para o vetor que sera escrito
  vet_escrita = (double *) malloc(meu_tamanho);
  
  // Gera os pares aleatorios (x, y)
  geraVetorEscrita(num_doubles, m, b, vet_escrita);

  #ifdef DEBUG_FLAG
  printf("Gerados pelo processo %d:\n", meu_ranque);
  for(i = 0; i < num_doubles; i++)
    printf("%.2f\t", vet_escrita[i]);
  printf("\n");
  #endif

  // Abre o arquivo para o qual escreveremos os dados
  MPI_File_open(MPI_COMM_WORLD, "xydata",
     MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &arquivo);

  // Primeira coisa a ser impressa no arquivo: o valor de n
  // Apenas o processo raiz fara isso
  if(meu_ranque == 0){
    MPI_File_seek(arquivo, 0, MPI_SEEK_SET);
    MPI_File_write(arquivo, &n, 1, MPI_INT, MPI_STATUS_IGNORE);
  }

  // Move os ponteiros individuais para o lugar certo
  MPI_File_seek(arquivo, offset, MPI_SEEK_SET);

  // Adiciona os pares gerados ao arquivo de saida
  MPI_File_write(arquivo, vet_escrita, num_doubles,
     MPI_DOUBLE, MPI_STATUS_IGNORE);

  // Fecha o arquivo
  MPI_File_close(&arquivo);

  MPI_Finalize();

  return 0;

}
