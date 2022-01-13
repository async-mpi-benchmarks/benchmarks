CC = mpicc 
CFLAGS = -O3 -Wall

all: 1 pap recouvrement hybride recouvrement_hybride recouvrement_hybride_pthread

1 : 1.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp
	
pap : pap.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp


recouvrement : recouvrement.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp

recouvrement_hybride : recouvrement_hybride.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp -lpthread

recouvrement_hybride_pthread : recouvrement_hybride_pthread.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp -lpthread
		
hybride : hybride.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp	

clean :	
	rm 1 pap recouvrement hybride recouvrement_hybride recouvrement_hybride_pthread
	
run : 
	mpirun ./1 &&  mpirun -np 2 pap && mpirun -np 2 ./recouvrememt 
	
