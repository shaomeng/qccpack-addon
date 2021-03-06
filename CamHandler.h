#include <cstdio>
#include <iostream>
#include <cassert>
#include "vapor/NetCDFSimple.h"
#include "vapor/NetCDFCollection.h"
#include "vapor/Compressor.h"
//#include "netcdf.h"

#ifndef _CAMHANDLER_H_
#define _CAMHANDLER_H_

namespace VAPoR
{

class CamHandler
{
    public:
    /* Constructor */
    CamHandler( string &mapfile, string &facefile );


    /*
     * Converts a data array in "homme" order into the raw order.
     * 
     * Note that the homme array contains 6 faces. So does orig array.
     *
     * Input: 
     *      homme_buf     :  1D array containing homme ordered data points.
     *      homme_size    :  size of the homme_orig_buf array
     *      LEV           :  number of levels (Z dimension) of cam data.
     *      orig_size     :  size of the orig_buf array
     * Output:
     *      orig_buf:        1D array containing all data points in raw order.
     *
     */
    void cam2raw( float* homme_buf,                
                  size_t homme_size,
                  int    LEV,
                  float* orig_buf,
                  size_t orig_size );

    /*
     * Converts an data array in raw order into the "homme" order.
     *
     * Note that the raw array contains 6 faces. So does homme array.
     * 
     * Input: 
     *      orig_buf      :   1D array containing raw ordered data points.
     *      orig_size     :   size of the orig_buf array
     *      homme_size    :   size of the homme_orig_buf array
     *      LEV           :   number of levels (Z dimension) of cam data.
     * Output:
     *      homme_buf     :   1D array containing all data points in homme order.
     *
     */
    void raw2cam( float* orig_buf,                
                  size_t orig_size,
                  float* homme_buf,
                  size_t homme_size,
                  int    LEV );


    /*
     * Takes an input homme array, converts to a raw array,
     * applies speck3d encoding with dyadic DWT, and writes to file.
     *
     * Note, don't include ".face1" part into the output filename.
     * 
     * Input:
     *      homme_buf       :   input homme array
     *      homme_size      :   size of input homme array
     *      LEV             :   number of levels (Z dimension) of cam data
     *      numDWTLevels    :   number of levels of DWT to apply
     *      targetRate      :   target rate for speck encoding
     * Output:
     *      outputFilename  :   output file name for compressed data.
     */
    void  speckEncode3D( float* homme_buf,
                        size_t homme_size,
                        int LEV,
                        int numDWTLevels,
                        float targetRate,
                        char* outputFilename );
                        
        
    /*
     * Takes an input homme array, converts to a raw array,
     * applies speck3d encoding with wavelet-packet DWT, and writes to file.
     *
     * Note, don't include ".face1" part into the output filename.
     * 
     * Input:
     *      homme_buf       :   input homme array
     *      homme_size      :   size of input homme array
     *      LEV             :   number of levels (Z dimension) of cam data
     *      XYnumDWTLevels  :   number of levels of DWT to apply on XY plane
     *      ZnumDWTLevels   :   number of levels of DWT to apply on Z dimension 
     *      targetRate      :   target rate for speck encoding
     * Output:
     *      outputFilename  :   output file name for compressed data.
     */
    void  speckEncode2Dp1D( float* homme_buf,
                           size_t homme_size,
                           int LEV,
                           int XYNumDWTLevels,
                           int ZNumDWTLevels,
                           float targetRate,
                           char* outputFilename );


    /*
     * Takes an input homme array, converts to a raw array,
     * applies speck2d encoding with dyadic DWT, and writes to file.
     *
     * Note, don't include ".face1" part into the output filename.
     * 
     * Input:
     *      homme_buf       :   input homme array
     *      homme_size      :   size of input homme array
     *      numDWTLevels    :   number of levels of DWT to apply
     *      targetRate      :   target rate for speck encoding
     * Output:
     *      outputFilename  :   output file name for compressed data.
     */
    void speckEncode2D( float* homme_buf,
                        size_t homme_size,
                        int numDWTLevels,
                        float targetRate,
                        char* outputFilename );
                        
	void speckEncodeMany2D( float* homme_buf,
							size_t homme_size,
							int LEV,
							int XYNumDWTLevels,
							float targetRate,
							char* outputFilename );

