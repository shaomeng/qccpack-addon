#include "myspeck.h"

int myspeckencode3d( float* srcBuf,
                 int srcX,
                 int srcY,
                 int srcZ,
                 char* outputFilename,
                 int nLevels,
                 float TargetRate )
{
    /*
     * Creates a QccIMGImageCube struct to hold the input data.
     */
    QccIMGImageCube imagecube;
    QccIMGImageCubeInitialize( &imagecube );
    imagecube.num_cols = srcX;
    imagecube.num_rows = srcY;
    imagecube.num_frames = srcZ;
    if( QccIMGImageCubeAlloc( &imagecube ) )
        QccErrorPrintMessages();

    double min = MAXDOUBLE;
    double max = -MAXDOUBLE;
    int frame, row, col;
    long idx = 0;
    /*
     * Presuming the X dimension varies fastest, then Y, and then Z.
     */
    for( frame = 0; frame < imagecube.num_frames; frame++ )
        for( row = 0; row < imagecube.num_rows; row++ )
            for( col = 0; col < imagecube.num_cols; col++ )
            {
                if( srcBuf[idx] < min )         min = srcBuf[idx];
                if( srcBuf[idx] > max )         max = srcBuf[idx];
                imagecube.volume[frame][row][col]   = srcBuf[idx];
                idx++;
            }
    imagecube.min_val = min;
    imagecube.max_val = max;

    /*
     * Sets up parameters for DWT and SPECK encoding.
     */
    int TransformType = QCCWAVSUBBANDPYRAMID3D_DYADIC;
    int SpatialNumLevels = nLevels;
    int TemporalNumLevels = nLevels;
    QccWAVWavelet Wavelet;
    QccWAVWaveletInitialize(&Wavelet);
    QccString WaveletFilename = QCCWAVWAVELET_DEFAULT_WAVELET;
    QccString Boundary = "symmetric";
    if (QccWAVWaveletCreate(&Wavelet, WaveletFilename, Boundary)) 
    {
      QccErrorAddMessage("Error calling QccWAVWaveletCreate()");
      QccErrorExit();
    }    

    /*
     * Dimension check.
     */
    int ImageNumFrames = imagecube.num_frames;
    int ImageNumRows = imagecube.num_rows;
    int ImageNumCols = imagecube.num_cols;
    int NumPixels = ImageNumFrames * ImageNumRows * ImageNumCols;
    long long int pxlcount = (long long int)ImageNumFrames * ImageNumRows * ImageNumCols;
    if( pxlcount > INT_MAX ) 
    {
        QccErrorAddMessage("NumPixels overflow. Please try smaller data sets.");
        QccErrorExit();
    }

    /*
     * Prepare QccBitButter and encode.
     */
    QccBitBuffer OutputBuffer;
    QccBitBufferInitialize(&OutputBuffer);
    QccStringCopy( OutputBuffer.filename, outputFilename );
    OutputBuffer.type = QCCBITBUFFER_OUTPUT;
    if (QccBitBufferStart(&OutputBuffer))
    {
        QccErrorAddMessage("Error calling QccBitBufferStart()");
        QccErrorExit();
    }
    int TargetBitCnt = (int)(ceil((NumPixels * TargetRate)/8.0))*8;
    long long int bitcount = (long long int)(ceil((NumPixels * TargetRate)/8.0))*8;
    if( bitcount > INT_MAX ) 
    {
        QccErrorAddMessage("TargetBitCnt overflow. Please try smaller data sets.");
        QccErrorExit();
    }
    if ( QccSPECK3DEncode( &imagecube, 
                           NULL, 
                           TransformType, 
                           TemporalNumLevels, 
                           SpatialNumLevels, 
                           &Wavelet, 
                           &OutputBuffer, 
                           TargetBitCnt ) )
    {
      QccErrorAddMessage("Error calling QccSPECK3DEncode()");
      QccErrorExit();
    }

    /*
     * Finish up
     */
    float ActualRate = (double)OutputBuffer.bit_cnt / NumPixels;
    if (QccBitBufferEnd(&OutputBuffer)) 
    {
        QccErrorAddMessage("Error calling QccBitBufferEnd()" );
        QccErrorExit();
    }
    QccIMGImageCubeFree( &imagecube );
    QccWAVWaveletFree( &Wavelet );

    /* 
     * print out info
     */
    /*
    printf("3D-SPECK encoding to output file: %s:\n", outputFilename );
    printf("  Target rate: %f bpv\n", TargetRate);
    printf("  Actual rate: %f bpv\n", ActualRate);
    */


    return 0;
}

