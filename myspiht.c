#include "myspiht.h"

int myspihtencode( float* srcBuf,
                 int srcX,
                 int srcY,
                 int srcZ,
                 char* outputFilename,
                 short nLevels,
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
     * Sets up parameters for DWT and SPIHT encoding.
     */
    int TransformType = QCCWAVSUBBANDPYRAMID3D_DYADIC;
    int ZerotreeType = QCCSPIHT3D_ZEROTREE_DYADIC;
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
  if (ImageNumFrames !=
      ((int)(ImageNumFrames >> (TemporalNumLevels + 1)) << (TemporalNumLevels + 1)))
    {
      QccErrorAddMessage("Image-cube size (%d x %d x %d) is not a multiple of %d (as needed for a temporal decomposition of %d levels)",
                         ImageNumCols, ImageNumRows, ImageNumFrames,
                         (1 << (TemporalNumLevels + 1)), TemporalNumLevels);
      QccErrorExit();
    }
  if (ImageNumCols !=
      ((int)(ImageNumCols >> (SpatialNumLevels + 1)) << (SpatialNumLevels + 1)))
    {
      QccErrorAddMessage("Image-cube size (%d x %d x %d) is not a multiple of %d (as needed for a spatial decomposition of %d levels)",
                         ImageNumCols, ImageNumRows, ImageNumFrames,
                         (1 << (SpatialNumLevels + 1)), SpatialNumLevels);
      QccErrorExit();
    }
  if (ImageNumRows !=
      ((int)(ImageNumRows >> (SpatialNumLevels + 1)) << (SpatialNumLevels + 1)))
    {
      QccErrorAddMessage("Image-cube size (%d x %d x %d) is not a multiple of %d (as needed for a spatial decomposition of %d levels)",
                         ImageNumCols, ImageNumRows, ImageNumFrames,
                         (1 << (SpatialNumLevels + 1)), SpatialNumLevels);
      QccErrorExit();
    }
    int NumPixels = ImageNumFrames * ImageNumRows * ImageNumCols;

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
    long long int bitcount = (long long int)(ceil((NumPixels * TargetRate)/8.0))*8;
    if( bitcount > INT_MAX ) {
        QccErrorAddMessage("TargetBitCnt overflow. Please try smaller data sets.");
        QccErrorExit();
    }
    int TargetBitCnt = (int)(ceil((NumPixels * TargetRate)/8.0))*8;
    int NoArithmeticCoding = 0;
    if ( QccSPIHT3DEncode(&(imagecube), NULL, &OutputBuffer, TransformType, ZerotreeType,
         TemporalNumLevels, SpatialNumLevels, &Wavelet, TargetBitCnt, NoArithmeticCoding) )
    {
      QccErrorAddMessage("Error calling QccSPIHT3DEncode()");
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

    printf("3D-SPIHT coding to output file: %s:\n", outputFilename );
    printf("  Target rate: %f bpv\n", TargetRate);
    printf("  Actual rate: %f bpv\n", ActualRate);


    return 0;
}


int myspihtdecode( char*  inputFilename,
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
    
    QccIMGImageCube imagecube;
    QccIMGImageCubeInitialize( &imagecube );

    if (QccBitBufferStart(&InputBuffer)) {
        QccErrorAddMessage("Error calling QccBitBufferStart()" );
        QccErrorExit();
    }

    int TransformType;
    int ZerotreeType;
    int TemporalNumLevels;
    int SpatialNumLevels;
    int NumFrames, NumRows, NumCols;
    double ImageMean;
    int MaxCoefficientBits;
    int ArithmeticCoded;
    
    if (QccSPIHT3DDecodeHeader(&InputBuffer, &TransformType, &ZerotreeType,
                             &TemporalNumLevels, &SpatialNumLevels,
                             &NumFrames, &NumRows, &NumCols,
                             &ImageMean, &MaxCoefficientBits, &ArithmeticCoded))
    {
      QccErrorAddMessage("Error calling QccSPIHT3DDecodeHeader()");
      QccErrorExit();
    }

    int NumPixels = NumFrames * NumRows * NumCols;
    if( NumPixels != outSize ) {
        QccErrorAddMessage("Decode output buffer size doesn't match signal length.");
        QccErrorExit();
    }    
    imagecube.num_frames = NumFrames;
    imagecube.num_rows = NumRows;
    imagecube.num_cols = NumCols;
    int TargetBitCnt = QCCENT_ANYNUMBITS;
    if (QccIMGImageCubeAlloc(&imagecube) ) {
      QccErrorAddMessage("Error calling QccIMGImageCubeAlloc()" );
      QccErrorExit();
    }
    if (QccSPIHT3DDecode(&InputBuffer, &imagecube, NULL, TransformType, ZerotreeType,
                       TemporalNumLevels, SpatialNumLevels, &Wavelet, ImageMean,
                       MaxCoefficientBits, TargetBitCnt, ArithmeticCoded))
    {
        QccErrorAddMessage("Error calling QccSPIHT3DDecode()" );
        QccErrorExit();
    }
    if (QccBitBufferEnd(&InputBuffer)) {
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
