#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
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
	string path = "/home/users/samuelli/Datasets/cam42_32_always_2D";
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
		size_t start[] = {1, 0};			// 0th time slice is the initial condition
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
            //cout << "Found variable: " << vars[i].GetName();
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
	{
		//cout << ",\t2D" << endl;
		return 2;
	}
    else if( varDimNames.size() == 3 )
	{
		//cout << ",\t3D" << endl;
		return 3;
	}
	else 
		return -1;
}

/*
 * Calculate the Z score for each data point of the mth member.
 * See equation (6) from Baker paper. 
 */
void CalcZ( float** data_buf, 
			long m_len, 				// Input. 101
			long z_len,					// Input. data_buf[m_len][z_len]
			int m,						// Input. the mth member to calculate 
			double* z )					// Output. len(z) == z_len.
{
	float xi[m_len - 1];					// 101 - 1
	double tol = 1e-12;
	vector< double > largeZ;
	double threshold = 1e+05;				// threshold to print out larger values
	for( long i = 0; i < z_len; i++ )		// z_len = LEN2D or LEN3D
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
		if( stddev > tol )	
			z[i] = (data_buf[m][i] - mean) / stddev;
		else
			z[i] = 0;

		/* print debug mean and sd */
		printf("x=%e, mean=%e, stddev=%e, z=%e\n", data_buf[m][i], mean, stddev, z[i] );

		/*
		 * Print out enormous big z scores
	 	 *
		if( z[i] > threshold )
		{
			largeZ.push_back( z[i] );
			printf("z[%ld]=%e, x[%ld]=%e, mean=%e, stddev=%e\n", 
					i, z[i], i, data_buf[m][i], mean, stddev );
			for( int j = 0; j < m_len-1; j++ )
				printf("\t%e\n", xi[j] );
		}
		*/
		/*
		if( i == 1437024 )
		{
			printf("z[%ld]=%e, x[%ld]=%e, mean=%e, stddev=%e\n", 
					i, z[i], i, data_buf[m][i], mean, stddev );
			for( int j = 0; j < m_len-1; j++ )
				printf("\t%e\n", xi[j] );
		}
		*/
	}
	if( largeZ.size() > 0 )
	{
		double rmsz = 0;
		for( long i = 0; i < largeZ.size(); i++ )
			rmsz += pow(largeZ[i], 2.0);
		rmsz /= (double)largeZ.size();
		rmsz = sqrt( rmsz );
		printf("%ld out of %ld z scores are above %f, their rmsz is %e\n", 
				largeZ.size(), z_len, threshold, rmsz );
	}
}

double CalcRMSZ( double* arr, long len )
{
	double sum = 0;
	double c = 0.0;
	long cnt = 0;		// non-zero counts
	for( long i = 0; i < len; i ++ )
	{
		if( arr[i] != 0.0 )
		{
			cnt++;
			double y = arr[i] * arr[i] - c;
			double t = sum + y;
			c = t - sum - y;
			sum = t;
			//if( arr[i] * arr[i] > 10 )
			//	cout << "Alert: arr[i] = " << arr[i] << endl;
		}
	}
	sum /= (double)cnt;
	/* print cnt for debug */
	//printf("non-zero z count = %ld\n", cnt );
	return sqrt( sum );
}

void WriteRMSZ( double* rmsz, long len, const string &variable )
{
	string filename = "/home/users/samuelli/Git/qccpack-addon/results/rmsz/bin/"
					  + variable + ".orig.bin";
	FILE* f = fopen( filename.c_str(), "wb" );
	fwrite(rmsz, sizeof(double), len, f );
	fclose(f);
}

struct point
{
	long idx;
	float orig_val;
	float comp_val;
};

bool mycomp( const struct point &v1, const struct point &v2 )
{
	return fabs(v1.orig_val - v1.comp_val) > fabs(v2.orig_val - v2.comp_val);
}


