#include "pa2.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main(int argc, char *argv[]) {
	// Ingest arguments
	string inFileName{argv[1]};
	string outFileName1{argv[2]};
	string outFileName2{argv[3]};
	string outFileName3{argv[4]};
	string outFileName4{argv[5]};

	shared_ptr<BlockNode> root{buildTree(inFileName)};
	// write dims to outfile 1
	writePackingDims(root, computeFirstPackingDims, outFileName1);
	// write packing to outfile 2
	determineFirstPacking(root, outFileName2);
	// write optimal dims to outfile 3
	clearInternalNodeDims(root);
	writePackingDims(root, computeOptimalPackingDims, outFileName3);
	return 0;
}