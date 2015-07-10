/*
 * This program reads in a subband_pyramid in SPBT format, and a compression ratio,
 * calculates the nth largest element accordingly,
 * and zero out all coefficients smaller than that one.
 *
 * The coeffs after zeroing out is written back in SPBT format. 
 *
 * Programmer: Samuel Li
 * Date: 7/9/2015
 *
 */

#include "libQccPack.h"
/*
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
*/

#define USG_STRING "%s:inputSPBT %d:cratio %s:outputSPBT"

/*
float FindThreshold( int cratio, QccWAVSubbandPyramid3D* pyramid )
{
    long size = (pyramid -> num_frames) * (pyramid -> num_rows) * (pyramid -> num_cols);
    if( size % cratio != 0 )
        printf(" coeffs size not divided by cratio!\n" );
    long n = size / cratio - 1;     // the nth largest, indexing from 1.

    std::vector<float> allCoeffs;
    for (int frame = 0; frame < pyramid->num_frames; frame++)
        for (int row = 0; row < pyramid->num_rows; row++)
            for (int col = 0; col < pyramid->num_cols; col++)
                if( pyramid->volume[frame][row][col] > 0 )
                    allCoeffs.push_back( -1.0 * pyramid->volume[frame][row][col] );
                else
                    allCoeffs.push_back( pyramid->volume[frame][row][col] );

    std::nth_element( allCoeffs.begin(), allCoeffs.begin()+n, allCoeffs.end() );
    float nth = -1.0 * allCoeffs[n];

    return nth;
}

void ZeroOut( int cratio, QccWAVSubbandPyramid3D* pyramid )
{
    float nth = FindThreshold( cratio, pyramid );
    float nnth = -1.0 * nth;
    for (int frame = 0; frame < pyramid->num_frames; frame++)
        for (int row = 0; row < pyramid->num_rows; row++)
            for (int col = 0; col < pyramid->num_cols; col++)
                if( pyramid->volume[frame][row][col] < nth ||
                    pyramid->volume[frame][row][col] > nnth )
                    pyramid->volume[frame][row][col] = 0.0;

}
*/

int main( int argc, char* argv[] )
{
    QccWAVSubbandPyramid3D pyramid;
    int cratio;
    QccString outputSPBT;

    if( QccParseParameters( argc, argv, USG_STRING, pyramid.filename, &cratio, outputSPBT ) )
        QccErrorExit();

    if( QccWAVSubbandPyramid3DRead( &pyramid ) ) {
        QccErrorAddMessage( "%s: Read input subband pyramid failed: %s", argv[0], argv[1] );
        QccErrorExit();
    }

/*
    ZeroOut( cratio, &pyramid );
*/
    QccStringCopy( pyramid.filename, outputSPBT );
    if( QccWAVSubbandPyramid3DWrite( &pyramid ) ){
        QccErrorAddMessage( "%s: Write output SPBT failed: %s", argv[0], argv[2] );
        QccErrorExit();
    }

    return 0;
}
