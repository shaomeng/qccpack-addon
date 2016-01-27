#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>
#include "vapor/NetCDFSimple.h"
#include "vapor/MatWaveWavedec.h"
#include "CamHandler.h"

#define LEN2D   48602
#define LEN3D  (48602*30)
#define ENS_SIZE 101		// ensemble size
#define NX 91
#define NY 91

using std::vector;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

void FillOrigNames( vector<string> &names )
{
	string path = "/opt/Research_Data/ensemble_orig";
	string prefix = "/cesm1_1.FC5.ne30_g16."; 
	string suffix = ".cam.h0.0001-01-01-00000.nc";
	char idx[64];
	for( int i = 0; i < ENS_SIZE; i++ )
	{
		sprintf( idx, "%03d", i );
		names.push_back( path + prefix + idx + suffix );
	}
}

void ReadVar( const string &ncname, const string &varname, 
			  float* buf, long len )
{
    /* Examine if the file and variable is valid */
    VAPoR::NetCDFSimple ncsimple;
    int rc = ncsimple.Initialize( ncname );
    vector <VAPoR::NetCDFSimple::Variable> vars = ncsimple.GetVariables();
    int varIdx = -1;
    for( int i = 0; i < vars.size(); i++ )
    {
        if( vars[i].GetName().compare( varname ) == 0 )
        {
            varIdx = i;
            break;
        }
    }
    assert( varIdx != -1 );

	/* Read the data out in 2D and 3D cases */
	vector< string > varDimNames = vars[varIdx].GetDimNames();
	if( varDimNames.size() == 2 )
	{
		size_t ncol = ncsimple.DimLen( varDimNames[1] );
		assert( ncol == len );
		size_t start[] = {1, 0};			// RAW data start from 1
											// RECONSTRUCTED starts from 0
		size_t count[] = {1, ncol};
		ncsimple.OpenRead( vars[varIdx] );
		rc = ncsimple.Read( start, count, buf );
		ncsimple.Close();
		assert( rc >= 0 );
	}
	else if( varDimNames.size() == 3 )
	{
		size_t lev  = ncsimple.DimLen( varDimNames[1] );
		size_t ncol = ncsimple.DimLen( varDimNames[2] );
		assert( lev * ncol == len );
		size_t start[] = {1, 0, 0};		// RAW data starts from 1
										// RECONSTRUCTED starts from 0
		size_t count[] = {1, lev, ncol};
		ncsimple.OpenRead( vars[varIdx] );
		rc = ncsimple.Read( start, count, buf );
		ncsimple.Close();
		assert( rc >= 0 );
	}
	else
	{
		cerr << "ERROR: variable dimensions not supported! " << varname << endl;
		exit(1);
	}
}

int GetVarDims( const string &ncname, const string &varname )
{
    /* Examine if the file and variable valid */
    VAPoR::NetCDFSimple ncsimple;
    int rc = ncsimple.Initialize( ncname );
    if( rc < 0 )
    {
        cerr << "ncsimple.Initialize() error: " << ncname << endl;
        exit(1);
    }
    vector <VAPoR::NetCDFSimple::Variable> vars = ncsimple.GetVariables();
    int varIdx = -1;
    for( int i = 0; i < vars.size(); i++ )
    {
        if( vars[i].GetName().compare( varname ) == 0 )
        {
            cout << "Found variable: " << vars[i].GetName() << endl;
            varIdx = i;
            break;
        }
    }
    if( varIdx == -1 )
	{
		cerr << "ERROR: Variable not found: " << varname << endl;
		return -1;
	}

    vector< string > varDimNames = vars[varIdx].GetDimNames();
    if( varDimNames.size() == 2 )
		return 2;
    else if( varDimNames.size() == 3 )
		return 3;
	else 
		return -1;
}

