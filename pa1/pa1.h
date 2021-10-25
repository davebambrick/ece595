#include <fstream>
#include <memory>


using namespace std;

struct InvParams {
	double inC;
	double outC;
	double outR;
};

struct WireParams {
	double unitC;
	double unitR;
};

struct NodeData {
	int label;
	double sinkCap;
	double leftLength;
	double rightLength;
};

struct RCNode {
	int label;
	double sinkCap;
	double leftLength, rightLength;
	double totalCap;
	// store subtree capacitance sum to reduce recursive calls in Elmore computation
	double subtreeCapSum;
	double elmoreDelay;
	int nInverters;

	shared_ptr<struct RCNode> left;
	shared_ptr<struct RCNode> right;
	// default constructor
	RCNode() {};
	// NodeData constructor
	RCNode(NodeData& data);

};

WireParams getWireParams(const string& wireParamFname);
InvParams getInvParams(const string& invParamFname);
// functions to construct new nodes and full RC tree
shared_ptr<struct RCNode> newNode(NodeData& data);
shared_ptr<struct RCNode> buildRCTree(const string& topoFname);
// functions to write pre-order traversal to file
void writePreOrderUtil(shared_ptr<RCNode> node, FILE* outfile);
void writePreOrder(shared_ptr<RCNode> root, const string& fname);
// functions to compute total capacitance at each node
void computeTotalCapUtil(shared_ptr<RCNode> node, double inEdgeLength, double unitC, double outC);
void computeTotalCap(shared_ptr<RCNode> root, double unitC, double outC);
// function to compute subtree sums of capacitances
double sumSubtreeCaps(shared_ptr<RCNode> node);
// functions to compute elmore delays at all nodes
void computeElmoreDelaysUtil(shared_ptr<RCNode> node, double inEdgeLength, double parentDelay, double unitR);
void computeElmoreDelays(shared_ptr<RCNode> root, double unitR, double outR);
// functions to write elmore delays to binary files
void writeLeafElmoreDelaysUtil(shared_ptr<RCNode> node, FILE* outfile);
void writeLeafElmoreDelays(shared_ptr<RCNode> root, const string& fname);

// TODO: insert nInverters
void insertInvertersUtil(shared_ptr<RCNode> node, double t_limit);
// write post-order to text
void writePostOrderUtil(shared_ptr<RCNode> node, FILE* outfile);
void writePostOrder(shared_ptr<RCNode> root, const string& fname);
// write post-order to binary
void writePostOrderBinUtil(shared_ptr<RCNode> node, FILE* outfile);
void writePostOrderBin(shared_ptr<RCNode> root, const string& fname);
