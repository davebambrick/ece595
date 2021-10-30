#include "pa2.h"
#include <stack>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctype.h>
#include <limits.h>

using namespace std;
// Pair2D constructors
Pair2D::Pair2D() {}
Pair2D::Pair2D(int x, int y) : x(x), y(y) {}
// Pair2D operators
Pair2D Pair2D::operator+(const Pair2D& other) const {
	return Pair2D(x+other.x, y+other.y);
}
Pair2D Pair2D::operator-(const Pair2D& other) const {
	return Pair2D(x-other.x, y-other.y);
}
bool Pair2D::operator< (const Pair2D& other) const {
	if (x >= other.x) {
		return (y < other.y);
	} 
	return true;
}
bool Pair2D::operator<= (const Pair2D& other) const {
	if (x > other.x) {
		return (y <= other.y);
	} 
	return true;
}
bool Pair2D::operator==(const Pair2D& other) const {
	return (x == other.x && y == other.y);
}
// Cut dimension function
Pair2D getVertCutDims(const Pair2D& a, const Pair2D& b) {
	return Pair2D(a.x+b.x, max(a.y,b.y));
}
Pair2D getHorizCutDims(const Pair2D& a, const Pair2D& b) {
	return Pair2D(max(a.x, b.x), a.y+b.y);
 
}

// BlockNode constructor from parsed data stored in NodeData
BlockNode::BlockNode(const NodeData& data) : label(data.label), 
	cutType(data.cutType),  firstDims(data.firstDims), dimMap(data.dimMap) {}
// wrapper function to create new BlockNodes via shared_ptrs
shared_ptr<BlockNode> newNode(NodeData& data) {
    shared_ptr<BlockNode> nodePtr{make_shared<BlockNode>(data)};
    return nodePtr;
}
// builds a tree from a text file
shared_ptr<BlockNode> buildTree(const string& inFileName) {
	stack<shared_ptr<BlockNode>> nodeStack {};
	ifstream inFile{inFileName};
	if (inFile.is_open()) {
		string line {};
		// each line represents a node
		while (getline(inFile, line)) {
			stringstream iss{line};
			string label {};
			NodeData data {};
			getline(iss, label, '(');
			// leaf nodes
			if (isdigit(label[0])) {
				data.label = stoi(label);
				bool first = true;
				string s {};
				int w {}, h {};
				while (iss) {
					getline(iss,s,'(');
					getline(iss,s,',');
					// skip last two characters, which 
					// are always right-facing parens
					if (s != ")") {
						w = stoi(s); // get width
						getline(iss,s,')'); // read to next paren
						h = stoi(s); // get height
						Pair2D dims{w, h};
						if (first) {
							data.firstDims = dims;
							first = false;
						}
						pair<const Pair2D, const Pair2D> nullPair{Pair2D(-1,-1), Pair2D(-1,-1)};
						data.dimMap.emplace(dims, nullPair);
					}
				}
				shared_ptr<BlockNode> node{newNode(data)};
				nodeStack.push(node);
			// internal nodes
			} else {
				data.label = -1;
				data.cutType = label;
				shared_ptr<BlockNode> node = newNode(data);
				// pop entries from top of stack
				node->right = nodeStack.top();
				nodeStack.pop();
				node->left = nodeStack.top();
				nodeStack.pop();
				// push internal node back onto stack
				nodeStack.push(node);
			}
		}
		inFile.close();
	}
	return nodeStack.top();
}

Pair2D computeFirstPackingDims(shared_ptr<BlockNode> node) {
	Pair2D dims;
	// base case: first packing of a single rectangle 
	// is just its first implementation
	if  (node->label != -1) {
		dims = node->firstDims;
	} else {
		Pair2D d1{computeFirstPackingDims(node->left)};
		Pair2D d2{computeFirstPackingDims(node->right)};
		// horizontal cut
		if (node->cutType == "H") {
			dims = getHorizCutDims(d1,d2);
		// vertical cut
		} else {
			dims = getVertCutDims(d1,d2);
		}
		node->firstDims = dims;
	}
	return dims;
}

