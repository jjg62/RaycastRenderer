#include "kdtree.h"
#include "ray.h"
#include "hit.h"
#include "light.h"
#include "pointlight.h"
#include "photonmap.h"
#include "BRDF.h"
#include "object.h"
#include "sphere.h"
#include "physics.h"

#include <math.h>
#include <time.h>

//Seperate different types of photons to different KD trees
KDTree globalDirect;
KDTree globalIndirect;
KDTree globalShadow;
KDTree caustics;

const int choices = 1000; //Number of random outcomes available
void randomDirection(Ray &ray){
	//Generate random direction
	Vector dir;
	do{
		ray.direction.x = (rand() % choices) - choices / 2;
		ray.direction.y = (rand() % choices) - choices / 2;
		ray.direction.z = (rand() % choices) - choices / 2;
	}while(ray.direction.size()==0);
	ray.direction.normalise();
}

//0: brdf (diffuse/glossy specular) reflection
//1: absorbed
int russianRoulette(Object * surface){
	//In here, "diffuse" refers to any BRDF reflection (even along glossy specular direction)
	BRDF mat = *surface->brdf;

	//Probabilities
	float diffuseThreshold = max(max(mat.diffuseK[0] + mat.specularK[0], mat.diffuseK[1] + mat.specularK[1]), mat.diffuseK[2] + mat.specularK[2]);

	float rng = (rand() % 100) / 100.0f;

	if(rng < diffuseThreshold) 	return 0;
	else return 1;
}


const float powerThreshold = 0.05f; //If power is below this, stop recursing
//Trace the path of a given photon ray (for the global photon map)
//Uses intersection test function passed from main file!
void tracePhoton(Ray photonRay, float * power, bool direct, int depth, void(intersectionTest(Ray, Hit&))){

	//Check that power is enough to keep recursing
	float totalPower = power[0] + power[1] + power[2];
	if(totalPower < powerThreshold || depth <= 0){
		return;
	}

	//Test intersection with photon ray
	Hit photonHit;
	intersectionTest(photonRay, photonHit);

	if(photonHit.flag){

		//Create photon, copy information
		Photon photon;

		for(int i = 0; i < 3; i++) {
			photon.power[i] = power[i];
		}

		photon.dir.x = photonRay.direction.x;
		photon.dir.y = photonRay.direction.y;
		photon.dir.z = photonRay.direction.z;

		photon.pos.x = photonHit.position.x;
		photon.pos.y = photonHit.position.y;
		photon.pos.z = photonHit.position.z;

		//Only store in direct map if light has not been reflected
		if(direct){
			globalDirect.add(photon);
		}else{
			globalIndirect.add(photon);
		}

		//Use russian roullette to determine outcome
		int roulette = russianRoulette(photonHit.what);

		//If Reflected:
		if(roulette == 0){
			Ray nextPhotonRay;

			//Ensure random direction is not moving inside the surface
			do{
				randomDirection(nextPhotonRay);
			}while(nextPhotonRay.direction.dot(photonHit.normal) <= 0);

			//Avoid self-intersection
			nextPhotonRay.position = photonHit.position.copy();
			Vector smallE = nextPhotonRay.direction.copy();
			smallE.scale(0.1f);
			nextPhotonRay.position.add(smallE.x, smallE.y, smallE.z);

			float nextPower[3] = {0, 0, 0};
			//Get power of this reflected ray using BRDF
			photonHit.what->brdf->getReflectedLight(photonRay.direction, nextPhotonRay.direction, photonHit.normal, power, nextPower);

			//Recursively call
			tracePhoton(nextPhotonRay, nextPower, false, depth-1, intersectionTest);
		}

		//Shadow Photon
		if(direct){

			//Continue ray in same direction but starting ahead of hit surface
			Ray shadowPhotonRay;
			shadowPhotonRay.direction = photonRay.direction;

			//Avoid self-intersection
			shadowPhotonRay.position = photonHit.position;
			Vector smallShadowE = shadowPhotonRay.direction.copy();
			smallShadowE.scale(0.1f);
			shadowPhotonRay.position.add(smallShadowE.x, smallShadowE.y, smallShadowE.z);

			//Test intersection
			Hit shadowPhotonHit;
			intersectionTest(shadowPhotonRay, shadowPhotonHit);

			while(shadowPhotonHit.flag){ //While rays continue to hit new objects, cast shadows there
				//Record new shadow photon
				Photon shadowPhoton;
				shadowPhoton.pos.x = shadowPhotonHit.position.x;
				shadowPhoton.pos.y = shadowPhotonHit.position.y;
				shadowPhoton.pos.z = shadowPhotonHit.position.z;
				globalShadow.add(shadowPhoton);

				//Start new shadow ray
				shadowPhotonRay.position = shadowPhotonHit.position;
				shadowPhotonRay.position.add(smallShadowE.x, smallShadowE.y, smallShadowE.z);
				intersectionTest(shadowPhotonRay, shadowPhotonHit);
			}

		}

	}
}


