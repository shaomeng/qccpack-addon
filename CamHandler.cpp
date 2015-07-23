#include "CamHandler.h"

using namespace VAPoR;
using namespace VetsUtil;

CamHandler::CamHandler( string &mapfile, string &facefile )
{
    _NX = 91;
    _NY = 91;
    _ILEV = 31;
    _NCOL = 48602;

    _faceIndicesAll.resize( 6 );

    InitializeFaceIndicesAll( mapfile, facefile );
}

void CamHandler::cam2raw( float* homme_orig_buf,                
                          float* orig_buf )                     
{
	float min = 0.0, max = 0.0;
	for (int face=0; face<6; face++) 
    {
		const vector <int> &faceIndices = _faceIndicesAll[face];
		bool first = true;
		float mymin = 0.0, mymax = 0.0;
		for (int z = 0; z < _ILEV; z++) 
        {
			for (int i = 0; i<faceIndices.size(); i++) {
				float t = homme_orig_buf[z*_NCOL + faceIndices[i]];
		        if (first) {
				    mymin = min = t;
				    mymax = max = t;
					first = false;
			    }
				if (t<mymin) mymin = t;
				if (t>mymax) mymax = t;
				orig_buf[z*_NX*_NY + i] = t;
			}
		}
		if (mymin<min) min = mymin;
		if (mymax>max) max = mymax;
	}
}

void CamHandler::raw2cam( float* orig_buf,                
                          float* homme_orig_buf )                     
{
	float min = 0.0, max = 0.0;
	for (int face=0; face<6; face++) 
    {
		const vector <int> &faceIndices = _faceIndicesAll[face];
		bool first = true;
		float mymin = 0.0, mymax = 0.0;

		for (int z = 0; z<_ILEV; z++) 
        {
			for (int i = 0; i<faceIndices.size(); i++) {
                float t = orig_buf[ z*_NX*_NY + i ];
                if( first ) {
                    mymin = min = t;
                    mymax = max = t;
                    first = false;
                }
                if( t < mymin )     mymin = t;
                if( t > mymax )     mymax = t;
				homme_orig_buf[z*_NCOL + faceIndices[i]] = t;
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
	for (int i = 0; i<ncenters; i++) {
		neighbors.n0 = buf[i*4+0];
		neighbors.n1 = buf[i*4+1];
		neighbors.n2 = buf[i*4+2];
		neighbors.n3 = buf[i*4+3];
		neighbormap.push_back(neighbors);
	}

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

	for (int i = 0; i<ncol; i++) {
		facemap.push_back(buf[i]);
	}

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
	for (int c=0; c<neighbormap.size(); c++) {
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
		for (int c=0; c<neighbormap.size(); c++) {
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
		for (int c=0; c<neighbormap.size(); c++) {
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
		for (int c=0; c<neighbormap.size(); c++) {
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
		for (int c=0; c<neighbormap.size(); c++) {
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
	for (int i=0; i<facemap.size(); i++) 
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
}
            
/*
 * Helper functions for SPECK encoding and decoding.
 */
extern "C"
{
    int myspeckencode3d( float* srcBuf,
                   int srcX,
                   int srcY,
                   int srcZ,
                   char* outputFilename,
                   int nLevels,
                   float TargetRate );

    int myspeckencode2p1d( float* srcBuf,
                       int srcX,
                       int srcY,
                       int srcZ,
                       char* outputFilename,
                       int XYNumLevels,
                       int ZNumLevels,
                       float TargetRate );

    int myspeckdecode( char*  inputFilename,
                     float* dstBuf,
                     int    outSize );
};

int  CamHandler::speckEncode3D( float* srcBuf,
                                char* outputFilename,
                                int numLevels,
                                float targetRate )
{
    return myspeckencode3d( srcBuf, _NX, _NY, _ILEV, 
                            outputFilename, numLevels, targetRate );
}

int CamHandler::speckEncode2Dp1D( float* srcBuf,
                                  char* outputFilename,
                                  int XYNumLevels,
                                  int ZNumLevels,
                                  float targetRate )
{
    return myspeckencode2p1d( srcBuf, _NX, _NY, _ILEV, outputFilename, 
                              XYNumLevels, ZNumLevels, targetRate );
}

int CamHandler::speckdecode( char*  inputFilename,
                             float* dstBuf )
{
    return myspeckdecode( inputFilename, dstBuf,
                          _NX * _NY * _ILEV );

}

void CamHandler::evaluate2arrays( float* A, float* B, int len, 
                                  double* rms, double* lmax )
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