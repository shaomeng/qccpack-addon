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
 * Fill an 1D array with values from a QccIMGImageCube.
 *
 * Input: 
 *      imagecube   : source of the values
 *      size        : size of the output array
 *
 * Output:
 *      buf         : 1D array to be filled.
 *
 * Note: there are two versions handling 32-bit and 64-bit 
 * floating points accordingly.
 */
/*
void ImageCube2Array( QccIMGImageCube* imagecube,
                      float* buf, size_t size );
void ImageCube2Array_64bit( QccIMGImageCube* imagecube,
                            double* buf, size_t size );
*/

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
static void FillImageCube( const float* buf, 
                    int X, int Y, int Z,
                    QccIMGImageCube* imagecube );
static void FillImageCube_64bit( const double* buf, 
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
static void FillImageComponent( const float* buf, int X, int Y,
                         QccIMGImageComponent* imagecomponent );
static void FillImageComponent_64bit( const double* buf, int X, int Y,
                               QccIMGImageComponent* imagecomponent );

/*
 * Find the smallest non-zero absolutely value: minfabs.
 * If zero exists in the input array, writes minfabs to file.
 * If no zero exists, writes -1.
 *
 * Note: the saved values is always in 64bit.
 *
 * Input:
 *		buf			: array to examine
 *		len			: length of the input array
 * Output:
 *		filename	: filename to write onto disk
 */
static void WriteMinPos_32bit( const float* buf,
                             long len,
                             const char* filename );
static void WriteMinPos_64bit( const double* buf,
                             long len,
                             const char* filename );

/*
 * Reads a single floating point from disk.
 *
 * Input:
 *		filename	: filename to write onto disk
 * Output:
 *		val			: value to read
 */
static void ReadDouble( const char* filename,
                        double* val );

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
void myspeckencode3d( const float* srcBuf, 
                   int srcX,
                   int srcY,
                   int srcZ,
                   const char* outputFilename,
                   int nLevels,
                   float TargetRate );
void myspeckencode3d_64bit( const double* srcBuf, 
                           int srcX,
                           int srcY,
                           int srcZ,
                           const char* outputFilename,
                           int nLevels,
                           float TargetRate );

/*
 * This is the actual function that performs DWT and SPECK encoding.
 * It should only be called by myspeckencode3d() and myspeckencode3d_64bit().
 */
static void encode3d( QccIMGImageCube* imagecube,
               const char* outputFilename,
               int nLevels,
               float TargetRate );

/*
 * Similar to myspeckencode3d, 
 * but able to specify levels of DWT on XY plane and Z dimension separately.
 */
void myspeckencode2p1d( const float* srcBuf, 
                       int srcX,
                       int srcY,
                       int srcZ,
                       const char* outputFilename,
                       int XYNumLevels,
                       int ZNumLevels,
                       float TargetRate );
void myspeckencode2p1d_64bit( const double* srcBuf, 
                              int srcX,
                              int srcY,
                              int srcZ,
                              const char* outputFilename,
                              int XYNumLevels,
                              int ZNumLevels,
                              float TargetRate );

/*
 * This is the actual function that performs DWT and SPECK encoding.
 * It should only be called by myspeckencode2p1d() and myspeckencode2p1d_64bit().
 */
static void encode2p1d( QccIMGImageCube* imagecube,
                 const char* outputFilename,
                 int XYNumLevels,
                 int ZNumLevels,
                 float TargetRate );

/*
 * Modified API to apply 3D SPECK decoding.
 *
 * Input: 
 *  inputFilename: the name of the input bitstream file.
 *  outSize: size of the output buffer (in number of floats)
 *  
 * Output:
 *  dstBuf:  buffer of floating points to keep the decoded data.
 *
 */
void myspeckdecode3d( const char*  inputFilename,
                      float* dstBuf,
                      int    outSize );
void myspeckdecode3d_64bit( const char*  inputFilename,
                            double* dstBuf,
                            int     outSize );
                 

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
void myspeckencode2d( const float* srcBuf, 
                      int srcX,
                      int srcY,
                      const char* outputFilename,
                      int nLevels,
                      float TargetRate );
void myspeckencode2d_64bit( const double* srcBuf, 
                            int srcX,
                            int srcY,
                            const char* outputFilename,
                            int nLevels,
                            float TargetRate );

/*
 * Helper function that actually does the DWT and SPECK encoding.
 * It should be called only by myspeckencode2d() and myspeckencode2d_64bit().
 */
static void encode2d( QccIMGImageComponent* imagecomponent,
               const char* outputFilename,
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
void myspeckdecode2d( const char*  inputFilename,
                      float* dstBuf,
                      int    outSize );
void myspeckdecode2d_64bit( const char*  inputFilename,
                            double* dstBuf,
                            int     outSize );

/*
 * Evaluates two arrays using RMS and LMax criteria.
 * CamHandler.cpp has a version of evaluation that 
 * also returns min, max, normalized errors.
 */
void evaluate2arrays( const float* A, 
                      const float* B, 
                      int len, 
                      double* rms, 
                      double* lmax );
void evaluate2arrays_64bit( const double* A, 
                            const double* B, 
                            int len, 
                            double* rms, 
                            double* lmax );

#endif

