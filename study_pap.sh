rm time_pap.txt

for ((i=1;i<100;i=i+1));
	do
		mpirun -np 2 ./pap $i 1000 100000 ; 
done;
