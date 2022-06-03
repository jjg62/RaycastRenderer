/*
 * cuboid.cpp
 *
 *  Created on: 18 Nov 2021
 *      Author: Jac
 */

#include "cuboid.h"

using namespace std;

//As a special case of a polymesh, create a cuboid defined by two corner points: Top left front and bottom right back.
Cuboid::Cuboid(Vertex v1, Vertex v2, BRDF * brdf) : PolyMesh(brdf){

	this->vertex_count = 8;
	this->vertex = new Vertex[8];
	this->vertex[0] = v1; //Top Left Front
	this->vertex[1] = Vertex(v1.x, v2.y, v1.z); //Bottom Left Front
	this->vertex[2] = Vertex(v2.x, v1.y, v1.z); //Top Right Front
	this->vertex[3] = Vertex(v2.x, v2.y, v1.z); //Bottom Right Front
	this->vertex[4] = v2; //Bottom Right Back
	this->vertex[5] = Vertex(v1.x, v2.y, v2.z); //Bottom Left Back
	this->vertex[6] = Vertex(v2.x, v1.y, v2.z); //Top Right Back
	this->vertex[7] = Vertex(v1.x, v1.y, v2.z); //Top Left Back


	int trianglesToLoad[12][3] = {
			{0, 2, 1}, {1, 2, 3}, //Front
			{0, 7, 6}, {0, 6, 2}, //Top
			{1, 4, 5}, {1, 3, 4}, // Bottom
			{5, 7, 6}, {5, 6, 4}, //Back
			{1, 5, 7}, {1, 7, 0}, //Left
			{3, 6, 4}, {3, 2, 6} //Right
	};

	this->triangle_count = 12;
	this->triangle = new TriangleIndex[12];

	for(int i = 0; i < 12; i++){
		for (int j = 0; j < 3; j++){
			triangle[i][j] = trianglesToLoad[i][j];
		}
	}



}
