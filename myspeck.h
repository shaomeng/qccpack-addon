/*
 * This program aims to provide a modified, but easier API to use the speck encoder.
 *  
 * Programmer: Samuel Li
 * Date: 7/21/2015
 *
 */

#ifndef _MYSPECK
#define _MYSPIHT_

#include "libQccPack.h"

/*
 * Modified API to apply SPECK encoding.
 *
 * Input: 
 *  srcBuf: pointer to 1D array of all data points in float.
 *  srcX  : X dimension of src data
 *  srcY  : Y dimension of src data
 *  srcZ  : Z dimension of src data
 *  nLevels: how many levels of wavelet transforms to perform.
 *  TargetRate: how many bits to allocate to one voxel. 32 means no compression.
 *  
 * Output:
 *  outputFilename: the name of the file to write the output bitstream.
 *
 * Note: even though the input data is in 1D array, it represents a 3D volume.
 *       Thus, the size of srcBuf should equal to srcX * srcY * srcZ.
 */
int myspeckencode3d( float* srcBuf, 
                   int srcX,
                   int srcY,
                   int srcZ,
                   char* outputFilename,
                   int nLevels,
                   float TargetRate );

int myspeckencode2p1d( float* srcBuf, 
                   int srcX,
                   int srcY,
                   int srcZ,
                   char* outputFilename,
                   int XYNumLevels,
                   int ZNumLevels,
                   float TargetRate );

/*
 * Modified API to apply SPIHT decoding.
 *
 * Input: 
 *  inputFilename: the name of the input bitstream file.
 *  
 * Output:
 *  dstBuf:  buffer of floating points to keep the decoded data.
 *  outSize: size of the output buffer (in number of floats)
 *
 */
int myspeckdecode( char*  inputFilename,
                 float* dstBuf,
                 int    outSize );
                 
/*
 * Evaluates two arrays using RMS and LMax criteria.
 */
void evaluate2arrays( float* A, float* B, int len, double* rms, double* lmax );

#endif

