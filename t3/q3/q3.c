/*********************************************************/
/* Crivo de Eratóstenes Paralelo em OpenMP               */
/* Entrada: numero N > 1                                 */
/* Saida: quantidade de primos entre 2 e N, inclusive    */
/* Ultima revisão 11/12/2016                             */
/*                                                       */
/*********************** Algoritmo ***********************/
/* 1. A thread master le a entrada e prepara o vetor de  */
/* marcacao.                                             */
/* 2. Cada thread marca o vetor para uma parcela dos     */
/* numeros entre 2 e N.                                  */
/* 3. Cada thread olha uma parte do vetor de marcacoes e */
/* incrementa seu contador de primos.                    */
/* 4. Reducao de soma entre os contadores.               */
/*********************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_THREADS 4

int main (int argc, char *argv[]){

  char *marcacoes;
  int i, j, n, n_primos;
  double tempo_inicial, tempo_final;
  
  omp_set_num_threads(NUM_THREADS);

  scanf("%d", &n);
  if(n < 2){
    printf("O numero de entrada deve ser maior que 2.\n");
    return 1;
  }

  /* Inclusive n */
  n++;

  tempo_inicial = omp_get_wtime();

  marcacoes = (char*) malloc(n * sizeof(char));

  /* Inicializa todo o vetor de marcacoes para zero */
  #pragma omp parallel for \
          shared(marcacoes, n) \
          private(i) \
          default(none)
  for(i = 0; i < n; i++)
    marcacoes[i] = 0; 

  /* 0 e 1 nao sao primos por definicao. */
  marcacoes[0] = marcacoes[1] = 1;

  /**************************************************/
  /* Cada thread marca o vetor para uma parte dos   */
  /* numeros de 2 a N.                              */
  /* Como as iteracoes sao desbalanceadas, dynamic  */
  /* parece ser um schedule mais eficiente.         */
  /**************************************************/
  #pragma omp parallel for \
          shared(marcacoes, n) \
          private(i, j) \
          default(none)
  for(i = 2; i < n; i++){
    for(j = 2; i*j < n; j++){
      marcacoes[i*j] = 1;
    }
  }

  n_primos = 0;
  #pragma omp parallel for \
          shared(marcacoes, n) \
          private(i) \
          default(none) \
          reduction(+:n_primos)
  for(i = 2; i < n; i++){
    if(!marcacoes[i]){
      n_primos++;
    }
  }

  #ifndef STATS_FLAG
  printf("# de primos entre 2 e %d: %d\n", n-1, n_primos);
  #endif

  tempo_final = omp_get_wtime();
	
  #ifdef STATS_FLAG
  printf("%f\t", tempo_final-tempo_inicial);
  #endif

  return 0;

}
