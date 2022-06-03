#ifndef CM30075_CODE_STARTING_POINT_POINTLIGHT_H_
#define CM30075_CODE_STARTING_POINT_POINTLIGHT_H_

#include "light.h"

class PointLight : public Light {
public:
	PointLight(Vertex _pos, float r, float g, float b) : Light(r, g, b){
		this->pos = _pos;
	}

	//In the case of a point light, direction is calculated using its position (these have no direction)
	Vector getDirection(Vertex atPoint){
		return Vector(pos, atPoint, true);
	}

};


#endif
