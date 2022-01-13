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
#include <omp.h>
#include <pthread.h>

inline
unsigned long long rdtsc(void)
{
    unsigned long long tsc;
    asm volatile(
        // `LFENCE`: Serializes all load (read) operations that ocurred prior
        // to the `LFENCE` instruction in the program instruction stream, but
        // does not affect store operations.
        // If software requires `RDTSCP` to be executed prior to execution of
        // any subsequent instruction (including any memory accesses), it can
        // execute `LFENCE` immediately after `RDTSCP`.
        //
        // Referenced from:
        // Intel 64 and IA-32 Architectures software developer's manual
        // Volume 3, section 8.2.5
        "rdtscp                  \n\t"
        "lfence                  \n\t"
        "shl     $0x20, %%rdx    \n\t"
        "or      %%rdx, %%rax    \n\t"
        : "=a" (tsc)
        :
        : "rdx", "rcx");
    return tsc;
}


struct arg_compute{
	float *array_compute;
	int position;
	unsigned long long M;
};


struct arg_recv{
	float *array;
	unsigned long long size_array ;
};

void *compute_thread(void* arg){
	for (unsigned long long  j = 0 ; j < ((struct arg_compute*)arg)->M ; j++){
		((struct arg_compute*)arg)->array_compute[((struct arg_compute*)arg)->position] += 1.0;
	}	
}


void *recv_thread(void* arg){
		MPI_Request request = MPI_REQUEST_NULL ; 
		MPI_Irecv(((struct arg_recv*)arg)->array , ((struct arg_recv*)arg)->size_array , MPI_FLOAT , 0 , 1 , MPI_COMM_WORLD , &request ) ;	
		MPI_Wait(&request , MPI_STATUS_IGNORE) ; 	
}

