#include <cstdio>
#include <iostream>
#include <cassert>
#include "vapor/NetCDFSimple.h"
#include "vapor/NetCDFCollection.h"
#include "vapor/Compressor.h"
#include "netcdf.h"

using namespace VAPoR;
using namespace VetsUtil;

const size_t NX = 91;
const size_t NY = 91;
const size_t NCOL = 48602;
const size_t LEV = 30;
const size_t ILEV = 31;
const float CRATIO_2D = 5;
const float CRATIO_3D = 5;

//#define	DOUBLE
#ifdef	DOUBLE
typedef double coeff_t;
#else
typedef float coeff_t;
#endif


void Usage(string pname) {
	cerr << pname << ": HommeMapping.nc faceIdsNeNp304.nc datafiles..." << endl;
}


int myExp10(int exponent) {
  int i;
  int expVal = 1;
  for (i = 0; i < exponent; i++) {
    expVal *= 10;
  }
  return expVal; 
}

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
typedef struct {
	int n0, n1, n2, n3;
} neighbors_t;

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

int getFaceIndecies(
	int startnode, 
	const vector <int> &facemap,
	const vector <neighbors_t> &neighbormap,
	int face,
	vector <int> &faceIndecies,
	int &nx,
	int &ny
) {
	faceIndecies.clear();
	nx = 1;
	ny = 1;

	int nodej = startnode;
	faceIndecies.push_back(startnode);
	bool done = false;
	bool first = true;
	assert(getFaces(facemap, nodej).size() == 3);
	while (! done) {
		int node = nodej;
		int mynx = 1;
		assert(getFaces(facemap, node).size() >= 2);
		int nodenext;
		while ((nodenext = getXNeighbor(node, facemap, neighbormap, face)) >= 0) { 
			faceIndecies.push_back(nodenext);
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
			faceIndecies.push_back(nodenext);
			nodej = nodenext;
			ny++;
		}
	}
	assert(faceIndecies.size() == nx*ny);

	return(0);
}

void remove_missing(
	coeff_t *data,
	size_t n,
	float mv,
	SignificanceMap &smap
) {
	smap.Clear();

	vector <size_t> dims;
	dims.push_back(n);
	smap.Reshape(dims);

	double ave = 0.0;
	double count = 0;
	for (size_t i=0; i<n; i++) {
		if (data[i] == mv) {
			smap.Set(i);
		}
		else {
			ave += data[i];
			count++;
		}
	}
	if (count) ave /= count;

	for (size_t i=0; i<n; i++) {
		if (data[i] == mv) data[i] = ave;
	}
}

void replace_missing(
	coeff_t *data,
	float mv,
	const SignificanceMap &smap
) {
	vector <size_t> dims;
	smap.GetShape(dims);
	assert(dims.size() == 1);

	for (size_t i=0; i<dims[0]; i++) {
		if (smap.Test(i)) data[i] = mv;
	}
}

void compute_error(
    const float *odata, const float *cdata, size_t nelements,
	bool has_missing, float mv,
    double *lmaxerr, double *lmax,
    double *l2sqrerr, double *l2sqr
) {
    double delta;

    *lmaxerr = 0.0;
    *l2sqrerr = 0.0;
    *lmax = 0.0;
    *l2sqr = 0.0;
    for (size_t idx=0; idx<nelements; idx++) {
		if (has_missing && odata[idx] == mv) continue;

        delta = fabs(odata[idx] - cdata[idx]);
        if (delta > *lmaxerr) *lmaxerr = delta;
        if (fabs(odata[idx]) > *lmax) *lmax = fabs(odata[idx]);

        *l2sqrerr += (odata[idx] - cdata[idx]) * (odata[idx] - cdata[idx]);
        *l2sqr += odata[idx] * odata[idx];
    }
}

void dump_grid(size_t ts, string var, int face, float *data, size_t n) 
{
	char path[1024];
	sprintf(path, "%s.%4.4d.%d", var.c_str(), (int) ts, face);
	FILE *fp = fopen(path, "w");
	assert(fp!=NULL);

	fwrite(data, sizeof(data[0]), n, fp);
	fclose(fp);
}

