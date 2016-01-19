#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int* get_vector( int num)
{
	int i;
    int* m;

	m = (int*) malloc (sizeof(int)*(num));
	
	return m;
}

void inicializa_vector(int *m, int num){
	int i = 0;
	for (i=0 ; i<num ; i++) {
		m[i] = rand() % 11; //numeros de 0 a 10
	}  
    
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
	int tamanio = 16;
	double startwtime, endwtime;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	int* vectorA = get_vector(tamanio);
	int* vectorAuxA = get_vector(tamanio/size+size);//de manera exacta es tamanio/size+(size-1) como vector maximo...
	int* vectorB = get_vector(tamanio);
	int* vectorAuxB = get_vector(tamanio/size+size);
	int resultado = 0;
	int final = 0;

	int restantes = (tamanio)%size;//elementos restantes
    int *cantidadMandados;//array de elementos enviados a cada trabajador    
    int *desplazamiento;//desplazamiento entre los envios (total datos / size)
	int sum = 0;

	cantidadMandados = malloc(sizeof(int)*size);
    desplazamiento = malloc(sizeof(int)*size);

	if(myrank == 0){
		
		inicializa_vector(vectorA, tamanio);
		inicializa_vector(vectorB, tamanio);

		if(tamanio <= 16) { //Mostramos hasta vectores de 15 elementos	
			print_vector ( vectorA, tamanio);
			print_vector ( vectorB, tamanio);
		}
	   	startwtime = MPI_Wtime();
	}

	for (i = 0; i < size; i++) {
        cantidadMandados[i] = (tamanio)/size;
		
        if (restantes > 0) {
            cantidadMandados[i]++;
            restantes--;
        }
 
       	desplazamiento[i] = sum;
        sum += cantidadMandados[i];		
    }	

		/*if (0 == myrank) {
        	for (i = 0; i < size; i++) {
            	printf("cantidadMandados[%d] = %d\tdesplazamiento[%d] = %d\n", i, cantidadMandados[i], i, desplazamiento[i]);
        	}
    	}*/

	//Repartimos el vector A	
	MPI_Scatterv(&vectorA[0], cantidadMandados, desplazamiento, MPI_INT, &vectorAuxA[0], tamanio/size+size, MPI_INT, 0, MPI_COMM_WORLD);
	//Repartimos el vector B	
	MPI_Scatterv(&vectorB[0], cantidadMandados, desplazamiento, MPI_INT, &vectorAuxB[0], tamanio/size+size, MPI_INT, 0, MPI_COMM_WORLD);

	//Procesamiento
	for(i = 0; i < cantidadMandados[myrank]; i++)//cada elemento del vector
    {
    	resultado += vectorAuxA[i]*vectorAuxB[i];     
   	} 

	//Reunimos los datos y aplicamos la suma
	MPI_Reduce(&resultado, &final, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	if(myrank == 0){
		printf("\nEl resultado del producto escalar de los vectores es: %d\n\n", final);
		endwtime = MPI_Wtime();	 
		printf("Tiempo de ejecucion: %f, usando %d+1 maquinas\n", endwtime - startwtime, size - 1);	 
	}

	if(vectorA != NULL) free(vectorA);
	if(vectorB != NULL) free(vectorB);
	if(vectorAuxA != NULL) free(vectorAuxA);
	if(vectorAuxB != NULL) free(vectorAuxB);

	MPI_Finalize();	

	return 0;

}
