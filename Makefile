all:

bov2imgcube: bov2imgcube.c
	gcc -O -Wall -fPIC -I/home/samuel/Tools/QccPack/include   -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o bov2imgcube bov2imgcube.c -L/home/samuel/Tools/QccPack/lib -lQccPack   -lm

imgcube2bov: imgcube2bov.c
	gcc -O -Wall -fPIC -I/home/samuel/Tools/QccPack/include   -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o imgcube2bov imgcube2bov.c -L/home/samuel/Tools/QccPack/lib -lQccPack   -lm

testbov2imgcube: bov2imgcube
	./bov2imgcube 3 3 3 3_cube.bin 3_imagecube.icb

testimgcube2bov: imgcube2bov
	./imgcube2bov 3_imagecube.icb binfile

clean:
	rm bov2imgcube imgcube2bov
