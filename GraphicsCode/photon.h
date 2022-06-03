#ifndef CM30075_CODE_STARTING_POINT_PHOTON_H_
#define CM30075_CODE_STARTING_POINT_PHOTON_H_

#include "vertex.h"
#include "vector.h"


//Store position, direction and power in each photon
class Photon {
public:
	Vertex pos;
	Vector dir;
	float power[3];
};

#endif
