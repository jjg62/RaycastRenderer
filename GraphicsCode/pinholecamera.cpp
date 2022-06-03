#include "pinholecamera.h"

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

PinholeCamera::PinholeCamera(Vertex position, Vertex look, Vector up, float d) {
	this->position = position;
	this->look = look;
	this->up = up;
	this->d = d;

	this->up.normalise();
	//How much world space each pixel should take up when calculating rays
	pixelSize = 0.001;

	//Calculate Orthonormal Basis
	w = Vector(look, position, true);
	this->up.cross(w, u);
	u.normalise();
	w.cross(u, v);

}

float lensRadius = 0.2f; //Size of lens in small lens model

//Return a random point on the lens
Vertex PinholeCamera::randomPointOnLens(){
	float r = lensRadius * (rand() % 100)/100.0f; //Random number between 0 and radius
	float angle = 2*M_PI * (rand() % 100)/100.0f; //Random angle between 0 and 2pi


	//Add rcos(angle) and rsin(angle) to center of lens
	Vector addU = u.copy();
	addU.scale(r*cos(angle));

	Vector addV = v.copy();
	addV.scale(r*sin(angle));

	Vertex randomPoint = position.copy();
	randomPoint.add(addU.x + addV.x, addU.y + addV.y, addU.z + addV.z);

	return randomPoint;
}

//Given pixel co-ordinates on image plane, calculate direction of ray
void PinholeCamera::getDirectionFromPixel(int i, int j, int fbWidth, int fbHeight, Ray &ray){

	float planeX = (fbWidth/2 - i)*pixelSize;
	float planeY = (fbHeight/2 - j)*pixelSize;

	Vector xu = Vector(u.x, u.y, u.z); //Copy u
	xu.scale(planeX);
	Vector yv = Vector(v.x, v.y, v.z); //Copy v
	yv.scale(planeY);
	Vector dw = Vector(w.x, w.y, w.z); //Copy w
	dw.scale(-d);

	//direction = xu + yv - dw
	Vector dir = xu;
	dir.add(yv);
	dir.add(dw);
	dir.normalise();

	//Store results in passed ray pointer
	ray.position = position;
	ray.direction = dir;

}

//For depth of field - generate some rays that start from a random pos on lens and all intersect on the focal plane
void PinholeCamera::makeSampledRays(int i, int j, int fbWidth, int fbHeight, float focalDistance, int sampleSize, Ray *rays){
	if(sampleSize < 1){
		cout << "ERROR: invalid sample size for pinholecamera" << endl;
		return;
	}else{
		//Make first ray an accurate one (pinhole camera)
		getDirectionFromPixel(i, j, fbWidth, fbHeight, rays[0]);


		for(int k = 1; k < sampleSize; k++){
			Vertex startPos = randomPointOnLens(); //Start at random point on lens


			//Calculate cosine of angle between accurate ray direction and look direction
			Vector minusW = w.copy();
			minusW.negate();
			float cosang = minusW.dot(rays[0].direction);

			//To calculate direction, add vector going from startPos to center of lens to the vector
			// going from the center of the lens to the focal plane.
			Vector difference = Vector(startPos, position, false);
			Vector toFocalPoint = rays[0].direction.copy();
			//Scale to reach the focal plane
			toFocalPoint.scale(focalDistance / cosang);

			Vector dir = difference;
			dir.add(toFocalPoint);

			//Then normalise
			dir.normalise();

			//Store in passed pointer
			rays[k].position = startPos;
			rays[k].direction = dir;

		}
	}

}

