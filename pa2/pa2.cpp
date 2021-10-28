#include "pa2.h"
#include <stack>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctype.h>

// Pair2D constructor
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

// BlockNode constructor  
BlockNode::BlockNode(const NodeData& data) : label(data.label), 
	cutType(data.cutType),  dimVec(data.dimVec) {}
// wrapper function to create new BlockNodes via shared_ptrs
BlockNodePtr newNode(NodeData& data) {
    BlockNodePtr nodePtr = make_shared<BlockNode>(data);
    return nodePtr;
}
// builds a tree from a text file
BlockNodePtr buildTree(const string& inFileName) {
	stack<BlockNodePtr > nodeStack;
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
				BlockNodePtr node = newNode(data);
				nodeStack.push(node);
			// internal nodes
			} else {
				data.label = -1;
				data.cutType = label;
				BlockNodePtr node = newNode(data);
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

Pair2D firstPackingDims(BlockNodePtr node) {
	// base case: packing of a single rectangle 
	// is just its own dimensions
	if  (node->label != -1) {
		return node->dimVec.front();
	} else {
		Pair2D d1 = computeFirstPackingDims(node->left);
		Pair2D d2 = computeFirstPackingDims(node->right);
		// horizontal cut
		if (node->cutType == "H") {
			Pair2D dims(max(d1.x, d2.x), d1.y+d2.y);
			node->dimVec.push_back(dims);
			return dims;
		// vertical cut
		} else {
			Pair2D dims(d1.x+d2.x, max(d1.y,d2.y));
			node->dimVec.push_back(dims);
			return dims;
		}
	}
}

void writePackingDims(BlockNodePtr node, 
	std::function<BlockNodePtr(Pair2D)> dimFunc, const string& fname) {
	FILE* outfile = fopen(fname.c_str(), "w");
	Pair2D pair = dimFunc(node);
	fprintf(outfile, "(%d,%d)\n", pair.x, pair.y);
	fclose(outfile);
}

void determinePacking(BlockNodePtr root, const string& fname) {
	Pair2D origin(0,0);
	FILE* outfile = fopen(fname.c_str(), "w");	
	determinePackingUtil(root, origin, outfile);
	fclose(outfile);
}

void determinePackingUtil(BlockNodePtr node, Pair2D& origin, FILE* outfile) {
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
			determinePackingUtil(node->left, newOrigin, outfile);
			// right subtree is below horizontal cut
			determinePackingUtil(node->right, origin, outfile);
		// vertical cut
		} else {
			// left subtree is to left of cut
			determinePackingUtil(node->left, origin, outfile);
			// right subtree is to right of cut
			Pair2D newOrigin = origin + Pair2D(node->left->dimVec.front().x, 0);
			determinePackingUtil(node->right, newOrigin, outfile);
		}
	}
}

// void determineOptimalPacking(BlockNodePtr root, const string& fname) {
// ;
// }
// void determineOptimalPackingUtil(BlockNodePtr node, Pair2D& origin, FILE* outfile) {
// ;
// }