//Generate a given amount of photon rays for global photon map
//(amount of PHOTONS created is higher than passed 'amount' - as many are created for each ray cast)
void createGlobalPhotons(Light * light, int amount, void(intersectionTest(Ray, Hit&))){

	srand(time(NULL)); //Get new random seed

	for(int i = 0; i < amount; i++){
		//Create ray from light going in random direction
		Ray photonRay;
		randomDirection(photonRay);
		photonRay.position = light->pos;

		tracePhoton(photonRay, light->intensity, true, 4, intersectionTest);
	}
}

//Used to render diffuse interreflection
//Average power and direction of nearby photons and apply BRDF
float * averageIndirectPhotons(int howMany, Vertex point, BRDF * brdf, Vertex viewPoint, Vector normal){
	int amount;
	OrderedNodeList * within = globalIndirect.allNodesWithin(0.7f, point, amount); //Find all photons within 0.7 of the point
	float * avgColour = new float[3]{0, 0, 0};
	Vector viewRay = Vector(point, viewPoint, true); //For BRDF

	float avgPower[3] = {0, 0, 0};
	Vector avgDirection = Vector(0, 0, 0);

	OrderedNode * current = within->head;

	while(current != NULL){
		//Average the direction vectors
		Photon currentPhoton = current->node->data;

		avgDirection.x += currentPhoton.dir.x / (float)amount;
		avgDirection.y += currentPhoton.dir.y / (float)amount;
		avgDirection.z += currentPhoton.dir.z / (float)amount;

		//Average the power values for each colour channel
		for(int c = 0; c < 3; c++) avgPower[c] += currentPhoton.power[c] / (float)amount;

		current = current->next;
	}

	delete within;
	//Apply BRDF
	if(amount > 0) brdf->getReflectedLight(avgDirection, viewRay, normal, avgPower, avgColour);

	return avgColour;
}

//Use number of nearby shadow photons and direct photons to quickly see large shadows and bright places
float getShadowChance(Vertex point){
	int shadowAmount, directAmount;
	delete globalShadow.allNodesWithin(0.5f, point, shadowAmount);
	delete globalDirect.allNodesWithin(0.5f, point, directAmount);

	float chance;
	if(shadowAmount + directAmount > 0){
		chance = (float)(shadowAmount)/(float)(shadowAmount+directAmount);
	}else{
		//No photons so no information about shadow or not - have to use a shadow ray
		chance = 0.5f;
	}

	return chance;
}


//----------CAUSTICS----------

//Calculate what happens to photon on a specular surface
//0 - refract
//1 - reflect perfectly
//2 - absorb
int causticRussianRoulette(Object * surface){
	float rng = (rand() % 100) / 100.0f;

	//Thresholds calculated using average of coefficients
	float refractThreshold = (surface->refractK[0] + surface->refractK[1] + surface->refractK[2])/3.0f;
	float reflectThreshold = refractThreshold + (surface->reflectK[0] + surface->reflectK[1] + surface->reflectK[2])/3.0f;

	if(rng < refractThreshold) return 0;
	else if(rng < reflectThreshold) return 1;
	else return 2;
}

//Check that an caustics can be stored on a given object
bool isDiffuse(Object * obj){
	float total = obj->reflectK[0] +  obj->reflectK[1] +  obj->reflectK[2] + obj->refractK[0] + obj->refractK[1] + obj->refractK[2];
	return total < 0.1f;
}


