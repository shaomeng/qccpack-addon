#!/bin/bash

baseline=${varname}_512.float
resultfile=${varname}.vdc.result

VAR3D=( 'LCLOUD' 'TOT_ICLD_VISTAU' 'CLDFSNOW' 'FREQL' 'FREQS' 'ANSNOW' 'TOT_CLD_VISTAU' 'AREL' 'AREI' 'FICE' 'AWNC' 'CONCLD' 'AWNI' 'ICWMR' 'ANRAIN' 'ICLDTWP' 'CLOUD' 'FREQR' 'FREQI' )

prefix='/opt/Research_Data/ensemble_orig/cesm1_1.FC5.ne30_g16.'
suffix='.cam.h0.0001-01-01-00000.nc'
variable='FREQL'
savefile=${variable}.txt
bitrate=32

for i in $(seq -f "%03g" 0 1);
do
	filename=${prefix}$i${suffix}
#	echo 'working on' ${filename}
#	echo 'File' ${i} >> ${savefile}
#	bin/camtest ${filename} ${variable} ${bitrate} >> ${savefile}
#	echo '' >> ${savefile}

	for v in ${VAR3D[@]}
	do
		bin/findminfabs ${filename} ${v}
	done
done
