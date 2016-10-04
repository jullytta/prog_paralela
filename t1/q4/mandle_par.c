/* Parallel Mandlebrot program */

/* Compile with mpicc -o mp mandle_par.c -lm -lX11 */


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "mpi.h"

#define		X_RESN	800       /* x resolution */
#define		Y_RESN	800       /* y resolution */

typedef struct complextype
	{
        float real, imag;
	} Compl;


void main (int argc, char** argv)
{
	Window		win;                            /* initialization for a window */
	unsigned
	int             width, height,                  /* window size */
                        x, y,                           /* window position */
                        border_width,                   /*border width in pixels */
                        display_width, display_height,  /* size of screen */
                        screen;                         /* which screen */

	char            *window_name = "Mandelbrot Set", *display_name = NULL;
	GC              gc;
	unsigned long	valuemask = 0;
	XGCValues	values;
	Display		*display;
	XSizeHints	size_hints;
	Pixmap		bitmap;
	XPoint		points[800];
	FILE		*fp, *fopen ();
	char		str[100];
	
	
	XSetWindowAttributes attr[1];

       /* Mandlebrot variables */
        int i, j, k;
        Compl	z, c;
        float	lengthsq, temp;
        
        //Variáveis usadas para a paralelização
        int raiz = 0, tag=50;
        int my_rank, tamanho_vetor;           /* Rank do meu processo */
        int p;                 /* O número de processos */
        int *pixels, *imagem; //Represento a matriz como um vetor. Os índices [i, j] são acessados por [i*n + j], assumindo-se uma matriz com n linhas.
        int increment, inicio, fim; /* definem o inicio e fim dos índices de varredura de cada processo */
        
        /* Permite ao sistema iniciar o  MPI */
	MPI_Init(&argc, &argv);
	
	/* Pega o rank do meu processo */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	/* Encontra quantos processos estão ativos */
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	
	if(p<=1 || X_RESN % p != 0){
		if(my_rank==0)
			printf("O número de processos deve ser divisor de 800 maior que 1\n");
		MPI_Finalize();
		exit(0);
	}
	
	increment = X_RESN/p;
	
	inicio=my_rank*increment;
	fim=inicio+increment;
	
	//Aloco o vetor para dizer se deve ou não haver pintura do pixel na tela
	//Inicialmente, todos os pixels estão em 0, valor que indica que não devem ser impressos
	//O valor 1 indica que o pixel deve ser impresso na tela
	tamanho_vetor=increment*Y_RESN;
	pixels=malloc(tamanho_vetor*sizeof(int)); //matriz de increment linhas por Y_RESN colunas

	//Faço o cálculo para definir quais pixels são desenhados
	
	int cnt=0;
	int index=0;
	
	for(i=inicio; i < fim; i++){ //Varro as linhas, sendo cada processo processo responsável por uma porção das linhas
		for(j=0; j < Y_RESN; j++) { //Varro as colunas

			z.real = z.imag = 0.0;
			c.real = ((float) j - 400.0)/200.0;               /* scale factors for 800 x 800 window */
			c.imag = ((float) i - 400.0)/200.0;
			k = 0;

			do  {                                             /* iterate for pixel color */

				temp = z.real*z.real - z.imag*z.imag + c.real;
				z.imag = 2.0*z.real*z.imag + c.imag;
				z.real = temp;
				lengthsq = z.real*z.real+z.imag*z.imag;
				k++;

			} while (lengthsq < 4.0 && k < 100);

			if (k == 100){
					cnt++;										
					pixels[(index*increment) + j]=1; //Indico que este pixel deve ser desenhado
				} else pixels[(index*increment) + j] = 0;

		}
		index++;
	}
		
	//Agrupo todos os pixels na matriz imagem	
	int total=0;
	MPI_Reduce(&cnt, &total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if(my_rank==raiz)
		imagem=malloc(tamanho_vetor*p*sizeof(int));

	MPI_Gather(pixels, tamanho_vetor, MPI_INT, imagem, tamanho_vetor, MPI_INT, raiz, MPI_COMM_WORLD);	
	
	if(my_rank==raiz){
		printf("Total de pixels para desenhar: %d\n", total);
		total=0;
		for(i=0; i < tamanho_vetor*p; i++){
			if(imagem[i]==1)
				total++;
		}
		printf("Total calculado: %d\n", total);
	}
	
	MPI_Finalize();
	exit(0);

//	free(pixels);		
	
	if(my_rank==raiz){
       
		/* connect to Xserver */

		if (  (display = XOpenDisplay (display_name)) == NULL ) {
		   fprintf (stderr, "drawon: cannot connect to X server %s\n",
					XDisplayName (display_name) );
		exit (-1);
		}
	
		/* get screen size */

		screen = DefaultScreen (display);
		display_width = DisplayWidth (display, screen);
		display_height = DisplayHeight (display, screen);

		/* set window size */

		width = X_RESN;
		height = Y_RESN;

		/* set window position */

		x = 0;
		y = 0;

		/* create opaque window */

		border_width = 4;
		win = XCreateSimpleWindow (display, RootWindow (display, screen),
					x, y, width, height, border_width, 
					BlackPixel (display, screen), WhitePixel (display, screen));

		size_hints.flags = USPosition|USSize;
		size_hints.x = x;
		size_hints.y = y;
		size_hints.width = width;
		size_hints.height = height;
		size_hints.min_width = 300;
		size_hints.min_height = 300;
	
		XSetNormalHints (display, win, &size_hints);
		XStoreName(display, win, window_name);

		/* create graphics context */

		gc = XCreateGC (display, win, valuemask, &values);

		XSetBackground (display, gc, WhitePixel (display, screen));
		XSetForeground (display, gc, BlackPixel (display, screen));
		XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

		attr[0].backing_store = Always;
		attr[0].backing_planes = 1;
		attr[0].backing_pixel = BlackPixel(display, screen);

		XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

		XMapWindow (display, win);
		XSync(display, 0);
	      	 
		/* Calculate and draw points */
		int contador=0;
		//Provavelmente está errado
		for(i=0; i < X_RESN; i++) {
			for(j=0; j < Y_RESN; j++)
				if (imagem[(i*X_RESN)+ j]!=0){ XDrawPoint (display, win, gc, i, j);
			       		contador++; 
				}
		}
	
		printf("%d\n", contador);
		 
		XFlush (display);

		sleep (30);
	
	}
	
	MPI_Finalize();
	exit(0);

	/* Program Finished */

}
