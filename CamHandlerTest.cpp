/* 
 * This program tests the CamHandler class.
 *
 * Programmer: Samuel Li
 * Date: 7/26/2015
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "CamHandler.h"

const size_t _NX = 91;
const size_t _NY = 91;
const size_t _NCOL = 48602;


int main( int argc, char* argv[] )
{
    if( argc != 4 )
    {
        std::cerr << "please specify NetCDF file, variable name, and target bit rate! " << endl;
        exit (1);
    }
	string netcdfInput = argv[1];
    string varname    = argv[2];
    float targetRate = atof( argv[3] );

    string hommeMap   = "./HommeMap/reorderedCbasedHommeMapping.nc";
    string faceMap    = "./HommeMap/faceIdsNeNp304.nc";
	//string netcdfInput = "/opt/Research_Data/ensemble_orig/cesm1_1.FC5.ne30_g16.000.cam.h0.0001-01-01-00000.nc";

    /* Examine if the file and variable valid */
    VAPoR::NetCDFSimple ncsimple;
    int rc = ncsimple.Initialize( netcdfInput );
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

	size_t lev, homme_size;
	float* homme_buf = NULL;
	if(  varDimNames.size() == 3 )
	{
		assert( varDimNames[0].compare( "time" ) == 0 );
		assert( varDimNames[1].compare( "lev" ) == 0 );
		assert( varDimNames[2].compare( "ncol" ) == 0 );

		lev = ncsimple.DimLen( "lev" );
		size_t ncol = ncsimple.DimLen( "ncol" );
		size_t start[] = {1, 0, 0};
		size_t count[] = {1, lev, ncol};
		homme_size = lev * ncol;
		homme_buf = new float[ homme_size ];

		ncsimple.OpenRead( vars[varIdx] );
		rc = ncsimple.Read( start, count, homme_buf );
		ncsimple.Close();
		if( rc < 0 ) {
			cerr << "ncsimple.Read() error: " << endl;
			exit(1);
		}
	}
	else if( varDimNames.size() == 2)
	{
		assert( varDimNames[0].compare( "time" ) == 0 );
        assert( varDimNames[1].compare( "ncol" ) == 0 );
		
		lev = 1;
		size_t ncol = ncsimple.DimLen( "ncol" );
		size_t start[] = {1, 0};            
        size_t count[] = {1, ncol};
        homme_size = lev * ncol;
        homme_buf = new float[ homme_size ];

        ncsimple.OpenRead( vars[varIdx] );
        rc = ncsimple.Read( start, count, homme_buf );
        ncsimple.Close();
        if( rc < 0 ) {
            cerr << "ncsimple.Read() error: " << endl;
            exit(1);
        }
	}

    /* Evaluation:
     * This chunk of code uses SPECK to encode, then decode, and evaluate.
     */
	VAPoR::CamHandler handler( hommeMap, faceMap );
    int numXYDWTLevels = 4;
    int numZDWTLevels = 2;
    char filename[] = "/flash_buffer/Sam/bit.stream";
    float* homme_reconstruct = new float[ homme_size ];

	if(  varDimNames.size() == 3 ) {
		cerr << "start 2D+1D DWT and SPECK encoding... " << endl;
		handler.speckEncode2Dp1D( homme_buf, homme_size, lev,
							   numXYDWTLevels, numZDWTLevels, targetRate, filename );
		cerr << "start 3D decoding... " << endl;
		handler.speckDecode3D( filename, homme_size, lev, homme_reconstruct ); 
	}
	else if (  varDimNames.size() == 2 ) {
		cerr << "start 2D SPECK encoding... " << endl;
		handler.speckEncode2D( homme_buf, homme_size, 
							   numXYDWTLevels, targetRate, filename );
		cerr << "start 2D decoding... " << endl;
		handler.speckDecode2D( filename, homme_size, homme_reconstruct );
	}

    double rmse, lmax, nrmse, nlmax, minA, maxA, minB, maxB, meanA, meanB, lmaxA, lmaxB, maxRE;
    handler.evaluate2arrays( homme_buf, homme_reconstruct, homme_size,
                             &rmse, &lmax,
                             &nrmse, &nlmax,
                             &minA, &maxA,
                             &minB, &maxB,
                             &meanA, &meanB, 
                             &lmaxA, &lmaxB,
							 &maxRE );
    printf("Groud truth: mean = %.8e, min = %e, max = %e\n", meanA, minA, maxA );
    printf("Reconstruct: mean = %.8e, min = %e, max = %e\n", meanB, minB, maxB );
    printf("Mean Difference: %e\n", meanA - meanB );
    printf("Reconstruction RMSE  = %e, LMAX  = %e\n", rmse, lmax );
    printf("Reconstruction NRMSE = %e, NLMAX = %e\n", nrmse, nlmax );
	printf("LMAX occurs with A[i] = %e, B[i] = %e.\n", lmaxA, lmaxB );
	/*
	printf("\t%e, %e\n", rmse, lmax );
	printf("\t%e, %e, %e\n", nrmse, nlmax, maxRE );
	printf("\t%e, %e\n", minA, maxA );
	*/

	/* 
	 * Debug info: output every data point
 	 *
	size_t pos_count = 0;
    size_t neg_count = 0;
	cout << endl;
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
	*/

    delete[] homme_reconstruct;
    /* End Evaluation */

	delete[] homme_buf;


/*

    cerr << "start building map... " << endl;
    VAPoR::CamHandler handler( hommeMap, faceMap );

    float* homme_buf_comp = new float[ homme_size ];

    cerr << "start 3D DWT and SPECK encoding... " << endl;
    handler.speckEncode3D( homme_buf, homme_size, LEV,
                           numZDWTLevels, targetRate, filename );



    delete[] homme_buf;
    delete[] homme_buf_comp;
*/

}
