/*

- this example shows differnts methods to measure time or tics
- time is measured on all processes

*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>


typedef unsigned long long uint64 ; 


// rdtscp are from : 
// https://gist.github.com/savanovich/f07eda9dba9300eb9ccf
// rdtscp_before
inline
uint64 sync_rdtscp_b(void)
{
	uint64 a, d; 
	asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
	asm volatile("rdtsc" : "=a" (a), "=d" (d)); 
	return ((uint64)a) | (((uint64)d) << 32); 
}

// rdtscp after
inline
uint64 sync_rdtscp_a(void)
{
	uint64 a, d; 
	asm volatile("rdtscp" : "=a" (a), "=d" (d)); 
	asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
	return ((uint64)a) | (((uint64)d) << 32); 
}


inline
uint64 rdtsc(void)
{
  uint64 a, d;
 
  __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
  
  return (d << 32) | a;
}



void measure_time(uint64 nn , uint64 *time_rdtsc, uint64 *time_sync_rdtscp){

		double inc = 0.0 ;		
		uint64 start ; 
		uint64 end ; 
		for (uint64 i = 0 ; i < 1000000 ; i++){
			inc = inc + 1.; 
		}
		start = rdtsc() ; 
		for (uint64 i = 0 ; i < nn ; i++){
			inc = inc + 1.; 
		}
		end = rdtsc() ; 
		(*time_rdtsc) = end - start ; 		
		printf("     rdtsc timer : %llu %f\n", (*time_rdtsc), inc);; 			
		start = sync_rdtscp_b() ; 		
		for (uint64 i = 0 ; i < nn ; i++){
			inc = inc + 1.; 
		}
		end = sync_rdtscp_a() ; 
		(*time_sync_rdtscp) = end - start ;
		for (uint64 i = 0 ; i < 1000000 ; i++){
			inc = inc + 1.; 
		}		
		printf("     rdtsc timer : %llu %f\n", (*time_rdtsc), inc);		
		printf("sync rdtsc timer : %llu %f\n", (*time_sync_rdtscp), inc);		

}

void treatment(uint64 *rdtsc_mean, uint64 *rdtsc_min, uint64 *rdtsc_max, uint64 *tmp, int world_size){
	(*rdtsc_mean) += (*tmp)/(world_size - 1) ; 
	if ((*tmp) < (*rdtsc_min)){
		(*rdtsc_min) = (*tmp) ; 
	}
	if ((*tmp) > (*rdtsc_max)){
		(*rdtsc_max) = (*tmp) ; 
	}

}


int main(int argc, char** argv){

	int world_rank;
	int world_size;
	int world_res ; 
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	
	double inc = 0.0 ;
	uint64 max_iter = 600; 
	uint64 nn ;  	

	//if (argc==2){
	//	char *tmpstring ; 
	//	nn = strtoul(argv[1], &tmpstring, 10);
	//}	
	
	uint64 start ; 
	uint64 end ; 
	
	
	uint64 time_rdtsc ; 
	uint64 time_sync_rdtscp ; 

	

	
	if (world_rank !=0 && world_rank < world_size)
	{	
		for (uint64 i = 1 ; i < max_iter ; i++){
			nn = i ; 
			measure_time(nn, &time_rdtsc , &time_sync_rdtscp)  ;
			MPI_Send(&time_rdtsc, 1, MPI_UNSIGNED_LONG_LONG , 0 ,  2*i , MPI_COMM_WORLD) ; 
			MPI_Send(&time_sync_rdtscp, 1, MPI_UNSIGNED_LONG_LONG , 0 , 2*i + 1 , MPI_COMM_WORLD) ;
		}		
			
	}
	
	if (world_rank == 0){
		FILE *fp;
		fp = fopen("time.txt" , "w") ;
		
		
		for (uint64 i = 1 ; i < max_iter ; i++){
			nn = i ; 
			printf("\nvaleur de i : %llu\n\n", i);
		  
			unsigned long long tmp = 0 ; 
		
			unsigned long long rdtsc_mean = 0 ; 
			unsigned long long sync_rdtscp_mean = 0 ; 		
			unsigned long long rdtsc_mean_in = 0 ; 
			unsigned long long sync_rdtscp_mean_in = 0 ; 


			unsigned long long rdtsc_min = 1000000000000000 ; 
			unsigned long long sync_rdtscp_min = 1000000000000000 ; 
			unsigned long long rdtsc_min_in = 1000000000000000 ; 
			unsigned long long sync_rdtscp_min_in = 1000000000000000 ; 	

			unsigned long long rdtsc_max_in = 0 ; 
			unsigned long long sync_rdtscp_max_in = 0 ; 		
			unsigned long long rdtsc_max = 0 ; 
			unsigned long long sync_rdtscp_max = 0 ; 	

			for (int j = 1 ; j < world_size ; j++){
			
				MPI_Recv(&tmp , 1 , MPI_UNSIGNED_LONG_LONG , j , 2 * i ,  MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; // for rdtsc
				printf("     rdtsc timer : %llu with value %f\n", tmp, 0.0);
				treatment(&rdtsc_mean, &rdtsc_min, &rdtsc_max, &tmp, world_size);
		
				MPI_Recv(&tmp , 1 , MPI_UNSIGNED_LONG_LONG , j , 2 * i + 1 ,  MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; // for sync_rdtscp
				printf("sync rdtsc timer : %llu with value %f\n", tmp, 0.0);
				treatment(&sync_rdtscp_mean, &sync_rdtscp_min, &sync_rdtscp_max, &tmp, world_size);
								
			}
			
			printf("     rdtsc timer : %llu\n", rdtsc_mean);
			printf("sync rdtsc timer : %llu\n", sync_rdtscp_mean);	
			
			printf("     rdtsc min : %llu\n", rdtsc_min);
			printf("sync rdtsc min : %llu\n", sync_rdtscp_min);
			
			printf("     rdtsc max : %llu\n", rdtsc_max);
			printf("sync rdtsc max : %llu\n", sync_rdtscp_max);
			// write result in file
			
			fprintf(fp, "%llu %llu %llu %llu %llu %llu %llu\n" , nn, rdtsc_mean , rdtsc_min , rdtsc_max , sync_rdtscp_mean , sync_rdtscp_min , sync_rdtscp_max);
		}

	fclose(fp);
			
	}



	MPI_Finalize();

}	