int main( int argc, char* argv[] )
{
	string var = "TS";
	int member = 100;
	if( argc == 3 )
	{
		var = argv[1];
		member = atoi( argv[2] );
	}
	vector<string> orig_names;
	vector<string> comp_names;
	FillOrigNames( orig_names );
	FillCompNames( comp_names );

	long buf_size = 0;		
	int dims = GetVarDims( orig_names[0], var );
	if( dims == 2 )
		buf_size = LEN2D;
	else if( dims == 3 )
		buf_size = LEN3D;
	else
	{
		cerr << "ERROR: variable dimension error! " << var << endl;
		exit(1);
	}

	/* read in original ensemble data */
	float** orig_data_buf = new float*[ ENS_SIZE ];
	for( int m = 0; m < ENS_SIZE; m++ )
	{
		orig_data_buf[m] = new float[buf_size];
		ReadVar( orig_names[m], var, orig_data_buf[m], buf_size );
	}
	/* read in compressed ensemble data */
	float** comp_data_buf = new float*[ ENS_SIZE ];
	for( int m = 0; m < ENS_SIZE; m++ )
	{
		comp_data_buf[m] = new float[buf_size];
		ReadVar( comp_names[m], var, comp_data_buf[m], buf_size );
	}
	/* print out min, max 
	double min = data_buf[0][0];
	double max = data_buf[0][0];
	for( long i = 0; i < buf_size; i++ )
		if( data_buf[0][i] < min )		min = data_buf[0][i];
		else if( data_buf[0][i] > max)	max = data_buf[0][i];
	printf("member 000: min=%e, max=%e\n", min, max );
	*/


	/* calculate z score 
	double** z_buf = new double*[ ENS_SIZE ];
	double rmsz[ ENS_SIZE ];
	for( int m = 0; m < ENS_SIZE; m++ )
	{
		//cerr << "ensemble member = " << m << endl;
		z_buf[m] = new double[buf_size];
		CalcZ( data_buf, ENS_SIZE, buf_size, m, z_buf[m] );
		rmsz[m] = CalcRMSZ( z_buf[m], buf_size );
		printf("%.10e\n", rmsz[m] );
	}
	for( int m = 0; m < ENS_SIZE; m++ )
		delete[] z_buf[m];
	delete[] z_buf;
	*/
	//WriteRMSZ( rmsz, ENS_SIZE, var );


	/* For debug: calculate only 1 member */
	int m = member;	
	std::vector<struct point> points;
	double* orig_z0 = new double[ buf_size ];
	double* comp_z0 = new double[ buf_size ];
	cout << "Start original results: " << endl;
	CalcZ( orig_data_buf, ENS_SIZE, buf_size, m, orig_z0 );
	cout << "Start compressed results: " << endl;
	CalcZ( comp_data_buf, ENS_SIZE, buf_size, m, comp_z0 );
	double orig_rmsz0 = CalcRMSZ( orig_z0, buf_size );
	double comp_rmsz0 = CalcRMSZ( comp_z0, buf_size );
	printf("orignal rmsz=%.9e, compressed rmsz=%.9e\n", orig_rmsz0, comp_rmsz0 );
	for( long i = 0; i < buf_size; i++ )
	{
		struct point p;
		p.idx = i;
		p.orig_val = orig_z0[i];
		p.comp_val = comp_z0[i];
		points.push_back(p);
	}
	std::sort( points.begin(), points.end(), mycomp );
	for( long i = 0; i < 10; i++ )
		printf("%ld:   %e - %e = %e\n", points[i].idx,
									    points[i].orig_val,  points[i].comp_val,
								 		points[i].orig_val - points[i].comp_val );
	delete[] orig_z0;
	delete[] comp_z0;
	
	
	for( int m = 0; m < ENS_SIZE; m++ )
	{
		delete[] orig_data_buf[m];
		delete[] comp_data_buf[m];
	}
	delete[] orig_data_buf;
	delete[] comp_data_buf;
}
