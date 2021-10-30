#include <map>
#include <memory>
#include <string>
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
	Pair2D operator+(const Pair2D& other) const;
	Pair2D operator-(const Pair2D& other) const;
	bool operator< (const Pair2D& other) const;
	bool operator<= (const Pair2D& other) const;
	bool operator==(const Pair2D& other) const;
};

struct NodeData {
	int label;
	string cutType;
	Pair2D firstDims;
	// map a given dimension to the pointers of its constituent blocks
	map<Pair2D, pair<const Pair2D, const Pair2D>> dimMap;
};
// Node data structure representing SBT construction
// for block configuration
struct BlockNode {
	int label;
	string cutType;
	Pair2D firstDims;
	map<Pair2D, pair<const Pair2D, const Pair2D>> dimMap;

	Pair2D bestDims;

	shared_ptr<BlockNode> left;
	shared_ptr<BlockNode> right;

	BlockNode(const NodeData& data); // from data struct

};
// Tree construction methods
shared_ptr<BlockNode> newNode(NodeData& data);
shared_ptr<BlockNode> buildTree(const string& inFileName);
// utility functions to get dimensions of rectangle formed by
// horizontal or vertical cutlines respectively
Pair2D getVertCutDims(const Pair2D& a, const Pair2D& b);
Pair2D getHorizCutDims(const Pair2D& a, const Pair2D& b);
// Wrapper to write packing to file
void writePackingDims(shared_ptr<BlockNode> node, Pair2D (*computeDimFunc)(shared_ptr<BlockNode>), const string& fname);
// Compute dimensions of first packing
Pair2D computeFirstPackingDims(shared_ptr<BlockNode> node);
// Determine first-option packing coordinates and write to file
void determineFirstPacking(shared_ptr<BlockNode> root, const string& fname);
void determineFirstPackingUtil(shared_ptr<BlockNode> node, Pair2D& origin, FILE* outfile);

// Compute dimensions of optimal packing
Pair2D computeOptimalPackingDims(const shared_ptr<BlockNode> root);
map<Pair2D, pair<const Pair2D, const Pair2D>> computeOptimalPackingDimsUtil(shared_ptr<BlockNode> node);

// Determine optimal packing coordinates and write to file
void determineOptimalPacking(shared_ptr<BlockNode> root, const string& fname);
void determineOptimalPackingUtil(shared_ptr<BlockNode> node, const Pair2D& origin, FILE* outfile);
