#include "pa1.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main(int argc, char *argv[]) {
        // Ingest arguments
	vector<string> args(argv, argv+argc);
        double t_limit = atof(argv[1]);
	string invParamFname = argv[2];
	string wireParamFname = argv[3];
	string topoFname = argv[4];
	string preOrderFname = argv[5];
	string delaysFname = argv[6];
	string topoOutFname = argv[7];
	string topoBinFname = argv[8];
	// Read in inverter parameters
	InvParams iParams = getInvParams(invParamFname);
	WireParams wParams = getWireParams(wireParamFname);
	auto root = buildRCTree(topoFname);
	// write pre-order traversal sequence to output file
	writePreOrder(root, preOrderFname);
	// compute total capacitance at each node
	computeTotalCap(root, wParams.unitC, iParams.outC);
	// compute Elmore delays at all leaf nodes
	computeElmoreDelays(root, wParams.unitR, iParams.outR);
	writeLeafElmoreDelays(root, delaysFname);

	writePostOrder(root, topoOutFname);
	writePostOrderBin(root, topoBinFname);

	return EXIT_FAILURE;
}
