#ifndef CM30075_CODE_STARTING_POINT_CUBOID_H_
#define CM30075_CODE_STARTING_POINT_CUBOID_H_

using namespace std;

#include "polymesh.h"
#include "vertex.h"

//Cuboid defined by two points
class Cuboid : public PolyMesh{
public:
	Cuboid(Vertex v1, Vertex v2, BRDF * brdf);

};

#endif
