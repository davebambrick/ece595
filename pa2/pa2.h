#include <string>
#include <memory>
#include <utility>
#include <vector>

using namespace std;

// Simple 2D-coordinate container
struct Pair2D {
	int x, y;
	// constructor
	Pair2D();
	Pair2D(int x, int y);
	// operators
	Pair2D operator+(const Pair2D& other);
	Pair2D operator-(const Pair2D& other);
	bool operator==(const Pair2D& other);
};

struct NodeData {
	int label;
	string cutType;
	vector<Pair2D> dimVec;
};
// Node data structure representing SBT construction
// for block configuration
struct BlockNode {
	int label;
	string cutType;
	vector<Pair2D> dimVec;

	Pair2D bestDims;

	shared_ptr<BlockNode> left;
	shared_ptr<BlockNode> right;

	BlockNode(const NodeData& data); // from data struct

};
// Tree construction methods
shared_ptr<BlockNode> newNode(NodeData& data);
shared_ptr<BlockNode> buildTree(const string& inFileName);
// Utility function to clear internal node dimensions
void clearInternalNodeDims(shared_ptr<BlockNode> root);
// Wrapper to write packing to file
void writePackingDims(shared_ptr<BlockNode> node, Pair2D (*dimFunc)(shared_ptr<BlockNode>), const string& fname);
// Compute dimensions of first packing
Pair2D computeFirstPackingDims(shared_ptr<BlockNode> node);
// Determine first-option packing coordinates and write to file
void determineFirstPacking(shared_ptr<BlockNode> root, const string& fname);
void determineFirstPackingUtil(shared_ptr<BlockNode> node, Pair2D& origin, FILE* outfile);

// Compute dimensions of optimal packing
Pair2D computeOptimalPackingDims(shared_ptr<BlockNode> root);
vector<Pair2D> computeOptimalPackingDimsUtil(shared_ptr<BlockNode> node);

// Determine optimal packing coordinates and write to file
void determineOptimalPacking(shared_ptr<BlockNode> root, const string& fname);
void determineOptimalPackingUtil(shared_ptr<BlockNode> node, Pair2D& origin, FILE* outfile);
