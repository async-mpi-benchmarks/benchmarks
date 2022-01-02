/*

- this is a simple hello world in MPI
- specify the number of processes with ./ hello_world [number]

*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>


int main(int argc, char** argv){

	int world_rank;
	int world_size;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	
	
	if (world_rank < world_size){
		printf("Hello from process %d\n", world_rank);
	}
	
	
	MPI_Finalize();	

}