int myspeckencode2p1d( float* srcBuf,
                 int srcX,
                 int srcY,
                 int srcZ,
                 char* outputFilename,
                 int XYNumLevels,
                 int ZNumLevels,
                 float TargetRate )
{
    /*
     * Creates a QccIMGImageCube struct to hold the input data.
     */
    QccIMGImageCube imagecube;
    QccIMGImageCubeInitialize( &imagecube );
    imagecube.num_cols = srcX;
    imagecube.num_rows = srcY;
    imagecube.num_frames = srcZ;
    if( QccIMGImageCubeAlloc( &imagecube ) )
        QccErrorPrintMessages();

    double min = MAXDOUBLE;
    double max = -MAXDOUBLE;
    int frame, row, col;
    long idx = 0;
    /*
     * Presuming the X dimension varies fastest, then Y, and then Z.
     */
    for( frame = 0; frame < imagecube.num_frames; frame++ )
        for( row = 0; row < imagecube.num_rows; row++ )
            for( col = 0; col < imagecube.num_cols; col++ )
            {
                if( srcBuf[idx] < min )         min = srcBuf[idx];
                if( srcBuf[idx] > max )         max = srcBuf[idx];
                imagecube.volume[frame][row][col]   = srcBuf[idx];
                idx++;
            }
    imagecube.min_val = min;
    imagecube.max_val = max;

    /*
     * Sets up parameters for DWT and SPECK encoding.
     * More details could be found in the QccPack documentation.
     */
    int TransformType = QCCWAVSUBBANDPYRAMID3D_PACKET;
    int SpatialNumLevels = XYNumLevels;
    int TemporalNumLevels = ZNumLevels;
    QccWAVWavelet Wavelet;
    QccWAVWaveletInitialize(&Wavelet);
    QccString WaveletFilename = QCCWAVWAVELET_DEFAULT_WAVELET;
    QccString Boundary = "symmetric";
    if (QccWAVWaveletCreate(&Wavelet, WaveletFilename, Boundary)) 
    {
      QccErrorAddMessage("Error calling QccWAVWaveletCreate()");
      QccErrorExit();
    }    

    int ImageNumFrames = imagecube.num_frames;
    int ImageNumRows = imagecube.num_rows;
    int ImageNumCols = imagecube.num_cols;
    int NumPixels = ImageNumFrames * ImageNumRows * ImageNumCols;
    long long int pxlcount = (long long int)ImageNumFrames * ImageNumRows * ImageNumCols;
    if( pxlcount > INT_MAX ) 
    {
        QccErrorAddMessage("NumPixels overflow. Please try smaller data sets.");
        QccErrorExit();
    }

    /*
     * Prepare QccBitButter and encode.
     */
    QccBitBuffer OutputBuffer;
    QccBitBufferInitialize(&OutputBuffer);
    QccStringCopy( OutputBuffer.filename, outputFilename );
    OutputBuffer.type = QCCBITBUFFER_OUTPUT;
    if (QccBitBufferStart(&OutputBuffer))
    {
        QccErrorAddMessage("Error calling QccBitBufferStart()");
        QccErrorExit();
    }
    int TargetBitCnt = (int)(ceil((NumPixels * TargetRate)/8.0))*8;
    long long int bitcount = (long long int)(ceil((NumPixels * TargetRate)/8.0))*8;
    if( bitcount > INT_MAX ) {
        QccErrorAddMessage("TargetBitCnt overflow. Please try smaller data sets.");
        QccErrorExit();
    }
    if ( QccSPECK3DEncode( &imagecube, 
                           NULL, 
                           TransformType, 
                           TemporalNumLevels, 
                           SpatialNumLevels, 
                           &Wavelet, 
                           &OutputBuffer, 
                           TargetBitCnt ) )
    {
      QccErrorAddMessage("Error calling QccSPECK3DEncode()");
      QccErrorExit();
    }

    /*
     * Finish up
     */
    float ActualRate = (double)OutputBuffer.bit_cnt / NumPixels;
    if (QccBitBufferEnd(&OutputBuffer)) {
        QccErrorAddMessage("Error calling QccBitBufferEnd()" );
        QccErrorExit();
    }
    QccIMGImageCubeFree( &imagecube );
    QccWAVWaveletFree( &Wavelet );

    /*
     * Print out info
     */
    /*
    printf("3D-SPECK encoding to output file: %s:\n", outputFilename );
    printf("  Target rate: %f bpv\n", TargetRate);
    printf("  Actual rate: %f bpv\n", ActualRate);
    */

    return 0;
}


