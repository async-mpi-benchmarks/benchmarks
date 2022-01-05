// Objective : quantify time and evaluate covering effect on timings
// between sync and async paradigm through MPI for 2 processes exchange

/*
Protocol : 
	- send array from rank 0 to to each working process and recieve them back

	- This is the sync and async example
*/

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



int main(int argc, char** argv)
{

	MPI_Init(&argc,&argv);
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	
	
	unsigned long long size_array , i_max, repetition, burn_repetition ;

	if (argc != 4){
		// defualt size
		size_array = 100000;
		repetition = 1000 ;
		burn_repetition = 1000 ; 	
	}	
	else{
		char *tmpstring ; 	
		size_array = strtoul(argv[1], &tmpstring, 10) ;//100000
		burn_repetition = strtoul(argv[2], &tmpstring, 10) ;	 //200000	
		repetition = strtoul(argv[3], &tmpstring, 10) ;	 //200000
	}
	
	if (world_rank == 0){
		printf("\n") ; 
		if (argc != 4){
			printf("Please use arguments : [size of array] [burn_repetition] [repetition]\n");		
		}
		
		printf("size of array : %llu \n", size_array) ;
		printf("burn repetition of sending process: %llu \n", repetition) ;
		printf("repetition of sending process: %llu \n", repetition) ;

			
		
		if (size_array < world_size){
			printf("Please use size of array >= %d\n", world_size);
		}		
			
	}	

	if (size_array < world_size){
		// avoid non authorized memory access 
		size_array = world_size + 1 ;
	}

	// allocate array for each process
	double * array = malloc( size_array * sizeof(double)) ; 


	if (world_rank == 0) {
		printf("The number of processes is %d\n\n" , world_size) ;
		
		MPI_Request requests[4 * world_size] ;
		//MPI_Request *requests = malloc( 4 * world_size * sizeof(MPI_Request)) ; 		
		//MPI_Status *statuss = malloc( 4 * world_size * sizeof(MPI_Status)) ; 		
		
		////////////////////////////
		///////////////// sync part 
		////////////////////////////
		// init array values
		for (unsigned long long i = 0 ; i < size_array ; i++){
			array[i] = 2.124844854 ; 
		}
		
		// timer
		unsigned long long start_send, end_send , start_recv, end_recv;
		unsigned long long sync_time_send, sync_time_recv , async_time_send, async_time_recv, full_async_time_send, full_async_time_recv;
		
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Send(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD);
			}
		}	
		start_send = rdtsc();			
		for (unsigned long long i = 0 ; i < repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Send(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD);
			}
		}			
		end_send = rdtsc();
		sync_time_send = (end_send - start_send)/(repetition) ;		
		printf("%llu spend send array\n" , sync_time_send) ;	
		
		// time spend in Send execution  			
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Recv(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}
		start_recv = rdtsc() ; 	
		for (unsigned long long i = 0 ; i < repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Recv(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}	
		end_recv = rdtsc() ; 
		sync_time_recv = (end_recv - start_recv)/(repetition) ; 			
		printf("%llu spend recv array\n" , sync_time_recv) ;
	
		sleep(0.1);
		///////////////////////////
		/////////////// async part with direct wait
		///////////////////////////
				
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Isend(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD, &requests[j]);
				MPI_Wait(&requests[j] , MPI_STATUS_IGNORE); 
			}
		}	
		start_send = rdtsc();			
		for (unsigned long long i = 0 ; i < repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Isend(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD, &requests[j + world_size]);
				MPI_Wait(&requests[j +  world_size] , MPI_STATUS_IGNORE); 
			}
		}			
		end_send = rdtsc();
		async_time_send = (end_send - start_send)/(repetition) ;		
		printf("%llu spend send array\n" , async_time_send) ;	
		
		// time spend in Send execution  			
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Irecv(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD,  &requests[j + 2 * world_size]);
				MPI_Wait(&requests[j + 2 * world_size] , MPI_STATUS_IGNORE); 
			}
		}
		start_recv = rdtsc() ; 	
		for (unsigned long long i = 0 ; i < repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Irecv(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD,  &requests[j + 3 * world_size]);
				MPI_Wait(&requests[j + 3 * world_size] , MPI_STATUS_IGNORE); 
			}
		}	
		end_recv = rdtsc() ; 
		async_time_recv = (end_recv - start_recv)/(repetition) ; 			
		printf("%llu spend recv array\n" , async_time_recv) ;	
		

		sleep(0.1);
		///////////////////////////
		/////////////// async part
		///////////////////////////
				
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Isend(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD, &requests[j]);
			}
			for (int j = 1 ; j < world_size ; j++){
				MPI_Wait(&requests[j] , MPI_STATUS_IGNORE); 
			}
		}	
		start_send = rdtsc();			
		for (unsigned long long i = 0 ; i < repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Isend(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD, &requests[j + world_size]);
			}
			for (int j = 1 ; j < world_size ; j++){
				MPI_Wait(&requests[j +  world_size] , MPI_STATUS_IGNORE); 
			}
			
		}			
		end_send = rdtsc();
		full_async_time_send = (end_send - start_send)/(repetition) ;		
		printf("%llu spend send array\n" , full_async_time_send) ;	
		
		// time spend in Send execution  			
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Irecv(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD,  &requests[j + 2 * world_size]);
			}
			for (int j = 1 ; j < world_size ; j++){
				MPI_Wait(&requests[j + 2 * world_size] , MPI_STATUS_IGNORE); 
			}
		}
		start_recv = rdtsc() ; 	
		for (unsigned long long i = 0 ; i < repetition ; i++){	
			for (int j = 1 ; j < world_size ; j++){
				MPI_Irecv(array, size_array, MPI_DOUBLE, j, j + i*world_size, MPI_COMM_WORLD,  &requests[j + 3 * world_size]);
			}
			for (int j = 1 ; j < world_size ; j++){
				MPI_Wait(&requests[j + 3 * world_size] , MPI_STATUS_IGNORE); 
			}
		}	
		end_recv = rdtsc() ; 
		full_async_time_recv = (end_recv - start_recv)/(repetition) ; 			
		printf("%llu spend recv array\n" , full_async_time_recv) ;	
	
	
		// write at the end of a save file
		FILE * SaveFile;
		SaveFile = fopen("time_pap.txt", "a") ; 
		if(SaveFile==NULL) {
		    perror("Error opening file");
		} 
		   
		fprintf(SaveFile , "%llu %u %llu %llu %llu %llu %llu %llu %llu %llu\n", size_array , world_size , burn_repetition , repetition ,  sync_time_send,  sync_time_recv, async_time_send, async_time_recv, full_async_time_send, full_async_time_recv )			;
		fclose(SaveFile) ;		     	
	} else if (world_rank < world_size ) {
		////////////////////////
		////////////// sync part
		////////////////////////
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
				MPI_Recv(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);				
		}
		for (unsigned long long i = 0 ; i < repetition ; i++){	
				MPI_Recv(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);				
		}
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
				MPI_Send(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD);				
		} 	
		for (unsigned long long i = 0 ; i < repetition ; i++){	
				MPI_Send(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD);				
		}
		////////////////////////
		//////////// async part with direct wait
		////////////////////////
		
		MPI_Request request0 , request1, request2, request3 ; 
		//MPI_Request statuss0 , statuss1, statuss2, statuss3 ; 

		
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
				MPI_Irecv(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, &request0);
				MPI_Wait(&request0 , MPI_STATUS_IGNORE); 				
		}
		for (unsigned long long i = 0 ; i < repetition ; i++){	
				MPI_Irecv(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, &request1);	
				MPI_Wait(&request1 , MPI_STATUS_IGNORE); 		
		}
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
				MPI_Isend(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, &request2);
				MPI_Wait(&request2 , MPI_STATUS_IGNORE); 			
		} 	
		for (unsigned long long i = 0 ; i < repetition ; i++){	
				MPI_Isend(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, &request3);
				MPI_Wait(&request3 , MPI_STATUS_IGNORE); 			
		}
	
		////////////////////////
		//////////// async part
		////////////////////////

		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
				MPI_Irecv(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, &request0);
				MPI_Wait(&request0 , MPI_STATUS_IGNORE); 				
		}
		
		for (unsigned long long i = 0 ; i < repetition ; i++){	
				MPI_Irecv(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, &request1);	
				MPI_Wait(&request1 , MPI_STATUS_IGNORE); 		
		}
		for (unsigned long long i = 0 ; i < burn_repetition ; i++){	
				MPI_Isend(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, &request2);
				MPI_Wait(&request2 , MPI_STATUS_IGNORE); 			
		} 	
		for (unsigned long long i = 0 ; i < repetition ; i++){	
				MPI_Isend(array, size_array, MPI_DOUBLE, 0, world_rank + i*world_size, MPI_COMM_WORLD, &request3);
				MPI_Wait(&request3 , MPI_STATUS_IGNORE); 			
		}	
	
		
	}	
	MPI_Finalize();
	return 0 ; 
}



