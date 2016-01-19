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
	int f = 200, c = f;
	int completados = 0;
	int res;
	int *resRow;
	int *rowA;
	MPI_Status status[f+1], stats;
	MPI_Request request[f+1];
	int suma;
 	double startwtime, endwtime;
	int filasEnviadas = 0, filasRecibidas = 0;
	int receptor = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);	
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	int **matrizA = get_matriz(f,c);//reserva, pero no inicializa
	int **matrizB = get_matriz(c,f);//reserva, pero no inicializa
	int **matrizC = get_matriz(f,f);//reserva, pero no inicializa

	rowA   = malloc((1+((f*c)/size)) * sizeof(int));//mandamos	
	resRow = malloc((1+((f*c)/size)) * sizeof(int));//recibimos

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

		
		while(filasEnviadas < f-(f/size)){//el proceso coordinador va enviar un trozo de la matriz a cada uno(excepto a el mismo)

			for(i = 0; i < (f/size); i++){
				for(j = 0; j < c; j++){
					rowA[i*f+j] = matrizA[(filasEnviadas+i)][j];
				}
			}
			rowA[((f*c)/size)] = filasEnviadas;//apuntamos que fila estamos mandando(puede ser mas de una pero estan seguidas)

			receptor++;
			MPI_Isend(&rowA[0], f*c/size+1, MPI_INT, receptor, 2, MPI_COMM_WORLD, &request[filasEnviadas]);//etiqueta 2
			
			MPI_Wait(&request[filasEnviadas], &status[filasEnviadas]);

			filasEnviadas += f/size;

		}

		for(i = 0; i < (f/size); i++){
			for(j = 0; j < c; j++){
				rowA[i*f+j] = matrizA[(filasEnviadas+i)][j];
			}
		}

		rowA[((f*c)/size)] = filasEnviadas;//apuntamos que fila estamos mandando(puede ser mas de una pero estan seguidas)
    } 	
	else{//el coordinador no recibe nada
		MPI_Recv(&rowA[0], c*f/size+1, MPI_INT, 0, 2, MPI_COMM_WORLD, &stats);
	}

	res = MPI_Bcast(&matrizB[0][0], c*f, MPI_INT, 0, MPI_COMM_WORLD);

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
	resRow[((f*c)/size)] = rowA[((f*c)/size)];

	endwtime = MPI_Wtime();

	if(myrank != 0){//solo los trabajadores mandan
		MPI_Send(&resRow[0], c*f/size+1, MPI_INT, 0, 1, MPI_COMM_WORLD);//send
	}
	else{//cordinador recibe y los coloca en matrizC

		int fila = rowA[c*f/size];//la fila que vamos a tratar de la matriz A
		for(i = 0; i < (f/size); i++){
			for(j = 0; j < c; j++){
				matrizC[fila+i][j] = resRow[i*f+j];
			}
		}

		while(filasRecibidas < (f-(f/size))){
			MPI_Irecv(&resRow[0], c*f/size+1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &request[completados]);
			MPI_Wait(&request[completados], &status[completados]);
			int fila = resRow[c*f/size];//la fila que vamos a tratar de la matriz A
			for(i = 0; i < (f/size); i++){
				for(j = 0; j < c; j++){
					matrizC[fila+i][j] = resRow[i*f+j];
				}
			}
			filasRecibidas += f/size;
		}

	}
  

	if(myrank == 0) {	
		if(c <= 8 && f <= 8) print_matriz(matrizC,f,f);

		printf("Tiempo de ejecucion: %f, usando %d+1 maquinas\n", endwtime - startwtime, size - 1);
	}

	if (matrizA!=NULL) free(matrizA);
	if (matrizB!=NULL) free(matrizB);
	if (matrizC!=NULL) free(matrizC);
	if (resRow!=NULL) free(resRow);
	if (rowA!=NULL) free(rowA); 

	MPI_Finalize();	

	return 0;

}