int myspeckdecode( char*  inputFilename,
                   float* dstBuf,
                   int    outSize )
{
    QccBitBuffer InputBuffer;
    QccBitBufferInitialize( &InputBuffer );
    QccStringCopy( InputBuffer.filename, inputFilename );
    InputBuffer.type = QCCBITBUFFER_INPUT;

    QccWAVWavelet Wavelet;
    QccWAVWaveletInitialize( &Wavelet );
    QccString WaveletFilename = QCCWAVWAVELET_DEFAULT_WAVELET;
    QccString Boundary = "symmetric";
    if (QccWAVWaveletCreate(&Wavelet, WaveletFilename, Boundary))
    {
      QccErrorAddMessage("Error calling QccWAVWaveletCreate()");
      QccErrorExit();
    }    
    if (QccBitBufferStart(&InputBuffer)) 
    {
        QccErrorAddMessage("Error calling QccBitBufferStart()" );
        QccErrorExit();
    }

    int TransformType;
    int TemporalNumLevels;
    int SpatialNumLevels;
    int NumFrames, NumRows, NumCols;
    double ImageMean;
    int MaxCoefficientBits;
    if (QccSPECK3DDecodeHeader( &InputBuffer, 
                                &TransformType, 
                                &TemporalNumLevels, 
                                &SpatialNumLevels,
                                &NumFrames, 
                                &NumRows, 
                                &NumCols,
                                &ImageMean, 
                                &MaxCoefficientBits ))
    {
      QccErrorAddMessage("Error calling QccSPECK3DDecodeHeader()");
      QccErrorExit();
    }

    int NumPixels = NumFrames * NumRows * NumCols;
    long long int pxlcount = (long long int)NumFrames * NumRows * NumCols;
    if( pxlcount > INT_MAX ) 
    {
        QccErrorAddMessage("NumPixels overflow. Please try smaller data sets.");
        QccErrorExit();
    }
    if( outSize != NumPixels ) 
    {
        QccErrorAddMessage("Decode output buffer size doesn't match signal length.");
        QccErrorExit();
    }    

    QccIMGImageCube imagecube;
    QccIMGImageCubeInitialize( &imagecube );
    imagecube.num_frames = NumFrames;
    imagecube.num_rows = NumRows;
    imagecube.num_cols = NumCols;
    if (QccIMGImageCubeAlloc(&imagecube) ) 
    {
      QccErrorAddMessage("Error calling QccIMGImageCubeAlloc()" );
      QccErrorExit();
    }
    int TargetBitCnt = QCCENT_ANYNUMBITS;

    if (QccSPECK3DDecode( &InputBuffer, 
                          &imagecube, 
                          NULL, 
                          TransformType, 
                          TemporalNumLevels, 
                          SpatialNumLevels, 
                          &Wavelet, 
                          ImageMean,
                          MaxCoefficientBits, 
                          TargetBitCnt ))
    {
        QccErrorAddMessage("Error calling QccSPIHT3DDecode()" );
        QccErrorExit();
    }
    if (QccBitBufferEnd(&InputBuffer)) 
    {
      QccErrorAddMessage("Error calling QccBitBufferEnd()" );
      QccErrorExit();
    }

    int idx = 0;
    int frame, row, col;
    for( frame = 0; frame < imagecube.num_frames; frame++ )
        for( row = 0; row < imagecube.num_rows; row++ )
            for( col = 0; col < imagecube.num_cols; col++ )
                dstBuf[ idx++ ] = imagecube.volume[frame][row][col];        

    QccIMGImageCubeFree( &imagecube );
    QccWAVWaveletFree( &Wavelet );

    return 0;
}


void evaluate2arrays( float* A, float* B, int len, double* rms, double* lmax )
{
    double sum = 0.0;
    double c = 0.0;
    double max = 0.0;
    double tmp;
    int i;
    for( i = 0; i < len; i++) {
        tmp = (double)A[i] - (double)B[i];
        if (tmp < 0)        tmp *= -1.0;
        if (tmp > max)      max = tmp;
        double y = tmp * tmp - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    sum /= (double)len;
    sum = sqrt( sum );

    *rms = sum;
    *lmax = max;
}
