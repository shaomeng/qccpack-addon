/*
 * #include "myspiht.h"
 */
#include "myspeck.h"
#include "math.h"

int main( int argc, char* argv[] )
{
    float rate = 32.0;
    if( argc == 2 )
        rate = atof( argv[1] );
    int srcX = 64;
    int srcY = 64;
    int srcZ = 1;
    int totalV = srcX * srcY * srcZ; 
    float* inputbuf = (float*) malloc( sizeof(float) * totalV );
    int i;
    for( i = 0; i < totalV; i++ )
        inputbuf[i] = sin( i / 10.0 );

    char* filename = "tmp.bitstream";
    int nl = 2;

    myspeckencode2d(inputbuf, srcX, srcY, filename, nl, rate);

    
    float* reconstructbuf = (float*) malloc( sizeof(float) * totalV );
    myspeckdecode2d(filename, reconstructbuf, totalV );

    double rms, lmax;
    evaluate2arrays( inputbuf, reconstructbuf, totalV, &rms, &lmax );
    printf("%f bits per voxel, rms=%e, lmax=%e.\n", rate, rms, lmax );

    free( inputbuf );
    free( reconstructbuf );

    return 0;
}
