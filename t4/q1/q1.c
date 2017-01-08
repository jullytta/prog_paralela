// TODO(jullytta): descricao do programa

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

#define DEFAULT_N 10
#define MAX_RAND_X 1000
#define MAX_RAND_M 100
#define MAX_RAND_B 1000

// Parametros gerados aleatoriamente
void geraParametros(double *m, double *b){
  *m = rand()%MAX_RAND_M;
  *b = rand()%MAX_RAND_B;
}

int main (int argc, char *argv[]) {

  int meu_ranque, num_procs, meu_tamanho;
  int i, n = DEFAULT_N;
  double m, b;
  MPI_Offset offset;
  MPI_File arquivo;
  MPI_Status estado;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  // Gera aleatoriamente parametros para a reta
  geraParametros(&m, &b);

  #ifdef DEBUG_FLAG
  if(meu_ranque == 0)
    printf("Parametros: n = %d\tm = %.2f\tb = %.2f\n", n, m, b);
  #endif

  // Calcula quantos pares (x, y) sao escritos pelo processo
  meu_tamanho = n/num_procs;

  // O ultimo processo fica com as sobras
  if(meu_ranque == num_procs-1){
    meu_tamanho += n%num_procs;
  }

  #ifdef DEBUG_FLAG
  printf("Processo %d escreve %d pares.\n",
     meu_ranque, meu_tamanho);
  #endif

  // Cada processo tem um offset diferente, mas todos devem
  // considerar o N escrito no inicio

  // Abre o arquivo para o qual escreveremos os dados
  // TODO

  // Prepara para a criacao de numeros aleatorios
  // Semente deve ser diferente para cada processo para evitar
  // a geracao de numeros iguais.
  srand(time(NULL)+meu_ranque);
  
  // Primeira coisa a ser impressa no arquivo: o valor de n
  // TODO

  // Seguido de n linhas de pares x, y que facam
  // parte da equacao y = xm + b
  for(i = 0; i < meu_tamanho; i++){
    // Gera x aleatoriamente
    double x = rand()%MAX_RAND_X;

    // Encontra o y correto
    double y = x*m + b;

    #ifdef DEBUG_FLAG
    printf("x = %.2f\t\ty = %.2f\n", x, y);
    #endif

    // Adiciona um erro aleatorio 'a coordenada y
    // para justicar a utilizacao de minimos quadrados
    // para encontrar a equacao
    y += rand()/(double)RAND_MAX;

    // Adiciona o par gerado ao arquivo de saida
    // TODO
  }

  // Fecha o arquivo
  // TODO

  MPI_Finalize();

  return 0;

}
