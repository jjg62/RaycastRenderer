/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2019.
 *
 * Do what you like with this code as long as you retain this comment.
 */

// Object is the base class for objects.
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "ray.h"
#include "hit.h"
#include <iostream>
#include "BRDF.h"

using namespace std;

typedef float LightCoeff[3];

class Object {
public:


	//Refract and reflect coefficients
	LightCoeff reflectK;
	LightCoeff refractK;

	BRDF * brdf;

	Object * boundingSphere; //Must be dynamically castable to sphere, or bounding sphere won't function

	float ior = 1.0f; //Index of refraction

	Object(BRDF * _brdf)
	{
		this->brdf = _brdf;
		this->boundingSphere = NULL;
	}
	
	//Objects must implement an intersection method
	virtual void intersection(Ray ray, Hit &hit)
	{

	}

	void setLightCoeff(LightCoeff coeff, float r, float g, float b){
		coeff[0] = r;
		coeff[1] = g;
		coeff[2] = b;
	}

};

#endif
