// Minimos Quadrados Paralelo
// Entrada: N pares (x,y) representando pontos
// Saida: aproximacao da equacao y = x*m + b
// que passa por esses pontos.
// Ultima revisão 03/10/2016 

// Feito em cima da versao de Dora Abdullah

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv) {

  double *x, *y;
  double mySUMx, mySUMy, mySUMxy, mySUMxx, SUMx, SUMy, SUMxy,
         SUMxx, SUMres, res, slope, y_intercept, y_estimate;

  double tempo_inicial, tempo_final;

  int i,j,n,myid,numprocs,naverage,nremain,mypoints,ishift;

  /*int new_sleep (int seconds);*/
  MPI_Status istatus;
  FILE *infile;

  infile = fopen("xydata", "r");
  if (infile == NULL) printf("error opening file\n");

  MPI_Init(&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);

  /* ----------------------------------------------------------
   * Step 1: Process 0 reads data and sends the value of n
   * ---------------------------------------------------------- */
  if (myid == 0) {
    #ifndef STATS_FLAG
    printf ("Number of processes used: %d\n", numprocs);
    printf ("-------------------------------------\n");
    printf ("The x coordinates on worker processes:\n");
    /* this call is used to achieve a consistent output format */
    #endif
    /* new_sleep (3);*/
    fscanf (infile, "%d", &n);
  }

  // Processo raiz envia para todos o valor de n.
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  // Todos entao podem alocar a memoria necessaria.
  x = (double *) malloc (n*sizeof(double));
  y = (double *) malloc (n*sizeof(double));
  
  // Agora o processo raiz le os dados
  if(myid == 0){
    for (i=0; i<n; i++)
      fscanf (infile, "%lf %lf", &x[i], &y[i]);
  }

  // Desconsidera a leitura da entrada no calculo do tempo gasto.
  tempo_inicial = MPI_Wtime();

  /* ---------------------------------------------------------- */
  
  naverage = n/numprocs;
  nremain = n % numprocs;

  /* ----------------------------------------------------------
   * Step 2: Process 0 sends x and y 
   * ---------------------------------------------------------- */

  // O enunciado da questao sugere enviar todos os valores para
  // todos os processos. Parece um desperdicio, mas seguindo a risca:

  MPI_Bcast(x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(y, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Os processos precisam saber de que parte sao responsaveis
  ishift = myid*naverage;
  mypoints = (myid < numprocs -1) ? naverage : naverage + nremain;

  /* ----------------------------------------------------------
   * Step 3: Each process calculates its partial sum
   * ---------------------------------------------------------- */
  mySUMx = 0; mySUMy = 0; mySUMxy = 0; mySUMxx = 0;
  
  for (j=0; j<mypoints; j++) {
    mySUMx = mySUMx + x[ishift+j];
    mySUMy = mySUMy + y[ishift+j];
    mySUMxy = mySUMxy + x[ishift+j]*y[ishift+j];
    mySUMxx = mySUMxx + x[ishift+j]*x[ishift+j];
  }
  
  /* ----------------------------------------------------------
   * Step 4: Process 0 receives partial sums from the others 
   * ---------------------------------------------------------- */
  
  // Reducao das somas
  MPI_Reduce(&mySUMx, &SUMx, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&mySUMy, &SUMy, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&mySUMxy, &SUMxy, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&mySUMxx, &SUMxx, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  /* ----------------------------------------------------------
   * Step 5: Process 0 does the final steps
   * ---------------------------------------------------------- */
  if (myid == 0) {
    slope = ( SUMx*SUMy - n*SUMxy ) / ( SUMx*SUMx - n*SUMxx );
    y_intercept = ( SUMy - slope*SUMx ) / n;
    
    
    tempo_final = MPI_Wtime(); // Computacao concluida.
    
    #ifndef STATS_FLAG
   
    /* this call is used to achieve a consistent output format */
    /*new_sleep (3);*/
    printf ("\n");
    printf ("The linear equation that best fits the given data:\n");
    printf ("       y = %6.2lfx + %6.2lf\n", slope, y_intercept);
    printf ("--------------------------------------------------\n");
    printf ("   Original (x,y)     Estimated y     Residual\n");
    printf ("--------------------------------------------------\n");
    
    SUMres = 0;
    for (i=0; i<n; i++) {
      y_estimate = slope*x[i] + y_intercept;
      res = y[i] - y_estimate;
      SUMres = SUMres + res*res;
      printf ("   (%6.2lf %6.2lf)      %6.2lf       %6.2lf\n", 
        x[i], y[i], y_estimate, res);
    }
    printf("--------------------------------------------------\n");
    printf("Residual sum = %6.2lf\n", SUMres);
    // Se queremos apenas stats (no caso, tempo de execucao).
    #else
    printf("%lf\t", tempo_final - tempo_inicial);
    #endif
  }

  /* ---------------------------------------------------------- */
  MPI_Finalize();
}