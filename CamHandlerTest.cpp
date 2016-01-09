/* 
 * This program tests the CamHandler class.
 *
 * Programmer: Samuel Li
 * Date: 7/26/2015
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "CamHandler.h"

const size_t _NX = 91;
const size_t _NY = 91;
const size_t _NCOL = 48602;

//using namespace VAPoR;

int ReadAscii( char* filename, float* buf, size_t len )
{
    std::ifstream ifs( filename, std::ifstream::in );
    char arr[256];

    size_t i = 0;
    for( i = 0; i < len && ifs.getline( arr, 256 ); i++ )
        buf[i] = (float) atof( arr );
    
    ifs.close();

    if( i != len )
    {
        cerr << "file length doesn't match buffer size! " << endl;
        return 1;
    }
    
    return 0;
}


int main( int argc, char* argv[] )
{
    if( argc != 2 )
    {
        std::cerr << "please specify target bit rate! " << endl;
        exit (1);
    }
    float targetRate = atof( argv[1] );

    char asciiInput[] = "./HommeMap/FSDSC";
    char filename[] = "./dump/bit.stream";
    string hommeMap   = "./HommeMap/reorderedCbasedHommeMapping.nc";
    string faceMap    = "./HommeMap/faceIdsNeNp304.nc";

    size_t LEV = 1;
    int numXYDWTLevels = 4;
    int numZDWTLevels = 2;

    size_t homme_size = _NCOL * LEV;
    float* homme_buf = new float[ homme_size ];
    cerr << "start reading input... " << endl;
    ReadAscii( asciiInput, homme_buf, homme_size );

    cerr << "start building map... " << endl;
    VAPoR::CamHandler handler( hommeMap, faceMap );

    float* homme_buf_comp = new float[ homme_size ];

/*
    cerr << "start 3D DWT and SPECK encoding... " << endl;
    handler.speckEncode3D( homme_buf, homme_size, LEV,
                           numZDWTLevels, targetRate, filename );
*/

/*
    cerr << "start 2D+1D DWT and SPECK encoding... " << endl;
    handler.speckEncode2Dp1D( homme_buf, homme_size, LEV,
                           numXYDWTLevels, numZDWTLevels, targetRate, filename );


    cerr << "start 3D decoding... " << endl;
    handler.speckDecode3D( filename, homme_size, LEV, homme_buf_comp ); 
*/


    cerr << "start 2D SPECK encoding... " << endl;
    handler.speckEncode2D( homme_buf, homme_size, 
                           numXYDWTLevels, targetRate, filename );
    cerr << "start 2D decoding... " << endl;
    handler.speckDecode2D( filename, homme_size, homme_buf_comp );


    
    double rmse, lmax, nrmse, nlmax, minA, maxA, minB, maxB, meanA, meanB;
    handler.evaluate2arrays( homme_buf, homme_buf_comp, homme_size, 
                             &rmse, &lmax, 
                             &nrmse, &nlmax, 
                             &minA, &maxA,
                             &minB, &maxB,
                             &meanA, &meanB );

    cerr << "1st array min = " << minA << ", max = " << maxA << ", mean = " << meanA << endl;
    cerr << "2nd array min = " << minB << ", max = " << maxB << ", mean = " << meanB << endl;
    cerr << "RMS  = " << rmse << ",  LMax  = " << lmax << endl;
    cerr << "NRMS = " << nrmse << ", NLMax = " << nlmax << endl;

    double Ai, Bi;
    double max = 0.0;
    int idx = 0;
    for( int i = 0; i < homme_size; i++ ) {
        double tmp = fabs( homme_buf[i] - homme_buf_comp[i] );
        if( tmp > max ) {
            idx = i;
            max = tmp;
            Ai = homme_buf[i];
            Bi = homme_buf_comp[i];
        }
    }
    printf("\nAt index %d, (%e) - (%e) = (%e)\n", idx, Ai, Bi, max );
        

    delete[] homme_buf;
    delete[] homme_buf_comp;
}
