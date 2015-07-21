/* This program converts an image cube in ICB format
 * into a bov file.
 *
 * Programmer: Samuel Li
 * Date: 7/2/2015
 */

#include "libQccPack.h"
#include <stdio.h>
#include <stdlib.h>

#define USG_STRING "%s:input_name %s:output_name"

int ImageCubeWriteData( QccString filename, QccIMGImageCube* image_cube )
{
    int frame, row, col;
    long idx;
    long planeSize = (image_cube->num_cols) * (image_cube->num_rows);
    float* buf = malloc( sizeof(float) * planeSize );
    FILE* outfile = fopen( filename, "wb" );
    if( outfile  == NULL ) {
        printf( "Read file open error!\n" );
        exit(1);
    }

    for (frame = 0; frame < image_cube->num_frames; frame++) {
        idx = 0;
        for (row = 0; row < image_cube->num_rows; row++)
            for (col = 0; col < image_cube->num_cols; col++)
                buf[ idx++ ] = image_cube -> volume[frame][row][col];

        fseek( outfile, 0, SEEK_END );
        long result = fwrite( buf, sizeof(float), planeSize, outfile );
        if( result != planeSize ) {
            printf( "Output file write error!\n");
            exit(1);
        }  
    }

    fclose( outfile );
    free( buf );
    return 0;
}

int main (int argc, char* argv[] )
{
    QccIMGImageCube imagecube;
    QccIMGImageCubeInitialize( &imagecube );
    QccString output_name;    

    if( QccParseParameters( argc, argv, USG_STRING, imagecube.filename, output_name ) )
        QccErrorExit();

    if( QccIMGImageCubeRead( &imagecube ) ) {
        QccErrorAddMessage( "%s: Read input imagecube failed: %s", argv[0], argv[1] );
        QccErrorExit();
    }
    ImageCubeWriteData( output_name, &imagecube );

//    QccIMGImageCubePrint( &imagecube );

    QccIMGImageCubeFree( &imagecube );

    return 0;
}
