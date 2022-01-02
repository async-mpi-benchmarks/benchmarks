/*

- this is a simple example in MPI
- process 0 send to each processes a vector via broadcast

*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include  <unistd.h>



int main(int argc, char** argv){

	int world_rank;
	int world_size;
	int world_res ; 
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
		
	int size_array = 10000 ; 
	if (argc==2){
		char *tmpstring ; 
		size_array = strtoul(argv[1], &tmpstring, 10);
	}
	
	float * array = malloc(size_array * sizeof(float)) ; 
	
	
	// initialization of array
	if (world_rank == 0){
		for (int i = 0 ; i < size_array ; i++){
			array[i] = (float)i ;
		}
	}
	else{
		for (int i = 0 ; i < size_array ; i++){
			array[i] = 0.0 ;
		}
	}
	
	if (world_rank == 0){
		printf("\nBroadcast data in a sync way ..\n\n") ; 
	}	
	world_res = MPI_Bcast(array, size_array , MPI_FLOAT , 0 , MPI_COMM_WORLD) ; 
	
	if (world_rank < world_size){
		int is_ok = 1 ; 	
		for (int i = 0 ; i < size_array ; i++){
			if (array[i] != (float)i){
				is_ok = 0 ; 
			}
		}
		sleep(1);
		if (is_ok == 1){
			printf(" sync process %d correctly recieved \n", world_rank);
		} 
		else{
			printf(" sync process %d badly recieved \n", world_rank);		
		}				
	}
 
	
	
	MPI_Status status ;	
	MPI_Request request = MPI_REQUEST_NULL ;	
	
	// initialization of array
	if (world_rank == 0){
		for (int i = 0 ; i < size_array ; i++){
			array[i] = (float)i ;
		}
	}
	else{
		for (int i = 0 ; i < size_array ; i++){
			array[i] = 0.0 ;
		}
	}
	
	
	if (world_rank == 0){
		printf("\nBroadcast data in an async way ..\n\n") ; 
	}		
	world_res = MPI_Ibcast(array, size_array , MPI_FLOAT , 0 , MPI_COMM_WORLD, &request) ; 
	MPI_Wait(&request , &status) ;   
	
	if (world_rank < world_size){
		int is_ok = 1 ; 	
		for (int i = 0 ; i < size_array ; i++){
			if (array[i] != (float)i){
				is_ok = 0 ; 
			}
		}
		sleep(1);
		if (is_ok == 1){
			printf("async process %d correctly recieved \n", world_rank);
		} 
		else{
			printf("async process %d badly recieved \n", world_rank);		
		}				
	}	
	
	
	MPI_Finalize();
	
}	

