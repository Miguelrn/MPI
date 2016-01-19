#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

float **get_matriz ( int f, int c )
{
	int i,j;
    float **m;
	m = (float**) malloc (sizeof(float*)*f);
	for(i = 0; i < f; i++){
		m[i] = (float*) malloc(sizeof(float)*c);
	}

    for (i=0 ; i<f ; i++)
    	for (j=0 ; j<c ; j++)
        {
			m[i][j] = rand() % 11; //numeros de 0 a 10
		}              
	return m;
}

void print_matriz ( float **m, int f, int c )
{
	int i,j;
    printf("-----------------------------------------\n");
    for (i=0 ; i<f ; i++)
    {
    	for (j=0 ; j<c ; j++)
        	printf("%13.5g ",m[i][j]);
        printf("\n");
	}            
    printf("-----------------------------------------\n");        
}


int main(int argc, char *argv[]){
	
	srand (time(NULL));
	int i, j, z;
	int f = 4, c = 4;
	double startwtime, endwtime;
	int myrank, size;

	float** matrizA, **matrizB, **matrizC;

	matrizC = (float**) malloc (sizeof(float*)*f);
	for(i = 0; i < f; i++){
		matrizC[i] = (float*) malloc(sizeof(float)*f);
	}

	matrizA = get_matriz(f,c);
	matrizB = get_matriz(c,f);

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);	
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	if(c <= 8 && f <= 8) {	
		print_matriz ( matrizA, f, c);
		print_matriz ( matrizB, c, f);
	}

	startwtime = MPI_Wtime();

	for(i = 0; i < f; i++)//cada fila del proceso, sobre la matriz A
    {
    	for (j=0; j < c; j++)//cada columna del proceso
        {
        	matrizC[i][j] = 0;
            for (z = 0; z < c ; z++){//cada fila de la columna j
            	matrizC[i][j] += matrizA[i][z] * matrizB[z][j];
			}
		}            
   	} 

	endwtime = MPI_Wtime();

	if(c <= 8 && f <= 8) {
		print_matriz ( matrizC, f, f);	
	}       	
	printf("Tiempo de ejecucion: %f, usando %d+1 maquinas\n", endwtime - startwtime, size - 1);		  
	if (matrizA!=NULL) free(matrizA);
	if (matrizB!=NULL) free(matrizB);
	if (matrizC!=NULL) free(matrizC);


	MPI_Finalize();	
	return 0;

}
