CC = mpicc 
CFLAGS = -O3 -Wall

all: 1 pap recouvrement hybride recouvrement_hybride

1 : 1.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp
	
pap : pap.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp


recouvrement : recouvrement.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp

recouvrement_hybride : recouvrement_hybride.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp
		
hybride : hybride.c
	${CC} ${CFLAGS} -march=native -mtune=native -funroll-loops -finline-functions -fpeel-loops -ftree-vectorize -ftree-loop-vectorize $^ -o $@  -lm -fopenmp	

clean :	
	rm 1 pap recouvrement hybride recouvrement_hybride
	
run : 
	mpirun ./1 &&  mpirun -np 2 pap && mpirun -np 2 ./recouvrememt 
	
