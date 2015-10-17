/* This program reads in a subband_pyramid (spbt format),
 * and output all elements in the bov format.
 *
 * Programmer: Samuel Li
 * Date: 10/16/2015
 */

#include "libQccPack.h"
#include <stdio.h>
#include <stdlib.h>

#define USG_STRING "%s:input_spbt_name %s:output_bov_name "

int compare( const void* a, const void* b )
{
    if ( *(float*)b - *(float*)a  < 0 )
        return -1;
    else if ( *(float*)b - *(float*)a  > 0 )
        return 1;
    else
        return 0;
}

void SubbandPyramidWriteData( QccWAVSubbandPyramid3D* pyramid, QccString filename )
{
    int frame, row, col;
    long idx;
    long planeSize = (pyramid->num_cols) * (pyramid->num_rows);
    float* buf = (float*) malloc( sizeof(float) * planeSize );
    FILE* outfile = fopen( filename, "wb" );
    if( outfile  == NULL ) {
        printf( "Output file open error!\n" );
        exit(1);
    }

    for (frame = 0; frame < pyramid->num_frames; frame++) {
        idx = 0;
        for (row = 0; row < pyramid->num_rows; row++)
            for (col = 0; col < pyramid->num_cols; col++)
                buf[ idx++ ] = pyramid -> volume[frame][row][col];

        fseek( outfile, 0, SEEK_END );
        long result = fwrite( buf, sizeof(float), planeSize, outfile );
        if( result != planeSize ) {
            printf( "Output file write error!\n");
            exit(1);
        }
    }

    fclose( outfile );
    free( buf );
}


int main (int argc, char* argv[] )
{
    QccWAVSubbandPyramid3D pyramid;
    QccWAVSubbandPyramid3DInitialize( &pyramid );
    QccString outfile;

    if( QccParseParameters( argc, argv, USG_STRING, pyramid.filename, outfile) )
        QccErrorExit();

    if( QccWAVSubbandPyramid3DRead( &pyramid ) ) {
        QccErrorAddMessage( "%s: Read input subband_pyramid failed: %s", argv[0], argv[1] );
        QccErrorExit();
    }
    
    SubbandPyramidWriteData( &pyramid, outfile );

    QccWAVSubbandPyramid3DFree( &pyramid );

    return 0;
}
