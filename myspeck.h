/*
 * This program aims to provide a modified, but easier API to use the speck encoder.
 *  
 * Programmer: Samuel Li
 * Date: 7/21/2015
 *
 */

#ifndef _MYSPECK_
#define _MYSPECK_

#include "libQccPack.h"


/*
 * Fill content in a float array into a QccIMGImageCube structure.
 *
 * Input:
 *      buf     : data to fill. 
 *      X, Y, Z : dimensions of the 3D volume.
 *
 * Output:
 *      imagecube : the QccIMGImageCube structure to fill
 *      
 * Note: buf should have length X*Y*Z.
 */
void FillImageCube( float* buf, 
                    int X, int Y, int Z,
                    QccIMGImageCube* imagecube );

/*
 * Fill content in a float array into a QccIMGImageComponent structure.
 *
 * Input:
 *      buf     : data to fill. 
 *      X, Y    : dimensions of the 2D volume.
 *
 * Output:
 *      imagecomponent : the QccIMGImageComponent structure to fill
 *
 * Note: buf should have length X*Y.
 */
void FillImageComponent( float* buf, int X, int Y,
                         QccIMGImageComponent* imagecomponent );

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
void myspeckencode3d( float* srcBuf, 
                   int srcX,
                   int srcY,
                   int srcZ,
                   char* outputFilename,
                   int nLevels,
                   float TargetRate );

/*
 * Similar to myspeckencode3d, 
 * but able to specify levels of DWT on XY plane and Z dimension separately.
 */
void myspeckencode2p1d( float* srcBuf, 
                   int srcX,
                   int srcY,
                   int srcZ,
                   char* outputFilename,
                   int XYNumLevels,
                   int ZNumLevels,
                   float TargetRate );

/*
 * Modified API to apply 3D SPECK decoding.
 *
 * Input: 
 *  inputFilename: the name of the input bitstream file.
 *  
 * Output:
 *  dstBuf:  buffer of floating points to keep the decoded data.
 *  outSize: size of the output buffer (in number of floats)
 *
 */
void myspeckdecode3d( char*  inputFilename,
                     float* dstBuf,
                     int    outSize );
                 

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
void myspeckencode2d( float* srcBuf, 
                       int srcX,
                       int srcY,
                       char* outputFilename,
                       int nLevels,
                       float TargetRate );

/*
 * Modified API to apply 2D SPECK decoding.
 *
 * Input: 
 *  inputFilename: the name of the input bitstream file.
 *  
 * Output:
 *  dstBuf  :  buffer of floating points to keep the decoded data.
 *  outSize : size of the output buffer (in number of floats)
 *
 */
void myspeckdecode2d( char*  inputFilename,
                     float* dstBuf,
                     int    outSize );

/*
 * Evaluates two arrays using RMS and LMax criteria.
 */
void evaluate2arrays( float* A, 
                      float* B, 
                      int len, 
                      double* rms, 
                      double* lmax );

#endif

