#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

int **get_matriz(int f, int c) {
    int *data = (int *)malloc(f*c*sizeof(int));
    int **array= (int **)malloc(f*sizeof(int*));
	int i,j;
    for (i=0; i<f; i++)
        array[i] = &(data[c*i]);

	
    return array;
}

void inicializa_matriz(int **m, int f, int c){
	int i,j;
	for (i=0 ; i<f ; i++)
    	for (j=0 ; j<c ; j++)
        {
		m[i][j] = rand() % 11; //numeros de 0 a 10
	} 
}

void print_matriz ( int **m, int f, int c )
{
	int i,j;
    printf("-----------------------------------------\n");
    for (i=0 ; i<f ; i++)
    {
    	for (j=0 ; j<c ; j++)
        	printf("\t%d ",m[i][j]);
        printf("\n");
	}            
    printf("-----------------------------------------\n");        
}


int main(int argc, char *argv[]){
	
	srand (time(NULL));
	int myrank, size;
	int i, j, z, x, y, k;
	int f = 16, c = 16;
	int completados = 0;
	int res;
	int *resRow;
	int *rowA;
	MPI_Status stats;
	MPI_Request request;
	int suma;
 	double startwtime, endwtime;
	int filasEnviadas = 0, filasRecibidas = 0;
	int receptor;

	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);	
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	int restantes = (f*c)%size;//elementos restantes
    int *cantidadMandados;//array de elementos enviados a cada trabajador    
    int *desplazamiento;//desplazamiento entre los envios (total datos / size)
	int sum = 0;

	int **matrizA = get_matriz(f,c);//reserva, pero no inicializa
	int **matrizB = get_matriz(c,f);//reserva, pero no inicializa
	int **matrizC = get_matriz(f,f);//reserva, pero no inicializa

    cantidadMandados = malloc(sizeof(int)*size);
    desplazamiento = malloc(sizeof(int)*size);

	if (myrank== 0){

		//inicializamos
		inicializa_matriz(matrizA,f,c);
		inicializa_matriz(matrizB,c,f);
				
		if(c <= 8 && f <= 8) {
			print_matriz(matrizA, f, c);
			print_matriz(matrizB, c, f);	
		}		
    } 
	
	for (i = 0; i < size; i++) {
		cantidadMandados[i] = (f*c - sum)/(size - i);
		
		if((cantidadMandados[i] % f) != 0){//es decir no tiene la ultima fila completa
			cantidadMandados[i] += (f - (cantidadMandados[i] % f));
		}
		desplazamiento[i] = sum;
		sum += cantidadMandados[i];
	
    }

	rowA = malloc(cantidadMandados[myrank] * sizeof(int));	
	resRow = malloc(cantidadMandados[myrank] * sizeof(int));
	/*if (0 == myrank) {
        for (i = 0; i < size; i++) {
            printf("cantidadMandados[%d] = %d\tdesplazamiento[%d] = %d\n", i, cantidadMandados[i], i, desplazamiento[i]);
        }
    }*/


	res = MPI_Bcast(&matrizB[0][0], c*f, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Scatterv(&matrizA[0][0], cantidadMandados, desplazamiento, MPI_INT, &rowA[0], cantidadMandados[myrank], MPI_INT, 0, MPI_COMM_WORLD);

	if(myrank == 0)
		startwtime = MPI_Wtime();
	
	//Procesamiento
	for (k = 0; k < cantidadMandados[myrank]/f; k++){
		for(i = 0; i < c; i++){//numero de columnas de B
			suma = 0;
			for(j = 0; j < f; j++){
				suma += (rowA[k*f + j] * matrizB[j][i]);
			}
			resRow[k*f +i] = suma;
		}
	}

	if(myrank == 0)
		endwtime = MPI_Wtime();

	MPI_Gatherv(&resRow[0], cantidadMandados[myrank], MPI_INT, &matrizC[0][0], cantidadMandados, &desplazamiento[myrank], MPI_INT, 0, MPI_COMM_WORLD);//5º parametro es int *recvcount
	
	MPI_Finalize();	  

	if(myrank == 0) {	

		if(c <= 8 && f <= 8) print_matriz(matrizC,f,f);

		printf("Tiempo de ejecucion: %f, usando %d+1 maquinas\n", endwtime - startwtime, size - 1);
	}

	if (matrizA!=NULL) free(matrizA);
	if (matrizB!=NULL) free(matrizB);
	if (matrizC!=NULL) free(matrizC);
	if (resRow!=NULL) free(resRow);
	if (rowA!=NULL) free(rowA); 

	return 0;

}
