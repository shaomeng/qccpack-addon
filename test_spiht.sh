#!/bin/bash

module load qccpack-test

# number of bits per voxel
num_bit=( 1 )

cd /opt/flash_buffer/samuel
prefix=vx_512
encoder=spiht

icbbaseline=${encoder}.${prefix}.icb
resultfile=${encoder}.${prefix}.result
./bov2imgcube 512 512 512 ${prefix}.float ${icbbaseline}

for i in ${num_bit[@]}
do
    echo "echo: start working on num_bit=${i}"
    compressed_bitstream=${encoder}.${prefix}_${i}bit.stream
    ${encoder}encode3d -nl 6 ${i} ${icbbaseline} ${compressed_bitstream}
    echo "echo: finish ${encoder} compression"

    decompressed_icb=${encoder}.${prefix}_${i}bit.icb
    ${encoder}decode3d ${compressed_bitstream} ${decompressed_icb}
    echo "echo: finish decompression from bitstream"

    decompressed_float=${encoder}.${prefix}_${i}bit.float
    ./imgcube2bov ${decompressed_icb} ${decompressed_float}
    echo "echo: finish conversion from ICB to floats"

    echo "${i} bit:" >> ${resultfile}
    ./calcerr 512 ${decompressed_float} ${prefix}.float >> ${resultfile}
    echo " " >> ${resultfile}
    echo "echo: finish calculating errors"

    rm -f ${compressed_bitstream} ${decompressed_icb} ${decompressed_float}


done

rm -f ${icbbaseline}
