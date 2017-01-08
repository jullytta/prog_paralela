// Minimos Quadrados Paralelo
// Entrada: N pares (x,y) representando pontos, lidos de xydata
// Saida: aproximacao da equacao y = x*m + b
// que passa por esses pontos.
// Ultima revisão 08/01/2016

// Feito em cima da versao de Dora Abdullah

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv) {

  double *x, *y, *buffer_read;
  double mySUMx, mySUMy, mySUMxy, mySUMxx, SUMx, SUMy, SUMxy,
         SUMxx, SUMres, res, slope, y_intercept, y_estimate;

  int i, j, n, myid, numprocs, naverage, nremain,
      mypoints, mysize;

  MPI_Offset offset;
  MPI_File file;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);

  // Abre o arquivo
  MPI_File_open(MPI_COMM_WORLD, "xydata", MPI_MODE_RDONLY,
     MPI_INFO_NULL, &file);

  // Processo 0 le N e envia para todos os processos
  if(myid == 0){
    MPI_File_seek(file, 0, MPI_SEEK_SET);
    MPI_File_read(file, &n, 1, MPI_INT, &status);
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Cada processo calcula o quanto vai ler e seu offset
  naverage = n/numprocs;
  nremain = n%numprocs;
  mypoints = naverage;
  mysize = naverage*2*sizeof(double);

  offset = sizeof(int) + mysize*myid;

  #ifdef DEBUG_FLAG
  printf("Processo %d, offset %d\n", myid, offset);
  #endif

  if(myid == numprocs-1){
    mypoints += nremain;
    mysize += nremain*2*sizeof(double);
  }

  // Processos movem os ponteiros individuais para
  // as posicoes corretas
  MPI_File_set_view(file, offset, MPI_DOUBLE, MPI_DOUBLE,
     "native", MPI_INFO_NULL);

  // Alocacao de memoria para os vetores de x e y
  // e o buffer de leitura
  x = (double *) malloc (mypoints*sizeof(double));
  y = (double *) malloc (mypoints*sizeof(double)); 
  buffer_read = (double *) malloc (mysize);

  // Leitura do arquivo
  MPI_File_read(file, buffer_read, mypoints*2,
     MPI_DOUBLE, &status);

  #ifdef DEBUG_FLAG
  printf("Processo %d, %d pontos lidos:\n", myid, mypoints);
  for(i = 0; i < mypoints*2; i++)
    printf("%.2f\t", buffer_read[i]);
  printf("\n");
  #endif

  // Fecha o arquivo
  MPI_File_close(&file);

  // Cada processo calcula sua soma parcial
  mySUMx = 0; mySUMy = 0; mySUMxy = 0; mySUMxx = 0;
  
  for (j = 0; j < mypoints; j++) {
    mySUMx = mySUMx + x[j];
    mySUMy = mySUMy + y[j];
    mySUMxy = mySUMxy + x[j]*y[j];
    mySUMxx = mySUMxx + x[j]*x[j];
  }
  
  // Reducao das somas
  MPI_Reduce(&mySUMx, &SUMx, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&mySUMy, &SUMy, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&mySUMxy, &SUMxy, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&mySUMxx, &SUMxx, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  // Processo zero faz os calculos finais
  if (myid == 0) {
    slope = ( SUMx*SUMy - n*SUMxy ) / ( SUMx*SUMx - n*SUMxx );
    y_intercept = ( SUMy - slope*SUMx ) / n;
    
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
  }

  /* ---------------------------------------------------------- */
  MPI_Finalize();
}