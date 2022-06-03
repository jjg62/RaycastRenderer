#ifndef CM30075_CODE_STARTING_POINT_KDTREE_H_
#define CM30075_CODE_STARTING_POINT_KDTREE_H_

#include "photon.h"
#include <iostream>

using namespace std;

//My implementation of a KD-tree

//Nodes in the tree
class Node{
public:
	Node(Photon data);
	void add(Node * node, int depth);
	Node * left;
	Node * right;

	static const int dims = 3;
	float properties[dims];
	Photon data;
};

//Helper classes for keeping nodes ordered while finding k-nearest nodes.
class OrderedNode{
public:
	Node * node;
	OrderedNode * next;
	void add(Node * nodeToAdd, Vertex point);
	~ OrderedNode(){
		delete next;
	}
};

class OrderedNodeList {
public:
	int length = 0;
	OrderedNode * head;
	void add(Node * node, Vertex Target);
	void pop();
	~ OrderedNodeList(){
		delete head;
	}
};

//The tree
class KDTree {
public:
	Node * root = NULL;
	void add(Photon data);
	Node * closestNode(Node * root, Vertex point, int depth);
	Node * closestNode(Vertex point);
	void kClosestNodes(int k, Node * root, Vertex point, int depth, OrderedNodeList * list);
	void allNodesWithin(float maxDistance, Node * root, Vertex point, int depth, OrderedNodeList * list);
	Photon * kClosestNodes(int k, Vertex point, float &radius);
	OrderedNodeList * allNodesWithin(float distance, Vertex point, int &amount);
};







#endif
