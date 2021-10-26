#include "pa1.h"
#include <string>
#include <stack>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

InvParams getInvParams(const string& invParamFname) {
    InvParams invParams; 
    double inC, outC, outR;
    ifstream file(invParamFname);
	if (file.is_open()){
        string line;
        while ( getline(file,line) ) {
            stringstream iss(line);
            iss >> inC >> outC >> outR;
        }
        invParams.inC = inC;
        invParams.outC = outC;
        invParams.outR = outR;
        file.close();
    }
    return invParams;
}

WireParams getWireParams(const string& wireParamFname) {
    WireParams wireParams;
    double unitC, unitR;
    ifstream file(wireParamFname);
	if (file.is_open()){
        string line;
        while ( getline(file,line) ) {
            stringstream iss(line);
            iss >> unitR >> unitC;
        }
        wireParams.unitR = unitR;
        wireParams.unitC = unitC;
        file.close();
    } 
    return wireParams;
}

RCNode::RCNode(NodeData& data) : label(data.label), sinkCap(data.sinkCap), 
    leftLength(data.leftLength), rightLength(data.rightLength), 
    totalCap(0), subtreeCapSum(0), elmoreDelay(0), nInverters(0) {}

shared_ptr<struct RCNode> newNode(NodeData& data) {
    auto nodePtr = make_shared<RCNode>(data);
    return nodePtr;
}

shared_ptr<struct RCNode> buildRCTree(const string& topoFname) {
    stack<RCNode> nodeStack;
    ifstream file(topoFname);
    if (file.is_open()) {
        string line;
        while (getline(file,line)) {
            stringstream iss(line);
            string s;
            NodeData data;
            getline(iss, s, '(');
            // parse data for leaf nodes
            if (!s.empty()) {
                data.label = atoi(s.c_str());
                getline(iss, s, ')');
                data.sinkCap = atof(s.c_str());
                data.leftLength = -1;
                data.rightLength = -1;
                RCNode node(data);
                nodeStack.push(node);
            // parse data for internal nodes
            } else {
                data.label = -1;
                data.sinkCap = 0;
                getline(iss, s, ' ');
                data.leftLength = atof(s.c_str());
                getline(iss, s, ')');
                data.rightLength = atof(s.c_str());
                // init new node
                RCNode node(data);
                // pop children from top of stack
                RCNode right = nodeStack.top();
                nodeStack.pop();
                RCNode left = nodeStack.top();
                nodeStack.pop();
                // assign children pointers to object
                node.left = make_shared<RCNode>(left);
                node.right = make_shared<RCNode>(right);
                nodeStack.push(node);
            }
        }
        file.close();
    }
    return make_shared<RCNode>(nodeStack.top());
}

// Util and main caller for write to preorder
void writePreOrderUtil(shared_ptr<RCNode> node, FILE* outfile) {
    if (node != nullptr) {
        if (node->label == -1) {
            fprintf(outfile, "(%.10le %.10le)\n", node->leftLength, node->rightLength);
        } else {
            fprintf(outfile, "%d(%.10le)\n", node->label, node->sinkCap);
        }
        writePreOrderUtil(node->left, outfile);
        writePreOrderUtil(node->right, outfile);
    }
}
void writePreOrder(shared_ptr<RCNode> root, const string& fname) {
    FILE* fHandle = fopen(fname.c_str(), "w");
    writePreOrderUtil(root, fHandle);
    fclose(fHandle);
}


void computeTotalCapUtil(shared_ptr<RCNode> node, double inEdgeLength, double unitC, double outC) {
    if (node != nullptr) {
        // leaf node
        if (node->label != -1) {
            node->totalCap = (inEdgeLength * unitC / 2) + node->sinkCap;
        } else {
            double cap = (inEdgeLength + node->leftLength + node->rightLength) * unitC / 2;
            // add driver capacitance for source node
            if (inEdgeLength == 0) {
                cap += outC;
            }
            node->totalCap = cap;
        }
        computeTotalCapUtil(node->left, node->leftLength, unitC, outC);
        computeTotalCapUtil(node->right, node->rightLength, unitC, outC);
    }
}

void computeTotalCap(shared_ptr<RCNode> root, double unitC, double outC) {
    computeTotalCapUtil(root, 0, unitC, outC);
}

