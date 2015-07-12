/* This program reads in a subband_pyramid (spbt format) and a ratio (cratio),
 * finds out the nth largest element in this pyramid,
 * and zero out all elements smaller than the nth largest one.
 *
 * Programmer: Samuel Li
 * Date: 7/10/2015
 */

#include "libQccPack.h"
#include <stdio.h>
#include <stdlib.h>

#define USG_STRING "%s:input_spbt_name %d:cratio %s:output_spbt_name "

int compare( const void* a, const void* b )
{
    if ( *(float*)b - *(float*)a  < 0 )
        return -1;
    else if ( *(float*)b - *(float*)a  > 0 )
        return 1;
    else
        return 0;
}

void ZeroOut( QccWAVSubbandPyramid3D* pyramid, int cratio )
{
    if( cratio > 1 ) {
    long size = (pyramid->num_cols) * (pyramid->num_rows) * (pyramid->num_frames);
    float* buf = malloc( sizeof(float) * size );

    int frame, col, row;
    long idx = 0;
    float f;
    for (frame = 0; frame < pyramid->num_frames; frame++)
        for (row = 0; row < pyramid->num_rows; row++)
            for (col = 0; col < pyramid->num_cols; col++) {
                f = pyramid -> volume[frame][row][col];
                if( f < 0 )     buf[ idx++ ] = f * -1.0;
                else            buf[ idx++ ] = f;
            }

    qsort( buf, size, sizeof(float), compare );
    long n = size / cratio - 1;
    float t  = buf[ n ];
    float nt = t * -1.0; 
    free( buf );
    long count = 0;

    for (frame = 0; frame < pyramid->num_frames; frame++)
        for (row = 0; row < pyramid->num_rows; row++)
            for (col = 0; col < pyramid->num_cols; col++) {
                f = pyramid -> volume[frame][row][col];
                if( f < t && f > nt ) {
                    pyramid -> volume[frame][row][col] = 0.0;
                    count++;
                }
            }

    printf("cratio=%d, threshold=%f, smashed=%ld\n", cratio, t, count );
    
    }
}

int main (int argc, char* argv[] )
{
    QccWAVSubbandPyramid3D pyramid;
    QccWAVSubbandPyramid3DInitialize( &pyramid );
    int cratio;
    QccString outfile;

    if( QccParseParameters( argc, argv, USG_STRING, pyramid.filename, &cratio, outfile) )
        QccErrorExit();

    if( QccWAVSubbandPyramid3DRead( &pyramid ) ) {
        QccErrorAddMessage( "%s: Read input subband_pyramid failed: %s", argv[0], argv[1] );
        QccErrorExit();
    }
    
    ZeroOut( &pyramid, cratio );

    QccStringCopy( pyramid.filename, outfile );
    if( QccWAVSubbandPyramid3DWrite( &pyramid ) ) {
        QccErrorAddMessage( "%s: Write input subband_pyramid failed: %s", argv[0], argv[1] );
        QccErrorExit();
    }

    return 0;
}
