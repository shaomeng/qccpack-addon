#include "CamHandler.h"

using namespace VAPoR;
using namespace VetsUtil;

CamHandler::CamHandler( string &mapfile, string &facefile )
{
    _NX = 91;
    _NY = 91;
    _NCOL = 48602;

    /* Make sure the two files are there */
    FILE* f = fopen( mapfile.c_str(), "r" );
    if( f == NULL ) {
        cerr << "File open error: " << mapfile << endl;
        exit(1);
    }
    else    fclose( f );

    FILE* g = fopen( facefile.c_str(), "r" );
    if( g == NULL ) {
        cerr << "File open error: " << facefile << endl;
        exit(1);
    }
    else    fclose( g );

    _faceIndicesAll.resize( 6 );

    InitializeFaceIndicesAll( mapfile, facefile );
}



void CamHandler::cam2raw( float* homme_buf,                
                          size_t homme_size,
                          int    LEV,
                          float* orig_buf,
                          size_t orig_size )                     
{
    /* sanity check on dimensions */
    assert( homme_size % _NCOL == 0 );
    assert( homme_size / _NCOL == LEV );
    assert( orig_size == 6 * _NX * _NY * LEV );

	float min = 0.0, max = 0.0;
	for (int face=0; face<6; face++) 
    {
        size_t faceOffset = face * _NX * _NY * LEV;
		const vector <int> &faceIndices = _faceIndicesAll[face];
		bool first = true;
		float mymin = 0.0, mymax = 0.0;
		for (int z = 0; z < LEV; z++) 
        {
			for (size_t i = 0; i<faceIndices.size(); i++) {
				float t = homme_buf[z*_NCOL + faceIndices[i]];
		        if (first) {
				    mymin = min = t;
				    mymax = max = t;
					first = false;
			    }
				if (t<mymin) mymin = t;
				if (t>mymax) mymax = t;
				orig_buf[faceOffset + z*_NX*_NY + i] = t;
			}
		}
		if (mymin<min) min = mymin;
		if (mymax>max) max = mymax;
	}
}

void CamHandler::raw2cam( float* orig_buf,                
                          size_t orig_size,
                          float* homme_buf,
                          size_t homme_size,
                          int    LEV )                     
{
    /* sanity check on dimensions */
    assert( homme_size % _NCOL == 0 );
    assert( homme_size / _NCOL == LEV );
    assert( orig_size == 6 * _NX * _NY * LEV );

	float min = 0.0, max = 0.0;
	for (int face=0; face<6; face++) 
    {
        size_t faceOffset = face * _NX * _NY * LEV;
		const vector <int> &faceIndices = _faceIndicesAll[face];
		bool first = true;
		float mymin = 0.0, mymax = 0.0;
		for (int z = 0; z<LEV; z++) 
        {
			for (size_t i = 0; i<faceIndices.size(); i++) {
                float t = orig_buf[ faceOffset + z*_NX*_NY + i ];
                if( first ) {
                    mymin = min = t;
                    mymax = max = t;
                    first = false;
                }
                if( t < mymin )     mymin = t;
                if( t > mymax )     mymax = t;
				homme_buf[z*_NCOL + faceIndices[i]] = t;
            }
        }
        if( mymin < min )       min = mymin;
        if( mymax > max )       max = mymax;
	}
}





/*
 * Helper function for InitializeFaceIndeciesAll().
 */
int myExp10(int exponent) {
  int i;
  int expVal = 1;
  for (i = 0; i < exponent; i++) {
    expVal *= 10;
  }
  return expVal; 
}

/*
 * Helper function for InitializeFaceIndeciesAll().
 */
bool isOnFace(int faceId,int  faceIndex) {
  int val = (faceId / myExp10(faceIndex)) % 10;
  return (val != 0);
}

//
//	n0----n3
//	|     |
//  |     |
//	n1----n2
//
/*
 * Helper function for InitializeFaceIndeciesAll().
 */
typedef struct {
	int n0, n1, n2, n3;
} neighbors_t;

/*
 * Helper function for InitializeFaceIndeciesAll().
 */
