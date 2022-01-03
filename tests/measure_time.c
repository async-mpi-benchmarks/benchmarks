/*

- this example shows differnts methods to measure time or tics
- time is measured on all processes

*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include  <unistd.h>

// https://gist.github.com/savanovich/f07eda9dba9300eb9ccf

// rdtscp_before
inline
unsigned long long sync_rdtscp_b(void)
{
	unsigned a, d; 
	asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
	asm volatile("rdtsc" : "=a" (a), "=d" (d)); 
	return ((unsigned long)a) | (((unsigned long)d) << 32); 
}

// rdtscp after
inline
unsigned long long sync_rdtscp_a(void)
{
	unsigned a, d; 
	asm volatile("rdtscp" : "=a" (a), "=d" (d)); 
	asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
	return ((unsigned long)a) | (((unsigned long)d) << 32); 
}


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
	int world_res ; 
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	
	double inc = 0.0 ;
	unsigned long long nn = 40000;  	

	if (argc==2){
		char *tmpstring ; 
		nn = strtoul(argv[1], &tmpstring, 10);
	}	
	
	unsigned long long start ; 
	unsigned long long end ; 
	
	
	unsigned long long time_rdtsc ; 
	unsigned long long time_sync_rdtscp ; 
	unsigned long long time_rdtsc_in ;
	unsigned long long time_sync_rdtscp_in ; 
	

	
	if (world_rank !=0 && world_rank < world_size)
	{
	
		for (unsigned long long i = 0 ; i <  nn ; i++){
			inc = inc + 1.; 
		}
		inc = 0.0 ; 
		start = rdtsc() ; 
		for (unsigned long long i = 0 ; i < nn ; i++){
			inc = inc + 1.; 
		}
		end = rdtsc() ; 
		time_rdtsc = end - start ; 
		printf("     rdtsc timer : %llu with value %f\n", time_rdtsc, inc);
		
		inc = 0.0 ; 		
		start = sync_rdtscp_b() ; 		
		for (unsigned long long i = 0 ; i < nn ; i++){
			inc = inc + 1.; 
		}
		end = sync_rdtscp_a() ; 
		time_sync_rdtscp = end - start ; 
		printf("sync rdtsc timer : %llu with value %f\n", time_sync_rdtscp, inc);	
		
		for (unsigned long long i = 0 ; i <  nn ; i++){
			inc = inc + 1.; 
		}	
		
		MPI_Send(&time_rdtsc, 1, MPI_UNSIGNED_LONG_LONG , 0 , 0 , MPI_COMM_WORLD) ; 
		MPI_Send(&time_sync_rdtscp, 1, MPI_UNSIGNED_LONG_LONG , 0 , 1 , MPI_COMM_WORLD) ;		


			
	}
	
	if (world_rank == 0){
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

		for (int i = 1 ; i < world_size ; i++){
			MPI_Recv(&tmp , 1 , MPI_UNSIGNED_LONG_LONG , i , 0 ,  MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; // for rdtsc
			printf("     rdtsc timer : %llu with value %f\n", tmp, 0.0);
			rdtsc_mean += tmp ; 
			if (tmp < rdtsc_min){
				rdtsc_min = tmp ; 
			}
			if (tmp > rdtsc_max){
				rdtsc_max = tmp ; 
			}
			
			MPI_Recv(&tmp , 1 , MPI_UNSIGNED_LONG_LONG , i , 1 ,  MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; // for sync_rdtscp
			printf("sync rdtsc timer : %llu with value %f\n", tmp, 0.0);
			sync_rdtscp_mean += tmp ; 			
			if (tmp < sync_rdtscp_min){
				sync_rdtscp_min = tmp ; 
			}
			if (tmp > sync_rdtscp_max){
				sync_rdtscp_max = tmp ; 
			}
			
			
			
			
							
		}
		rdtsc_mean = rdtsc_mean / (world_size - 1) ; 
		sync_rdtscp_mean = sync_rdtscp_mean / (world_size - 1) ;
		
		printf("     rdtsc timer : %llu\n", rdtsc_mean);
		printf("sync rdtsc timer : %llu\n", sync_rdtscp_mean);	

		
		printf("     rdtsc min : %llu\n", rdtsc_min);
		printf("sync rdtsc min : %llu\n", sync_rdtscp_min);

		
		printf("     rdtsc max : %llu\n", rdtsc_max);
		printf("sync rdtsc max : %llu\n", sync_rdtscp_max);				



			
	}



	MPI_Finalize();

}	

