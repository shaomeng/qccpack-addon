all: bov2imgcube imgcube2bov

bov2imgcube: bov2imgcube.c
	gcc -O -Wall -fPIC -I/home/samuel/Tools/QccPack/include -I/usr/include  -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o bin/bov2imgcube bov2imgcube.c -L/home/samuel/Tools/QccPack/lib -lQccPack  -L/usr/lib -lpthread  -lm

imgcube2bov: imgcube2bov.c
	gcc -O -Wall -fPIC -I/home/samuel/Tools/QccPack/include -I/usr/include  -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o bin/imgcube2bov imgcube2bov.c -L/home/samuel/Tools/QccPack/lib -lQccPack  -L/usr/lib  -lpthread  -lm

zeroout: zerooutsubbandpyramid.c
	gcc -O -Wall -fPIC -I/home/samuel/Tools/QccPack/include -I/usr/include  -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o bin/zeroout zerooutsubbandpyramid.c -L/home/samuel/Tools/QccPack/lib -lQccPack  -L/usr/lib -lpthread  -lm

myspiht.o: myspiht.h myspiht.c
	gcc -O -Wall -fPIC -I/home/samuel/Tools/QccPack/include -I/usr/include  -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK -c -o bin/myspiht.o myspiht.c 

myspiht_test: myspiht.o myspiht_test.c
	gcc -O -Wall -o bin/myspiht_test bin/myspiht.o myspiht_test.c -I/home/samuel/Tools/QccPack/include -L/home/samuel/Tools/QccPack/lib -lQccPack  -L/usr/lib -lpthread  -lm


testbov2imgcube: bov2imgcube
	./bov2imgcube 3 3 3 3_cube.bin 3_imagecube.icb

testimgcube2bov: imgcube2bov
	./imgcube2bov 3_imagecube.icb binfile

clean:
	rm bov2imgcube imgcube2bov
