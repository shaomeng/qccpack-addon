#!/bin/bash

module load qccpack-test

# number of bits per voxel
num_bit=( 1 2 4 8 12 )

cd /opt/flash_buffer/samuel
prefix=vx_512

./bov2imgcube 512 512 512 ${prefix}.float ${prefix}.icb

for i in ${num_bit[@]}
do
    echo "echo: start working on num_bit=${i}"
    compressed_bitstream=${prefix}_${i}bit.stream
    spihtencode3d -nl 5 ${i} ${prefix}.icb ${compressed_bitstream}
    echo "echo: finish SPIHT compression"

    decompressed_icb=${prefix}_${i}bit.icb
    spihtdecode3d ${compressed_bitstream} ${decompressed_icb}
    echo "echo: finish decompression from bitstream"

    decompressed_float=${prefix}_${i}bit.float
    ./imgcube2bov ${decompressed_icb} ${decompressed_float}
    echo "echo: finish conversion from ICB to floats"

    echo "${i} bit:" >> ${prefix}.result
    ./calcerr 512 ${decompressed_float} ${prefix}.float >> ${prefix}.result
    echo " " >> ${prefix}.result

    rm -f ${compressed_bitstream} ${decompressed_icb} ${decompressed_float}


done

rm -f ${prefix}.icb
