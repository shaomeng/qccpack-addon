/*
 * This program aims to provide a modified, but easier API to use the spiht encoder.
 *  
 * Programmer: Samuel Li
 * Date: 7/16/2015
 *
 */

#ifndef _MYSPIHT_
#define _MYSPIHT_

#include "libQccPack.h"

/*
 * Modified API to apply SPIHT encoding.
 *
 * Input: 
 *  srcBuf: pointer to 1D array of all data points in float.
 *  srcX  : X dimension of src data
 *  srcY  : Y dimension of src data
 *  srcZ  : Z dimension of src data
 *  nLevels: how many levels of wavelet transforms to perform.
 *  
 * Output:
 *  outputFilename: the name of the file to write the output bitstream.
 *
 * Note: even though the input data is in 1D array, it represents a 3D volume.
 *       Thus, the size of srcBuf should equal to srcX x srcY x srcZ.
 */
int spihtencode( float* srcBuf, 
                 int srcX,
                 int srcY,
                 int srcZ,
                 char* outputFilename,
                 short nLevels );


/*
 * Modified API to apply SPIHT decoding.
 *
 * Input: 
 *  inputFilename: the name of the input bitstream file.
 *  
 * Output:
 *  dstBuf:  buffer of floating points to keep the decoded data.
 *  outSize: size of the output buffer (in byte)
 *
 */
int spihtdecode( char*  inputFilename,
                 float* dstBuf,
                 int    outSize );
                 

#endif

