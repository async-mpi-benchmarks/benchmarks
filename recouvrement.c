///////////////////////////////////////////////////////////////////////////
// this is a code to test the MPI_Test call. 
// Syntax : int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status)
// flag is True if the operation identified by the request is complete
// protocol : 
//	- async array Isend from process 0 to 1
//	- async Irecv in process 1
//	- compute loop in process 1
//		- expensive arithmetic operations
//		- MPI_Test call to test  during computation AND during recv
//		- save to what tick when flag is set to True
//	- barrier MPI_Wait to enforce terminaison in this point 

// objective : - Is it possible to determine covering in async MPI with MPI_Test ?
//	       - What is the cost of an MPI_Test call ?
//////////////////////////////////////////////////////////////////////////////


#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>


inline
unsigned long long rdtsc(void)
{
  unsigned long long a, d;
 
  __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
  
  return (d << 32) | a;
}


int main(int argc, char** argv){

	int world_rank;
	int world_size;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	

			
	int size_array = 100000000 ; 
	unsigned long long  loop_max_iteration = 100 ;
	unsigned long long  loop_iteration = loop_max_iteration/10 ; 
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
		



	
	unsigned long long start_mpi, start_compute, end_test, end_wait, end_last_loop; 
	unsigned long long time_compute , time_mpi ; 
	int flag = 0 ; 

	MPI_Barrier(MPI_COMM_WORLD);

	// in a async way
	if (world_rank == 0){
		printf("\nSending array values to processes in a async way ..\n\n") ; 
		// send value to every other processes


		MPI_Request *requests ;
		requests = malloc( (world_size - 1 ) * sizeof(MPI_Request) ) ;  
		for (int i = 0 ; i < world_size - 1 ; i++)
		{
			requests[i] = MPI_REQUEST_NULL ; 
		}


		MPI_Barrier(MPI_COMM_WORLD);
		//start = rdtsc() ; 	
		for (int i = 1 ; i < world_size ; i++){
			MPI_Isend(array, size_array , MPI_FLOAT , i , i , MPI_COMM_WORLD, &requests[i-1]) ;  
		}

		MPI_Waitall(world_size - 1 , requests , MPI_STATUS_IGNORE) ;  
		//end_wait = rdtsc() ; 
		//total_wait = end_wait - start ; 	
		//printf("%llu time_wait from process 0\n" , total_wait) ;
		//printf("If the two times for a specific process are similars, then the MPI_Test function works fine\n");	
		
		MPI_Barrier(MPI_COMM_WORLD);		
				
	}
	else if (world_rank < world_size){
		MPI_Request request = MPI_REQUEST_NULL ; 
		unsigned long long i = 0 ; 
		unsigned long long i2 = 0 ; 
		double compute = 0.0 ; 
		MPI_Barrier(MPI_COMM_WORLD);
		
		start_mpi = rdtsc() ; 			
		MPI_Irecv(array , size_array , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD , &request ) ;	
	
		/////////////////////////////////////////////////////////////////
		// compute & tests many times. 
		// exit : 
		//	- if loop made more than loop_max_iteration additions. 
		//		thus, end_test is the computation time
		//	or
		//	- if the MPI transaction is finished.
		//		thus, end_test is the MPI transaction time, and
		//		computation have to continue in a normal time
		/////////////////////////////////////////////////////////////////
		start_compute = rdtsc() ;		
		while ( (flag == 0) && (i < loop_max_iteration)  ){
			for (unsigned long long  j = 0 ; j < loop_iteration ; j++){
				compute += 0.054398 ; // float operation
				i += 1 ; 	      // int operation
			}
			MPI_Test(&request , &flag,  MPI_STATUS_IGNORE) ;
			//printf("%d\n", flag) ;    
		}	
		end_test = rdtsc() ;
		MPI_Wait(&request , MPI_STATUS_IGNORE) ; 
		end_wait = rdtsc() ; 
		i2 = i ; 	
		while (i < loop_max_iteration){
			for (int j = 0 ; j < loop_iteration ; j++){
				compute += 0.054398 ; // float operation
				i += 1 ; 	// int operation
			}  
		}			
		
		end_last_loop = rdtsc() ;		
		
		if (i2 >= loop_max_iteration){
			// first loop ended due to loop_max_iteration
			time_compute = end_test - start_compute ; 
			time_mpi = end_wait - start_mpi ;
			printf("computation faster for process %d\n", world_rank);
		}
		else{
			// first loop ended du to finished MPI transaction
			time_compute = end_last_loop - start_compute ; 
			time_mpi = end_wait - start_mpi ; 
			printf("MPI transaction faster for process %d\n", world_rank);
		}
		printf("  computed values from process %d are %llu and %f\n" , world_rank , i, compute) ;		 		
		printf("    %llu time_compute from process %d\n" , time_compute, world_rank) ;
		printf("    %llu time_mpi     from process %d\n" , time_mpi, world_rank) ;
		
		MPI_Barrier(MPI_COMM_WORLD);		
	}		
		
	
	MPI_Finalize();
	
}	




