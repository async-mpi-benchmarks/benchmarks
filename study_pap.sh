rm time_pap.txt
((j = 1)) ; 
((inc = 1)) ; 
((lim = 10)) ; 
((burn = 10)) ; 
((iteration = 10)) ; 
for ((i=1;i<60;i=i+1));
	do
		((j = j+ inc)) ;
		echo $j ; 
		mpirun -np 2 ./pap $j $burn $iteration ; 
		if (( "$j" == "$lim"))
		then
			((inc = lim));
			((lim = lim * 10));
		fi
done;