/*
void compress_var(
	size_t ts,
	string var,
	NetCDFCollection &ncdfc,
	const vector < vector <int> > &faceIndeciesAll,
	Compressor *cmp,
	size_t num_wave_coef,
	float *homme_orig_buf,
	float *homme_comp_buf,
	coeff_t *orig_buf,
	coeff_t *comp_buf
) {
	int rc = ncdfc.OpenRead(ts,var);
	if (rc<0) exit(1);

	vector <size_t> dims = ncdfc.GetSpatialDims(var);
	assert(dims.size() <= 2);

	size_t start[3] = {0,0,0};
	size_t count[3] = {1,1,1};
	size_t nz = 0;
	if (dims.size() == 1) {
		count[0] = dims[0];
		nz = 1;
	}
	else {
		count[0] = dims[0];
		count[1] = dims[1];
		nz = dims[0];
	}	
	rc = ncdfc.ReadNative(start, count, homme_orig_buf);
	ncdfc.Close();

	double mv = 0.0;
	bool has_missing = ncdfc.GetMissingValue(var, mv);

	SignificanceMap mismap;
	SignificanceMap sigmap(NX*NY*nz);
	double lmax = 0.0;
	double lmaxerr = 0.0;
	double l2sqr = 0.0;
	double l2sqrerr = 0.0;
	float min = 0.0, max = 0.0;
	for (int face=0; face<6; face++) {
		
		const vector <int> &faceIndecies = faceIndeciesAll[face];
		bool first = true;
		float mymin = 0.0, mymax = 0.0;
		for (int z = 0; z<nz; z++) {
			for (int i = 0; i<faceIndecies.size(); i++) {
				float t = homme_orig_buf[z*NCOL + faceIndecies[i]];
				if (! (has_missing && t == mv)) {
					if (first) {
						mymin = min = t;
						mymax = max = t;
						first = false;
					}
				
					if (t<mymin) mymin = t;
					if (t>mymax) mymax = t;
				}
				orig_buf[z*NX*NY + i] = t;
			}
		}
		if (mymin<min) min = mymin;
		if (mymax>max) max = mymax;

		if (has_missing) {
			remove_missing(orig_buf, nz * faceIndecies.size(), mv, mismap);
		}


		sigmap.Clear();
		cmp->Compress(orig_buf, comp_buf, num_wave_coef, &sigmap);

		cmp->ClampMinOnOff() = true;
		cmp->ClampMaxOnOff() = true;
		cmp->ClampMin() = mymin;
		cmp->ClampMax() = mymax;
		cmp->Decompress(comp_buf, orig_buf, &sigmap);

		if (has_missing) {
			replace_missing(orig_buf, mv, mismap);
		}

		for (int z = 0; z<nz; z++) {
			for (int i = 0; i<faceIndecies.size(); i++) {
				homme_comp_buf[z*NCOL + faceIndecies[i]] = orig_buf[z*NX*NY + i];
			}
		}
	}

	compute_error(
		homme_orig_buf, homme_comp_buf, nz*NCOL,
		has_missing, mv,
		&lmaxerr, &lmax, &l2sqrerr, &l2sqr
	);

	double rmsErr = sqrt(l2sqrerr / (double) (6*nz*NCOL));
	double nrms = ((max-min) != 0) ? rmsErr / (max-min) : 0.0;
    double l2Err = sqrt(l2sqrerr);
    double l2 = sqrt(l2sqr);
    double lmaxrel = 0.0;
    double l2rel = 0.0;
    if ((max-min) != 0.0) lmaxrel = lmaxerr / (max-min);
    if (l2 != 0.0) l2rel = l2Err / l2;

	cout << "   range : " << min << " .. " << max << endl;
	cout << "   Lmax : " << lmaxerr << endl;
	cout << "   Lmax Relative: " << lmaxrel << endl;
	cout << "   L2 : " << l2Err << endl;
	cout << "   L2 Relative: " << l2rel << endl;
	cout << "   RMS : " << rmsErr << endl;
	cout << "   Normalized RMS : " << nrms << endl;

	string file;
	size_t localts;
	ncdfc.GetFile(ts, var, file, localts);

	NetCDFSimple::Variable varinfo;
	ncdfc.GetVariableInfo(var, varinfo);
	int varid = varinfo.GetVarID();

	int ncid;
	rc = nc_open(file.c_str(), NC_WRITE, &ncid);
	assert (rc == NC_NOERR);

	size_t mystart[3] = {localts,0,0};
	size_t mycount[3] = {1,count[0],count[1]};

	rc = nc_put_vara_float(ncid, varid, mystart, mycount, homme_comp_buf);
	assert(rc == NC_NOERR);

	nc_close(ncid);

}
*/

