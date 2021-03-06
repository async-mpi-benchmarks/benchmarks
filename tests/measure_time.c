/*

- this example shows differnts methods to measure time or tics
- time is measured on all processes

*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>





typedef unsigned long long uint64 ; 


// these rdtsc are from @dssgabriel

static inline uint64 fenced_rdtscp()
{
    uint64 tsc;
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

static inline uint64 shifted_rdtsc()
{
    uint64 tsc;
    asm volatile(
        "rdtsc                  \n\t"
        "shl    $0x20, %%rdx    \n\t"
        "or     %%rdx, %%rax    \n\t"
        : "=a" (tsc)
        :
        : "rdx");
    return tsc;
}


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
// --------------


inline
uint64 sync_rdtscp()
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



void measure_time(uint64 nn , uint64 *time_rdtsc, uint64 *time_sync_rdtscpba,  uint64 *time_shifted_rdtscp,  uint64 *time_fenced_rdtscp ){

		double inc = 0.0 ;		
		uint64 start ; 
		uint64 end ; 
		uint64 boost = 1000000 ;
		
		// first burn loop + show out of order effects
		for (uint64 i = 0 ; i < boost ; i++){
			inc = inc + 1.; 
		}
		inc = 0.0 ;
		
		
		// rdtsc basic measure
		start = rdtsc() ; 
		for (uint64 i = 0 ; i < nn ; i++){
			inc = inc + 1.; 
		}
		end = rdtsc() ; 
		(*time_rdtsc) = end - start ; 		
		printf("     rdtsc timer : %llu %f\n", (*time_rdtsc), inc);
		// new burn loop
		inc = 0.0 ;
		for (uint64 i = 0 ; i < boost ; i++){
			inc = inc + 1.; 
		}
		inc = 0.0 ;	
		
					
		// sync_rdtsc_a_b measures
		start = sync_rdtscp_b() ; 		
		for (uint64 i = 0 ; i < nn ; i++){
			inc = inc + 1.; 
		}
		end = sync_rdtscp_a() ; 
		(*time_sync_rdtscpba) = end - start ;		
		printf("sync rdtscpba timer : %llu %f\n", (*time_sync_rdtscpba), inc);
		inc = 0.0 ;	
		// new burn loop
		for (uint64 i = 0 ; i < boost ; i++){
			inc = inc + 1.; 
		}	
		inc = 0.0 ;
		
					
		// shifted_rdtsc basic measure
		start = shifted_rdtsc() ; 
		for (uint64 i = 0 ; i < nn ; i++){
			inc = inc + 1.; 
		}
		end = shifted_rdtsc() ; 
		(*time_shifted_rdtscp) = end - start ; 		
		printf("   shifted  rdtscp timer : %llu %f\n", (*time_shifted_rdtscp), inc);
		// new burn loop
		inc = 0.0 ;
		for (uint64 i = 0 ; i < boost ; i++){
			inc = inc + 1.; 
		}
		inc = 0.0 ;
			
			
		// fenced_rdtsc basic measure
		start = fenced_rdtscp() ; 
		for (uint64 i = 0 ; i < nn ; i++){
			inc = inc + 1.; 
		}
		end = fenced_rdtscp() ; 
		(*time_fenced_rdtscp) = end - start ; 		
		printf("   fenced  rdtscp timer : %llu %f\n", (*time_fenced_rdtscp), inc);
			
		
		
		// loop to test out of order effects
		for (uint64 i = 0 ; i < boost ; i++){
			inc = inc + 1.; 
		}		
		printf("     rdtsc timer : %llu %f\n", (*time_rdtsc), inc);			
		printf("sync rdtscpba timer : %llu %f\n", (*time_sync_rdtscpba), inc);		
		printf("sync shifted_rdtscp timer : %llu %f\n", (*time_shifted_rdtscp), inc);			
		printf("sync fenced_rdtscpba timer : %llu %f\n", (*time_fenced_rdtscp), inc);


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
	uint64 max_iter = 400; 
	uint64 nn ;  	

	//if (argc==2){
	//	char *tmpstring ; 
	//	nn = strtoul(argv[1], &tmpstring, 10);
	//}	
	
	uint64 start ; 
	uint64 end ; 
	
	
	uint64 time_rdtsc ; 
	uint64 time_sync_rdtscp ; 
	uint64 time_sync_rdtscpba ; 
	uint64 time_shifted_rdtscp  ;
	uint64 time_fenced_rdtscp ;	

	
	if (world_rank !=0 && world_rank < world_size)
	{	
		for (uint64 i = 1 ; i < max_iter ; i++){
			nn = i ; 
			measure_time(nn, &time_rdtsc , &time_sync_rdtscpba, &time_shifted_rdtscp, &time_fenced_rdtscp)  ;
			MPI_Send(&time_rdtsc, 1, MPI_UNSIGNED_LONG_LONG , 0 ,  4*i , MPI_COMM_WORLD) ;  // for rdtsc
			MPI_Send(&time_sync_rdtscpba, 1, MPI_UNSIGNED_LONG_LONG , 0 , 4*i + 1 , MPI_COMM_WORLD) ; // for rdtscpba
			MPI_Send(&time_shifted_rdtscp, 1, MPI_UNSIGNED_LONG_LONG , 0 , 4*i + 2 , MPI_COMM_WORLD) ; // for shifted_rdtscp
			MPI_Send(&time_fenced_rdtscp, 1, MPI_UNSIGNED_LONG_LONG , 0 , 4*i + 3 , MPI_COMM_WORLD) ; // for fenced_rdtscp
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
			unsigned long long sync_rdtscpba_mean = 0 ; 		
			unsigned long long shifted_rdtscp_mean = 0 ; 	
			unsigned long long fenced_rdtscp_mean = 0 ;  


			unsigned long long rdtsc_min = 1000000000000000 ; 
			unsigned long long sync_rdtscpba_min = 1000000000000000 ; 
			unsigned long long shifted_rdtscp_min = 1000000000000000 ; 
			unsigned long long fenced_rdtscp_min = 1000000000000000 ; 	
		
			unsigned long long rdtsc_max = 0 ; 
			unsigned long long sync_rdtscpba_max = 0 ; 	
			unsigned long long shifted_rdtscp_max = 0 ; 
			unsigned long long fenced_rdtscp_max = 0 ; 


			for (int j = 1 ; j < world_size ; j++){
			
				MPI_Recv(&tmp , 1 , MPI_UNSIGNED_LONG_LONG , j , 4 * i ,  MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; // for rdtsc
				printf("     rdtsc timer : %llu with value %f\n", tmp, 0.0);
				treatment(&rdtsc_mean, &rdtsc_min, &rdtsc_max, &tmp, world_size);

	
				MPI_Recv(&tmp , 1 , MPI_UNSIGNED_LONG_LONG , j , 4 * i + 1 ,  MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; // for sync_rdtscp
				printf("sync rdtscpba timer : %llu with value %f\n", tmp, 0.0);
				treatment(&sync_rdtscpba_mean, &sync_rdtscpba_min, &sync_rdtscpba_max, &tmp, world_size);
				
				MPI_Recv(&tmp , 1 , MPI_UNSIGNED_LONG_LONG , j , 4 * i + 2 ,  MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; // for shifted_rdtscp
				printf("shifted rdtscp timer : %llu with value %f\n", tmp, 0.0);
				treatment(&shifted_rdtscp_mean, &shifted_rdtscp_min, &shifted_rdtscp_max, &tmp, world_size);				
				
				MPI_Recv(&tmp , 1 , MPI_UNSIGNED_LONG_LONG , j , 4 * i + 3 ,  MPI_COMM_WORLD, MPI_STATUS_IGNORE) ; // for fenced_rdtscp
				printf("fenced rdtscp timer : %llu with value %f\n", tmp, 0.0);
				treatment(&fenced_rdtscp_mean, &fenced_rdtscp_min, &fenced_rdtscp_max, &tmp, world_size);			
				
				
								
			}
			
			printf("     rdtsc timer : %llu\n", rdtsc_mean);
			printf("sync rdtscpba timer : %llu\n", sync_rdtscpba_mean);	
			printf("shifted rdtscp timer : %llu\n", shifted_rdtscp_mean);	
			printf("fenced rdtscp timer : %llu\n", fenced_rdtscp_mean);	
			
			//printf("     rdtsc min : %llu\n", rdtsc_min);
			//printf("sync rdtscp min : %llu\n", sync_rdtscp_min);
			//printf("sync rdtscpba min : %llu\n", sync_rdtscpba_min);
			
			//printf("     rdtsc max : %llu\n", rdtsc_max);
			//printf("sync rdtscp max : %llu\n", sync_rdtscp_max);
			//printf("sync rdtscpba max : %llu\n", sync_rdtscpba_max);
			// write result in file
			
			fprintf(fp, "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n" , nn, rdtsc_mean , rdtsc_min , rdtsc_max , sync_rdtscpba_mean , sync_rdtscpba_min , sync_rdtscpba_max, shifted_rdtscp_mean , shifted_rdtscp_min , shifted_rdtscp_max , fenced_rdtscp_mean , fenced_rdtscp_min , fenced_rdtscp_max );
		}

	fclose(fp);
			
	}



	MPI_Finalize();

}	

