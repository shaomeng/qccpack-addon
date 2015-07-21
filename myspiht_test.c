#include "myspiht.h"
#include "math.h"

int main( int argc, char* argv[] )
{
    float rate = 32.0;
    if( argc == 2 )
        rate = atof( argv[1] );
    int srcX = 32;
    int srcY = 32;
    int srcZ = 32;
    int totalV = srcX * srcY * srcZ;
    float* inputbuf = (float*) malloc( sizeof(float) * totalV );
    int i;
    for( i = 0; i < totalV; i++ )
        inputbuf[i] = sin( i / 10.0 );

    char* filename = "tmp.bitstream";
    int xynl = 3;
    int znl = 2;

    printf("encode return: %d\n", 
            myspihtencode2p1d(inputbuf, srcX, srcY, srcZ, filename, xynl, znl, rate) );

    
    float* outputbuf = (float*) malloc( sizeof(float) * totalV );
    printf("decode return: %d\n", myspihtdecode(filename, outputbuf, totalV) );

    double rms, lmax;
    evaluate2arrays( inputbuf, outputbuf, totalV, &rms, &lmax );
    printf("%f bits per voxel, rms=%e, lmax=%e.\n", rate, rms, lmax );

    free( inputbuf );
    free( outputbuf );

    return 0;
}