void determineFirstPacking(shared_ptr<BlockNode> node, Pair2D& origin, FILE* outfile) {
	// base case: single rectangle is set at the current origin
	if  (node->label != -1) {
		Pair2D dims{node->firstDims};
		fprintf(outfile, "%d((%d,%d)(%d,%d))\n", 
			node->label, dims.x, dims.y, origin.x, origin.y);
	} else {
		// horizontal cut
		if (node->cutType == "H") {
			// left subtree is above horizontal cut; shift
			// origin of left subtree by height of right subtree
			Pair2D newOrigin{origin + Pair2D(0,node->right->firstDims.y)};
			determineFirstPacking(node->left, newOrigin, outfile);
			// right subtree is below horizontal cut
			determineFirstPacking(node->right, origin, outfile);
		// vertical cut
		} else {
			// left subtree is to left of cut
			determineFirstPacking(node->left, origin, outfile);
			// right subtree is to right of cut
			Pair2D newOrigin{origin + Pair2D(node->left->firstDims.x, 0)};
			determineFirstPacking(node->right, newOrigin, outfile);
		}
	}
}

map<Pair2D, pair<const Pair2D, const Pair2D>> computeOptimalPackingDimsUtil(shared_ptr<BlockNode> node) {
	// leaf nodes just return the whole stack
	if (node->label != -1) {
		return node->dimMap;
	// internal nodes
	} else {
		map<Pair2D, pair<const Pair2D, const Pair2D>> mapLeft{computeOptimalPackingDimsUtil(node->left)};
		map<Pair2D, pair<const Pair2D, const Pair2D>> mapRight{computeOptimalPackingDimsUtil(node->right)};
		for (auto const& x : mapLeft) {
			for (auto const& y : mapRight) {
				Pair2D dims {};
				// horizontal cut
				if (node->cutType == "H") {
					dims = getHorizCutDims(x.first, y.first);
				// vertical cut
				} else {
					dims = getVertCutDims(x.first, y.first);
				}
				pair<const Pair2D, const Pair2D> childDims{x.first, y.first};
				node->dimMap.emplace(dims, childDims);
			}
		}
		return node->dimMap;
	}
}

Pair2D computeOptimalPackingDims(shared_ptr<BlockNode> root) {
	map<Pair2D, pair<const Pair2D, const Pair2D>> dimMap{computeOptimalPackingDimsUtil(root)};
	int bestArea{INT_MAX};
	Pair2D bestDims {};
	for (auto const& v : dimMap) {
		if (v.first.x * v.first.y < bestArea) {
			bestArea = v.first.x * v.first.y;
			bestDims = v.first;
		}
	}
	root->bestDims = bestDims;
	return bestDims;
}

void determineOptimalPacking(shared_ptr<BlockNode> node, Pair2D& origin, FILE* outfile) {
	// base case: single rectangle is set at the current origin
	if  (node->label != -1) {
		Pair2D dims{node->bestDims};
		fprintf(outfile, "%d((%d,%d)(%d,%d))\n",
			node->label, dims.x, dims.y, origin.x, origin.y);
	} else {
		pair<const Pair2D, const Pair2D> predPairs{node->dimMap[node->bestDims]};
		node->left->bestDims = predPairs.first;
		node->right->bestDims = predPairs.second;
		// horizontal cut
		if (node->cutType == "H") {
			// left subtree is above horizontal cut; shift
			// origin of left subtree by height of right subtree
			Pair2D newOrigin{origin + Pair2D(0,predPairs.second.y)};
			determineOptimalPacking(node->left, newOrigin, outfile);
			// right subtree is below horizontal cut
			determineOptimalPacking(node->right, origin, outfile);
		// vertical cut
		} else {
			// left subtree is to left of cut
			determineOptimalPacking(node->left, origin, outfile);
			// right subtree is to right of cut
			Pair2D newOrigin{origin + Pair2D(predPairs.first.x, 0)};
			determineOptimalPacking(node->right, newOrigin, outfile);
		}
	}
}

void writePackingDims(shared_ptr<BlockNode> node, Pair2D (*computeDimFunc)(shared_ptr<BlockNode>), const string& fname) {
	FILE* outfile{fopen(fname.c_str(), "w")};
	Pair2D pair{computeDimFunc(node)};
	fprintf(outfile, "(%d,%d)\n", pair.x, pair.y);
	fclose(outfile);
}

void writePackingCoords(shared_ptr<BlockNode> root, void (*packingFunc) (shared_ptr<BlockNode>, Pair2D&, FILE*), const string& fname) {
	Pair2D origin{0, 0};
	FILE* outfile{fopen(fname.c_str(), "w")};
	packingFunc(root, origin, outfile);
	fclose(outfile);
}
