export OMP_NUM_THREADS=16
mpirun -np 2 --map-by l3cache:PE=8 --bind-to core ./hybride
