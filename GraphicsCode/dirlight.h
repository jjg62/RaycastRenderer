#ifndef CM30075_CODE_STARTING_POINT_DIRLIGHT_H_
#define CM30075_CODE_STARTING_POINT_DIRLIGHT_H_

#include "light.h"
#include "vertex.h"
#include "vector.h"
#include <iostream>

using namespace std;

//Directional light
class DirLight : public Light {
public:

	Vector direction;
	DirLight(Vector dir, float r, float g, float b) : Light(r, g, b){
		//Ensure directional lights always cause shadows (here position is not used to dictate direction, but used in shadow tests)
		pos.x=0;
		pos.y=999999999;
		pos.z=0;


		direction.x = dir.x;
		direction.y = dir.y;
		direction.z = dir.z;
		direction.normalise();
	}

	//Return direction vector
	Vector getDirection(Vertex point){
		return direction.copy();
	}

};

#endif /* CM30075_CODE_STARTING_POINT_DIRLIGHT_H_ */
