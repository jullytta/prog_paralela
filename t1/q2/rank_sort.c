// RankSort (Enumeration Sort) Paralelo
// Entrada: vetor de ate 10^8 numeros distintos
// Saida: vetor ordenado
// Ultima revisão 01/10/2016
 
// Algoritmo
// 1. O processo raiz le o vetor original e o compartilha
// com os outros processos.
// 2. Se temos um vetor de tamanho n e p processos,
// cada processo calculara o rank de n/p numeros.
// 3. Todos os processos mandam seus resultados
// para o processo raiz, que monta o vetor ordenado.
 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define MAX_TAM 100000000
// Comente a linha abaixo caso queira desativar as
// mensagens de debug.
// #define DEBUG_FLAG
// Comente a linha abaixo caso nao queira a solucao
// impressa, mas apenas estatisticas sobre a execucao.
// # define STATS_FLAG

void imprime_vetor(double *v, int tam){

  int i;
  for(i = 0; i < tam; i++){
    printf("%g\n", v[i]);
  }
  
}

int min(int a, int b){
  return a < b? a : b;
}

int main (int argc, char *argv[]) {

  int i, j, n, p, tam_parte, meu_ranque, meu_inicio, meu_fim, raiz = 0;
  int *pos_envia, *pos_recebe;
  double tempo_inicial, tempo_final;
  double *a, *b;

  MPI_Init(&argc, &argv);  
  MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
  MPI_Comm_size(MPI_COMM_WORLD, &p);

  // Todos os processos vao receber a.
  // Estamos gastando muito espaco com isso.
  // Uma melhoria seria ter um vetor de tamanho n
  // para cada processo.

  a = (double*) malloc(MAX_TAM * sizeof(double));
  
  if(meu_ranque == raiz){

    // Apenas o processo raiz precisa de um vetor final.
    b = (double*) malloc(MAX_TAM * sizeof(double));

    // O processo raiz le a entrada no vetor a. 
    n = 0;
    while(scanf("%lf", &a[n]) != EOF)
      n++;

    #ifdef DEBUG_FLAG
    printf("Vetor original: "); imprime_vetor(a, n);
    #endif

  }

  // Desconsidera a leitura da entrada no calculo do tempo gasto.
  tempo_inicial = MPI_Wtime();

  // Primeiro o vetor 0 avisa a todos os vetores sobre o
  // tamanho da entrada.
  MPI_Bcast(&n, 1, MPI_INT, raiz, MPI_COMM_WORLD);

  // Uma vez que todos ja sabem o quanto precisam receber,
  // o vetor a e' enviado. 
  MPI_Bcast(a, n, MPI_DOUBLE, raiz, MPI_COMM_WORLD);
  
  #ifdef DEBUG_FLAG
  printf("Processo %d recebeu o vetor: ", meu_ranque);
  imprime_vetor(a, n);
  // Colocando uma pequena espera para garantir que a saida
  // fique mais legivel.
  MPI_Wtime();
  #endif

  // Tamanho da parte que cada processo e' responsavel por.
  // Vale o mesmo para todos os processos.
  tam_parte = ceil((double)n/p);

  // Onde comeca a porcao sob a responsabilidade desse processo.
  meu_inicio = meu_ranque*tam_parte;
  
  // Onde termina a porcao sob a responsabilidade desse processo.
  meu_fim = min(meu_inicio + tam_parte, n) - 1;

  #ifdef DEBUG_FLAG
  printf("Processo: %d\tInicio: %d\tFim: %d\n",
     meu_ranque, meu_inicio, meu_fim);
  #endif

  // Cada processo aloca espaco para seus resultados.
  pos_envia = (int*) malloc(tam_parte*sizeof(int));

  // O processo raiz aloca memoria para receber os resultados
  if(meu_ranque == raiz)
    pos_recebe = (int*) malloc(tam_parte*p*sizeof(int));

  // Finalmente, os processos calculam os ranks dos numeros
  // que lhe foram cedidos.
  for(i = meu_inicio; i <= meu_fim; i++){
    pos_envia[i-meu_inicio] = 0;
    for(j = 0; j < n; j++){
      if(a[j] < a[i])
        pos_envia[i-meu_inicio]++;
    }
  }

  // Hora de juntar todas essas posicoes calculadas.
  MPI_Gather (pos_envia,  tam_parte, MPI_INT,
              pos_recebe, tam_parte, MPI_INT,
              raiz, MPI_COMM_WORLD);

  #ifdef DEBUG_FLAG
  if(meu_ranque == raiz){
    printf("Posicoes recebidas: ");
    for(i = 0; i < n-1; i++){
      printf("%d ", pos_recebe[i]);
    }
    printf("%d\n", pos_recebe[n-1]);
  }
  #endif

  if(meu_ranque == raiz){
    // Monta a solucao no vetor b, usando as posicoes
    // recebidas.
    for(i = 0; i < n; i++){
      b[pos_recebe[i]] = a[i];
    }

    tempo_final = MPI_Wtime(); // Computacao concluida.

    // Imprime a solucao.
    #ifndef STATS_FLAG
    imprime_vetor(b, n);
    // Se queremos apenas stats (no caso, tempo de execucao).
    #else
    printf("%lf\t", tempo_final - tempo_inicial);
    #endif
  }

  MPI_Finalize();

  return 0;

}