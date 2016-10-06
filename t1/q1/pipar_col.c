/* Metodo do Trapézio Paralelo
*
* Entrada: Nenhuma.
* Saída: Estimativa da integral de a até b de f(x)
* usando o método do trapézio e n trapezóides.
*
* Algoritmo:
* 1. Cada processo calcula o seu intervalo
* de integração.
* 2. Cada processo estima a integral de f(x)
* sobre seu intervalo utilizando o método do trapézio
* 3a. Cada processo != 0 envia sua integral para 0. (Feito por redução)
* 3b. Processo 0 soma os valores recebidos dos
* processos individuais e imprime o resultado. (Feito por redução)
*
* Nota: f(x), a, b, e n são todos fixos.
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* Estaremos utilizando funções e definições MPI. */
#include "mpi.h"

#define pi ((double)(4.0*atan(1.0)))

double f(double x) {
	
	double return_val;
	return_val = 4/(1+x*x);
	return return_val;
}

double Trap(double local_a, double local_b, unsigned long int local_n, double h) {
	double integral; /* Armazena o resultado em integral */
	double x;
	unsigned long int i;
	integral = (f(local_a) + f(local_b))/2.0;
	x = local_a;
	for (i = 1; i != local_n-1; i++) {
		x += h;
		integral += f(x);
	}
	integral *= h;
	return integral;
} /* Trap */

void main(int argc, char** argv) {
	int my_rank;           /* Rank do meu processo */
	int p;                 /* O número de processos */
	double a = 0.0;         /* Limite esquerdo */
	double b = 1.0;         /* Limite direito */
	unsigned long int n = 10000000000;          /* Número de trapezóides */
	double h;               /* Comprimento da base do trapezoide */
	double local_a;         /* Limite esquedo do meu processo */
	double local_b;         /* Limite direito do meu processo  */
	unsigned long int local_n;           /* Numero de trapezoides que devo calcular */
	double integral;        /* Integral no meu intervalo */
	double total;           /* Integral total */
	int source;            /* Processo enviando a integral */
	int dest = 0;          /* Todas as mensagens vão para  0 */
	int tag = 50, meu_ranque;
	MPI_Status status;
	double tempo_inicial, tempo_final; /* Tempos de execução do processo */
	
	/* Permite ao sistema iniciar o  MPI */
	MPI_Init(&argc, &argv);
	
	tempo_inicial=MPI_Wtime(); //Recupero o tempo em que o programa iniciou sua tarefa
	
	/* Pega o rank do meu processo */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	/* Encontra quantos processos estão ativos */
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	
	h = (b-a)/n; /* h is the same for all processes */
	local_n = n/p; /* So is the number of trapezoids */
	
	/* Comprimento do intervalo de integração cada
	* processo = local_n*h. Então meu intervalo
	* inicia em: */
	
	local_a = a + my_rank * local_n * h;
	local_b = local_a + local_n * h;
	integral = Trap(local_a, local_b, local_n, h);
	
	/* Soma as integrais calculadas por cada processo */
	
	MPI_Reduce(&integral, &total, 1, MPI_DOUBLE, MPI_SUM, dest, MPI_COMM_WORLD);
	
	/* Imprime o resultado*/
	if (my_rank == 0) {
		printf("Com n = %ld trapezoides, nossa estimativa \n", n);
		printf("da integral de %1.0f até %1.0f = %15.15f \n", a, b, total);
		printf("Erro = %7.5f\n", fabs(total - pi));
		tempo_final=MPI_Wtime() - tempo_inicial;
		printf("O programa levou %f segundos para executar com %d processos.\n", tempo_final, p);
	}		

	/* Derruba o MPI */
	MPI_Finalize();

} /* main */
