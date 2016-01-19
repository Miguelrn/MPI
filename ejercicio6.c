#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

float* get_vector( int num )
{
	int i;
    float* m;
	
	m = (float*) malloc (sizeof(float)*num);

    for (i=0 ; i<num ; i++) {
		m[i] = rand() % 11; //numeros de 0 a 10
	}  
            
	return m;
}

void print_vector ( float* m, int num )
{
	int i;
    printf("---------------------------------------------\n");
    for (i=0 ; i<num ; i++)
    {
        printf("%2.5g ",m[i]);
	}            
    printf("\n---------------------------------------------\n");        
}


int main(int argc, char *argv[]){
	
	srand (time(NULL));
	int i, j, z;
	int tamanio = 400;

	float* vectorA;
	float* vectorB;
	int resultado = 0;
	double startwtime, endwtime;
	int myrank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	vectorA = get_vector(tamanio);
	vectorB = get_vector(tamanio);

	if(tamanio <= 15) { //Mostramos hasta vectores de 15 elementos	
		print_vector ( vectorA, tamanio);
		print_vector ( vectorB, tamanio);
	}

	startwtime = MPI_Wtime();
	
	for(i = 0; i < tamanio; i++)//cada elemento del vector
    {
    	resultado += vectorA[i]*vectorB[i];     
   	}

	endwtime = MPI_Wtime();
	printf("Tiempo de ejecucion: %f, usando %d+1 maquinas\n", endwtime - startwtime, size - 1);	

	printf("\nEl resultado del producto escalar de los vectores es: %d\n\n", resultado);	  
	MPI_Finalize();	

	return 0;

}