int getNeighborMap(string facefile, vector <neighbors_t> &neighbormap) {
	neighbormap.clear();

	NetCDFSimple ncsimple;

	int rc;
	rc = ncsimple.Initialize(facefile);
	if (rc < 0) return(-1);

	vector <NetCDFSimple::Variable> vars = ncsimple.GetVariables();
	assert (vars.size() == 1);

	vector <string> dimnames = vars[0].GetDimNames();
	assert(dimnames.size());
	size_t ncenters = ncsimple.DimLen(dimnames[0]);

	int *buf = new int[ncenters *4];

	rc = ncsimple.OpenRead(vars[0]);
	if (rc<0) return(-1);

	size_t start[] = {0,0};
	size_t count[] = {ncenters, 4};
	rc = ncsimple.Read(start, count, buf);
	if (rc<0) return(-1);

	(void) ncsimple.Close();

	neighbors_t neighbors;
	for (size_t i = 0; i<ncenters; i++) {
		neighbors.n0 = buf[i*4+0];
		neighbors.n1 = buf[i*4+1];
		neighbors.n2 = buf[i*4+2];
		neighbors.n3 = buf[i*4+3];
		neighbormap.push_back(neighbors);
	}

    delete[] buf;

	return(0);
}

/*
 * Helper function for InitializeFaceIndeciesAll().
 */
int getFaceMap(string file, vector <int> &facemap) {

	NetCDFSimple ncsimple;

	int rc;
	rc = ncsimple.Initialize(file);
	if (rc < 0) return(-1);

	vector <NetCDFSimple::Variable> vars = ncsimple.GetVariables();
	assert (vars.size() == 1);

	vector <string> dimnames = vars[0].GetDimNames();
	assert(dimnames.size());
	size_t ncol = ncsimple.DimLen(dimnames[0]);

	int *buf = new int[ncol];

	rc = ncsimple.OpenRead(vars[0]);
	if (rc<0) return(-1);

	size_t start[] = {0};
	size_t count[] = {ncol};
	rc = ncsimple.Read(start, count, buf);
	if (rc<0) return(-1);

	(void) ncsimple.Close();

	for (size_t i = 0; i<ncol; i++) {
		facemap.push_back(buf[i]);
	}

    delete[] buf;

	return(0);
}

/*
 * Helper function for InitializeFaceIndeciesAll().
 */
vector <int> getFaces(const vector <int> &facemap, int node) {

	assert(node>=0 && node < facemap.size());
	vector <int> faces;
	
	for (int face=0; face<6; face++) {
		if (isOnFace(facemap[node],face)) {
			faces.push_back(face);
		}
	}
	return(faces);
}

/*
 * Helper function for InitializeFaceIndeciesAll().
 */
int getFirstCenter(
	const vector <int> &facemap,
	const vector <neighbors_t> &neighbormap,
	int face
) {

	int center = -1;
	for (size_t c=0; c<neighbormap.size(); c++) {
		neighbors_t neighbors = neighbormap[c];

		// is this a corner node
		//
		vector <int> n0faces = getFaces(facemap, neighbors.n0);
		if (n0faces.size() == 3) { 
			vector <int> n1faces = getFaces(facemap, neighbors.n1);
			vector <int> n2faces = getFaces(facemap, neighbors.n2);
			vector <int> n3faces = getFaces(facemap, neighbors.n3);
			assert(n1faces.size() == 2);	// edge
			assert(n2faces.size() == 1); // interior
			assert(n3faces.size() == 2);	// edge

			if ((n2faces[0] == face) && 
				(find(n0faces.begin(), n0faces.end(), face) != n0faces.end()) &&
				(find(n1faces.begin(), n1faces.end(), face) != n1faces.end()) &&
				(find(n3faces.begin(), n3faces.end(), face) != n3faces.end()) 
			) {
				assert(center == -1);	// there can be only one
				center = c;
			}
		}
	}
	return(center);
}

//
// Get the cell (element) that has node 'node' at the position
// p (0,1,3,4)
//
/*
 * Helper function for InitializeFaceIndeciesAll().
 */
