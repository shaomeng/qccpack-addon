// #include "myspeck.h"
#include <math.h>
#include <cstdlib>
#include <cstdio>

extern "C" 
{
    int myspeckencode3d( float* srcBuf,
                   int srcX,
                   int srcY,
                   int srcZ,
                   char* outputFilename,
                   int nLevels,
                   float TargetRate );

    int myspeckencode2p1d( float* srcBuf,
                       int srcX,
                       int srcY,
                       int srcZ,
                       char* outputFilename,
                       int XYNumLevels,
                       int ZNumLevels,
                       float TargetRate );

    int myspeckdecode( char*  inputFilename,
                     float* dstBuf,
                     int    outSize );

    void evaluate2arrays( float* A, 
                          float* B, 
                          int len, 
                          double* rms, 
                          double* lmax );
};

int main( int argc, char* argv[] )
{
    float rate = 32.0;
    if( argc == 2 )
        rate = atof( argv[1] );
    int srcX = 64;
    int srcY = 64;
    int srcZ = 64;
    int totalV = srcX * srcY * srcZ;
    float* inputbuf = (float*) malloc( sizeof(float) * totalV );
    float* outputbuf = (float*) malloc( sizeof(float) * totalV );
    int i;
    for( i = 0; i < totalV; i++ )
        inputbuf[i] = sin( i / 10.0 );

    char filename[] = "tmp.bitstream";
    int xynl = 3;
    int znl = 3;
    double rms, lmax;

    /*
     * 3D test
     */
    printf("encode return: %d\n", 
            myspeckencode3d(inputbuf, srcX, srcY, srcZ, filename, xynl, rate) );
    
    printf("decode return: %d\n", myspeckdecode(filename, outputbuf, totalV) );

    evaluate2arrays( inputbuf, outputbuf, totalV, &rms, &lmax );
    printf("3D DWT: %f bits per voxel, rms=%e, lmax=%e.\n", rate, rms, lmax );

    /*
     * 2D+1D test
     */
    printf("encode return: %d\n", 
            myspeckencode2p1d(inputbuf, srcX, srcY, srcZ, filename, xynl, znl, rate) );

    printf("decode return: %d\n", myspeckdecode(filename, outputbuf, totalV) );

    evaluate2arrays( inputbuf, outputbuf, totalV, &rms, &lmax );
    printf("2D+1D DWT: %f bits per voxel, rms=%e, lmax=%e.\n", rate, rms, lmax );

    /*
     * Free allocated memory
     */
    free( inputbuf );
    free( outputbuf );

    return 0;
}
