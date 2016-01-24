/* 
 * This program reads a netcdf file, and outputs as raw data.
 *
 * Programmer: Samuel Li
 * Date: 1/8/2016
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "CamHandler.h"

const size_t NX = 91;
const size_t NY = 91;
const size_t NCOL = 48602;


int main( int argc, char* argv[] )
{

	string varname    = argv[1];
    string hommeMap   = "./HommeMap/reorderedCbasedHommeMapping.nc";
    string faceMap    = "./HommeMap/faceIdsNeNp304.nc";
    string netcdfInput = "/opt/Research_Data/ensemble_orig/cesm1_1.FC5.ne30_g16.000.cam.h0.0001-01-01-00000.nc";
	string rawOutput  = "./Datasets/faces/" + varname + ".float";

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
		/* VarInfo:
		 * This chunk of code lists all variables and their dimensions.
		 *
		cout << "Found variable: " << vars[i].GetName() << endl;
		vector< string > varDimNames = vars[i].GetDimNames();
		for( int j = 0; j < varDimNames.size(); j++ )
			cout << "\tdimension name: " << varDimNames[j] 
				 << ",\tlength: " << ncsimple.DimLen(varDimNames[j]) << endl;
		 * End VarInfo */
		 
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

	/* make sure this is a normal variable by testing it's dimension names */
	size_t lev, ncol, homme_size;
	float* homme_buf = NULL;
	if( varDimNames.size() == 3 )
	{
		assert( varDimNames[0].compare( "time" ) == 0 );
		assert( varDimNames[1].compare( "lev" ) == 0 );
		assert( varDimNames[2].compare( "ncol" ) == 0 );
		
		lev = ncsimple.DimLen( "lev" );
		ncol = ncsimple.DimLen( "ncol" );	
		size_t start[] = {1, 0, 0};
		size_t count[] = {1, lev, ncol};
		homme_size = lev * ncol;
		homme_buf = new float[ homme_size ];
		ncsimple.OpenRead( vars[varIdx] );
		rc = ncsimple.Read( start, count, homme_buf );
		ncsimple.Close();
	}
	else if( varDimNames.size() == 2 )
	{
		assert( varDimNames[0].compare( "time" ) == 0 );
		assert( varDimNames[1].compare( "ncol" ) == 0 );
		
		lev = 1;
		ncol = ncsimple.DimLen( "ncol" );	
		size_t start[] = {1, 0};
		size_t count[] = {1, ncol};
		homme_size = lev * ncol;
		homme_buf = new float[ homme_size ];
		ncsimple.OpenRead( vars[varIdx] );
		rc = ncsimple.Read( start, count, homme_buf );
		ncsimple.Close();
	}
	else
	{
		cerr << "ERROR: variable dimension not supported: " << vars[varIdx] << endl;
		exit(1);
	}

	if( rc < 0 )
	{
		cerr << "ncsimple.Read() error: " << endl;
		exit(1);
	}

	/* CamHandler to convert to RAW format */
	VAPoR::CamHandler handler( hommeMap, faceMap );
	size_t raw_size = 6 * lev * NX * NY;
	float* raw_buf  = new float[ raw_size ];
	handler.cam2raw( homme_buf, homme_size, lev, raw_buf, raw_size );

	/* save the 6 faces separately */
	for( int face = 0; face < 6; face++ )
    {
        /* locate start index for each face */
        size_t faceOffset = face * NX * NY * lev;

        /* generate filenames for each face */
        char  tmpName[ 256 ];
        char suffix[16];
        sprintf( suffix, ".face%d", face );
        strcpy( tmpName, rawOutput.c_str());
        strcat( tmpName, suffix );

		FILE* f = fopen( tmpName, "wb" );
		assert( f != NULL );
		fwrite( raw_buf + faceOffset, sizeof(float), NX*NY*lev, f );
		fclose(f);
    }
	

	/* Evaluation:
	 * This chunk of code uses SPECK to encode, then decode, and evaluate.
	 *
    int numXYDWTLevels = 4;
    int numZDWTLevels = 2;
	float targetRate = 8.0;
	char filename[] = "./dump/bit.stream";
	float* homme_reconstruct = new float[ homme_size ];
	handler.speckEncode2Dp1D ( homme_buf, homme_size, lev, numXYDWTLevels, 
								numZDWTLevels, targetRate, filename );
	handler.speckDecode3D( filename, homme_size, lev, homme_reconstruct );

	double rmse, lmax, nrmse, nlmax, minA, maxA, minB, maxB, meanA, meanB;
    handler.evaluate2arrays( homme_buf, homme_reconstruct, homme_size,
                             &rmse, &lmax,
                             &nrmse, &nlmax,
                             &minA, &maxA,
                             &minB, &maxB,
							 &meanA, &meanB );
	printf("Groud truth: mean = %.8e, min = %e, max = %e\n", meanA, minA, maxA );
	printf("Reconstruct: mean = %.8e, min = %e, max = %e\n", meanB, minB, maxB );
	printf("Mean Difference: %e\n", meanA - meanB );
	printf( "Reconstruction RMS  = %e, LMAX  = %e\n", rmse, lmax );
	printf( "Reconstruction NRMS = %e, NLMAX = %e\n", nrmse, nlmax );

	size_t pos_count = 0;
	size_t neg_count = 0;
	for( size_t i = 0; i < homme_size; i++ )
	{
		if( homme_buf[i] - homme_reconstruct[i] > 0 )
		{
			pos_count++;
			printf("%e ,\t%e,\t+ \n", homme_buf[i], homme_reconstruct[i]);
		}
		else if( homme_buf[i] - homme_reconstruct[i] < 0 )
		{
			neg_count++;
			printf("%e ,\t%e,\t- \n", homme_buf[i], homme_reconstruct[i]);
		}
		else
			printf("%e ,\t%e,\t= \n", homme_buf[i], homme_reconstruct[i]);
	}
	printf("plus = %lu, minus = %lu\n", pos_count, neg_count );

	delete[] homme_reconstruct;
	* End Evaluation */

	delete[] homme_buf;
	delete[] raw_buf;
}
