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
Pair2D Pair2D::operator+(const Pair2D& other) {
	return Pair2D(x+other.x, y+other.y);
}
Pair2D Pair2D::operator-(const Pair2D& other) {
	return Pair2D(x-other.x, y-other.y);
}
bool Pair2D::operator==(const Pair2D& other) {
	return (x == other.x && y == other.y);
}
// Cut dimension function
Pair2D getVertCutDims(Pair2D& a, Pair2D& b) {
	return Pair2D(a.x+b.x, max(a.y,b.y));
}
Pair2D getHorizCutDims(Pair2D& a, Pair2D& b) {
	return Pair2D(max(a.x, b.x), a.y+b.y);
 
}

// BlockNode constructor  
BlockNode::BlockNode(const NodeData& data) : label(data.label), 
	cutType(data.cutType),  dimVec(data.dimVec) {}
// wrapper function to create new BlockNodes via shared_ptrs
shared_ptr<BlockNode> newNode(NodeData& data) {
    shared_ptr<BlockNode> nodePtr = make_shared<BlockNode>(data);
    return nodePtr;
}
// builds a tree from a text file
shared_ptr<BlockNode> buildTree(const string& inFileName) {
	stack<shared_ptr<BlockNode>> nodeStack;
	ifstream inFile(inFileName);
	if (inFile.is_open()) {
		string line;
		// each line represents a node
		while (getline(inFile, line)) {
			stringstream iss(line);
			string label;
			NodeData data;
			getline(iss, label, '(');
			// leaf nodes
			if (isdigit(label[0])) {
				data.label = stoi(label);
				string s;
				int w, h;
				// dims of format (w,h)
				while (iss) {
					getline(iss,s,'(');
					getline(iss,s,',');
					// skip last two characters, which 
					// are always right-facing parens
					if (s != ")") {
						w = stoi(s);
						getline(iss,s,')');
						h = stoi(s);
						Pair2D dims(w, h);
						data.dimVec.push_back(dims);
					}
				}
				shared_ptr<BlockNode> node = newNode(data);
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

void clearInternalNodeDims(shared_ptr<BlockNode> node) {
	if (node->label != -1) {
		node->dimVec.clear();
		clearInternalNodeDims(node->left);
		clearInternalNodeDims(node->right);
	}
}

Pair2D computeFirstPackingDims(shared_ptr<BlockNode> node) {
	Pair2D dims;
	// base case: first packing of a single rectangle 
	// is just its first implementation
	if  (node->label != -1) {
		dims = node->dimVec.front();
	} else {
		Pair2D d1 = computeFirstPackingDims(node->left);
		Pair2D d2 = computeFirstPackingDims(node->right);
		// horizontal cut
		if (node->cutType == "H") {
			dims = getHorizCutDims(d1,d2);
			node->dimVec.push_back(dims);
		// vertical cut
		} else {
			dims = getVertCutDims(d1,d2);
			node->dimVec.push_back(dims);
		}
	}
	return dims;
}

void writePackingDims(shared_ptr<BlockNode> node, Pair2D (*dimFunc)(shared_ptr<BlockNode>), const string& fname) {
	FILE* outfile = fopen(fname.c_str(), "w");
	Pair2D pair = dimFunc(node);
	fprintf(outfile, "(%d,%d)\n", pair.x, pair.y);
	fclose(outfile);
}

void determineFirstPacking(shared_ptr<BlockNode> root, const string& fname) {
	Pair2D origin(0,0);
	FILE* outfile = fopen(fname.c_str(), "w");	
	determineFirstPackingUtil(root, origin, outfile);
	fclose(outfile);
}

void determineFirstPackingUtil(shared_ptr<BlockNode> node, Pair2D& origin, FILE* outfile) {
	// base case: single rectangle is set at the current origin
	if  (node->label != -1) {
		Pair2D dims = node->dimVec.front();
		fprintf(outfile, "%d((%d,%d)(%d,%d))\n", 
			node->label, dims.x, dims.y, origin.x, origin.y);
	} else {
		// horizontal cut
		if (node->cutType == "H") {
			// left subtree is above horizontal cut; shift
			// origin of left subtree by height of right subtree
			Pair2D newOrigin = origin + Pair2D(0,node->right->dimVec.front().y);
			determineFirstPackingUtil(node->left, newOrigin, outfile);
			// right subtree is below horizontal cut
			determineFirstPackingUtil(node->right, origin, outfile);
		// vertical cut
		} else {
			// left subtree is to left of cut
			determineFirstPackingUtil(node->left, origin, outfile);
			// right subtree is to right of cut
			Pair2D newOrigin = origin + Pair2D(node->left->dimVec.front().x, 0);
			determineFirstPackingUtil(node->right, newOrigin, outfile);
		}
	}
}

vector<Pair2D> computeOptimalPackingDimsUtil(shared_ptr<BlockNode> node) {
	// uncomputed leaf nodes just return the whole stack
	if (node->label != -1) {
		return node->dimVec;
	// internal nodes
	} else {
		vector<Pair2D> vecLeft = computeOptimalPackingDimsUtil(node->left);
		vector<Pair2D> vecRight = computeOptimalPackingDimsUtil(node->right);
		for (int i = 0;  i < vecLeft.size(); i++) {
			for (int j = 0;  j < vecRight.size(); j++) {
				Pair2D dims;
				// horizontal cut
				if (node->cutType == "H") {
					dims = getHorizCutDims(vecLeft[i],vecRight[j]);
				// vertical cut
				} else {
					dims = getVertCutDims(vecLeft[i],vecRight[j]);
				}
				node->dimVec.push_back(dims);
			}
		}
		return node->dimVec;
	}
}

Pair2D computeOptimalPackingDims(shared_ptr<BlockNode> root) {
	vector<Pair2D> dimVec = computeOptimalPackingDimsUtil(root);
	int bestArea = INT_MAX;
	Pair2D bestDims;
	for (auto v : dimVec) {
		if (v.x * v.y < bestArea) {
			bestArea = v.x * v.y;
			bestDims = v;
		}
	}
	return bestDims;
}
void determineOptimalPacking(shared_ptr<BlockNode> root, const string& fname) {

}
void determineOptimalPackingUtil(shared_ptr<BlockNode> node, Pair2D& origin, FILE* outfile) {

}
