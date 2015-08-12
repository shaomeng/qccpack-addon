#include "myspeck.h"

void FillImageCube( const float* buf,
                    int X, int Y, int Z,
                    QccIMGImageCube* imagecube )
{
    QccIMGImageCubeInitialize( imagecube );
    imagecube -> num_cols = X;
    imagecube -> num_rows = Y;
    imagecube -> num_frames = Z;
    if( QccIMGImageCubeAlloc( imagecube ) )
        QccErrorPrintMessages();

    double min = MAXDOUBLE;
    double max = -MAXDOUBLE;
    int frame, row, col;
    long idx = 0;
    /*
     * Presuming the X dimension varies fastest, then Y, and then Z.
     */
    for( frame = 0; frame < imagecube -> num_frames; frame++ )
        for( row = 0; row < imagecube -> num_rows; row++ )
            for( col = 0; col < imagecube -> num_cols; col++ )
            {
                if( buf[idx] < min )         min = buf[idx];
                if( buf[idx] > max )         max = buf[idx];
                imagecube -> volume[frame][row][col] = buf[idx];
                idx++;
            }
    imagecube -> min_val = min;
    imagecube -> max_val = max;
}

void FillImageCube_64bit( const double* buf,
                    int X, int Y, int Z,
                    QccIMGImageCube* imagecube )
{
    QccIMGImageCubeInitialize( imagecube );
    imagecube -> num_cols = X;
    imagecube -> num_rows = Y;
    imagecube -> num_frames = Z;
    if( QccIMGImageCubeAlloc( imagecube ) )
        QccErrorPrintMessages();

    double min = MAXDOUBLE;
    double max = -MAXDOUBLE;
    int frame, row, col;
    long idx = 0;
    /*
     * Presuming the X dimension varies fastest, then Y, and then Z.
     */
    for( frame = 0; frame < imagecube -> num_frames; frame++ )
        for( row = 0; row < imagecube -> num_rows; row++ )
            for( col = 0; col < imagecube -> num_cols; col++ )
            {
                if( buf[idx] < min )         min = buf[idx];
                if( buf[idx] > max )         max = buf[idx];
                imagecube -> volume[frame][row][col] = buf[idx];
                idx++;
            }
    imagecube -> min_val = min;
    imagecube -> max_val = max;
}

void FillImageComponent( const float* buf, int X, int Y,
                         QccIMGImageComponent* imagecomponent )
{
    QccIMGImageComponentInitialize( imagecomponent );
    imagecomponent -> num_cols = X;
    imagecomponent -> num_rows = Y;
    if( QccIMGImageComponentAlloc( imagecomponent ) )
        QccErrorPrintMessages();

    double min = MAXDOUBLE;
    double max = -MAXDOUBLE;
    int row, col;
    long idx = 0;
    /*
     * Presuming the X dimension varies fastest, then Y.
     */
    for( row = 0; row < imagecomponent -> num_rows; row++ )
        for( col = 0; col < imagecomponent -> num_cols; col++ )
        {
            if( buf[idx] < min )         min = buf[idx];
            if( buf[idx] > max )         max = buf[idx];
            imagecomponent -> image[row][col] = buf[idx];
            idx++;
        }
    imagecomponent -> min_val = min;
    imagecomponent -> max_val = max;
}

void FillImageComponent_64bit( const double* buf, int X, int Y,
                               QccIMGImageComponent* imagecomponent )
{
    QccIMGImageComponentInitialize( imagecomponent );
    imagecomponent -> num_cols = X;
    imagecomponent -> num_rows = Y;
    if( QccIMGImageComponentAlloc( imagecomponent ) )
        QccErrorPrintMessages();

    double min = MAXDOUBLE;
    double max = -MAXDOUBLE;
    int row, col;
    long idx = 0;
    /*
     * Presuming the X dimension varies fastest, then Y.
     */
    for( row = 0; row < imagecomponent -> num_rows; row++ )
        for( col = 0; col < imagecomponent -> num_cols; col++ )
        {
            if( buf[idx] < min )         min = buf[idx];
            if( buf[idx] > max )         max = buf[idx];
            imagecomponent -> image[row][col] = buf[idx];
            idx++;
        }
    imagecomponent -> min_val = min;
    imagecomponent -> max_val = max;
}

void myspeckencode3d( const float* srcBuf,
                     int srcX,
                     int srcY,
                     int srcZ,
                     const char* outputFilename,
                     int nLevels,
                     float TargetRate )
{
    /*
     * Creates a QccIMGImageCube struct to hold the input data.
     */
    QccIMGImageCube imagecube;
    FillImageCube( srcBuf, srcX, srcY, srcZ, &imagecube );
        
    encode3d( &imagecube, outputFilename, nLevels, TargetRate );

    QccIMGImageCubeFree( &imagecube );
}

