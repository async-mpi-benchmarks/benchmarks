/*

- this is a simple example in MPI
- process 1 send to each processes a scalar value and return it 

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
	
	float value = 0.0 ; 
	float recieved = 0.0 ; 
	
	if (world_rank == 0){
		// send value to every other processes
		// in a sync way
		for (int i = 1 ; i < world_size ; i++){
			value = value + 1.0 ;
			MPI_Send(&value, 1 , MPI_FLOAT , i , i , MPI_COMM_WORLD) ;  
		}
		// recieve values from every other processes
		// in a sync way
		for (int i = 1 ; i < world_size ; i++){
			MPI_Recv(&recieved, 1 , MPI_FLOAT , i ,i , MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; 
			if (recieved == (float)i){
				printf("process %d correctly send back\n", world_rank);
			} 
			else{
				printf("process %d badly send back %f (%f excepted) \n", world_rank, recieved , (float)i);				
			}
		}			
	}
	else if (world_rank < world_size){
	
		MPI_Recv(&value , 1 , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD , MPI_STATUS_IGNORE ) ; 
		MPI_Send(&value , 1 , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD ) ;	
	}	
	
	
	
	
	
	MPI_Finalize();
	
}	