void Evaluate2Arrays( const float* A, const float* B, long len, 
                      double* minmaxA, double* minmaxB, 
                      double* rmse, double* nrmse, 
                      double* lmax, double* nlmax,
                      double* lmaxAB,	// lmaxAB[0] from A, lmaxAB[1] from B
					  double* relativeLMax, 
					  double* relativeLMaxAB )	// [0] from A, [1] from B.
{
    double sum = 0.0;
    double c = 0.0;
    double max = 0.0;
    double tmp;
    double minA = A[0];
    double maxA = A[0];
    double minB = B[0];
    double maxB = B[0];
	double sum_A = 0.0;
	double sum_B = 0.0;
	*relativeLMax = 0.0;
    for( long i = 0; i < len; i++)
	{
		sum_A += A[i];	
		sum_B += B[i];	
        tmp = A[i] - B[i];
        if (tmp < 0)        tmp *= -1.0;
        if (tmp > max)
		{
			max = tmp;
			lmaxAB[0] = A[i];
			lmaxAB[1] = B[i];
		}
		if( A[i] != 0.0 && fabs( (B[i]-A[i])/A[i] ) > *relativeLMax )
		{
			*relativeLMax = fabs( (B[i]-A[i])/A[i] );
			relativeLMaxAB[0] = A[i];
			relativeLMaxAB[1] = B[i];
		}
        double y = tmp * tmp - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;

        /* Collect min, max */
        if( A[i] < minA )   minA = A[i];
        if( A[i] > maxA )   maxA = A[i];
        if( B[i] < minB )   minB = B[i];
        if( B[i] > maxB )   maxB = B[i];
    }
    sum /= (double)len;
    sum = sqrt( sum );

    *rmse = sum;
    *lmax = max;
    minmaxA[0] = minA;
    minmaxA[1] = maxA;
    minmaxB[0] = minB;
    minmaxB[1] = maxB;

    *nrmse = sum / (maxA - minA);
    *nlmax = max / (maxA - minA);
}

void DwtIDwt2D( const float* raw_buf, float* recon_buf,
                int numXYDWTLevels)		// NX, NY are global
{
	VAPoR::MatWaveWavedec mw("bior1.1");
    size_t nlevels2d = min(mw.wmaxlev(NX), mw.wmaxlev(NY));
	assert( numXYDWTLevels <= nlevels2d );
    size_t clen2d = mw.coefflength2(NX, NY, numXYDWTLevels);
    assert (clen2d = NX*NY);
	float* c2d = new float[clen2d];
	size_t L2d[(6*numXYDWTLevels)+4];

    mw.computeL2(NX, NY, numXYDWTLevels, L2d);
	int rc = mw.wavedec2( raw_buf, NX, NY, numXYDWTLevels, c2d, L2d );
	assert( rc>=0 );
	rc = mw.waverec2( c2d, L2d, numXYDWTLevels, recon_buf );
	assert( rc>=0 );

	delete[] c2d;
}

void DwtIDwt2D_64bit( const float* raw_buf, float* recon_buf,
                	  int numXYDWTLevels)		// NX, NY are global
{
	VAPoR::MatWaveWavedec mw("bior1.1");
    size_t nlevels2d = min(mw.wmaxlev(NX), mw.wmaxlev(NY));
	assert( numXYDWTLevels <= nlevels2d );
    size_t clen2d = mw.coefflength2(NX, NY, numXYDWTLevels);
    assert (clen2d = NX*NY);
	double* c2d = new double[clen2d];
	size_t L2d[(6*numXYDWTLevels)+4];

	double* in  = new double[NX*NY];
	double* out = new double[NX*NY];
	for( long i = 0; i < NX*NY; i++ )
		in[i] = raw_buf[i];

    mw.computeL2(NX, NY, numXYDWTLevels, L2d);
	int rc = mw.wavedec2( in, NX, NY, numXYDWTLevels, c2d, L2d );
	assert( rc>=0 );
	rc = mw.waverec2( c2d, L2d, numXYDWTLevels, out);
	assert( rc>=0 );

	for( long i = 0; i < NX*NY; i++ )
		recon_buf[i] = out[i];

	delete[] c2d;
	delete[] in;
	delete[] out;
}

void DwtIDwt3D( const float* raw_buf, float* recon_buf,
				long NZ )		// NX, NY are global
{
	VAPoR::MatWaveWavedec mw("bior4.4");
	size_t nlevels = min( min(mw.wmaxlev(NX), mw.wmaxlev(NY)), mw.wmaxlev(NZ));
    size_t clen = mw.coefflength3(NX, NY, NZ, nlevels);
	assert (clen = NX*NY*NZ );
    float *C = new float[clen];
	size_t L[ (21*nlevels) + 6];
	mw.computeL3(NX, NY, NZ, nlevels, L);
	int rc = mw.wavedec3( raw_buf, NX, NY, NZ, nlevels, C, L);
	assert( rc >= 0 );
	rc = mw.waverec3( C, L, nlevels, recon_buf );
	assert( rc >= 0 );
	
	delete[] C;
}

