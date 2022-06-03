#include "kdtree.h"

#include <iostream>
using namespace std;

//Node constructor
Node::Node(Photon photon){
	data = photon; //Store a photon
	//Child nodes
	left = NULL;
	right = NULL;

	//Properties to sort by are the photon's coordinates
	properties[0] = photon.pos.x;
	properties[1] = photon.pos.y;
	properties[2] = photon.pos.z;
}

int maxDepth = 0;
void Node::add(Node * node, int depth){
	if(depth > maxDepth) maxDepth = depth; //For debugging, store depth of tree

	int propertyIndex = depth % dims; //Cycle through properties to sort by
	if(this->properties[propertyIndex] > node->properties[propertyIndex]){
		//Node should be added to left
		if(this->left == NULL){
			this->left = node;
		}else{
			this->left->add(node, depth+1); //Change depth and thus the deciding property
		}

	}else{
		//Node should be added to right
		if(this->right == NULL){
			this->right = node;
		}else{
			this->right->add(node, depth+1);
		}

	}
}

//Add a photon to the tree
void KDTree::add(Photon photon) {
	//Create a node to wrap the photon
	Node* node = new Node(photon);

	//Add it to tree
	if(root == NULL){
		root = node;
	}else{
		root->add(node, 0);
	}
}

//Calculate squared distance between two vertices
float sqrDistance(Vertex p1, Vertex p2){
	return pow(p2.x - p1.x, 2.0f) + pow(p2.y - p1.y, 2.0f) + pow(p2.z - p1.z, 2.0f);
}

//Given two nodes, return the closest one to the target point
Node * closestNodeOfTwo(Node * node1, Node * node2, Vertex point){

	if(node1 == NULL) return node2;
	if(node2 == NULL) return node1;

	if(sqrDistance(node1->data.pos, point) <  sqrDistance(node2->data.pos, point)){
		return node1;
	}else{
		return node2;
	}
}


//Find the one closest node to a target point
Node * KDTree::closestNode(Node * root, Vertex point, int depth){
	if(root == NULL) return NULL; //At leaves, return nothing

	//Convert target point to array
	float pos[] = {point.x, point.y, point.z};
	int pi = depth % root->dims; //Property index - which property to compare

	Node * next;
	Node * other;

	//Decide next child to recursively call
	if(root->properties[pi] > pos[pi]){
		next = root->left;
		other = root->right;
	}else{
		next = root->right;
		other = root->left;
	}

	//Recursively find best node in child
	Node * temp = closestNode(next, point, depth+1);
	//Compare with root
	Node * best = closestNodeOfTwo(temp, root, point);

	//Check other side of tree
	float bestDistSqr = sqrDistance(best->data.pos, point);
	float dist = pos[pi] - root->properties[pi];

	//If distance (on one axis) from root to target is smaller than current best
	if(bestDistSqr >= dist * dist){

		temp = closestNode(other, point, depth+1);
		best = closestNodeOfTwo(best, temp, point);
	}

	return best;
}

Node * KDTree::closestNode(Vertex point){
	return closestNode(root, point, 0);
}


//For k-nearest nodes, we need an ordered list of nodes
//Add node to the list (recursively called version)
void OrderedNode::add(Node * nodeToAdd, Vertex point){
	if(sqrDistance(this->node->data.pos, point) < sqrDistance(nodeToAdd->data.pos, point)){
		//If this is the right place to add node
		OrderedNode * copy = new OrderedNode();
		copy->next = this->next;
		copy->node = this->node;

		this->next = copy;
		this->node = nodeToAdd;
	}else{
		//Add to next node
		if(next == NULL){ //But if it's null, make it the next node
			OrderedNode * newNode = new OrderedNode();
			newNode->next = NULL;
			newNode->node = nodeToAdd;
			next = newNode;
		}else{
			next->add(nodeToAdd, point);
		}
	}
}

//Add node to ordered list list
void OrderedNodeList::add(Node * nodeToAdd, Vertex point){
	if(head == NULL){
		//Make node the new head, with nothing after
		OrderedNode * newNode = new OrderedNode();
		newNode->next = NULL;
		newNode->node = nodeToAdd;
		head = newNode;
	}else if(sqrDistance(head->node->data.pos, point) < sqrDistance(nodeToAdd->data.pos, point)){
		//Make node the new head, move old head
		OrderedNode * newNode = new OrderedNode();
		newNode->next = head;
		newNode->node = nodeToAdd;
		head = newNode;
	}else{
		//Needs to be added after head
		if(head->next == NULL){
			//if head is only element, make new node the next one
			OrderedNode * newNode = new OrderedNode();
			newNode->next = NULL;
			newNode->node = nodeToAdd;
			head->next = newNode;
		}else{
			//Otherwise recursively add to next node
			head->next->add(nodeToAdd, point);
		}
	}
	length++;
}

