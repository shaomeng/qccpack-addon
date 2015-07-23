#include <cstdio>
#include <iostream>
#include <cassert>
#include "vapor/NetCDFSimple.h"
#include "vapor/NetCDFCollection.h"
#include "vapor/Compressor.h"
#include "netcdf.h"

#ifndef _CAMHANDLER_H_
#define _CAMHANDLER_H_

namespace VAPoR
{

class CamHandler
{
    public:
    CamHandler( string &mapfile, string &facefile );

    /*
     * Converts a data array in "homme" order into the raw order.
     * Input: 
     *      homme_orig_buf:  1D array containing homme ordered data points.
     * Output:
     *      orig_buf:        1D array containing all data points in raw order.
     *
     * Note: both homme_orig_buf and orig_buf should have the size 6*_NX*_NY*_NZ.
     */
    void cam2raw( float* homme_orig_buf,                
                  float* orig_buf );

    /*
     * Converts an data array in raw order into the "homme" order.
     * Input: 
     *      orig_buf:        1D array containing raw ordered data points.
     * Output:
     *      homme_orig_buf:  1D array containing all data points in homme order.
     *
     * Note: both orig_buf and homme_orig_buf should have the size 6*_NX*_NY*_NZ.
     */
    void raw2cam( float* orig_buf,                
                  float* homme_orig_buf );

    int  speckEncode3D( float* srcBuf,
                        char* outputFilename,
                        int numLevels,
                        float targetRate );
                        
        
    int  speckEncode2Dp1D( float* srcBuf,
                           char* outputFilename,
                           int XYNumLevels,
                           int ZNumLevels,
                           float targetRate );

    int speckdecode( char*  inputFilename,
                     float* dstBuf );

    void evaluate2arrays( float* A, 
                          float* B, 
                          int len, 
                          double* rms,
                          double* lmax );
                        
    private:
    size_t _NX, _NY, _ILEV, _NCOL;
    /*
     * Auxiliary data structure from InitializeFaceIndeciesAll().
     */
    vector < vector <int> > _faceIndicesAll;
    int InitializeFaceIndicesAll( string &mapfile, 
                                  string &facefile );
    


};  // finish class CamHandler

};  // finish namespace VAPoR

#endif