int getCell(
	int node,
	int p,
	const vector <int> &facemap,
	const vector <neighbors_t> &neighbormap,
	int face
) {
	if (p==0) {
		for (size_t c=0; c<neighbormap.size(); c++) {
			neighbors_t neighbors = neighbormap[c];
			if (neighbors.n0 == node && 
				isOnFace(facemap[neighbors.n0], face) &&
				isOnFace(facemap[neighbors.n1], face) &&
				isOnFace(facemap[neighbors.n2], face) &&
				isOnFace(facemap[neighbors.n3], face)) {

					return(c);
			}
		}
	}
	else if (p==1) {
		for (size_t c=0; c<neighbormap.size(); c++) {
			neighbors_t neighbors = neighbormap[c];
			if (neighbors.n1 == node  && 
				isOnFace(facemap[neighbors.n0], face) &&
				isOnFace(facemap[neighbors.n1], face) &&
				isOnFace(facemap[neighbors.n2], face) &&
				isOnFace(facemap[neighbors.n3], face)) {

					return(c);
				}
		}
	}
	else if (p==2) {
		for (size_t c=0; c<neighbormap.size(); c++) {
			neighbors_t neighbors = neighbormap[c];
			if (neighbors.n2 == node  && 
				isOnFace(facemap[neighbors.n0], face) &&
				isOnFace(facemap[neighbors.n1], face) &&
				isOnFace(facemap[neighbors.n2], face) &&
				isOnFace(facemap[neighbors.n3], face)) {

					return(c);
				}
		}
	}
	else if (p==3) {
		for (size_t c=0; c<neighbormap.size(); c++) {
			neighbors_t neighbors = neighbormap[c];
			if (neighbors.n3 == node  && 
				isOnFace(facemap[neighbors.n0], face) &&
				isOnFace(facemap[neighbors.n1], face) &&
				isOnFace(facemap[neighbors.n2], face) &&
				isOnFace(facemap[neighbors.n3], face)) {

					return(c);
				}
		}
	}

	return(-1);
}

/*
 * Helper function for InitializeFaceIndeciesAll().
 */
int getXNeighbor(
	int node,
	const vector <int> &facemap,
	const vector <neighbors_t> &neighbormap,
	int face
) {

	int p = 0;
	int c = getCell(node, p, facemap, neighbormap, face);
	if (c >= 0) {

		neighbors_t neighbors = neighbormap[c];

		int node3 = neighbors.n3;
		if (isOnFace(facemap[node],face) && isOnFace(facemap[node3],face)) {
			return(node3);
		}
	}
	else {
		p = 1;
		int c = getCell(node, p, facemap, neighbormap, face);
		if (c >=0) {
			neighbors_t neighbors = neighbormap[c];

			int node2 = neighbors.n2;
			if (isOnFace(facemap[node],face) && isOnFace(facemap[node2],face)) {
				return(node2);
			}
		}
	}
	return(-1);
}

/*
 * Helper function for InitializeFaceIndeciesAll().
 */
int getYNeighbor(
	int node,
	const vector <int> &facemap,
	const vector <neighbors_t> &neighbormap,
	int face
) {
	int p = 0;
	int c = getCell(node, p, facemap, neighbormap, face);
	if (c >= 0) {

		neighbors_t neighbors = neighbormap[c];

		int node1 = neighbors.n1;
		if (isOnFace(facemap[node],face) && isOnFace(facemap[node1],face)) {
			return(node1);
		}
	}
	return(-1);
}

/*
 * Helper function for InitializeFaceIndeciesAll().
 */
