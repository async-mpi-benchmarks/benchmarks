CC = mpicc 
CFLAGS = -O3 -Wall

all: 1 

1 : 1.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp

clean :	
	rm 1 
	
run : 
	mpirun ./1
	
