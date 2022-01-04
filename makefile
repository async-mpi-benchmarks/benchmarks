CC = mpicc 
CFLAGS = -O3 -Wall

all: 1 pap

1 : 1.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp
	
pap : pap.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp
	
	

clean :	
	rm 1 pap
	
run : 
	mpirun ./1 mpirun -np 2 pap
	