/*
void compress_files(
	vector <string> files,
	const vector < vector <int> > &faceIndeciesAll
) {
	const string wname = "bior3.3";

	NetCDFCollection ncdfc;
	
	vector <string> time_dimnames;
	time_dimnames.push_back("time");
	vector <string> time_coordvars;
	time_coordvars.push_back("time");

	ncdfc.SetMissingValueAttName("_FillValue");
	int rc = ncdfc.Initialize(files, time_dimnames, time_coordvars);
	if (rc < 0) exit(1);

	float *homme_orig_buf = new float[NCOL*ILEV];
	float *homme_comp_buf = new float[NCOL*ILEV];
	coeff_t *orig_buf = new coeff_t[NX*NY*ILEV];
	coeff_t *comp_buf = new coeff_t[NX*NY*ILEV];

	vector <size_t> dims_3dlev;
	dims_3dlev.push_back(NX);
	dims_3dlev.push_back(NY);
	dims_3dlev.push_back(LEV);

	vector <size_t> dims_3dilev;
	dims_3dilev.push_back(NX);
	dims_3dilev.push_back(NY);
	dims_3dilev.push_back(ILEV);

	vector <size_t> dims_2d;
	dims_2d.push_back(NX);
	dims_2d.push_back(NY);

	Compressor cmp_3dlev(dims_3dlev, wname);
	Compressor cmp_3dilev(dims_3dilev, wname);
	Compressor cmp_2d(dims_2d, wname);


	for (size_t ts = 0; ts<ncdfc.GetNumTimeSteps(); ts++) {

		cout << "Processing time step " << ts << endl;
		size_t num_wave_coef = (size_t) ((float) (NX*NY) /  (CRATIO_2D));
		vector <string> vars = ncdfc.GetVariableNames(1, true);

		for (int v = 0; v < vars.size(); v++) {
			vector <size_t> dims = ncdfc.GetSpatialDims(vars[v]);
			if (dims[dims.size()-1] != NCOL) continue; 

			cout << "  Processing 2d variable " << vars[v] << endl;

			compress_var(
				ts, vars[v], ncdfc, faceIndeciesAll, &cmp_2d, num_wave_coef,
				homme_orig_buf, homme_comp_buf, orig_buf, comp_buf
			);
		}

		vars = ncdfc.GetVariableNames(2, true);
		for (int v = 0; v < vars.size(); v++) {
			vector <size_t> dims = ncdfc.GetSpatialDims(vars[v]);
			if (dims[dims.size()-1] != NCOL) continue; 

			cout << "  Processing 3d variable " << vars[v] << endl;

			Compressor *cmp;
			size_t num_wave_coef = 0;
			if (ncdfc.GetSpatialDims(vars[v])[0] == ILEV) {
				cmp = &cmp_3dilev;
				num_wave_coef = (size_t) ((float) (NX*NY*ILEV) /  (CRATIO_3D));
			}
			else {
				cmp = &cmp_3dlev;
				num_wave_coef = (size_t) ((float) (NX*NY*LEV) /  (CRATIO_3D));
			} 
			

			compress_var(
				ts, vars[v], ncdfc, faceIndeciesAll, cmp, num_wave_coef,
				homme_orig_buf, homme_comp_buf, orig_buf, comp_buf
			);
		}
	}
}
*/

int main (int argc, char **argv) {

	MyBase::SetErrMsgFilePtr(stderr);

	if (argc < 4) {
		Usage(argv[0]);
		exit (1);
	}
	argc--; argv++;

	string mapfile = argv[0];
	argc--; argv++;
	string facefile = argv[0];
	argc--; argv++;
	string camfile = argv[0];



	
	vector <int> facemap;
	int rc = getFaceMap(facefile, facemap);
	if (rc < 0) exit(1);

	vector <neighbors_t> neighbormap;
	rc = getNeighborMap(mapfile, neighbormap);
	if (rc < 0) exit(1);

	cout << "2D compression factor " << CRATIO_2D << endl;
	cout << "3D compression factor " << CRATIO_3D << endl;
#ifdef	DOUBLE
	cout << "coefficient type : double" << endl;
#else
	cout << "coefficient type : float" << endl;
#endif
	cout << "Building remapping table" << endl;
	vector <int> facecount(6);
	for (int i=0; i<facemap.size(); i++) {
		int nfaces = 0;
		for (int face=0; face<6; face++) {
			if (isOnFace(facemap[i],face)) {
				nfaces++;
				facecount[face]++;
			}
		}
		if (nfaces == 3) {
//			cout << "node " << i << " has 3 faces" << endl;
		}

	}

	for (int face=0; face<6; face++) {
//		cout << "face " << face << " has " << facecount[face] << " nodes\n";
	}

	vector <int> startNodes;
	for (int face=0; face<6; face++) {
		int center = getFirstCenter(facemap, neighbormap, face);


//		neighbors_t neighbor = neighbormap[center];
		
		assert(center >= 0 && center < neighbormap.size());
		startNodes.push_back(neighbormap[center].n0);

//		cout << "face " << face << " center " << center << " start node " << neighbormap[center].n0 << endl;
		
	}

	vector < vector <int> > faceIndeciesAll(6);
	int nx, ny;
	for (int face=0; face<6; face++) {
		vector <int> &faceIndecies = faceIndeciesAll[face];
		rc = getFaceIndecies(
			startNodes[face], facemap, neighbormap, face, faceIndecies, nx, ny
		);
		if (rc < 0) exit(1);

		assert(faceIndecies.size() == nx*ny);
//		cout << "nx, ny : " << nx << ", " << ny << endl;
	}


	vector <string> files;
	for (int i=0; i<argc; i++) {
		files.push_back(argv[i]);
	}

//	compress_files(files, faceIndeciesAll);
}
