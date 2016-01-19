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
	int f = 400, c = f;
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

	int **matrizA = get_matriz(f,c);//reserva, pero no inicializa
	int **matrizB = get_matriz(c,f);//reserva, pero no inicializa
	int **matrizC = get_matriz(f,f);//reserva, pero no inicializa

	if (myrank== 0){

		//inicializamos
		inicializa_matriz(matrizA, f, c);
		inicializa_matriz(matrizB, c, f);
				
		if(c <= 8 && f <= 8) {
			print_matriz(matrizA, f, c);
			print_matriz(matrizB, c, f);	
		}
		if((f % size) != 0 ){ 
			printf("Necesita tener un numero par (correcto) de procesos\n"); return 0;
		}
    } 	

	rowA = malloc  (((f*c)/size) * sizeof(int));	
	resRow = malloc(((f*c)/size) * sizeof(int));	
	int offset = myrank*(f%size);
	MPI_Scatter(&matrizA[0][0], (f*c)/size, MPI_INT, &rowA[0], (f*c)/size, MPI_INT, 0, MPI_COMM_WORLD);

	res = MPI_Bcast(&matrizB[0][0], c*f, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);//todos los procesos comienzan la ejecucion a la vez

	startwtime = MPI_Wtime();

	//Procesamiento
	for (k = 0; k < f/size; k++){
		for(i = 0; i < c; i++){//numero de columnas de B
			suma = 0;
			for(j = 0; j < f; j++){
				suma += (rowA[k*f + j] * matrizB[j][i]);
			}
			resRow[k*f +i] = suma;
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);//otra barrera de seguridad

	endwtime = MPI_Wtime();

	int **matrizAux = get_matriz(f/size,c);

	for(i = 0; i < f/size; i++){
		for(j = 0; j < c; j++){
			matrizAux[i][j] = resRow[i*f +j];
		}
	}

	MPI_Gather(&matrizAux[0][0], (f*c)/size, MPI_INT, &matrizC[0][0], (f*c)/size, MPI_INT, 0, MPI_COMM_WORLD);

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
