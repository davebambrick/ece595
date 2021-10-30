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
	// write first packing dims and coords to outfile 1
	writePackingDims(root, computeFirstPackingDims, outFileName1);
	writePackingCoords(root, determineFirstPacking, outFileName2);
	// write optimal packing dims and coords to outfile 3
	writePackingDims(root, computeOptimalPackingDims, outFileName3);
	writePackingCoords(root, determineOptimalPacking, outFileName4);
	return 0;
}