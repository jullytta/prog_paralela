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

#define DOUBLES_POR_BLOCO 2

int main(int argc, char **argv) {

  double *x, *y, *buffer_read;
  double mySUMx, mySUMy, mySUMxy, mySUMxx, mySUMres,
         SUMx, SUMy, SUMxy, SUMxx, SUMres,
         res, slope, y_intercept, y_estimate;

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

  // Cria o tipo derivado do arquivo para fazer a vista
  // Precisamos do tipo indexed porque temos um numero diferente
  // de pontos para cada processo.
  MPI_Datatype filetype;
  int count = numprocs;
  int *block_lengths, *displs;

  block_lengths = (int *) malloc(numprocs*sizeof(int));
  displs = (int *) malloc(numprocs*sizeof(int));

  for(i = 0; i < numprocs; i++){
    block_lengths[i] = naverage;
    displs[i] = i*naverage;
  }
  block_lengths[numprocs-1] += nremain;

  MPI_Type_indexed(count, block_lengths, displs, MPI_DOUBLE, &filetype);
  MPI_Type_commit(&filetype);

  // Cria uma vista de arquivo unica com o mesmo filetype
  MPI_File_set_view(file, offset,
     MPI_DOUBLE, filetype, "native", MPI_INFO_NULL);

  // Alocacao de memoria para os vetores de x e y
  // e o buffer de leitura
  x = (double *) malloc (mypoints*sizeof(double));
  y = (double *) malloc (mypoints*sizeof(double)); 
  buffer_read = (double *) malloc (mysize);

  // Leitura do arquivo
  MPI_File_read_all(file, buffer_read, mypoints*2,
     MPI_DOUBLE, &status);

  #ifdef DEBUG_FLAG
  printf("Processo %d, %d pontos lidos:\n", myid, mypoints);
  for(i = 0; i < mypoints*2; i++)
    printf("%.2f\t", buffer_read[i]);
  printf("\n");
  #endif

  // Passa o arquivo lido para os vetores locais x e y
  j = 0;
  for(i = 0; i < mypoints*2; i++){
    if(i%2 == 0)
      x[j] = buffer_read[i];
    else{
      y[j] = buffer_read[i];
      j++;
    }
  }

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
  // Utilizamos all reduce para que todos os processos possam
  // estimar os parametros da reta e, em seguida, estimar valores
  // para y em seus pontos.
  MPI_Allreduce(&mySUMx, &SUMx, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Allreduce(&mySUMy, &SUMy, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Allreduce(&mySUMxy, &SUMxy, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Allreduce(&mySUMxx, &SUMxx, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  // Processos estimam os parametros da reta
  slope = ( SUMx*SUMy - n*SUMxy ) / ( SUMx*SUMx - n*SUMxx );
  y_intercept = ( SUMy - slope*SUMx ) / n;

  // Processo zero imprime a mensagem inicial
  if (myid == 0) {
    printf ("\n");
    printf ("The linear equation that best fits the given data:\n");
    printf ("       y = %6.2lfx + %6.2lf\n", slope, y_intercept);
    printf ("--------------------------------------------------\n");
    printf ("   Original (x,y)     Estimated y     Residual\n");
    printf ("--------------------------------------------------\n");
  }

  // Cada processo imprime seus pontos
  mySUMres = 0;
  for (i=0; i<mypoints; i++) {
    y_estimate = slope*x[i] + y_intercept;
    res = y[i] - y_estimate;
    mySUMres += res*res;
    printf ("   (%6.2lf %6.2lf)      %6.2lf       %6.2lf\n", 
      x[i], y[i], y_estimate, res);
  }

  // A soma residual individual e' reduzida
  MPI_Reduce(&mySUMres, &SUMres, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);  

  if(myid == 0){
    printf("--------------------------------------------------\n");
    printf("Residual sum = %6.2lf\n", SUMres);
  }

  /* ---------------------------------------------------------- */
  MPI_Finalize();
}