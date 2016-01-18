
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

#define LEN2D   48602
#define LEN3D  (48602*30)
#define ENS_SIZE 101		// ensemble size

using std::string;
using std::cerr;
using std::cout;
using std::endl;

string GetOrigName( string &var )
{
	string origname = "/home/users/samuelli/Git/qccpack-addon/results/rmsz/bin/"
						+ var + ".orig.bin";
	return origname;
}

string GetCompName( string &var )
{
	string compname = "/home/users/samuelli/Git/qccpack-addon/results/rmsz/bin/"
						+ var + ".comp.bin";
	return compname;
}

void ReadDouble( string filename, double* buf, long len )
{
	FILE* f = fopen( filename.c_str(), "rb" );
	if( f != NULL )
	{
		fread( buf, sizeof(double), len, f );
		fclose( f );
	}
	else
	{
		cerr << "fopen error: " << filename << endl;
		exit(1);
	}
}

struct myvar
{
	string name;
	double z;
};

bool mycomp( const struct myvar &v1, const struct myvar &v2 )
{
	return v1.z > v2.z;
}

int main (int argc, char* argv[])
{
	string varlist = "results/rmsz/varlist";
	std::ifstream file (varlist.c_str());
	if( !file.is_open())
	{
		cerr << "ifstream file open error: " << varlist << endl;
		exit(1);
	}
	string line;
	std::vector<struct myvar> myvars;
	long len = 101;			// 101 variables 
	double orig[len], comp[len];
	while( std::getline( file, line ) )
	{
		string origName = GetOrigName( line );	
		string compName = GetCompName( line );	
		ReadDouble( origName, orig, len );
		ReadDouble( compName, comp, len );
		double diff = 0;
		for( long i = 0; i < len; i++ )
			diff += orig[i] - comp[i];
		struct myvar v;
		v.name = line;
		v.z = diff;
		myvars.push_back(v);
	}

	std::sort( myvars.begin(), myvars.end(), mycomp );

	for( long i = 0; i < myvars.size(); i++ )
		printf("%15s    diff = %e\n", myvars[i].name.c_str(), myvars[i].z );

}
