#ifndef CM30075_CODE_STARTING_POINT_PHOTONMAP_H_
#define CM30075_CODE_STARTING_POINT_PHOTONMAP_H_

#include "pointlight.h"
#include "light.h"
#include "ray.h"
#include "hit.h"
#include "BRDF.h"

//Functions that create and read from photon maps

void createGlobalPhotons(Light * light, int amount, void(intersectionTest(Ray, Hit&)));

float * averageIndirectPhotons(int howMany, Vertex point, BRDF * brdf, Vertex viewPoint, Vector normal);

float getShadowChance(Vertex point);

void fireRaysTowardsSpecular(Object * obj, Light * light, int amount, void(intersectionTest(Ray, Hit&)));

float * averageCausticPhotons(Vertex point, BRDF * brdf, Vertex viewPoint, Vector normal);

void setCausticBrightness(float val);

#endif
