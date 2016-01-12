/* 
 * This program finds the smallest non-zero absolutely value for a variable, from a NetCDF file.
 *
 * Programmer: Samuel Li
 * Date: 1/11/2016
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "CamHandler.h"

const size_t NX = 91;
const size_t NY = 91;
const size_t NCOL = 48602;

#define FLOAT double

void SimpleStats( const float* A, size_t len, 
				  FLOAT* min, FLOAT* max,
				  FLOAT* minfabs )
{
	FLOAT minl = A[0];		// min in the local
	FLOAT maxl = A[0];		// max in the local
	FLOAT minfabsl;			// minfabs in the local
	for( size_t i = 0; i < len; i++ )
	{
		if( fabs(A[i]) > 0.0 )
		{
			minfabsl = fabs(A[i]);
			break;
		}
	}
	for( size_t i = 0; i < len; i++ )
	{
		if( A[i] > maxl )	maxl = A[i];
		if( A[i] < minl )	minl = A[i];
		if( fabs(A[i]) < minfabsl && fabs(A[i]) > 0.0 )
			minfabsl = fabs(A[i]);
	}
	*min = minl;
	*max = maxl;
	*minfabs = minfabsl;
}

int main( int argc, char* argv[] )
{
    if( argc != 3 )
    {
        std::cerr << "please specify NetCDF file and variable name! " << endl;
        exit (1);
    }

	string netcdfInput = argv[1];
	string varname = argv[2];

    string hommeMap   = "./HommeMap/reorderedCbasedHommeMapping.nc";
    string faceMap    = "./HommeMap/faceIdsNeNp304.nc";

	int rc;

	/* Examine if the file and variable valid */
	VAPoR::NetCDFSimple ncsimple;
	rc = ncsimple.Initialize( netcdfInput );
	if( rc < 0 )
	{
		cerr << "ncsimple.Initialize() error: " << netcdfInput << endl;
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
	assert( varIdx != -1 );
	vector< string > varDimNames = vars[varIdx].GetDimNames();
	for( int j = 0; j < varDimNames.size(); j++ )
		cout << "\tdimension name: " << varDimNames[j] 
			 << ",\tlength: " << ncsimple.DimLen(varDimNames[j]) << endl;

	/* make sure this is a normal 3D variable by testing it's dimension names */
	/* 2D variables are not supported here */
	assert( varDimNames.size() == 3 );
	assert( varDimNames[0].compare( "time" ) == 0 );
	assert( varDimNames[1].compare( "lev" ) == 0 );
	assert( varDimNames[2].compare( "ncol" ) == 0 );
	
	size_t lev = ncsimple.DimLen( "lev" );
	size_t ncol = ncsimple.DimLen( "ncol" );	
	size_t start[] = {1, 0, 0};
	size_t count[] = {1, lev, ncol};

	size_t homme_size = lev * ncol;
	float* homme_buf = new float[ homme_size ];

	ncsimple.OpenRead( vars[varIdx] );
	rc = ncsimple.Read( start, count, homme_buf );
	ncsimple.Close();
	if( rc < 0 )
	{
		cerr << "ncsimple.Read() error: " << endl;
		exit(1);
	}

	FLOAT min, max, minfabs;
	SimpleStats( homme_buf, homme_size, &min, &max, &minfabs );
	printf("\tminfabs = %e, min = %e, max = %e\n", minfabs, min, max );

	delete[] homme_buf;
}