int causticPhotonCount = 0;
//Caustic version of photon tracing
void causticPhotonTrace(Ray photonRay, float * power, int depth, void(intersectionTest(Ray, Hit&)), Object * inside, bool store){

	//Check that power is enough to keep recursing
	float totalPower = power[0] + power[1] + power[2];
	if(totalPower < powerThreshold || depth <= 0){
		return;
	}

	//Test intersection of photon ray
	Hit photonHit;
	intersectionTest(photonRay, photonHit);

	if(photonHit.flag){

		//Create photon and copy information
		Photon photon;

		for(int i = 0; i < 3; i++) {
			photon.power[i] = power[i];
		}

		photon.dir.x = photonRay.direction.x;
		photon.dir.y = photonRay.direction.y;
		photon.dir.z = photonRay.direction.z;

		photon.pos.x = photonHit.position.x;
		photon.pos.y = photonHit.position.y;
		photon.pos.z = photonHit.position.z;

		//If photon isn't direct and surface is diffuse, store in kd tree
		if(store && isDiffuse(photonHit.what)){
			caustics.add(photon);
			causticPhotonCount++; //Keep track of amount for rendering caustics later
		}

		//Use coefficients to get photon outcome
		int roulette = causticRussianRoulette(photonHit.what);

		if(roulette == 0){
			//Refraction
			Ray nextPhotonRay;
			//What object is the new ray inside?
			Object * newInside;

			//If hit surface is the one we're already inside, assume we're leaving the medium
			if(inside == photonHit.what){
				newInside = NULL;
			}else{
				newInside = photonHit.what;
			}


			//We won't be using these - pass just to be safe
			float fresReflection, fresRefraction;
			Vector * refractedDir = refract(photonRay.direction, photonHit.normal, inside, newInside, &fresReflection, &fresRefraction); //Using physics.cpp


			if(refractedDir != NULL){ //Check that there is no total internal reflection

				nextPhotonRay.direction = *refractedDir;
				nextPhotonRay.direction.normalise();
				nextPhotonRay.position = photonHit.position.copy();

				//Avoid self-intersection
				Vector t_e = nextPhotonRay.direction.copy();
				t_e.scale(0.1f);
				nextPhotonRay.position.add(t_e.x, t_e.y, t_e.z);

				//Calculate power of refracted photon using coefficients
				float newPower[3];
				for(int i = 0; i < 3; i++){
					newPower[i] = power[i] * photonHit.what->refractK[i];
				}

				//Recursively trace
				causticPhotonTrace(nextPhotonRay, newPower, depth-1, intersectionTest, newInside, true);

				delete refractedDir;
			}
		}else if(roulette == 1){
			//Reflection
			Ray nextPhotonRay;

			nextPhotonRay.direction = reflect(photonRay.direction, photonHit.normal); //Using physics.cpp
			nextPhotonRay.direction.normalise();
			nextPhotonRay.position = photonHit.position.copy();

			//Avoid self-intersection
			Vector r_e = nextPhotonRay.direction.copy();
			r_e.scale(0.1f);
			nextPhotonRay.position.add(r_e.x, r_e.y, r_e.z);

			//Calculate power of reflected photon
			float newPower[3];
			for(int i = 0; i < 3; i++){
				newPower[i] = power[i] * photonHit.what->reflectK[i];
			}

			causticPhotonTrace(nextPhotonRay, newPower, depth-1, intersectionTest, inside, true);
		}

	}
}

//Calculate a direction towards a caustics-causing object using bounding spheres
void fireRaysTowardsSpecular(Object * obj, Light * light, int amount, void(intersectionTest(Ray, Hit&))){

	srand(time(NULL)); //Generate random seed

	//Try to dynamically cast bounding sphere - if fails, this function returns, no photons fired
	Sphere * bound = dynamic_cast<Sphere*>(obj->boundingSphere);
	if(bound == NULL){
		return;
	}
	//Also check that object we're firing at is appropriately 'specular'
	float specularCheck = obj->reflectK[0] + obj->reflectK[1] + obj->reflectK[2] + obj->refractK[0] + obj->refractK[1] + obj->refractK[2];
	if(specularCheck < 0.1f){
		return;
	}

	for(int i = 0; i < amount; i++){
		//Generate random position in bounding sphere
		float phi = (rand() % choices) * 2 * M_PI / float(choices); //Random number between 0-2pi
		float theta = (rand() % choices) * 2 * M_PI / float(choices); //Random number between 0-2pi
		float r = (rand() % choices) * bound->radius / float(choices); //Random number between 0-sphere radius

		Vertex pointInBound = Vertex(r*sin(phi)*cos(theta), r*sin(phi)*sin(theta), r*cos(phi)); //Use parametric equation
		pointInBound.add(bound->center.x, bound->center.y, bound->center.z); //Centered at sphere centre

		//Cast ray towards that point
		Ray photonRay;
		photonRay.position.x = light->pos.x;
		photonRay.position.y = light->pos.y;
		photonRay.position.z = light->pos.z;

		Vector direction = Vector(light->pos, pointInBound, true);
		photonRay.direction = direction;

		causticPhotonTrace(photonRay, light->intensity, 4, intersectionTest, NULL, false);
	}

}

//Used to scale brightness of caustic effects
// - Increase this if caustics aren't visible enough
// - Decrease this if rendering caustics causes rest of scene to become too dark
float causticBrightness = 60;

void setCausticBrightness(float val){
	causticBrightness = val;
}

float * averageCausticPhotons(Vertex point, BRDF * brdf, Vertex viewPoint, Vector normal){
	int amount;
	OrderedNodeList * within = caustics.allNodesWithin(0.1f, point, amount);
	float * avgColour = new float[3]{0, 0, 0};
	Vector viewRay = Vector(point, viewPoint, true);

	float avgPower[3] = {0, 0, 0};
	Vector avgDirection = Vector(0, 0, 0);

	OrderedNode * current = within->head;

	while(current != NULL){

		Photon currentPhoton = current->node->data;

		//Average the direction vectors
		avgDirection.x += currentPhoton.dir.x / (float)amount;
		avgDirection.y += currentPhoton.dir.y / (float)amount;
		avgDirection.z += currentPhoton.dir.z / (float)amount;

		//Divide by total amount of photons, scale appropriately
		for(int c = 0; c < 3; c++) avgPower[c] += causticBrightness * currentPhoton.power[c] / (float)causticPhotonCount;

		current = current->next;
	}

	delete within;
	//Apply BRDF
	if(amount > 0) brdf->getReflectedLight(avgDirection, viewRay, normal, avgPower, avgColour);

	return avgColour;
}