int getFaceIndices(
	int startnode, 
	const vector <int> &facemap,
	const vector <neighbors_t> &neighbormap,
	int face,
	vector <int> &faceIndices,
	int &nx,
	int &ny
) {
	faceIndices.clear();
	nx = 1;
	ny = 1;

	int nodej = startnode;
	faceIndices.push_back(startnode);
	bool done = false;
	bool first = true;
	assert(getFaces(facemap, nodej).size() == 3);
	while (! done) {
		int node = nodej;
		int mynx = 1;
		assert(getFaces(facemap, node).size() >= 2);
		int nodenext;
		while ((nodenext = getXNeighbor(node, facemap, neighbormap, face)) >= 0) { 
			faceIndices.push_back(nodenext);
			node = nodenext;
			if (first) nx++;
			mynx++;
		}
		assert(getFaces(facemap, node).size() >= 2);
		if (first) {
			first = false;
		}
		else {
			assert(mynx == nx);
		}
		nodenext = getYNeighbor(nodej, facemap, neighbormap, face);
		if (nodenext < 0) {
			assert(getFaces(facemap, nodej).size() == 3);
			done = true;
		}
		else {
			assert(getFaces(facemap, nodenext).size() >= 2);
			faceIndices.push_back(nodenext);
			nodej = nodenext;
			ny++;
		}
	}
	assert(faceIndices.size() == nx*ny);

	return(0);
}


int CamHandler::InitializeFaceIndicesAll( string &mapfile,
                                           string &facefile )
{
	vector <int> facemap;
	int rc = getFaceMap(facefile, facemap);
	if (rc < 0) 
        return (1);

	vector <neighbors_t> neighbormap;
	rc = getNeighborMap(mapfile, neighbormap);
	if (rc < 0) 
        return (1);

	vector <int> facecount(6);
	for (size_t i=0; i<facemap.size(); i++) 
		for (int face=0; face<6; face++) 
			if (isOnFace(facemap[i],face)) 
				facecount[face]++;

	vector <int> startNodes;
	for (int face=0; face<6; face++) {
		int center = getFirstCenter(facemap, neighbormap, face);
		assert(center >= 0 && center < neighbormap.size());
		startNodes.push_back(neighbormap[center].n0);
	}

	int nx, ny;
	for (int face=0; face<6; face++) {
		vector <int> &faceIndices = _faceIndicesAll[face];
		rc = getFaceIndices(
			startNodes[face], facemap, neighbormap, face, faceIndices, nx, ny );
		if (rc < 0) 
            return (1);
		assert(faceIndices.size() == nx*ny);
	}

	return 0;
}
            
/*
 * Helper functions for SPECK encoding and decoding.
 */
extern "C"
{
    void myspeckencode3d( const float* srcBuf,
                   int srcX,
                   int srcY,
                   int srcZ,
                   const char* outputFilename,
                   int nLevels,
                   float TargetRate );

    void myspeckencode2p1d( const float* srcBuf,
                       int srcX,
                       int srcY,
                       int srcZ,
                       const char* outputFilename,
                       int XYNumLevels,
                       int ZNumLevels,
                       float TargetRate );

    void myspeckdecode3d( const char*  inputFilename,
                     float* dstBuf,
                     int    outSize );

    void myspeckencode2d( const float* srcBuf,
                           int srcX,
                           int srcY,
                           const char* outputFilename,
                           int nLevels,
                           float TargetRate );

    void myspeckdecode2d( const char*  inputFilename,
                         float* dstBuf,
                         int    outSize );
	
	void mySpeck3DEncodeOnly( double* coeff_buf,
				  long nx, long ny, long nz,
				  double image_mean,
				  int num_levels,
				  int target_bit_cnt,
				  const char* output_name );
};

/* 
 * Only performs SPECK encoding, but not including wavelets.
 * For research only.
 */
void CamHandler::Speck3DEncodeOnly( double* coeff_buf,
				  long nx, long ny, long nz,
				  double image_mean,
				  int num_levels,
				  int target_bit_cnt,
				  const char* output_name )
{
	mySpeck3DEncodeOnly( coeff_buf,
				  		 nx, ny, nz,
				  		 image_mean,
				  		 num_levels,
				  		 target_bit_cnt,
				  		 output_name );

}

// super simple test functions
void CamHandler::testencode2d( const float* srcBuf, int srcX, int srcY,
                       const char* outputFilename, int nLevels, float TargetRate ) 
{
    myspeckencode2d( srcBuf, srcX, srcY, outputFilename, nLevels, TargetRate );
}
void CamHandler::testdecode2d( const char* inputFilename, float* dstBuf, int outSize )
{
    myspeckdecode2d( inputFilename, dstBuf, outSize );
}
// finish test functions