    /*
     * Reads in SIX speck encoded files, performs 3D decoding into a raw array, 
     * converts into a cam array, and return.
     *
     * Note, don't include ".face1" part into the input filename.
     * 
     * Input:
     *      inputFilename   :   prefix of input files. 
     *      homme_size      :   size of output homme array.
     *      LEV             :   number of levels (Z dimension) of cam data
     * Output:
     *      homme_buf       :   reconstructed homme array.
     */
    void speckDecode3D( char*  inputFilename,
                        size_t homme_size,
                        int LEV,
                        float* homme_buf );

    void speckDecodeMany2D( char*  inputFilename,
							size_t homme_size,
							int LEV,
							float* homme_buf );

    /*
     * Reads in SIX speck encoded files, performs 2D decoding into a raw array, 
     * converts into a cam array, and return.
     *
     * Note, don't include ".face1" part into the input filename.
     * 
     * Input:
     *      inputFilename   :   prefix of input files. 
     *      homme_size      :   size of output homme array.
     * Output:
     *      homme_buf       :   reconstructed homme array.
     */
    void speckDecode2D( char*  inputFilename,
                        size_t homme_size,
                        float* homme_buf );


    /*
     * Evaluates two arrays of the same size.
     * Input:
     *      A, B            :   Two arrays to compare with
     *      len             :   length of the two arrays (same length)
     * Output:
     *      minA         
     *      maxA         
     *      minB         
     *      maxB         
     *      meanA         
     *      meanB         
	 *		lmaxA			: 	value from A that results in the lmax
	 *		lmaxB			: 	value from B that results in the lmax
     *      rms             :   absolute RMSE between the two.
     *      nrms            :   normalized RMSE (normalized by the range of A)
     *      lmax            :   maximum difference between the two.
     *      nlmax           :   normalized LMAX (normalized by the range of A)
	 *	 	maxRE			: 	max relative error (relative to that value of A)	
     */
    template <typename T> inline
    void evaluate2arrays( const T* A,    const T* B, size_t len,
                          double* rmse,  double* lmax, 
                          double* nrmse, double* nlmax, 
                          double* minA,  double* maxA, 
                          double* minB,  double* maxB,
						  double* meanA, double* meanB,
						  double* lmaxA, double* lmaxB,
						  double* maxRE )
{
    double sum = 0.0;
    double c = 0.0;
    double max = 0.0;
	double mean_a = 0;
	double mean_b = 0;
    double tmp;
    size_t i;
    *minA = A[0];
    *maxA = A[0];
    *minB = B[0];
    *maxB = B[0];
	*maxRE = 0.0;
    for( i = 0; i < len; i++) {
		mean_a += A[i];
		mean_b += B[i];
        tmp = (double)A[i] - (double)B[i];
        double y = tmp * tmp - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;

        /* Collect min, max */
        if (tmp < 0)        tmp *= -1.0;
        if (tmp > max) 
		{      
			max = tmp;
			*lmaxA = A[i];
			*lmaxB = B[i];
		}
		if( (A[i] != 0) && fabs(tmp / A[i]) > *maxRE )
			*maxRE = fabs(tmp / A[i]);
        if( A[i] < *minA )   *minA = A[i];
        if( A[i] > *maxA )   *maxA = A[i];
        if( B[i] < *minB )   *minB = B[i];
        if( B[i] > *maxB )   *maxB = B[i];
    }
    sum /= (double)len;
    mean_a /= (double)len;
    mean_b /= (double)len;

    *rmse = sqrt( sum );
    *lmax = max;
    *nrmse = *rmse / (*maxA - *minA);
    *nlmax = max   / (*maxA - *minA);
	*meanA = mean_a;
	*meanB = mean_b;
}

	/* 
	 * Only performs SPECK encoding, but not including wavelets.
	 * This function is intended for research only.
	 */
	void Speck3DEncodeOnly( double* coeff_buf,
				  long nx, long ny, long nz,
				  double image_mean,
				  int num_levels,
				  int target_bit_cnt,
				  const char* output_name );


/* 
 * super simple functions for test use only
 */
    void testencode2d( const float* srcBuf, int srcX, int srcY,
                       const char* outputFilename, int nLevels, float TargetRate );
    void testdecode2d( const char* inputFilename, float* dstBuf, int outSize );
                        
    private:
    size_t _NX, _NY,  _NCOL;
    /*
     * Auxiliary data structure from InitializeFaceIndeciesAll().
     */
    vector < vector <int> > _faceIndicesAll;
    int InitializeFaceIndicesAll( string &mapfile, 
                                  string &facefile );
    

};  // finish class CamHandler

};  // finish namespace VAPoR

#endif
