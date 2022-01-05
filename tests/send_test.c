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
	
	MPI_Status status ;	
	MPI_Request request = MPI_REQUEST_NULL ;
			
	int size_array = 1000000 ; 
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
		



	
	unsigned long long start, end_test, end_wait ; 
	unsigned long long total_wait , total_test ; 	
	int flag  = 0; 
	MPI_Barrier(MPI_COMM_WORLD);
	start = rdtsc() ; 	
	
	// in a async way
	if (world_rank == 0){
		printf("\nSending array values to processes in a async way ..\n\n") ; 
		// send value to every other processes

		for (int i = 1 ; i < world_size ; i++){
			MPI_Isend(array, size_array , MPI_FLOAT , i , i , MPI_COMM_WORLD, &request) ;  
		}

		while (flag == 0){
			MPI_Test(&request , &flag,  &status) ;   
		}
		end_test = rdtsc() ; 

		MPI_Wait(&request , &status) ;   

		end_wait = rdtsc() ; 
		total_test = end_test - start ;		
		total_wait = end_wait - start ; 	
		printf("%llu time test 0\n" , total_test) ;
		printf("%llu time_wait 0\n" , total_wait) ;
		printf("If the two previous times are similars, then the MPI_Test function works fine\n");			
	}
	else if (world_rank < world_size){
 	
		MPI_Irecv(array , size_array , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD , &request ) ;
		while (flag == 0){
			MPI_Test(&request , &flag,  &status) ;   
		}	
		end_test = rdtsc() ; 	
		MPI_Wait(&request , &status) ; 
		end_wait = rdtsc() ; 	
		
		total_test = end_test - start ;		
		total_wait = end_wait - start ; 	
		printf("%llu time test 1\n" , total_test) ;
		printf("%llu time_wait 1\n" , total_wait) ;
		
		
	}		
		
	
	MPI_Finalize();
	
}	