void CamHandler::speckEncode3D( float* homme_buf,
                                size_t homme_size,
                                int LEV,
                                int numDWTLevels,
                                float targetRate,
                                char* outputFilename )
{
    /* sanity check on dimensions */
    assert( homme_size % _NCOL == 0 );
    assert( homme_size / _NCOL == LEV );
    
    /* convert homme array to raw array */
    size_t raw_size = 6 * _NX * _NY * LEV ;
    float* raw_buf = new float[ raw_size ];
    cam2raw( homme_buf, homme_size, LEV, raw_buf, raw_size );
    
    /* speck encoding on each of the 6 faces */
    for( int face = 0; face < 6; face++ )
    {
        /* locate start index for each face */
        size_t faceOffset = face * _NX * _NY * LEV;

        /* generate filenames for each face */
        char  tmpName[ 1024 ];
        char suffix[64];
        sprintf( suffix, ".face%d", face );
        strcpy( tmpName, outputFilename );
        strcat( tmpName, suffix );

        myspeckencode3d( raw_buf + faceOffset, _NX, _NY, LEV, 
                         tmpName, numDWTLevels, targetRate );
    }

    delete[] raw_buf;
}

void CamHandler::speckEncode2Dp1D( float* homme_buf,
                                  size_t homme_size,
                                  int LEV,
                                  int XYNumDWTLevels,
                                  int ZNumDWTLevels,
                                  float targetRate,
                                  char* outputFilename )
{
    /* sanity check on dimensions */
    assert( homme_size % _NCOL == 0 );
    assert( homme_size / _NCOL == LEV );
    
    /* convert homme array to raw array */
    size_t raw_size = 6 * _NX * _NY * LEV ;
    float* raw_buf = new float[ raw_size ];
    cam2raw( homme_buf, homme_size, LEV, raw_buf, raw_size );
    
    /* speck encoding on each of the 6 faces */
    for( int face = 0; face < 6; face++ )
    {
        /* locate start index for each face */
        size_t faceOffset = face * _NX * _NY * LEV;

        /* generate filenames for each face */
        char  tmpName[ 256 ];
        char suffix[64];
        sprintf( suffix, ".face%d", face );
        strcpy( tmpName, outputFilename );
        strcat( tmpName, suffix );

		/* save min max values separately */
		float minmax[2];
		minmax[0] = *(raw_buf+faceOffset);
		minmax[1] = *(raw_buf+faceOffset);
		for( size_t i = 0; i < _NX*_NY*LEV; i++ )
		{
			if( *(raw_buf+faceOffset+i) < minmax[0] )
				minmax[0] = *(raw_buf+faceOffset+i);
			if( *(raw_buf+faceOffset+i) > minmax[1] )
				minmax[1] = *(raw_buf+faceOffset+i);
		}
		char minmaxName[256];
		strcpy( minmaxName, tmpName );
		strcat( minmaxName, ".minmax" );
		FILE* f = fopen( minmaxName, "wb" );
		fwrite( minmax, sizeof(float), 2, f );
		fclose(f);

        myspeckencode2p1d( raw_buf + faceOffset, _NX, _NY, LEV,
                           tmpName, XYNumDWTLevels, ZNumDWTLevels, targetRate );

    }

    delete[] raw_buf;
}