// sums total capacitances across all nodes in subtree rooted at node
// equivalent to second summation in second Elmore delay expression
double sumSubtreeCaps(shared_ptr<RCNode> node) {
    if (node != nullptr) {
        // if not yet computed
        if (node->subtreeCapSum == 0) {
            double sum = node->totalCap;
            // recurse
            sum += sumSubtreeCaps(node->left) + sumSubtreeCaps(node->right);
        node->subtreeCapSum = sum;
        }
        return node->subtreeCapSum;
    } else {
        return 0;
    }
    
}
// util function to compute Elmore delay for single node with ingoing edge and parent delay
void computeElmoreDelaysUtil(shared_ptr<RCNode> node, double inEdgeLength, double parentDelay, double unitR) {
    if (node != nullptr) {
        // skip compute for root node; handled by wrapper function
        if (parentDelay != 0) {
            node->elmoreDelay = parentDelay + inEdgeLength*unitR*sumSubtreeCaps(node);
        }
        computeElmoreDelaysUtil(node->left, node->leftLength, node->elmoreDelay, unitR);
        computeElmoreDelaysUtil(node->right, node->rightLength, node->elmoreDelay, unitR);
    }
}
void computeElmoreDelays(shared_ptr<RCNode> root, double unitR, double outR) {
    root->elmoreDelay = outR*sumSubtreeCaps(root);
    computeElmoreDelaysUtil(root, 0, 0, unitR);
}

// Utility and main caller to write leaf Elmore delays to binary
void writeLeafElmoreDelaysUtil(shared_ptr<RCNode> node, FILE* outfile) {
    if (node != nullptr) {
        // leaf nodes
        if (node->label != -1) {
            fwrite(&(node->label),sizeof(node->label), 1, outfile);
            fwrite(&(node->elmoreDelay), sizeof(node->elmoreDelay), 1, outfile);
        } else {
            writeLeafElmoreDelaysUtil(node->left, outfile);
            writeLeafElmoreDelaysUtil(node->right, outfile);
        }
    }
}
void writeLeafElmoreDelays(shared_ptr<RCNode> root, const string& fname) {
    FILE* outfile = fopen(fname.c_str(), "w");
    writeLeafElmoreDelaysUtil(root, outfile);
    fclose(outfile);
}
// write post order to text
void writePostOrderUtil(shared_ptr<RCNode> node, FILE* outfile) {
    if (node != nullptr) {
        writePostOrderUtil(node->left, outfile);
        writePostOrderUtil(node->right, outfile);

        if (node->label == -1) {
            fprintf(outfile, "(%.10le %.10le)\n", node->leftLength, node->rightLength);
        } else {
            fprintf(outfile, "%d(%.10le)\n", node->label, node->sinkCap, node->nInverters);
        }
    }
}
void writePostOrder(shared_ptr<RCNode> root, const string& fname) {
    FILE* fHandle = fopen(fname.c_str(), "w");
    writePostOrderUtil(root, fHandle);
    fclose(fHandle);
}
// write post order to binary
void writePostOrderBinUtil(shared_ptr<RCNode> node, FILE* outfile) {
    if (node != nullptr) {
        writePostOrderBinUtil(node->left, outfile);
        writePostOrderBinUtil(node->right, outfile);
        // non-leaf labels are -1
        fwrite(&(node->label),sizeof(node->label), 1, outfile);
        if (node->label == -1) {
            fwrite(&(node->sinkCap), sizeof(node->sinkCap), 1, outfile);
        } else {
            fwrite(&(node->leftLength), sizeof(node->leftLength), 1, outfile);
            fwrite(&(node->rightLength), sizeof(node->rightLength), 1, outfile);

        } 
    }
}
void writePostOrderBin(shared_ptr<RCNode> root, const string& fname) {
    FILE* outfile = fopen(fname.c_str(), "w");
    writePostOrderBinUtil(root, outfile);
    fclose(outfile);
}

double maxDelay(shared_ptr<RCNode> node) {
    if (node != nullptr) {
        if (node->label != -1) {
            return node->elmoreDelay;
        } else {
            return max(maxDelay(node->left), maxDelay(node->right));
        }
    } else {
        return 0;
    }
}
void insertInvertersUtil(shared_ptr<RCNode> node, double t_limit) {
    // non leaf nodes
    if (node != nullptr) {
        if (node->label == -1) {
            insertInvertersUtil(node->left, t_limit);
            insertInvertersUtil(node->right, t_limit);
            double maxT = maxDelay(node);
        }
    }
}