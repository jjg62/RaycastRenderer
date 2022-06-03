/*
 * light.h
 *
 *  Created on: 3 Nov 2021
 *      Author: Jac
 */

#ifndef CM30075_CODE_STARTING_POINT_LIGHT_H_
#define CM30075_CODE_STARTING_POINT_LIGHT_H_

#include "vector.h"
#include "vertex.h"

//Abstract class for lights - all subclasses must override getDirection
class Light {
	public:
		Vertex pos;
		float intensity[3];
		Light(float r, float g, float b){
			intensity[0] = r;
			intensity[1] = g;
			intensity[2] = b;
		}

		virtual Vector getDirection(Vertex atPoint){
			return Vector(0, 0, 0);
		}
};


#endif /* CM30075_CODE_STARTING_POINT_LIGHT_H_ */