void CamHandler::speckDecode3D( char*  inputFilename,
                                size_t homme_size,
                                int LEV,
                                float* homme_buf )
{
    /* sanity check on dimensions */
    assert( homme_size % _NCOL == 0 );
    assert( homme_size / _NCOL == LEV );
    
    size_t raw_size = 6 * _NX * _NY * LEV ;
    float* raw_buf = new float[ raw_size ];

    /* speck decoding on each of the 6 faces */
    for( int face = 0; face < 6; face++ )
    {
        /* locate start index for each face */
        size_t faceOffset = face * _NX * _NY * LEV;

        /* generate filenames for each face */
        char  tmpName[ 1024 ];
        char suffix[64];
        sprintf( suffix, ".face%d", face );
        strcpy( tmpName, inputFilename );
        strcat( tmpName, suffix );

        myspeckdecode3d( tmpName, raw_buf + faceOffset, _NX * _NY * LEV );

		/* read min max values separately,
		   and clamp reconstructed values to original range. */
		float minmax[2];
		char minmaxName[256];
		strcpy( minmaxName, tmpName );
		strcat( minmaxName, ".minmax" );
		FILE* f = fopen( minmaxName, "rb" );
		fread( minmax, sizeof(float), 2, f );
		fclose(f);
		for( size_t i = 0; i < _NX*_NY*LEV; i++ )
		{
			if( *(raw_buf+faceOffset+i) < minmax[0] )
				*(raw_buf+faceOffset+i) = minmax[0];
			if( *(raw_buf+faceOffset+i) > minmax[1] )
				*(raw_buf+faceOffset+i) = minmax[1];
		}
    }

    raw2cam( raw_buf, raw_size, homme_buf, homme_size, LEV );

    delete[] raw_buf;
}

void CamHandler::speckEncode2D( float* homme_buf,
                                size_t homme_size,
                                int numDWTLevels,
                                float targetRate,
                                char* outputFilename )
{
    /* sanity check on dimensions */
    assert( homme_size == _NCOL );

    /* convert homme array to raw array */
    size_t raw_size = 6 * _NX * _NY;
    float* raw_buf = new float[ raw_size ];
    cam2raw( homme_buf, homme_size, 1, raw_buf, raw_size );

    /* speck encoding on each of the 6 faces */
    for( int face = 0; face < 6; face++ )
    {
        /* locate start index for each face */
        size_t faceOffset = face * _NX * _NY;

        /* generate filenames for each face */
        char  tmpName[ 1024 ];
        char suffix[64];
        sprintf( suffix, ".face%d", face );
        strcpy( tmpName, outputFilename );
        strcat( tmpName, suffix );

		/* save min max values separately */
		float minmax[2];
		minmax[0] = *(raw_buf+faceOffset);
		minmax[1] = *(raw_buf+faceOffset);
		for( size_t i = 0; i < _NX*_NY; i++ )
		{
			if( *(raw_buf+faceOffset+i) < minmax[0] )
				minmax[0] = *(raw_buf+faceOffset+i);
			if( *(raw_buf+faceOffset+i) > minmax[1] )
				minmax[1] = *(raw_buf+faceOffset+i);
		}
		char minmaxName[256];
		strcpy( minmaxName, tmpName );
		strcat( minmaxName, ".minmax" );
		FILE* f = fopen( minmaxName, "wb" );
		fwrite( minmax, sizeof(float), 2, f );
		fclose(f);

        myspeckencode2d( raw_buf + faceOffset, _NX, _NY,
                         tmpName, numDWTLevels, targetRate );
    }

    delete[] raw_buf;
}

void CamHandler::speckEncodeMany2D( float* homme_buf,
                                    size_t homme_size,
                                    int LEV,
                                    int XYNumDWTLevels,
                                    float targetRate,
                                    char* outputFilename )
{
    /* sanity check on dimensions */
    assert( homme_size % _NCOL == 0 );
    assert( homme_size / _NCOL == LEV );

    /* convert homme array to raw array */
    size_t raw_size = 6 * _NX * _NY * LEV ;
    float* raw_buf = new float[ raw_size ];
    cam2raw( homme_buf, homme_size, LEV, raw_buf, raw_size );

    /* speck encoding on each of the 6 faces */
    for( int face = 0; face < 6; face++ )
    {
        /* locate start index for each face */
        size_t faceOffset = face * LEV * _NX * _NY;

		for( int l = 0; l < LEV; l++ )
		{
			size_t levelOffset = _NX * _NY * l;
			/* generate filenames for each layer */
			char  tmpName[ 256 ];
			char suffix[64];
			sprintf( suffix, ".face%d.level%d", face, l );
			strcpy( tmpName, outputFilename );
			strcat( tmpName, suffix );

        	myspeckencode2d( raw_buf + faceOffset + levelOffset, 
					_NX, _NY, tmpName, XYNumDWTLevels, targetRate );
		}

    }

    delete[] raw_buf;
}

