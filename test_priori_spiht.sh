#!/bin/bash

module load qccpack-test
cd /opt/flash_buffer/samuel

prefix=enstrophy_512
cratios=( 4 8 16 32 )

./bov2imgcube 512 512 512 ${prefix}.float ${prefix}.icb
spihtencode3d4 -nl 6 1 ${prefix}.icb ${prefix}.spbt ${prefix}.1bit.stream 

for i in ${cratios[@]}
do
    echo "cratio = $i "
    ./zeroout ${prefix}.spbt ${i} ${prefix}.rect.spbt

     spihtdecode3d4 ${prefix}.1bit.stream ${prefix}.rect.spbt ${prefix}.rect.icb 

     ./imgcube2bov ${prefix}.rect.icb ${prefix}.rect.float 

     ./calcerr 512 ${prefix}.rect.float ${prefix}.float >> ${prefix}.result

done

rm ${prefix}.1bit.stream *.spbt ${prefix}.rect.icb ${prefix}.rect.float ${prefix}.icb
