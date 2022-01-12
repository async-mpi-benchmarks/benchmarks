#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <omp.h>


int main(int argc, char** argv){

	int world_rank;
	int world_size;

	int namelen ; 
	int iam = 0 ; 
	int np = 1 ; 
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
		
	char processor_name[MPI_MAX_PROCESSOR_NAME];		
		
	MPI_Get_processor_name(processor_name, &namelen);	
		
	np = omp_get_num_threads();
	iam = omp_get_thread_num();
	printf("Hello from thread %d out of %d from process %d out of %d on %s\n",
	iam, np, world_rank, world_size, processor_name);

	
	
	float value = 0.0 ; 
	float recieved = 0.0 ; 


	// in a sync way	
	if (world_rank == 0){
		printf("\nSending scalar values to processes in a sync way ..\n\n") ; 
	
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
				printf("process %d correctly send back\n", i);
			} 
			else{
				printf("process %d badly send back %f (%f excepted) \n", i, recieved , (float)i);				
			}
		}			
	}
	else if (world_rank < world_size){
	
		MPI_Recv(&value , 1 , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD , MPI_STATUS_IGNORE ) ; 
		MPI_Send(&value , 1 , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD ) ;	
	}	

	value = 0.0 ; 
	recieved = 0.0 ; 
	
	MPI_Status status ;	
	MPI_Request request = MPI_REQUEST_NULL ;
	
	// in a async way
	if (world_rank == 0){
		printf("\nSending scalar values to processes in a async way ..\n\n") ; 
		// send value to every other processes
		for (int i = 1 ; i < world_size ; i++){
			value = value + 1.0 ;
			MPI_Isend(&value, 1 , MPI_FLOAT , i , i , MPI_COMM_WORLD, &request) ;  
		}
		
		// recieve values from every other processes
		for (int i = 1 ; i < world_size ; i++){
			MPI_Irecv(&recieved, 1 , MPI_FLOAT , i ,i , MPI_COMM_WORLD, &request) ; 
			MPI_Wait(&request , &status) ; 
			if (recieved == (float)i){
				printf("process %d correctly send back\n", i);
			} 
			else{
				printf("process %d badly send back %f (%f excepted) \n", i, recieved , (float)i);				
			}
		}			
	}
	else if (world_rank < world_size){
	
		MPI_Irecv(&value , 1 , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD , &request ) ; 
		int inc = 0 ;
		#pragma omp parallel for
		for (int i = 0 ; i < 1000 ; i++){
			inc += 1 ; 
		}
		MPI_Wait(&request , &status) ; 
		MPI_Isend(&value , 1 , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD, &request ) ;	
	}		
	
	
	
	MPI_Finalize();
	
}	
