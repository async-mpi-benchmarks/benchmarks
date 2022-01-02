CC = mpicc 
CFLAGS = -O3 -Wall

all: hello_world

hello_world : hello_world.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp

clean :	
	rm hello_world
	
run : 
	mpirun ./hello_world
	