//Remove head, make second element new head
void OrderedNodeList::pop(){
	OrderedNode * oldHead = head;
	head = oldHead->next;
	free(oldHead);
	length--;

}

//Add a node to the ordered list - if this takes length over k, remove the furthest node (head)
void AddAndKeepClosestK(OrderedNodeList * currentList, Node * newNode, Vertex point, int k){
	if(newNode == NULL) return;
	if(currentList->length < k){
		//No need to remove anything
		currentList->add(newNode, point);
	}else if(sqrDistance(currentList->head->node->data.pos, point) > sqrDistance(newNode->data.pos, point)){ //if the distance between the further point in list and target is less than the new point's
		currentList->pop(); //Remove head
		currentList->add(newNode, point); //Add new node
	}
}

//Search for the k closest nodes to a target point
void KDTree::kClosestNodes(int k, Node * root, Vertex point, int depth, OrderedNodeList * list){
	if(root == NULL){
		return; //Return on leaves
	}

	float pos[] = {point.x, point.y, point.z}; //Convert point to array
	int pi = depth % root->dims; //Which property to compare
	Node * next;
	Node * other;

	//Which child to search next
	if(root->properties[pi] > pos[pi]){
		next = root->left;
		other = root->right;
	}else{
		next = root->right;
		other = root->left;
	}

	//Recursively find k closest nodes in child
	kClosestNodes(k, next, point, depth+1, list);

	Node * compareNode;
	if(list->head == NULL){
		compareNode = NULL;
	}else{
		compareNode = list->head->node;
	}
	//Which is better - current root or worst in current list?
	Node * best = closestNodeOfTwo(compareNode, root, point);


	float compareDistance;
	if(list->head == NULL){
		compareDistance = 100000000;
	}else{
		compareDistance = sqrDistance(list->head->node->data.pos, point);
	}

	//Check other side of tree if dist <= ANY of the k nodes
	float dist = pos[pi] - root->properties[pi];

	//Search other child if distance of root to target is better than worst in list
	if(compareDistance >= dist * dist || list->length < k){
		kClosestNodes(k, other, point, depth+1, list);
		//Update best
		best = closestNodeOfTwo(compareNode, best, point);
	}

	//Add best candidate to list
	AddAndKeepClosestK(list, best, point, k);
}

//Return a list of all nodes within a certain distance
void KDTree::allNodesWithin(float maxDist, Node * root, Vertex point, int depth, OrderedNodeList * list){
	if(root == NULL){
		return; //Return at leaves
	}

	float pos[] = {point.x, point.y, point.z}; //Convert point to array
	int pi = depth % root->dims; //Find which property to compare
	Node * next;
	Node * other;

	//Which child to search
	if(root->properties[pi] > pos[pi]){
		next = root->left;
		other = root->right;
	}else{
		next = root->right;
		other = root->left;
	}

	//Recursively find all valid nodes within child
	allNodesWithin(maxDist, next, point, depth+1, list);
	//If root is within range, add it to list
	if(sqrDistance(root->data.pos, point) < maxDist*maxDist) list->add(root, point);

	//Check other side of tree if dist <= maxDist
	float dist = pos[pi] - root->properties[pi];

	if(maxDist * maxDist >= dist * dist){
		allNodesWithin(maxDist, other, point, depth+1, list);
	}

}

//Nicer interface for k closest nodes
Photon * KDTree::kClosestNodes(int k, Vertex point, float &radius){
	OrderedNodeList * list = new OrderedNodeList();
	list->head = NULL;
	list->length = 0;
	kClosestNodes(k, root, point, 0, list);

	//Also return radius (worst distance out of k best)
	radius = sqrt(sqrDistance(list->head->node->data.pos, point));
	OrderedNode * current = list->head;

	int count = 0;
	Photon * photonList = new Photon[k];
	//Convert from node list to list of photons
	while(current != NULL){
		photonList[count] = current->node->data;
		count++;
		current = current->next;
	}

	delete list;

	return photonList;
}

//Nicer interface for allNodesWithin
OrderedNodeList * KDTree::allNodesWithin(float distance, Vertex point, int &amount){
	OrderedNodeList * list = new OrderedNodeList();
	list->head = NULL;
	list->length = 0;

	allNodesWithin(distance, root, point, 0, list);

	//Also return amount in range
	amount = list->length;

	return list;
}
