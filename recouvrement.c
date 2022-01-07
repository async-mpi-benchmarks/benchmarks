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
	

			
	int size_array = 10000000 ; 
	unsigned long long  loop_max_iteration = 200000000 ;
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
		



	
	unsigned long long stop_comm, start_compute, stop_compute, start_total, stop_total; 
	unsigned long long time_compute, time_mpi, time_total ; 
	int flag, old_flag, dflag = 0 ; 
	double recouvrement = 0.0 ; 

	MPI_Barrier(MPI_COMM_WORLD);

	// in a async way
	if (world_rank == 0){
		printf("\nSending array values to processes in a async way ..\n\n") ; 
		// send value to every other processes


		MPI_Request *requests ;
		requests = malloc( (world_size - 3 ) * sizeof(MPI_Request) ) ;  
		for (int i = 0 ; i < world_size - 3 ; i++)
		{
			requests[i] = MPI_REQUEST_NULL ; 
		}


		MPI_Barrier(MPI_COMM_WORLD);
		//start = rdtsc() ; 	
		for (int i = 1 ; i < world_size - 2 ; i++){
			MPI_Isend(array, size_array , MPI_FLOAT , i , i , MPI_COMM_WORLD, &requests[i-1]) ;  
		}

		MPI_Waitall(world_size - 3 , requests , MPI_STATUS_IGNORE) ;  
		//end_wait = rdtsc() ; 
		//total_wait = end_wait - start ; 	
		//printf("%llu time_wait from process 0\n" , total_wait) ;
		//printf("If the two times for a specific process are similars, then the MPI_Test function works fine\n");	
		
		MPI_Barrier(MPI_COMM_WORLD);		
				
	}
	else if (world_rank < world_size - 2){
		MPI_Request request = MPI_REQUEST_NULL ; 
		unsigned long long i = 0 ; 
		double compute = 0.0 ; 
		MPI_Barrier(MPI_COMM_WORLD);
		
	
	
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
		
		/*

		while (i < loop_max_iteration) {
		    stop_comm += rdtsc()*dflag;        // mesure d'horloge si comm pas terminée
		  for (int j = 0; j < loop_iteration; j++) {
		    compute += 0.054398;        // FP operation
		    i += 1;                     // INT operation
		  }
		  old_flag = flag
		  MPI_Test(&request, &flag);     // test non-bloquant de complétion de la transaction MPI
		  dflag = flag - old_flag
		}
		stop_compute = rdtsc();         // mesure d’horloge
		MPI_Wait();                     // barriere sur la reception MPI
		if (flag == 0)
		  stop_comm = rdtsc()           // mesure d’horloge
		stop_total = rdtsc();           // mesure d’horloge	
		*/	
		stop_comm = 0 ; 
		start_total = rdtsc() ; 			
		MPI_Irecv(array , size_array , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD , &request ) ;		
		start_compute = rdtsc() ;		
		while (i < loop_max_iteration){
			stop_comm += rdtsc()*dflag ; 
			for (unsigned long long  j = 0 ; j < loop_iteration ; j++){
				compute += 0.054398 ; // float operation
				i += 1 ; 	      // int operation
			}
			old_flag = flag ; 
			MPI_Test(&request , &flag,  MPI_STATUS_IGNORE) ;
			dflag = flag - old_flag ;   
		}	
		stop_compute = rdtsc() ;
		MPI_Wait(&request , MPI_STATUS_IGNORE) ; 
		if (flag == 0){
			stop_comm = rdtsc();
		}
		stop_total = rdtsc() ;
		
		time_compute = stop_compute - start_compute;
		time_mpi = stop_comm - start_total ; 
		time_total = stop_total - start_total ;
		
		if (time_compute > time_mpi){
			printf("transfert faster for process %d\n", world_rank);			
		}
		
		if (time_compute <= time_mpi){
			printf("computation faster for process %d\n", world_rank);			
		}


		recouvrement = (time_mpi + time_compute - time_total) / (time_mpi) ; 
		
		 		
		printf("   time_compute %llu from process %d\n" , time_compute, world_rank) ;
		printf("   time_mpi     %llu from process %d\n" , time_mpi, world_rank) ;
		printf("   time_total   %llu from process %d\n" , time_total, world_rank) ;		
		printf("overlap %lf  from process %d\n" , recouvrement, world_rank) ;
		printf("       computed values from process %d are %llu and %f\n" , world_rank , i, compute) ;		
		MPI_Barrier(MPI_COMM_WORLD);		
	}	
	else if (world_rank == world_size - 2){
	//  time to compute reference
	unsigned long long i = 0 ; 
	double compute = 0.0 ; 
	MPI_Barrier(MPI_COMM_WORLD);
		
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
	while (i < loop_max_iteration){
		for (unsigned long long  j = 0 ; j < loop_iteration ; j++){
			compute += 0.054398 ; // float operation
			i += 1 ; 	      // int operation
		}
		//printf("%d\n", flag) ;    
	}	
	stop_compute = rdtsc() ;

	time_total = stop_compute - start_compute ; 
	printf("  reference computed values from process %d are %llu and %f\n" , world_rank , i, compute) ;	 		
	printf("    %llu reference time_compute from process %d\n" , time_total, world_rank) ;
	MPI_Barrier(MPI_COMM_WORLD);
	}	
	else if (world_rank == world_size - 1 ){
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);	
		
	}
		
	
	MPI_Finalize();
	
}	




