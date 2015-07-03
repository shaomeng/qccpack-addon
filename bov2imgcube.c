/* This program converts a block of values (bov) file
 * into a QccIMGImageCube object, then write this object
 * to an ICB-format file, using the QccIMGImageCube function.
 *
 * Programmer: Samuel Li
 * Date: 7/2/2015
 */

#include "libQccPack.h"
#include "libQccPackIMG.h"
#include <stdio.h>
#include <stdlib.h>

#define USG_STRING "%d:num_cols %d:num_rows %d:num_frames %s:input_name %s:output_name"

int ImageCubeReadData( QccString filename, QccIMGImageCube* image_cube )
{
    FILE* infile = fopen( filename, "rb" );
    if( infile  == NULL ) {
        printf( "Read file open error!\n" );
        exit(1);
    }

    fseek( infile, 0, SEEK_END );
    long size = ftell( infile );
    if( size % 4 == 0 ) {
        size /= 4;
        if( size != (image_cube->num_cols) * (image_cube->num_rows) *
                    (image_cube -> num_frames)  )   {
            printf( "Read file length error!\n" );
            exit(1);
        }
    }
    else{
        printf( "Read file broken!\n" );
        exit(1);
    }

    fseek( infile, 0, SEEK_SET );
    float* buf = malloc( sizeof(float) * size );
    long result = fread( buf, sizeof(float), size, infile );
    fclose( infile );
    if( result != size ) {
        printf( "Input file read error!\n");
        exit(1);
    }  

    double min = MAXDOUBLE;
    double max = -MAXDOUBLE;
    for( long i = 0; i < size; i++ ) {
        if( buf[i] < min )      min = buf[i];
        if( buf[i] > max )      max = buf[i];
    }
    image_cube -> min_val = min;
    image_cube -> max_val = max;
        
    
    int frame, row, col;
    long idx = 0;
    for (frame = 0; frame < image_cube->num_frames; frame++)
        for (row = 0; row < image_cube->num_rows; row++)
            for (col = 0; col < image_cube->num_cols; col++)
                image_cube -> volume[frame][row][col] = buf[ idx++ ]; 

    free( buf );
    return 0;
}

int main (int argc, char* argv[] )
{
    QccIMGImageCube imagecube;
    QccIMGImageCubeInitialize( &imagecube );
    QccString input_name;    

    if( QccParseParameters( argc, argv, USG_STRING, &imagecube.num_cols, &imagecube.num_rows,
                            &imagecube.num_frames, input_name, imagecube.filename ) )
        QccErrorExit();
 

    if( QccIMGImageCubeAlloc( &imagecube ) )
        QccErrorPrintMessages();


    ImageCubeReadData( input_name, &imagecube );
    QccIMGImageCubeWrite( &imagecube );

//    QccIMGImageCubePrint( &imagecube );

    return 0;
}
