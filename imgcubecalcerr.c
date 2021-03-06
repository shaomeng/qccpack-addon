/* This program reads two image cubes in ICB format,
 * and calculate Min, Max, RMSE, LMAX between the two.
 *
 * Programmer: Samuel Li
 * Date: 8/4/2015
 */

#include "libQccPack.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define USG_STRING "%s:input_name_1 %s:input_name_2"

#define FLOAT double

void ImageCube2Arr( QccIMGImageCube* image_cube, FLOAT* buf, size_t len )
{
    size_t npxl = (image_cube->num_cols) * (image_cube->num_rows) * (image_cube->num_frames);
    assert( npxl == len );

    size_t idx = 0;
    int frame, row, col;
    for (frame = 0; frame < image_cube->num_frames; frame++)
        for (row = 0; row < image_cube->num_rows; row++)
            for (col = 0; col < image_cube->num_cols; col++)
                buf[ idx++ ] = (FLOAT)image_cube -> volume[frame][row][col];
}

void Evaluate2Arrays( const FLOAT* A, const FLOAT* B, size_t len, 
                      double* minmaxA, double* minmaxB, 
                      double* rms, double* nrmse, 
                      double* lmax, double* nlmax,
                      double* meanA, double* meanB )
{
    double sum = 0.0;
    double c = 0.0;
    double max = 0.0;
    double tmp;
    size_t i;
    double minA = A[0];
    double maxA = A[0];
    double minB = B[0];
    double maxB = B[0];
	double sum_A = 0.0;
	double sum_B = 0.0;
    for( i = 0; i < len; i++)
	{
		sum_A += A[i];	
		sum_B += B[i];	
        tmp = A[i] - B[i];
        if (tmp < 0)        tmp *= -1.0;
        if (tmp > max)      max = tmp;
        double y = tmp * tmp - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;

        /* Collect min, max */
        if( A[i] < minA )   minA = A[i];
        if( A[i] > maxA )   maxA = A[i];
        if( B[i] < minB )   minB = B[i];
        if( B[i] > maxB )   maxB = B[i];
    }
    sum /= (double)len;
    sum = sqrt( sum );

    *rms = sum;
    *lmax = max;
    minmaxA[0] = minA;
    minmaxA[1] = maxA;
    minmaxB[0] = minB;
    minmaxB[1] = maxB;

    *nrmse = sum / (maxA - minA);
    *nlmax = max / (maxA - minA);

	*meanA = sum_A / (double)len;
	*meanB = sum_B / (double)len;
}

int main (int argc, char* argv[] )
{
    QccIMGImageCube cube1, cube2;
    QccIMGImageCubeInitialize( &cube1 );
    QccIMGImageCubeInitialize( &cube2 );

    if( QccParseParameters( argc, argv, USG_STRING, cube1.filename, cube2.filename ))
        QccErrorExit();

    if( QccIMGImageCubeRead( &cube1 ) ) {
        QccErrorAddMessage( "%s: Read input imagecube failed: %s", argv[0], argv[1] );
        QccErrorExit();
    }
    if( QccIMGImageCubeRead( &cube2 ) ) {
        QccErrorAddMessage( "%s: Read input imagecube failed: %s", argv[0], argv[2] );
        QccErrorExit();
    }
    assert( cube1.num_frames == cube2.num_frames );
    assert( cube1.num_rows == cube2.num_rows );
    assert( cube1.num_cols == cube2.num_cols );

    size_t npxl = cube1.num_cols * cube1.num_rows * cube1.num_frames;
    FLOAT* A = (FLOAT*) malloc( sizeof(FLOAT) * npxl );
    FLOAT* B = (FLOAT*) malloc( sizeof(FLOAT) * npxl );
    ImageCube2Arr( &cube1, A, npxl );
    ImageCube2Arr( &cube2, B, npxl );

    double minmaxA[2], minmaxB[2];
    double rms, nrms, lmax, nlmax, meanA, meanB;
    Evaluate2Arrays( A, B, npxl, minmaxA, minmaxB, 
                     &rms, &nrms, &lmax, &nlmax, &meanA, &meanB );
    printf("\t%s: min = %e, max = %e\n", cube1.filename, minmaxA[0], minmaxA[1] );
    printf("\t%s: min = %e, max = %e\n", cube2.filename, minmaxB[0], minmaxB[1] );
    printf("\tRMS = %e, LMAX = %e\n", rms, lmax );
    printf("\tNRMS = %e, NLMAX = %e\n", nrms, nlmax );
	printf("\tThe two files have mean values %.8e, %.8e.\n", meanA, meanB );
	printf("\tTheir difference is: %.8e.\n", (meanA - meanB));

	printf("True Values:\t\tReconstructed:\n");
	size_t i = 0;
	for(i = 0; i < npxl; i++ )
		printf("%e,\t%e\n", A[i], B[i] );


    free( A );
    free( B );
    QccIMGImageCubeFree( &cube1 );
    QccIMGImageCubeFree( &cube2 );

    return 0;
}
