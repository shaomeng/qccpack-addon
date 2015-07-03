/* This program converts an image cube in ICB format
 * into a bov file.
 *
 * Programmer: Samuel Li
 * Date: 7/2/2015
 */

#include "libQccPack.h"
#include "libQccPackIMG.h"
#include <stdio.h>
#include <stdlib.h>

#define USG_STRING "%s:input_name %s:output_name"

int ImageCubeWriteData( QccString filename, QccIMGImageCube* image_cube )
{
    long size = (image_cube->num_frames) * (image_cube->num_rows) *
                (image_cube->num_cols);
    float* buf = malloc( sizeof(float) * size );
    int frame, row, col;
    long idx = 0;
    for (frame = 0; frame < image_cube->num_frames; frame++)
        for (row = 0; row < image_cube->num_rows; row++)
            for (col = 0; col < image_cube->num_cols; col++)
                buf[ idx++ ] = image_cube -> volume[frame][row][col];

    FILE* outfile = fopen( filename, "wb" );
    if( outfile  == NULL ) {
        printf( "Read file open error!\n" );
        exit(1);
    }
    long result = fwrite( buf, sizeof(float), size, outfile );
    if( result != size ) {
        printf( "Output file write error!\n");
        exit(1);
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

    QccIMGImageCubeRead( &imagecube );
    ImageCubeWriteData( output_name, &imagecube );

//    QccIMGImageCubePrint( &imagecube );


    return 0;
}
