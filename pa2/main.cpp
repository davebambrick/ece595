#include "pa2.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main(int argc, char *argv[]) {
	// Ingest arguments
	vector<string> args(argv, argv+argc);
	string inFileName = argv[1];
	string outFileName1 = argv[2];
	string outFileName2 = argv[3];
	string outFileName3 = argv[4];
	string outFileName4 = argv[5];

	auto rootNode = buildTree(inFileName);
	Pair2D packing = computeFirstPackingDims(rootNode);
	// write dims to outfile 1
	writePackingDims(packing, outFileName1);
	// write packing to outfile 2
	determinePacking(rootNode, outFileName2);
	return 0;
}