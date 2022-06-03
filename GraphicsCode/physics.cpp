#include "physics.h"

//Physics helper functions


//Calculate direction of perfect reflection
Vector reflect(Vector incident, Vector normal){

	Vector reflection = incident.copy();
	reflection.normalise();

	float in = reflection.dot(normal); //I.N
	Vector scaledNormal = normal.copy();
	scaledNormal.normalise();
	scaledNormal.scale(-2.0f*in); //-2*I.N*N
	reflection.add(scaledNormal); // R = I - 2*I.N*N

	return reflection;
}


//Get Index of Reflection (if in air, this is 1)
float getIor(Object * obj){
	if(obj == NULL) return 1.0f;
	return obj->ior;
}



//Calculate direction of refraction
Vector* refract(Vector incident, Vector normal, Object * inside, Object * target, float *fresReflection, float *fresRefraction){

	//Copy vectors for safety
	Vector norm = normal.copy();
	Vector inc = incident.copy();

	//Want direction away from surface so negate
	inc.negate();
	inc.normalise();
	norm.normalise();

	//Index of reflection
	float ior = getIor(target) / getIor(inside);

	//Cosine of angle is dot product
	float cosi = norm.dot(inc);

	//If normal is on the wrong side, temporarily negate it
	if(cosi < 0){
		norm.negate();
		cosi = norm.dot(inc);
	}

	//Use Snell's law to calculate cos(t) squared
	float cost2 = 1.0f-((float)1.0f/(ior*ior))*(1-(cosi*cosi));
	if(cost2 < 0.0f){
		//If this is negative, we have total internal reflection
		*fresRefraction = 0.0f;
		*fresReflection = 1.0f;
		return NULL; //Return NULL, check for this where called
	}
	//Otherwise, safe to square root
	float cost = sqrt(cost2);

	//Apply equation to calculate new direction
	Vector * T = new Vector();
	*T = inc.copy();
	T->negate(); //Flip I back to original direction
	T->scale(1/ior); //(1/n)I

	Vector scaledN = norm.copy();
	scaledN.scale(-1 * (cost - (1/ior)*cosi)); //-(cos(t) - (1/n)cos(i))
	T->add(scaledN); //T = (1/n)I -(cos(t) - (1/n)cos(i))


	//Fresnel terms - store them in passed pointers
	float rPar = (ior*cosi - cost) / (ior*cosi + cost);
	float rPer = (cosi - ior*cost) / (cosi + ior*cost);

	*fresReflection = (rPar*rPar + rPer*rPer)/2.0f;
	*fresRefraction = 1 - *fresReflection;

	return T;
}