void CamHandler::speckDecodeMany2D( char*  inputFilename,
                            size_t homme_size,
                            int LEV,
                            float* homme_buf )
{
	/* sanity check on dimensions */
    assert( homme_size % _NCOL == 0 );
    assert( homme_size / _NCOL == LEV );

    size_t raw_size = 6 * _NX * _NY * LEV ;
    float* raw_buf = new float[ raw_size ];

    /* speck decoding on each of the 6 faces */
    for( int face = 0; face < 6; face++ )
    {
        /* locate start index for each face */
        size_t faceOffset = face * _NX * _NY * LEV;

		for( int l = 0; l < LEV; l++ )
		{
			size_t levelOffset = _NX*_NY*l;
			char  tmpName[ 256 ];
            char suffix[64];
            sprintf( suffix, ".face%d.level%d", face, l );
            strcpy( tmpName, inputFilename );
            strcat( tmpName, suffix );
		
			myspeckdecode2d( tmpName, raw_buf + faceOffset + levelOffset,
							 _NX * _NY );
		}
    }

    raw2cam( raw_buf, raw_size, homme_buf, homme_size, LEV );

    delete[] raw_buf;
}

void CamHandler::speckDecode2D( char*  inputFilename,
                                size_t homme_size,
                                float* homme_buf )
{
    /* Sanity check on dimensions */
    assert( homme_size == _NCOL );

    size_t raw_size = 6 * _NX * _NY;
    float* raw_buf = new float[ raw_size ];

    /* speck decoding on each of the 6 faces */
    for( int face = 0; face < 6; face++ )
    {
        /* locate start index for each face */
        size_t faceOffset = face * _NX * _NY;

        /* generate filenames for each face */
        char  tmpName[ 1024 ];
        char suffix[64];
        sprintf( suffix, ".face%d", face );
        strcpy( tmpName, inputFilename );
        strcat( tmpName, suffix );

        myspeckdecode2d( tmpName, raw_buf + faceOffset, _NX * _NY );

		/* read min max values separately,
		   and clamp reconstructed values to original range. */
		float minmax[2];
		char minmaxName[256];
		strcpy( minmaxName, tmpName );
		strcat( minmaxName, ".minmax" );
		FILE* f = fopen( minmaxName, "rb" );
		fread( minmax, sizeof(float), 2, f );
		fclose(f);
		for( size_t i = 0; i < _NX*_NY; i++ )
		{
			if( *(raw_buf+faceOffset+i) < minmax[0] )
				*(raw_buf+faceOffset+i) = minmax[0];
			if( *(raw_buf+faceOffset+i) > minmax[1] )
				*(raw_buf+faceOffset+i) = minmax[1];
		}
    }

    raw2cam( raw_buf, raw_size, homme_buf, homme_size, 1 );

    delete[] raw_buf;
}

/*
void CamHandler::evaluate2arrays( const float* A, const float* B, size_t len, 
                                  double* minmaxA, double* minmaxB, 
                                  double* rms, double* nrmse, 
                                  double* lmax, double* nlmax )
{
    double sum = 0.0;
    double c = 0.0;
    double max = 0.0;
    double tmp;
    size_t i;
    double minA = A[0];
    double maxA = A[0];
    double minB = B[0];
    double maxB = B[0];
    for( i = 0; i < len; i++) {
        tmp = (double)A[i] - (double)B[i];
        if (tmp < 0)        tmp *= -1.0;
        if (tmp > max)      max = tmp;
        double y = tmp * tmp - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;

        if( A[i] < minA )   minA = A[i];
        if( A[i] > maxA )   maxA = A[i];
        if( B[i] < minB )   minB = B[i];
        if( B[i] > maxB )   maxB = B[i];
    }
    sum /= (double)len;
    sum = sqrt( sum );

    *rms = sum;
    *lmax = max;
    minmaxA[0] = minA;
    minmaxA[1] = maxA;
    minmaxB[0] = minB;
    minmaxB[1] = maxB;

    *nrmse = sum / (maxA - minA);
    *nlmax = max / (maxA - minA);
}
*/




