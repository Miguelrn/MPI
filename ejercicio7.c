#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int* get_vector( int num, int ini, int esAux, int size )
{
	int i;
    int* m;
	
	if(esAux)
		m = (int*) malloc (sizeof(int)*(num/size));
	else
		m = (int*) malloc (sizeof(int)*num);

	if(ini == 1){
		for (i=0 ; i<num ; i++) {
			m[i] = rand() % 11; //numeros de 0 a 10
		}  
    }
        
	return m;
}

void print_vector ( int* m, int num )
{
	int i;
    printf("---------------------------------------------\n");
    for (i=0 ; i<num ; i++)
    {
        printf("%d ",m[i]);
	}            
    printf("\n---------------------------------------------\n");        
}


int main(int argc, char *argv[]){
	
	srand (time(NULL));
	int i, j, z;
	int myrank, size;
	int tamanio = 4000;
	double startwtime, endwtime;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	int* vectorA = get_vector(tamanio,0,0,size);
	int* vectorAuxA = get_vector(tamanio,0,1,size);//resA
	int* vectorB = get_vector(tamanio,0,0,size);
	int* vectorAuxB = get_vector(tamanio,0,1,size);
	int resultado = 0;
	int final = 0;

	if(myrank == 0){
		
		vectorA = get_vector(tamanio,1,0,size);
		vectorB = get_vector(tamanio,1,0,size);

		if(tamanio <= 15) { //Mostramos hasta vectores de 15 elementos	
			print_vector ( vectorA, tamanio);
			print_vector ( vectorB, tamanio);
		}

		if((tamanio%size) != 0 ){ 
			printf("Necesita tener un numero correcto de procesos\n"); return 0;
		}
	}
	startwtime = MPI_Wtime();

	//Repartimos el vector A	
	MPI_Scatter(&vectorA[0], tamanio/size, MPI_INT, &vectorAuxA[0], tamanio/size, MPI_INT, 0, MPI_COMM_WORLD);
	//Repartimos el vector B	
	MPI_Scatter(&vectorB[0], tamanio/size, MPI_INT, &vectorAuxB[0], tamanio/size, MPI_INT, 0, MPI_COMM_WORLD);

	//Procesamiento
	for(i = 0; i < tamanio/size; i++)//cada elemento del vector
    {
    	resultado += vectorAuxA[i]*vectorAuxB[i];     
   	} 
	
	endwtime = MPI_Wtime();

	//Reunimos los datos y aplicamos la suma
	MPI_Reduce(&resultado, &final, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	if(myrank == 0){
		printf("\nEl resultado del producto escalar de los vectores es: %d\n\n", final);	
		printf("Tiempo de ejecucion: %f, usando %d+1 maquinas\n", endwtime - startwtime, size - 1);		  
	}

	

	MPI_Finalize();	

	return 0;

}
