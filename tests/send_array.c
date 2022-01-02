/*

- this is a simple example in MPI
- process 1 send to each processes a vecto and return it 

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
		
	int size_array = 1 ; 
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
	
	// in a sync way	
	if (world_rank == 0){
		printf("\nSending array values to processes in a sync way ..\n\n") ; 
		
		
	
		// send value to every other processes
		// in a sync way
		for (int i = 1 ; i < world_size ; i++){
			MPI_Send(array, size_array, MPI_FLOAT , i , i , MPI_COMM_WORLD) ;  
		}
		// recieve values from every other processes
		// in a sync way
		for (int i = 1 ; i < world_size ; i++){
			MPI_Recv(array, size_array , MPI_FLOAT , i ,i , MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; 
			int is_ok = 1 ; 
			for (int j = 0 ; j < size_array ; j++){
				if (array[j] != (float)j){
					is_ok = 0 ; 
				}
			}
			if (is_ok == 1){
				printf("process %d correctly send back data\n", i);
			} 
			else{
				printf("process %d badly send back data \n", i);				
			}
		}			
	}
	else if (world_rank < world_size){
	
		MPI_Recv(array , size_array , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD , MPI_STATUS_IGNORE ) ; 
		MPI_Send(array , size_array , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD ) ;	

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
	
	// in a async way
	if (world_rank == 0){
		printf("\nSending array values to processes in a async way ..\n\n") ; 
		// send value to every other processes
		for (int i = 1 ; i < world_size ; i++){
			MPI_Isend(array, size_array , MPI_FLOAT , i , i , MPI_COMM_WORLD, &request) ;  
		}
		for (int i = 1 ; i < world_size ; i++){
			MPI_Wait(&request , &status) ;   
		}		

		// recieve values from every other processes
		for (int i = 1 ; i < world_size ; i++){
			MPI_Recv(array, size_array , MPI_FLOAT , i ,i , MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; 
			int is_ok = 1 ; 
			for (int j = 0 ; j < size_array ; j++){
				if (array[j] != (float)j){
					is_ok = 0 ; 
				}
			}
			if (is_ok == 1){
				printf("process %d correctly send back data\n", i);
			} 
			else{
				printf("process %d badly send back data \n", i);				
			}
		}			
	}
	else if (world_rank < world_size){
	
		MPI_Irecv(array , size_array , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD , &request ) ; 
		MPI_Wait(&request , &status) ; 
		MPI_Isend(array , size_array , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD, &request ) ;	
	}		
	
	
	
	MPI_Finalize();
	
}	

