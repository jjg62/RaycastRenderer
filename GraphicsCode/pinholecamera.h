#ifndef CM30075_CODE_STARTING_POINT_PINHOLECAMERA_H_
#define CM30075_CODE_STARTING_POINT_PINHOLECAMERA_H_

#include "vector.h"
#include "vertex.h"
#include "ray.h"

//Class that helps generate ray directions using image plane co-ordinates
class PinholeCamera {
private:
	Vertex look;
	Vector up;
	float d;

	float pixelSize;

	//Orthnormal Basis
	Vector w;
	Vector u;
	Vector v;

public:
	Vertex position;
	PinholeCamera(Vertex position, Vertex look, Vector up, float d);
	void getDirectionFromPixel(int i, int j, int fbWidth, int fbHeight, Ray &ray);
	void makeSampledRays(int i, int j, int fbWidth, int fbHeight, float focalDistance, int sampleSize, Ray *rays);
	Vertex randomPointOnLens();
};


#endif /* CM30075_CODE_STARTING_POINT_PINHOLECAMERA_H_ */
