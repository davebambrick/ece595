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
// Tree construction methods
shared_ptr<BlockNode> newNode(NodeData& data);
shared_ptr<BlockNode> buildTree(const string& inFileName);

// Compute dimensions of first packing
Pair2D computeFirstPackingDims(shared_ptr<BlockNode> node);
void writePackingDims(Pair2D& pair, const string& fname);

// Determine first-option packing coordinates and write to file
void determinePacking(shared_ptr<BlockNode> root, const string& fname);
void determinePackingUtil(shared_ptr<BlockNode> node, Pair2D& origin, FILE* outfile);

// Determine optimal packing coordinates and write to file
void determineOptimalPacking(shared_ptr<BlockNode> root, const string& fname);
void determineOptimalPackingUtil(shared_ptr<BlockNode> node, Pair2D& origin, FILE* outfile);