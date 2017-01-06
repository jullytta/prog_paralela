// Gera entradas para o problema de minimos quadrados.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define DEFAULT_N 10
#define MAX_RAND_X 1000
#define MAX_RAND_M 100
#define MAX_RAND_B 1000

void geraParametros(double *m, double *b){
  *m = rand()%MAX_RAND_M;
  *b = rand()%MAX_RAND_B;
}

int main (int argc, char *argv[]) {

  int i, n = DEFAULT_N;
  double m, b;

  // Prepara para a criacao de numeros aleatorios
  srand(time(NULL));

  // Gera aleatoriamente parametros para a reta
  geraParametros(&m, &b);

  // Abre o arquivo para o qual escreveremos os dados
  // TODO
  
  // Primeira coisa a ser impressa no arquivo: o valor de n
  // TODO

  #ifdef DEBUG_FLAG
  printf("n = %d\tm = %.2f\tb = %.2f\n", n, m, b);
  #endif

  // Seguido de n linhas de pares x, y que facam
  // parte da equacao y = xm + b
  for(i = 0; i < n; i++){
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

  return 0;

}
