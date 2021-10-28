#include <string>
#include <memory>
#include <utility>
#include <vector>

using namespace std;

// Simple 2D-coordinate container
struct Pair2D {
	int x, y;
	// constructor
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
	shared_ptr<struct BlockNode> left;
	shared_ptr<struct BlockNode> right;

	BlockNode(const NodeData& data); // from data struct

};
typedef shared_ptr<BlockNode> BlockNodePtr;
// Tree construction methods
BlockNodePtr newNode(NodeData& data);
BlockNodePtr buildTree(const string& inFileName);

// Compute dimensions of first packing
Pair2D computeFirstPackingDims(BlockNodePtr node);
void writePackingDims(Pair2D& pair, const string& fname);

// Determine first-option packing coordinates and write to file
void determinePacking(BlockNodePtr root, const string& fname);
void determinePackingUtil(BlockNodePtr node, Pair2D& origin, FILE* outfile);

// Compute dimensions of optimal packing
Pair2D computeOptimalPackingDims(BlockNodePtr node);
// Determine optimal packing coordinates and write to file
void determineOptimalPacking(BlockNodePtr root, const string& fname);
void determineOptimalPackingUtil(BlockNodePtr node, Pair2D& origin, FILE* outfile);