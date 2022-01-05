rm time_pap.txt
((j = 1)) ; 
((inc = 1)) ; 
((lim = 10)) ; 
((burn = 0)) ; 
((burn_lim = 0)) ;
((iteration = 1)) ; 
((iteration_lim = 1)) ; 
for ((i=1;i<70;i=i+1));
	do
		((j = j+ inc)) ;
		echo $j ; 
		mpirun -np 2 ./pap $j $burn $iteration ; 
		if (( "$j" == "$lim"))
		then
			((inc = lim));
			((lim = lim * 10));
			if (($"burn" == $"burn_lim"))
			then 
				echo
			else
				((burn = burn / 10));
			fi
			if (($"iteration" == $"iteration_lim"))
			then 
				echo
			else 
				((iteration = iteration / 10));
			fi
		fi
		
		
done;
