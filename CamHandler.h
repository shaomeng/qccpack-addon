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
/*
 * Converts a data array in "homme" order into the raw order.
 * Input: 
 *      faceIndeciesAll: auxiliary data structure from InitializeFaceIndeciesAll().
 *      homme_orig_buf:  1D array containing homme ordered data points.
 *      NX, NY, NZ:      dimension of the input data
 * Output:
 *      orig_buf:        1D array containing all data points in raw order.
 *
 * Note: both homme_orig_buf and orig_buf should have the size 6*NX*NY*NZ.
 */
void cam2raw( const vector < vector <int> > &faceIndeciesAll,
	         float* homme_orig_buf,                
             int    NX,
             int    NY,
             int    NZ,
	         float* orig_buf );

/*
 * Converts an data array in raw order into the "homme" order.
 * Input: 
 *      faceIndeciesAll: auxiliary data structure from InitializeFaceIndeciesAll().
 *      orig_buf:        1D array containing raw ordered data points.
 *      NX, NY, NZ:      dimension of the input data
 * Output:
 *      homme_orig_buf:  1D array containing all data points in homme order.
 *
 * Note: both orig_buf and homme_orig_buf should have the size 6*NX*NY*NZ.
 */
void raw2cam( const vector < vector <int> > &faceIndeciesAll,
	         float* orig_buf,                
             int    NX,
             int    NY,
             int    NZ,
	         float* homme_orig_buf );

    
int InitializeFaceIndeciesAll( string &mapfile,
                                string &facefile,   
                                vector < vector <int> > &faceIndeciesAll );


};  // finish class CamHandler

};  // finish namespace VAPoR

#endif
