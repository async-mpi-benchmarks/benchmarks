CC = mpicc 
CFLAGS = -O3 -Wall

all: hello_world send_scalar send_array

hello_world : hello_world.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp

send_scalar : send_scalar.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp

send_array : send_array.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp


clean :	
	rm hello_world send_scalar send_array
	
run : 
	mpirun ./hello_world && mpirun ./send_scalar && mpirun ./send_array
	
