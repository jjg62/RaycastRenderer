#ifndef CM30075_CODE_STARTING_POINT_TRIANGLE_H_
#define CM30075_CODE_STARTING_POINT_TRIANGLE_H_

#include "polymesh.h"
#include "vertex.h"

//Subclass of polymesh that allows creation of a single triangle
class Triangle : public PolyMesh {
public:
	Triangle(Vertex v1, Vertex v2, Vertex v3, BRDF * brdf) : PolyMesh(brdf){
		this->vertex_count = 3;
		this->vertex = new Vertex[3];
		vertex[0] = Vertex(v1.x, v1.y, v1.z);
		vertex[1] = Vertex(v2.x, v2.y, v2.z);
		vertex[2] = Vertex(v3.x, v3.y, v3.z);

		this->triangle_count = 1;
		this->triangle = new TriangleIndex[1];

		triangle[0][0] = 0;
		triangle[0][1] = 1;
		triangle[0][2] = 2;
	}
};



#endif
