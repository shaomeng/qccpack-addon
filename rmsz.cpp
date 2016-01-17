#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "vapor/NetCDFSimple.h"

#define LEN2D   48602
#define LEN3D  (48602*30)
#define ENS_SIZE 101		// ensemble size

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

void FillCompNames( vector<string> &names )
{
	string path = "/home/users/samuelli/Datasets/cam42_32";
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

/*
 * Calculate the Z score for each data point of the mth member.
 * See equation (6) from Baker paper. 
 */
void CalcZ( float** data_buf, 
			long m_len, 				// Input. 
			long z_len,					// Input. data_buf[m_len][z_len]
			int m,						// Input. the mth member to calculate 
			double* z )					// Output. len(1) == z_len.
{
	float xi[m_len - 1];					// 101 - 1
	for( long i = 0; i < z_len; i++ )		// z_len = 48k
	{
		int idx = 0;
		for( int j = 0; j < m_len; j++ )
			if( j != m )
				xi[idx++] = data_buf[j][i];

		//if( m == m_len - 1 && i == z_len - 1)
		//{
		//	cout << "m = " << m << endl;
		//	for( int j = 0; j < m_len-1; j++ )
		//		cout << xi[j] << endl;
		//	cout << x << endl;
		//}

		double mean = 0;
		double variance = 0;
		for( int j = 0; j < m_len-1; j++ )
			mean += (double)xi[j];
		mean /= (m_len-1);
		for( int j = 0; j < m_len-1; j++ )
			variance += pow((mean -xi[j]), 2);
		variance /= (m_len-1);
		double stddev = sqrt( variance );
		if( stddev != 0 )	
			z[i] = (data_buf[m][i] - mean) / stddev;
		else
			z[i] = 0;

		/*
		if( z[i] > 10 )
		{
			printf("z[%ld] = %e, x[%ld] = %e, mean = %e, stddev = %e\n", 
					i, z[i], i, data_buf[m][i], mean, stddev );
			for( int j = 0; j < m_len-1; j++ )
				printf("\t%e\n", xi[j] );
		}
		*/
	}
}

double CalcRMSZ( double* arr, long len )
{
	double sum = 0;
	double c = 0.0;
	for( long i = 0; i < len; i ++ )
	{
		double y = arr[i] * arr[i] - c;
		double t = sum + y;
		c = t - sum - y;
		sum = t;
		//if( arr[i] * arr[i] > 10 )
		//	cout << "Alert: arr[i] = " << arr[i] << endl;
		/*
		if( sum > 10000000000 )
			cout << "10000000000: i = " << i << endl;
		else if( sum > 1000000000 )
			cout << "1000000000: i = " << i << endl;
		else if( sum > 100000000 )
			cout << "100000000: i = " << i << endl;
		else if( sum > 10000000 )
			cout << "10000000: i = " << i << endl;
		else if( sum > 1000000 )
			cout << "1000000: i = " << i << endl;
		else if( sum > 100000 )
			cout << "100000: i = " << i << endl;
		else if( sum > 10000 )
			cout << "10000: i = " << i << endl;
		else if( sum > 1000 )
			cout << "1000: i = " << i << endl;
		else if( sum > 100 )
			cout << "100: i = " << i << endl;
		*/
	}
	sum /= (double)len;
	return sqrt( sum );
}

void WriteRMSZ( double* rmsz, long len, const string &variable )
{
	string filename = "/home/users/samuelli/Git/qccpack-addon/results/rmsz/"
					  + variable + ".orig.bin";
	FILE* f = fopen( filename.c_str(), "wb" );
	fwrite(rmsz, sizeof(double), len, f );
	fclose(f);
}


int main( int argc, char* argv[] )
{
	string var = "ANSNOW";
	if( argc == 2 )
		var = argv[1];
	vector<string> names;
	FillOrigNames( names );
	long buf_size = 0;		
	int dims = GetVarDims( names[0], var );
	if( dims == 2 )
		buf_size = LEN2D;
	else if( dims == 3 )
		buf_size = LEN3D;
	else
	{
		cerr << "ERROR: variable dimension error! " << var << endl;
		exit(1);
	}

	/* read in ensemble data */
	float** data_buf = new float*[ ENS_SIZE ];
	for( int m = 0; m < ENS_SIZE; m++ )
	{
		data_buf[m] = new float[buf_size];
		ReadVar( names[m], var, data_buf[m], buf_size );
		//cout << data_buf[m][buf_size-1] << endl;
	}

	/* calculate z score */
	double** z_buf = new double*[ ENS_SIZE ];
	double rmsz[ ENS_SIZE ];
	for( int m = 0; m < ENS_SIZE; m++ )
	{
		cerr << "iteration m = " << m << endl;
		z_buf[m] = new double[buf_size];
		CalcZ( data_buf, ENS_SIZE, buf_size, m, z_buf[m] );
		rmsz[m] = CalcRMSZ( z_buf[m], buf_size );
		printf("%.10e\n", rmsz[m] );
	}
	/*
	double* z0 = new double[ buf_size ];
	CalcZ(data_buf, ENS_SIZE, buf_size, 0, z0 );
	double rmsz0 = CalcRMSZ( z0, buf_size );
	printf("rmsz for 000 is: %9e\n", rmsz0 );
	delete[] z0;
	*/
	WriteRMSZ( rmsz, ENS_SIZE, var );
	
	for( int m = 0; m < ENS_SIZE; m++ )
	{
		delete[] z_buf[m];
		delete[] data_buf[m];
	}
	delete[] z_buf;
	delete[] data_buf;
}
