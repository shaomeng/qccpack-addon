all:

bovtoimgcube: bovtoimgcube.c
	gcc -std=c99 -O -Wall -fPIC -I/home/samuel/Tools/QccPack/include   -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o bovtoimgcube bovtoimgcube.c -L/home/samuel/Tools/QccPack/lib -lQccPack   -lm

test: bovtoimgcube
	./bovtoimgcube 3 3 3 3_cube.bin 3_imagecube.icb

clean:
	rm bovtoimgcube
