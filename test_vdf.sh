#!/bin/bash

module load vapor

# number of bits per voxel
comp_lod=( 0 1 2 3 4 )

cd /opt/flash_buffer/samuel

varname=wz
baseline=${varname}_512.float
resultfile=${varname}.vdf.result

for i in ${comp_lod[@]}
do
    echo "echo: start working on lod=${i}"

    decompressed=${varname}.lod${i}.float
    vdf2raw -lod ${i} -level -1 -varname ${varname} \
    /home/samuel/TaylorGreen/512_cube/bior4.vdf ${decompressed}

    echo "Lod ${i}:" >> ${resultfile}
    ./calcerr 512 ${decompressed} ${baseline} >> ${resultfile}
    echo " " >> ${resultfile}

    rm -f ${decompressed}


done

