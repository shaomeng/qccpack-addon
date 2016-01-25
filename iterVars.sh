#!/bin/bash

while read p;
do
	echo processing variable $p
	min=0.0
	max=0.0
	maxdetail=0.0
	for i in {0..5}
		do
		infile=Datasets/faces/${p}.float.face${i}
		outfile=Datasets/face_icb/${p}.face${i}.icb

		stats=($(speckdwttest -nl 1 ${outfile} /flash_buffer/Sam/spbt /flash_buffer/Sam/icb))
		lmin=${stats[0]}
		lmax=${stats[1]}
		lmaxdetail=${stats[2]}

		if [ "$i" == 0 ]
		then
			min=${lmin}
			max=${lmax}
			maxdetail=${lmaxdetail}
		else
			min=$(echo "min(${min},${lmin})" | bc ~/extensions.bc -l )
			max=$(echo "max(${max},${lmax})" | bc ~/extensions.bc -l )
			maxdetail=$(echo "max(${maxdetail},${lmaxdetail})" | bc ~/extensions.bc -l ) 
		fi	

#		size=$(stat -c %s ${infile})
#		if [ "$size" == 993720 ]
#		then
#			echo "$p is a 3D variable"	
#			bin/bov2imgcube 91 91 30 $infile $outfile
#		elif [ "$size" == 33124 ]
#		then
#			echo "$p is a 2D variable"	
#			bin/bov2imgcube 91 91 1 $infile $outfile
#		fi
	done
	per=$(echo "${maxdetail}/(${max}-${min})" | bc ~/extensions.bc -l)
	echo "maxdetail=${maxdetail}, percentage=${per}, min=${min}, max=${max}"
done < results/rmsz/varlist

