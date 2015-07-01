all:

asciitodat: asciitodat.c
	gcc -O -Wall -fPIC -I/home/samuel/Tools/QccPack/include   -DQCCCOMPRESS=/bin/gzip -DQCCUNCOMPRESS=/bin/gunzip -DQCCPACK_WAVELET_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Wavelets -DQCCPACK_CODES_PATH_DEFAULT=.:/home/samuel/Tools/QccPack/install/share/QccPack/Codes  -DHAVE_SPIHT -DHAVE_SPECK    -o asciitodat asciitodat.c -L/home/samuel/Tools/QccPack/lib -lQccPack   -lm

clean:
	rm asciitodat
