#ifndef CM30075_CODE_STARTING_POINT_BRDF_PHONGBRDF_H_
#define CM30075_CODE_STARTING_POINT_BRDF_PHONGBRDF_H_

#include <math.h>
#include <iostream>
#include "physics.h"
#include "BRDF.h"
using namespace std;

//BRDF which uses diffuse and glossy specular Phong model
class PhongBRDF : public BRDF {
private:
public:
	PhongBRDF(float * _diffuseK, float * _specularK, float _n){
		this->diffuseK = new float[3];
		this->specularK = new float[3];
		for(int i = 0; i < 3; i++){
			diffuseK[i] = _diffuseK[i];
			specularK[i] = _specularK[i];
		}
		this->n = _n;

	}

	void getReflectedLight(Vector incident, Vector reflectDirection, Vector normal, float * lightIntensity, float * col){

		//Get vector towards light
		Vector toLight = incident.copy();
		toLight.normalise();
		toLight.negate();

		float diffuse = max(toLight.dot(normal), 0.0f); //If normal is on wrong side, diffuse component isn't added

		//Specular
		Vector perfectReflect = reflect(incident, normal); //Call reflect in physics.cpp
		float specular = max(pow(perfectReflect.dot(reflectDirection), n), 0.0f);

		//Total contributions in passed pointer
		for(int i = 0; i < 3; i++){
			col[i] += lightIntensity[i] * (diffuse*diffuseK[i] + specular*specularK[i]);
		}
	}

};


#endif
