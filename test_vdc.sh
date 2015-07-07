#!/bin/bash

# source the installation script
. ~/Tools/vapor-3.0.0/bin/vapor-setup.sh

cd /opt/flash_buffer/samuel

comp_lod=( 4 3 2 1 0 )
varname=enstrophy

baseline=${varname}_512.float
resultfile=${varname}.vdc.result

for i in ${comp_lod[@]}
do
    echo "echo: start working on lod=${i}"

    decompressed=${varname}.lod${i}.float
    vdc2raw -lod ${i} -level -1 -varname ${varname} \
    /home/samuel/TaylorGreen/512_cube/master.nc ${decompressed}

    echo "Lod ${i}:" >> ${resultfile}
    ./calcerr 512 ${decompressed} ${baseline} >> ${resultfile}
    echo " " >> ${resultfile}

    rm -f ${decompressed}


done