void DwtIDwt3D_64bit( const float* raw_buf, float* recon_buf,
					  long NZ )		// NX, NY are global
{
	VAPoR::MatWaveWavedec mw("bior4.4");
	size_t nlevels = min( min(mw.wmaxlev(NX), mw.wmaxlev(NY)), mw.wmaxlev(NZ));
    size_t clen = mw.coefflength3(NX, NY, NZ, nlevels);
	assert (clen = NX*NY*NZ );
    double *C = new double[clen];
	size_t L[ (21*nlevels) + 6];

	double* in = new double[clen];
	double* out = new double[clen];
	for( size_t i = 0; i < clen; i++ )
		in[i] = raw_buf[i];

	mw.computeL3(NX, NY, NZ, nlevels, L);
	int rc = mw.wavedec3( in, NX, NY, NZ, nlevels, C, L);
	assert( rc >= 0 );
	rc = mw.waverec3( C, L, nlevels, out );
	assert( rc >= 0 );

	for( size_t i = 0; i < clen; i++ )
		recon_buf[i] = (float)out[i];
	
	delete[] out;
	delete[] in;
	delete[] C;
}

int main( int argc, char* argv[] )
{
	string var = "NUMLIQ";
	int member = 0;
	if( argc == 3 )
	{
		var = argv[1];
		member = atoi(argv[2]);
	}
	vector<string> origNames;
	FillOrigNames( origNames );
	long homme_buf_size, nlev;
	int dims = GetVarDims( origNames[member], var );
	if( dims == 2 )
	{
		homme_buf_size = LEN2D;
		nlev = 1;
	}
	else if( dims == 3 )
	{
		homme_buf_size = LEN3D;
		nlev = 30;
	}
	else
	{
		cerr << "ERROR: variable dimension error! " << var << endl;
		exit(1);
	}

	/* read in homme data */
	float* homme_buf = new float[ homme_buf_size ];
	ReadVar( origNames[member], var, homme_buf, homme_buf_size );

	/* Homme data to raw layout */
	string hommeMap   = "./HommeMap/reorderedCbasedHommeMapping.nc";
    string faceMap    = "./HommeMap/faceIdsNeNp304.nc";
	VAPoR::CamHandler handler( hommeMap, faceMap );
	long raw_buf_size = NX * NY * nlev * 6;
	float* raw_buf = new float[ raw_buf_size ];
	float* recon_buf = new float[ raw_buf_size ];
	handler.cam2raw( homme_buf, homme_buf_size, nlev, raw_buf, raw_buf_size );
	int numXYDWTLevels = 4;

	for( long face = 0; face < 6; face++ )
	{
		long faceOffset = face * NX * NY * nlev;
		/* 2D DWT
		 *
		for( long l = 0; l < nlev; l++ )
		{
			long levOffset = l * NX * NY;
			DwtIDwt2D_64bit( raw_buf + faceOffset + levOffset, 
					   recon_buf + faceOffset + levOffset,
					   numXYDWTLevels);
		}
		*/

		/* 3D DWT
		 */
		if( dims == 3 )
			DwtIDwt3D_64bit( raw_buf + faceOffset,
						   recon_buf + faceOffset,
						   nlev );	
		else
		{
			cout << var << ": 2D variable doesn't support 3D transforms " << endl;
			exit(1);
		}
	}
	

	//cout << "Doing simple math..." << endl;
	double minmaxA[2], minmaxB[2], lmaxAB[2], relativeLMaxAB[2];
	double rmse, nrmse, lmax, nlmax, relativeLMax;
	Evaluate2Arrays( raw_buf, recon_buf, raw_buf_size,
					 minmaxA, minmaxB, &rmse, &nrmse, 
					 &lmax, &nlmax, lmaxAB,
					 &relativeLMax, relativeLMaxAB );
	printf("%e, %e\n", rmse, lmax);
	printf("%e, %e, %e\n", nrmse, nlmax, relativeLMax);
	printf("%e, %e\n", minmaxA[0], minmaxA[1] );
	/*
	printf("Original:    min=%e, max=%e\n", minmaxA[0], minmaxA[1] );
	printf("Reconstruct: min=%e, max=%e\n", minmaxB[0], minmaxB[1] );
	printf("RMSE=%e, after normalization: %e\n", rmse, nrmse );
	printf("LMAX=%e, after normalization: %e, \n\tit happend when A[i]=%e, B[i]=%e\n",
		   lmax, nlmax, lmaxAB[0], lmaxAB[1] );
	printf("Relative LMAX=%e, \n\tit happend when A[i]=%e, B[i]=%e\n",
		   relativeLMax, relativeLMaxAB[0], relativeLMaxAB[1] );
	*/
	
	delete[] recon_buf;
	delete[] raw_buf;
	delete[] homme_buf;
}
