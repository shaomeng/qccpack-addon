
CC_FLAGS=-O2 -Wall -g -fPIC

ARCH=$(shell uname)

ifeq ($(ARCH), Linux)
CC=gcc
QCCPACK_INSTALL=/Users/samuel/Git/QccPack-git/Install
endif

ifeq ($(ARCH), Darwin)
CC=clang
QCCPACK_INSTALL=/Users/samuel/Git/QccPack-git/Install
endif

QCCPACK_FLAGS=-DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK
LINK_LIB=-lQccPack -lpthread  -lm

bov2imgcube: bov2imgcube.c
	${CC} ${CC_FLAGS} -I${QCCPACK_INSTALL}/include -I/usr/include ${QCCPACK_FLAGS} -o bin/bov2imgcube bov2imgcube.c -L${QCCPACK_INSTALL}/lib ${LINK_LIB}

imgcube2bov: imgcube2bov.c
	gcc ${CC_FLAGS} -I${QCCPACK_INSTALL}/include -I/usr/include  -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o bin/imgcube2bov imgcube2bov.c -L${QCCPACK_INSTALL}/lib ${LINK_LIB}

imgcubecalcerr: imgcubecalcerr.c
	gcc ${CC_FLAGS} -I${QCCPACK_INSTALL}/include -I/usr/include  -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o bin/imgcubecalcerr imgcubecalcerr.c -L${QCCPACK_INSTALL}/lib  ${LINK_LIB}

zeroout: zerooutsubbandpyramid.c
	gcc ${CC_FLAGS} -I${QCCPACK_INSTALL}/include -I/usr/include  -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o bin/zeroout zerooutsubbandpyramid.c -L${QCCPACK_INSTALL}/lib ${LINK_LIB}

spbt2bov: spbt2bov.c
	gcc ${CC_FLAGS} -I${QCCPACK_INSTALL}/include -I/usr/include  -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o bin/spbt2bov spbt2bov.c -L${QCCPACK_INSTALL}/lib ${LINK_LIB}

myspiht.o: myspiht.h myspiht.c
	gcc ${CC_FLAGS} -I${QCCPACK_INSTALL}/include -I/usr/include  -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK -c -o bin/myspiht.o myspiht.c 

myspiht_test: myspiht.o myspiht_test.c
	gcc -O -Wall -o bin/myspiht_test bin/myspiht.o myspiht_test.c -I${QCCPACK_INSTALL}/include -L${QCCPACK_INSTALL}/lib ${LINK_LIB}


testbov2imgcube: bov2imgcube
	./bov2imgcube 3 3 3 3_cube.bin 3_imagecube.icb

testimgcube2bov: imgcube2bov
	./imgcube2bov 3_imagecube.icb binfile

clean:
	rm bov2imgcube imgcube2bov
