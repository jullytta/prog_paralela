// Minimos Quadrados Paralelo
// Entrada: N pares (x,y) representando pontos
// Saida: aproximacao da equacao y = x*m + b
// que passa por esses pontos.
// Ultima revisão 05/10/2016 

// Feito em cima da versao de Dora Abdullah

// Observacao importante:
// O enunciado desta questao sugeria enviar os valores
// de todos os pontos para todos os processos.
// Contudo, para grandes valores de n a sobrecarga de
// comunicacao faz a eficiencia cair desastrosamente.
// Por isso, esse programa pode ser compilado com a flag
// BCAST_FLAG caso seja desejado seguir a sugestao acima
// mencionada. Caso contrario, o programa usa MPI_Scatterv,
// enviando apenas o necessario para cada processo para
// ganhar eficiencia.

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv) {

  double *x, *y, *buffer_x, *buffer_y;
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

    #ifndef BCAST_FLAG
    // Processo raiz aloca memoria para os dois vetores
    x = (double *) malloc (n*sizeof(double));
    y = (double *) malloc (n*sizeof(double));   
    #endif
  }

  // Processo raiz envia para todos o valor de n.
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  #ifdef BCAST_FLAG
  // Todos entao podem alocar a memoria necessaria.
  x = (double *) malloc (n*sizeof(double));
  y = (double *) malloc (n*sizeof(double));
  #endif

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

  // Os processos precisam saber de que parte sao responsaveis
  ishift = myid*naverage;
  mypoints = (myid < numprocs -1) ? naverage : naverage + nremain;

  #ifndef BCAST_FLAG
  // Os vetores criados abaixo sao necessarios para utilizar
  // o MPI_Scatterv. Usamos esse scatter porque n nao e'
  // sempre divisivel por numprocs. O ultimo processo fica
  // com o trabalho extra.

  // Esse vetor guarda como serao distribuidos os pontos
  int *distribuicao;
  distribuicao = (int *) malloc(numprocs*sizeof(int));
  for(i = 0; i < numprocs-1; i++){
    distribuicao[i] = naverage;
  }
  distribuicao[numprocs-1] = naverage + nremain;

  // Esse vetor guarda onde a parte de cada processo comeca
  int *deslocamento;
  deslocamento = (int *) malloc(numprocs*sizeof(int));
  for(i = 0; i < numprocs; i++){
    deslocamento[i] = naverage*i;
  }
  #endif

  #ifdef BCAST_FLAG
  // O enunciado da questao sugere enviar todos os valores para
  // todos os processos. Parece um desperdicio, mas seguindo a risca:

  MPI_Bcast(x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(y, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  #else

  // Fazendo de um jeito mais eficiente com scatter:
  buffer_x = (double *) malloc(mypoints*sizeof(double));
  buffer_y = (double *) malloc(mypoints*sizeof(double));

  MPI_Scatterv(x, distribuicao, deslocamento,
               MPI_DOUBLE, buffer_x, mypoints,
               MPI_DOUBLE, 0, MPI_COMM_WORLD);

  MPI_Scatterv(y, distribuicao, deslocamento,
               MPI_DOUBLE, buffer_y, mypoints,
               MPI_DOUBLE, 0, MPI_COMM_WORLD);

  #endif

  /* ----------------------------------------------------------
   * Step 3: Each process calculates its partial sum
   * ---------------------------------------------------------- */
  mySUMx = 0; mySUMy = 0; mySUMxy = 0; mySUMxx = 0;
  
  #ifdef BCAST_FLAG
  for (j=0; j<mypoints; j++) {
    mySUMx = mySUMx + x[ishift+j];
    mySUMy = mySUMy + y[ishift+j];
    mySUMxy = mySUMxy + x[ishift+j]*y[ishift+j];
    mySUMxx = mySUMxx + x[ishift+j]*x[ishift+j];
  }
  #else
  for (j=0; j<mypoints; j++) {
    mySUMx = mySUMx + buffer_x[j];
    mySUMy = mySUMy + buffer_y[j];
    mySUMxy = mySUMxy + buffer_x[j]*buffer_y[j];
    mySUMxx = mySUMxx + buffer_x[j]*buffer_x[j];
  }  
  #endif
  
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