#ifndef CM30075_CODE_STARTING_POINT_BRDF_BRDF_H_
#define CM30075_CODE_STARTING_POINT_BRDF_BRDF_H_

#include "vector.h"

//Abstract class for BRDFs
class BRDF {
public:
	//Need coefficients for Russian Roulette (and for calculations in Phong)
	float * diffuseK;
	float * specularK;
	float n;
	virtual void getReflectedLight(Vector incident, Vector reflect, Vector normal, float * lightIntensity,  float * col){

	}
};



#endif