int main(int argc, char** argv){

	int world_rank;
	int world_size;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	

			
	unsigned long long size_array = 2000000000 ; 
	unsigned long long size_array_compute = 128 ;
	unsigned long long  M = 1000000000;
	//unsigned long long  loop_iteration = loop_max_iteration/200 ; 
	float * array = malloc(size_array * sizeof(float)) ; 
	float * array_compute = malloc(size_array_compute * sizeof(float)) ; 	
	
	// initialization of array
	if (world_rank == 0){
		for (unsigned long long i = 0 ; i < size_array ; i++){
			array[i] = (float)i ;
		}
	}
	else{
		for (unsigned long long i = 0 ; i < size_array ; i++){
			array[i] = 0.0 ;
		}
	}
	
	
	if (world_rank == 0){
		for (unsigned long long i = 0 ; i < size_array_compute ; i++){
			array_compute[i] = 0.0;
		}
	}
	else{
		for (unsigned long long i = 0 ; i < size_array_compute ; i++){
			array_compute[i] = 0.0 ;
		}
	}
	
	// pthread
	int id [ 4 ];	
	pthread_t thread1, thread2 ; 
	int thr, thr2 ; 
	//pthread_create(&thread1, NULL, *thread, (void*) thr) ; 
	//pthread_create(&thread2, NULL, *thread, (void*) thr2) ; 
	//pthread_join(thread1, NULL);
	//pthread_join(thread2, NULL);		



	
	unsigned long long stop_comm, start_compute, stop_compute, start_total, stop_total; 
	unsigned long long start_mpi, end_test, end_wait, end_last_loop;
	unsigned long long time_compute, time_mpi, time_total ; 
	int flag, old_flag, dflag = 0 ; 
	double recouvrement = 0.0 ; 

	MPI_Barrier(MPI_COMM_WORLD);

	// in a async way
	if (world_rank == 0){
		printf("\nSending array values to processes in a async way ..\n\n") ; 
		// send value to every other processes


		MPI_Request *requests ;
		requests = malloc( (world_size - 1 ) * sizeof(MPI_Request) ) ;  
		MPI_Request *requests2 ;
		requests2 = malloc( (world_size - 1 ) * sizeof(MPI_Request) ) ;		
		for (int i = 0 ; i < world_size - 1 ; i++)
		{
			requests[i] = MPI_REQUEST_NULL ; 
			requests2[i] = MPI_REQUEST_NULL ; 
		}

		// envoi 0 to n
		MPI_Barrier(MPI_COMM_WORLD);
		for (int i = 1 ; i < world_size ; i++){
			MPI_Isend(array, size_array , MPI_FLOAT , i , i , MPI_COMM_WORLD, &requests[i-1]) ;  
		}
		MPI_Waitall(world_size - 1 , requests , MPI_STATUS_IGNORE) ;  
		//end_wait = rdtsc() ; 
		//total_wait = end_wait - start ; 	
		//printf("%llu time_wait from process 0\n" , total_wait) ;
		//printf("If the two times for a specific process are similars, then the MPI_Test function works fine\n");	
		
		
		for (int i = 0 ; i < world_size - 1 ; i++)
		{
			requests[i] = MPI_REQUEST_NULL ; 
		}		
		
		// envoi 0 to n for ref
		MPI_Barrier(MPI_COMM_WORLD);		
		start_mpi = rdtsc() ; 
		#pragma omp parallel
		{	
			if (omp_get_thread_num()==1){					
				for (int i = 1 ; i < world_size; i++){
					MPI_Isend(array, size_array , MPI_FLOAT , i , i , MPI_COMM_WORLD, &requests2[i-1]) ;  
				}
			}
		}		
		MPI_Waitall(world_size - 1 , requests2 , MPI_STATUS_IGNORE) ; 
		stop_comm = rdtsc() ; 
		time_mpi = stop_comm - start_mpi ; 		
		
				
		// calcul for ref
		MPI_Barrier(MPI_COMM_WORLD);		
		
		unsigned long long i = 0 ;
		
		struct arg_compute *argc = (struct arg_compute *)malloc(sizeof(struct arg_compute)) ; 		
		start_compute = rdtsc() ;
		

		argc->position = 0 ; 
		argc->M = M;
		argc->array_compute = array_compute ; 				
						
		pthread_create(&thread2, NULL, &compute_thread, (void*) argc);
		pthread_join(thread2, NULL);
		
		stop_compute = rdtsc() ;	
		time_compute = stop_compute - start_compute ; 
			//printf("  reference computed values from process %d are %llu and %f\n" , world_rank , i, compute) ;	 		
		printf("%llu reference time_compute\n" , time_compute) ;	
		
				
	}
	else if (world_rank < world_size ){
		MPI_Request request = MPI_REQUEST_NULL ; 
		unsigned long long i = 0 ; 
		unsigned long long i2 = 0 ; 
		double compute = 0.0 ; ;

				
		// envoi 0 to n
		MPI_Barrier(MPI_COMM_WORLD);
	
	


 			
		struct arg_recv *argr = (struct arg_recv *)malloc(sizeof(struct arg_recv)) ; 
		argr->size_array = size_array;
		argr->array = array ; 				
						
		pthread_create(&thread1, NULL, &recv_thread, (void*) argr);
		

		
	
		
		//compute_thread(array_compute, 0, M);
		struct arg_compute *argc = (struct arg_compute *)malloc(sizeof(struct arg_compute)) ; 
		argc->position = 0 ; 
		argc->M = M;
		argc->array_compute = array_compute ; 				
						
		pthread_create(&thread2, NULL, &compute_thread, (void*) argc);
		
		start_total = rdtsc();		
		
		start_mpi = rdtsc() ;		
		pthread_join(thread1, NULL);
		end_wait = rdtsc() ; 
				
		start_compute = rdtsc() ;		
		pthread_join(thread2, NULL);
		stop_compute = rdtsc() ;    

		stop_total = rdtsc();	
	
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
		time_compute = stop_compute - start_total ; 	
		time_mpi = end_wait - start_total ;

		time_total = stop_total - start_total;

		recouvrement = ((double)time_mpi + (double)time_compute - (double)time_total) / ((double)time_mpi) ; 
		
		 		
		printf("   time_compute %llu from process %d\n" , time_compute, world_rank) ;
		printf("   time_mpi     %llu from process %d\n" , time_mpi, world_rank) ;
		printf("   time_total   %llu from process %d\n" , time_total, world_rank) ;		
		printf("overlap %lf  from process %d\n" , recouvrement, world_rank) ;
		printf("       computed values from process %d are %llu and %f\n" , world_rank , i, compute) ;	
		
		
		// envoi 0 to n for ref	
		MPI_Barrier(MPI_COMM_WORLD);		
		start_mpi = rdtsc() ; 		
		MPI_Request request_reference  = MPI_REQUEST_NULL ; 		
		MPI_Irecv(array , size_array , MPI_FLOAT , 0 , world_rank , MPI_COMM_WORLD , &request_reference ) ;	
		MPI_Wait(&request_reference, MPI_STATUS_IGNORE);
		stop_comm = rdtsc() ; 
		time_mpi = stop_comm - start_mpi ; 
		if (world_rank == 1){		
			printf("%llu reference time_MPI\n" , time_mpi) ;			
		}
		
		// calcul for ref
		MPI_Barrier(MPI_COMM_WORLD);	
		
	}	

		
	
	MPI_Finalize();
	
}	