void myspeckencode3d_64bit( const double* srcBuf,
                            int srcX,
                            int srcY,
                            int srcZ,
                            const char* outputFilename,
                            int nLevels,
                            float TargetRate )
        {
    /*
     * Creates a QccIMGImageCube struct to hold the input data.
     */
    QccIMGImageCube imagecube;
    FillImageCube_64bit( srcBuf, srcX, srcY, srcZ, &imagecube );
        
    encode3d( &imagecube, outputFilename, nLevels, TargetRate );

    QccIMGImageCubeFree( &imagecube );
}

void encode3d( QccIMGImageCube* imagecube,
               const char* outputFilename,
               int nLevels,
               float TargetRate )
{
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
    int ImageNumFrames = imagecube->num_frames;
    int ImageNumRows = imagecube->num_rows;
    int ImageNumCols = imagecube->num_cols;
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
    if ( QccSPECK3DEncode( imagecube, 
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

    QccWAVWaveletFree( &Wavelet );

    /* 
     * print out info
     */
    /*
    printf("3D-SPECK encoding to output file: %s:\n", outputFilename );
    printf("  Target rate: %f bpv\n", TargetRate);
    printf("  Actual rate: %f bpv\n", ActualRate);
    */
}

void myspeckencode2p1d( const float* srcBuf,
                     int srcX,
                     int srcY,
                     int srcZ,
                     const char* outputFilename,
                     int XYNumLevels,
                     int ZNumLevels,
                     float TargetRate )
{
    /*
     * Creates a QccIMGImageCube struct to hold the input data.
     */
    QccIMGImageCube imagecube;
    FillImageCube( srcBuf, srcX, srcY, srcZ, &imagecube );

    encode2p1d( &imagecube, outputFilename, XYNumLevels, ZNumLevels, TargetRate );

    QccIMGImageCubeFree( &imagecube );
}

void myspeckencode2p1d_64bit( const double* srcBuf,
                              int srcX,
                              int srcY,
                              int srcZ,
                              const char* outputFilename,
                              int XYNumLevels,
                              int ZNumLevels,
                              float TargetRate )
{
    /*
     * Creates a QccIMGImageCube struct to hold the input data.
     */
    QccIMGImageCube imagecube;
    FillImageCube_64bit( srcBuf, srcX, srcY, srcZ, &imagecube );

    encode2p1d( &imagecube, outputFilename, XYNumLevels, ZNumLevels, TargetRate );

    QccIMGImageCubeFree( &imagecube );
}

void encode2p1d( QccIMGImageCube* imagecube,
                 const char* outputFilename,
                 int XYNumLevels,
                 int ZNumLevels,
                 float TargetRate )
{
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

    int ImageNumFrames = imagecube->num_frames;
    int ImageNumRows = imagecube->num_rows;
    int ImageNumCols = imagecube->num_cols;
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
    if ( QccSPECK3DEncode( imagecube, 
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
    QccWAVWaveletFree( &Wavelet );

    /*
     * Print out info
     */
    /*
    printf("3D-SPECK encoding to output file: %s:\n", outputFilename );
    printf("  Target rate: %f bpv\n", TargetRate);
    printf("  Actual rate: %f bpv\n", ActualRate);
    */
}

void myspeckdecode3d( const char*  inputFilename,
                      float*  dstBuf,
                      int     outSize )
{
    double* tmp = malloc( sizeof(double) * outSize );
    
    myspeckdecode3d_64bit( inputFilename, tmp, outSize );

    int i;
    for( i = 0; i < outSize; i++ )
        dstBuf[i] = (float) tmp[i];

    free (tmp);
}

void myspeckdecode3d_64bit( const char*  inputFilename,
                            double* dstBuf,
                            int     outSize )
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
        QccErrorAddMessage("Error calling QccSPECK3DDecode()" );
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
}

void myspeckencode2d( const float* srcBuf, 
                      int srcX,
                      int srcY,
                      const char* outputFilename,
                      int nLevels,
                      float TargetRate )
{
    /*
     * Creates a QccIMGImageComponent struct to hold the input data.
     */
    QccIMGImageComponent imagecomponent;
    FillImageComponent( srcBuf, srcX, srcY, &imagecomponent );

    encode2d( &imagecomponent, outputFilename, nLevels, TargetRate );
    
    QccIMGImageComponentFree( &imagecomponent );
}

void myspeckencode2d_64bit( const double* srcBuf, 
                            int srcX,
                            int srcY,
                            const char* outputFilename,
                            int nLevels,
                            float TargetRate )
{
    /*
     * Creates a QccIMGImageComponent struct to hold the input data.
     */
    QccIMGImageComponent imagecomponent;
    FillImageComponent_64bit( srcBuf, srcX, srcY, &imagecomponent );

    encode2d( &imagecomponent, outputFilename, nLevels, TargetRate );
    
    QccIMGImageComponentFree( &imagecomponent );
}

void encode2d( QccIMGImageComponent* imagecomponent,
               const char* outputFilename,
               int nLevels,
               float TargetRate )
{
    /*
     * Sets up parameters for DWT and SPECK encoding.
     */
    QccWAVWavelet Wavelet;
    QccWAVWaveletInitialize( &Wavelet );
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
    int ImageNumRows = imagecomponent->num_rows;
    int ImageNumCols = imagecomponent->num_cols;
    int NumPixels = ImageNumRows * ImageNumCols;
    long long int pxlcount = (long long int)ImageNumRows * ImageNumCols;
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
    if( QccSPECKEncode( imagecomponent,
                        NULL,
                        nLevels,
                        TargetBitCnt,
                        &Wavelet,
                        &OutputBuffer ) )
    {
      QccErrorAddMessage("Error calling QccSPECKEncode()");
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
    QccWAVWaveletFree( &Wavelet );

    /* 
     * print out info
     */
/*
    printf("2D-SPECK encoding to output file: %s:\n", outputFilename );
    printf("  Target rate: %f bpv\n", TargetRate);
    printf("  Actual rate: %f bpv\n", ActualRate);
*/
}


void myspeckdecode2d( const char*  inputFilename,
                      float* dstBuf,
                      int    outSize )
{
    double* tmp = malloc( sizeof(double) * outSize );

    myspeckdecode2d_64bit( inputFilename, tmp, outSize );

    int i;
    for( i = 0; i < outSize; i++ )
        dstBuf[i] = (float) tmp[i];

    free( tmp );
}

void myspeckdecode2d_64bit( const char*  inputFilename,
                            double* dstBuf,
                            int     outSize )
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

    int NumLevels, NumRows, NumCols;
    double ImageMean;
    int MaxCoefficientBits;
    if( QccSPECKDecodeHeader( &InputBuffer,
                              &NumLevels,
                              &NumRows, &NumCols,
                              &ImageMean,
                              &MaxCoefficientBits ))
    {
      QccErrorAddMessage("Error calling QccSPECKDecodeHeader()" );
      QccErrorExit();
    }

    int NumPixels = NumRows * NumCols;
    long long int pxlcount = (long long int)NumRows * NumCols;
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

    QccIMGImageComponent imagecomponent;
    QccIMGImageComponentInitialize( &imagecomponent );
    imagecomponent.num_rows = NumRows;
    imagecomponent.num_cols = NumCols;
    if (QccIMGImageComponentAlloc( &imagecomponent ) )
    {
      QccErrorAddMessage("Error calling QccIMGImageComponentAlloc()" );
      QccErrorExit();
    }
    int TargetBitCnt = QCCENT_ANYNUMBITS;

    if( QccSPECKDecode( &InputBuffer,
                        &imagecomponent,
                        NULL,
                        NumLevels,
                        &Wavelet,
                        ImageMean,
                        MaxCoefficientBits,
                        TargetBitCnt ) )
    {
      QccErrorAddMessage("Error calling QccSPECKDecode()" );
      QccErrorExit();
    }
    if (QccBitBufferEnd(&InputBuffer))
    {
      QccErrorAddMessage("Error calling QccBitBufferEnd()" );
      QccErrorExit();
    }

    int idx = 0;
    int row, col;
    for( row = 0; row < imagecomponent.num_rows; row++ )
        for( col = 0; col < imagecomponent.num_cols; col++ )
            dstBuf[ idx++ ] = imagecomponent.image[row][col];
                    
    QccIMGImageComponentFree( &imagecomponent );
    QccWAVWaveletFree( &Wavelet );
}

void evaluate2arrays( const float* A, const float* B, 
                      int len, double* rms, double* lmax )
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

void evaluate2arrays_64bit( const double* A, const double* B, 
                            int len, double* rms, double* lmax )
{
    double sum = 0.0;
    double c = 0.0;
    double max = 0.0;
    double tmp;
    int i;
    for( i = 0; i < len; i++) {
        tmp = A[i] - B[i];
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
