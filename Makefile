
CC_FLAGS=-O3 -Wall -g -fPIC
CXX_FLAGS=-O3 -std=c89 -Wall -g -fPIC -shared

INC_WILDCARD=
LINK_WILDCARD=

ARCH=$(shell uname)

ifeq ($(ARCH), Linux)
CC=gcc
CXX=g++
# Yellowstone
QCCPACK_INSTALL=/glade/u/home/shaomeng/Git/QccPack-git/Install
VAPOR_INSTALL=/glade/u/home/shaomeng/Tools/vapor-2.4.2

# Alaska
HOSTNAME=$(shell hostname)
ifeq ($(HOSTNAME), alaska)
QCCPACK_INSTALL=/home/users/samuelli/Git/QccPack-git/Install
VAPOR_INSTALL=/home/users/samuelli/Tools/vapor-2.4.2-src/Install
INC_WILDCARD=/home/users/samuelli/Install/include
LINK_WILDCARD=/home/users/samuelli/Install/lib
endif

endif

ifeq ($(ARCH), Darwin)
CC=clang
CXX=clang++
QCCPACK_INSTALL=/Users/samuel/Git/QccPack-git/Install
endif

QCCPACK_FLAGS=-DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:${QCCPACK_INSTALL}/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK


QCCPACK_LINK=-Wl,-rpath,${QCCPACK_INSTALL}/lib 
VAPOR_LINK=-Wl,-rpath,${VAPOR_INSTALL}/lib 

CAMHANDLER_LINK=-L${VAPOR_INSTALL}/lib -L${LINK_WILDCARD} -lvdf -lnetcdf -ludunits2 -lcommon -lproj -ludunits2 -L${QCCPACK_INSTALL}/lib -lQccPack -L./bin -lcamhandler ${VAPOR_LINK} ${QCCPACK_LINK}

LINK_LIB=${QCCPACK_LINK} -lQccPack -lpthread  -lm



myspeck.o: myspeck.h myspeck.c
	${CC} ${CC_FLAGS} -I${QCCPACK_INSTALL}/include ${QCCPACK_FLAGS} -c -o bin/myspeck.o myspeck.c 

CamHandler.o: CamHandler.cpp CamHandler.h
	${CXX} ${CXX_FLAGS} -I${QCCPACK_INSTALL}/include -I${VAPOR_INSTALL}/include -c CamHandler.cpp -o bin/CamHandler.o

libcamhandler.so: CamHandler.o myspeck.o
	${CXX} ${CXX_FLAGS} -o bin/libcamhandler.so bin/CamHandler.o bin/myspeck.o

libcamhandler.a: CamHandler.o myspeck.o
	ar -rsv bin/libcamhandler.a bin/CamHandler.o bin/myspeck.o

CamHandlerTest: 
	${CXX} -I${VAPOR_INSTALL}/include  CamHandlerTest.cpp -o bin/camtest ${CAMHANDLER_LINK}

bov2imgcube: bov2imgcube.c
	${CC} ${CC_FLAGS} -I${QCCPACK_INSTALL}/include ${QCCPACK_FLAGS} -o bin/bov2imgcube bov2imgcube.c -L${QCCPACK_INSTALL}/lib ${LINK_LIB}

imgcube2bov: imgcube2bov.c
	${CC} ${CC_FLAGS} -I${QCCPACK_INSTALL}/include  ${QCCPACK_FLAGS} -o bin/imgcube2bov imgcube2bov.c -L${QCCPACK_INSTALL}/lib ${LINK_LIB}

imgcubecalcerr: imgcubecalcerr.c
	${CC} ${CC_FLAGS} -I${QCCPACK_INSTALL}/include ${QCCPACK_FLAGS} -o bin/imgcubecalcerr imgcubecalcerr.c -L${QCCPACK_INSTALL}/lib  ${LINK_LIB}

zeroout: zerooutsubbandpyramid.c
	${CC} ${CC_FLAGS} -I${QCCPACK_INSTALL}/include ${QCCPACK_FLAGS} -o bin/zeroout zerooutsubbandpyramid.c -L${QCCPACK_INSTALL}/lib ${LINK_LIB}

spbt2bov: spbt2bov.c
	${CC} ${CC_FLAGS} -I${QCCPACK_INSTALL}/include ${QCCPACK_FLAGS} -o bin/spbt2bov spbt2bov.c -L${QCCPACK_INSTALL}/lib ${LINK_LIB}

myspiht.o: myspiht.h myspiht.c
	${CC} ${CC_FLAGS} -I${QCCPACK_INSTALL}/include ${QCCPACK_FLAGS} -c -o bin/myspiht.o myspiht.c 

myspecktest: myspeck.o 
	${CC} -O3 -Wall bin/myspeck.o myspecktest.c -o bin/myspecktest -I${QCCPACK_INSTALL}/include -L${QCCPACK_INSTALL}/lib ${LINK_LIB}

testbov2imgcube: bov2imgcube
	./bov2imgcube 3 3 3 3_cube.bin 3_imagecube.icb

testimgcube2bov: imgcube2bov
	./imgcube2bov 3_imagecube.icb binfile

clean:
	rm bov2imgcube imgcube2bov
