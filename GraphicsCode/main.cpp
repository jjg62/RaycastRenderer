#include "polymesh.h"
#include "framebuffer.h"
#include "ray.h"
#include "vector.h"
#include "sphere.h"
#include "hit.h"
#include "pinholecamera.h"
#include "object.h"
#include "light.h"
#include "dirlight.h"
#include "pointlight.h"
#include "cuboid.h"
#include "PhongBRDF.h"
#include "kdtree.h"
#include "photonmap.h"
#include "physics.h"
#include "triangle.h"

#include <iostream>
#include <math.h>
#include <pthread.h>

using namespace std;


//Scene - store all objects
Object ** scene;
int sceneSize;

//Lights
float ambientLight[] = {0.3, 0.3, 0.3};
int lightCount = 1;
Light** lights;



//Initialise objects in the scene and stored them in an object pointer array
void loadScene(Object ** scene){


	//Cornell Box Scene Setup
	
	//Define Light Coefficients
	float * whiteDiffuse = new float[3]{0.6f, 0.6f, 0.6f};
	float * noSpecular = new float[3]{0.0f, 0.0f, 0.0f};
	float * shinySpecular = new float[3]{0.4f, 0.4f, 0.4f};
	float * noDiffuse = new float[3]{0, 0, 0};
	float * redDiffuse = new float[3]{0.6f, 0, 0};
	float * blueDiffuse = new float[3]{0, 0, 0.6f};
	float * greenDiffuse = new float[3]{0, 0.6f, 0};
	
	//Create BRDFs
	BRDF * whiteRough = new PhongBRDF(whiteDiffuse, noSpecular, 40);
	BRDF * blueShiny = new PhongBRDF(noDiffuse, shinySpecular, 40);
	BRDF * greenRough = new PhongBRDF(greenDiffuse, shinySpecular, 40);
	BRDF * redRough = new PhongBRDF(redDiffuse, shinySpecular, 40);
	BRDF * blueRough = new PhongBRDF(blueDiffuse, noSpecular, 40);

	//Floor
	Triangle * floor = new Triangle(Vertex(-50, -1, -4), Vertex(0, -1, 30), Vertex(50, -1, -4), whiteRough);
	floor->setLightCoeff(floor->reflectK, 0, 0, 0);
	floor->setLightCoeff(floor->refractK, 0, 0, 0);

	//Ceiling
	Triangle * ceil = new Triangle(Vertex(-50, 3, -4), Vertex(50, 3, -4), Vertex(0, 3, 30), whiteRough);
	ceil->setLightCoeff(ceil->reflectK, 0, 0, 0);
	ceil->setLightCoeff(ceil->refractK, 0, 0, 0);

	//Back wall, reflective
	Triangle * back = new Triangle(Vertex(-5, -3, 12), Vertex(0, 50, 12), Vertex(5, -3, 12), whiteRough);
	back->setLightCoeff(back->reflectK, 0.9, 0.9, 0.9);
	back->setLightCoeff(back->refractK, 0, 0, 0);
	back->boundingSphere = new Sphere(Vertex(0, 0, 8), 6, NULL);

	//Front wall (behind camera)
	Triangle * front = new Triangle(Vertex(-5, -3, -6), Vertex(5, -3, -6), Vertex(0, 50, -6), blueRough);
	front->setLightCoeff(front->reflectK, 0, 0, 0);
	front->setLightCoeff(front->refractK, 0, 0, 0);

	//Left wall
	Triangle * left = new Triangle(Vertex(-3, 4, -4), Vertex(-3, 1, 50), Vertex(-3, -2, -4), redRough);
	left->setLightCoeff(left->reflectK, 0, 0, 0);
	left->setLightCoeff(left->refractK, 0, 0, 0);

	//Right wall
	Triangle * right = new Triangle(Vertex(3, 4, -4), Vertex(3, -2, -4), Vertex(3, 1, 50), greenRough);
	right->setLightCoeff(right->reflectK, 0, 0, 0);
	right->setLightCoeff(right->refractK, 0, 0, 0);

	//3 Spheres
	Sphere * sphere = new Sphere(Vertex(-1.5, 0, 1.5), 1, blueRough);
	sphere->setLightCoeff(sphere->reflectK, 0, 0, 0);
	sphere->setLightCoeff(sphere->refractK, 0.9, 0.9, 0.9);
	sphere->ior = 1.52f;
	sphere->boundingSphere = sphere;

	Sphere * sphere1 = new Sphere(Vertex(-1, 1, 4), 1, greenRough);
	sphere1->setLightCoeff(sphere1->reflectK, 0, 0, 0);
	sphere1->setLightCoeff(sphere1->refractK, 0.9f, 0.9f, 0.9f);
	sphere1->ior = 1.52f;
	sphere1->boundingSphere = sphere1;

	Sphere * sphere2 = new Sphere(Vertex(1, 1.5, 6), 1, redRough);
	sphere2->setLightCoeff(sphere2->reflectK, 0, 0, 0);
	sphere2->setLightCoeff(sphere2->refractK, 0.9f, 0.9f, 0.9f);
	sphere2->ior = 1.52f;
	sphere2->boundingSphere = sphere2;


	// The following transform allows 4D homogeneous coordinates to be transformed. It moves the supplied teapot model to somewhere visible.
	Transform *transform = new Transform(
			0.6f, 0.0f, 0.0f, 0.5f,
			0.0f, 0.6f, 0.0f, -1.0f,
			0.0f, 0.0f, 0.6f, 3.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

	// Read in the teapot model.
	PolyMesh *pm = new PolyMesh((char *)"teapot.ply", transform, whiteRough);

	pm->setLightCoeff(pm->reflectK, 0.0f, 0.0f, 0.0f);
	pm->setLightCoeff(pm->refractK, 0.0f, 0.0f, 0.0f);
	pm->boundingSphere = new Sphere(Vertex(0.5, -1, 3), 2.5, NULL);

	//Store objects in the scene array
	scene[0] = floor;
	scene[1] = back;
	scene[2] = right;
	scene[3] = left;
	scene[4] = ceil;
	scene[5] = sphere;
	scene[6] = front;
	scene[7] = pm;
	scene[8] = sphere1;
	scene[9] = sphere2;

}


//Given a ray, test its intersection with each object in the scene and store the closest one in hit
void intersectionTest(Ray ray, Hit &hit){
	//Start so high that any intersection will become the closest
	float closestT = 9999999999999999;
	hit.flag = false;
	Hit tempHit;

	for(int o = 0; o < sceneSize; o++){
		//For each object in the scene, calculate intersection.
		scene[o]->intersection(ray, tempHit);

		if(tempHit.flag && tempHit.t < closestT){
			//If this is closer than the former closest intersection, replace it.
			closestT = tempHit.t;

			//Copy hit information
			hit.flag = true;
			hit.t = tempHit.t;
			hit.what = tempHit.what;
			hit.position = tempHit.position;
			hit.normal = tempHit.normal;

		}

	}
}

//Given a view ray, return the radiance at its point of intersection.
//Recursively called for refraction and reflection
float * raytrace(Ray &ray, PinholeCamera cam, Object * inside, int depth){

	float * colour = new float[3];
	colour[0] = 0;
	colour[1] = 0;
	colour[2] = 0;

	//If raycast tree had reached required depth, stop
	if(depth <= 0) return colour;
	//Get closest intersection point of view ray
	Hit hit;
	intersectionTest(ray, hit);

	if(hit.flag){
		//Use photon map for ambient term
		delete[] colour;
		colour = averageIndirectPhotons(50, hit.position, hit.what->brdf, cam.position, hit.normal);
		//Caustics
		float * caustics;
		caustics = averageCausticPhotons(hit.position, hit.what->brdf, cam.position, hit.normal);
		for(int c = 0; c < 3; c++) colour[c] += caustics[c]; //Add their contribution
		delete[] caustics;

		//For each light in the scene
		for(int l = 0; l < lightCount; l++){

			Vector toLight = lights[l]->getDirection(hit.position);
			toLight.scale(-1); //Get L - direction to light

			//Shadow Checks
			bool inShadow;

			float shadowChance = getShadowChance(hit.position); //First try using shadow photons
			if(shadowChance < 0.1){
				inShadow = false;
			}else if(shadowChance > 0.96){
				inShadow = true;
			}else{
				//Need a shadow ray
				Ray shadowRay = Ray();

				shadowRay.direction = toLight;
				shadowRay.position = hit.position.copy();
				//Add small value to avoid self-intersection
				Vector e = shadowRay.direction.copy();
				e.scale(0.1f);
				shadowRay.position.add(e.x, e.y, e.z);

				Hit shadowHit;
				intersectionTest(shadowRay, shadowHit);

				//Objects on the other side of the light don't count
				float lightDistance = Vector(shadowRay.position, lights[l]->pos, false).size();
				inShadow = shadowHit.flag && shadowHit.t < lightDistance;
			}

			if(!inShadow){
				//Perform direct light calculation if not in shadow
				hit.what->brdf->getReflectedLight(lights[l]->getDirection(hit.position), Vector(hit.position, cam.position, true), hit.normal, lights[l]->intensity, colour);

			}
		}

		//Global Lighting

		//Refraction
		Ray tray = Ray();
		//What object is the new ray inside?
		Object * newInside;

		//If ray has hit the object it is currently inside, must mean that it is back in 'air'
		if(inside == hit.what){
			newInside = NULL;
		}else{
			newInside = hit.what;
		}

		float fresReflection, fresRefraction;
		//Calculate refracted ray direction and fresnel terms
		Vector * refractedDir = refract(ray.direction, hit.normal, inside, newInside, &fresReflection, &fresRefraction);

		//To optimise, first check if the refraction will make much contribution
		float refractKTotal = hit.what->refractK[0] + hit.what->refractK[1] + hit.what->refractK[2];
		if(refractKTotal > 0.05f){

			if(refractedDir != NULL){ //Check that there is no total internal reflection

				tray.direction = *refractedDir;
				tray.direction.normalise();
				tray.position = hit.position.copy();
				//Avoid self-intersection
				Vector t_e = tray.direction.copy();
				t_e.scale(0.1f);
				tray.position.add(t_e.x, t_e.y, t_e.z);

				//Recursively raytrace and add weighted result
				float * tColour = raytrace(tray, cam, newInside, depth-1);
				for(int c = 0; c < 3; c++) colour[c] += fresRefraction*hit.what->refractK[c] * tColour[c];
				delete[] tColour;
			}
		}

		delete refractedDir;

		//Reflection
		//To optimise, first check if the reflection will make much contribution
		float reflectKTotal = hit.what->reflectK[0] + hit.what->reflectK[1] + hit.what->reflectK[2] + refractKTotal;
		if(reflectKTotal > 0.05f){
			Ray rray = Ray();
			//Calculate reflected direction
			rray.direction = reflect(ray.direction, hit.normal);
			rray.direction.normalise();
			rray.position = hit.position.copy();
			//Avoid self-intersection
			Vector r_e = rray.direction.copy();
			r_e.scale(0.1f);
			rray.position.add(r_e.x, r_e.y, r_e.z);

			//Recursively raytrace and add weighted result
			float * rColour = raytrace(rray, cam, inside, depth-1);
			for(int c = 0; c < 3; c++) colour[c] += (fresReflection*hit.what->refractK[c] + hit.what->reflectK[c]) * rColour[c];
			delete[] rColour;
		}

	}

	return colour;
}

int main(){

	//Framebuffer dimensions
	int height = 256;
	int width = 256;

	sceneSize = 10; //Number of objects in scene
	scene = new Object*[sceneSize];
	loadScene(scene); //Initialise objects

	//Lights
	lights = new Light*[lightCount];
	lights[0] = new PointLight(Vertex(0, 2.5f, 3), 0.2, 0.2, 0.2);

	//Initialise camera
	PinholeCamera cam = PinholeCamera(Vertex(0.0001f, 2, -5.5f), Vertex(0, 0, 5), Vector(0, 1, 0),  width/1000.0f);

	//Create a framebuffer
	FrameBuffer *fb = new FrameBuffer(height,width);

	cout << "Begin Photon Mapping" << endl;

	//Photon totals
	int globalPhotons = 16000;
	int specularPhotons = 12000;

	//Shared between lights
	if(lightCount > 0){
		for(int l = 0; l < lightCount; l++){
			//Photon mapping current only supported for point lights
			PointLight * pointLight = dynamic_cast<PointLight*>(lights[l]);

			if(pointLight != NULL)
			for(int o = 0; o < sceneSize; o++){
				fireRaysTowardsSpecular(scene[o], pointLight, specularPhotons/lightCount, intersectionTest);
			}
			createGlobalPhotons(pointLight, globalPhotons/lightCount, intersectionTest);
		}
	}


	//Adjust this value to change appearance of caustics
	// - Increase this if caustics aren't visible enough
	// - Decrease this if rendering caustics causes rest of scene to become too dark
	setCausticBrightness(40);

	cout << "Finished Photon Mapping" << endl;


	//Depth of field with small lens model
	int sampleSize = 24;
	Ray rays[sampleSize];



	for(int i = 0; i < width; i++){
		for(int j = 0; j < height; j++){
			//For each pixel on the image plane, create some sampled rays
			//(first one will always be accurate as in the pinhole model)
			cam.makeSampledRays(i, j, width, height, 7, sampleSize, rays);

			float colour[] = {0, 0, 0};
			float * colourK;

			for(int k = 0; k < sampleSize; k++){

				colourK = raytrace(rays[k], cam, NULL, 4);
				//Average the contributions from each ray
				for(int c = 0; c < 3; c++) colour[c] += colourK[c]/(float)sampleSize;
				delete[] colourK;
			}
			//Draw onto framebuffer
			fb->plotPixel(i, j, colour[0], colour[1], colour[2]);

			//Optionally print progress
			cout << i*width + j << "/" << width*height << endl;
		}
	}



	// Output the framebuffer.
	fb->writeRGBFile((char *)"result.ppm");

	std::cout << "DONE!" << std::endl;

	return 0;
}


