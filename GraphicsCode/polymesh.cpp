/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2019.
 *
 * Do what you like with this code as long as you retain this comment.
 */

#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "hit.h"
#include "ray.h"
#include "polymesh.h"
#include "sphere.h"

using namespace std;

PolyMesh::PolyMesh(char *file, BRDF * brdf) : Object(brdf)
{
  Transform *transform = new Transform();

  this->do_construct(file, transform);
}

PolyMesh::PolyMesh(char *file, Transform *transform, BRDF * brdf) : Object(brdf)
{
  this->do_construct(file, transform);
}

//Subclasses only should call this constructor!
PolyMesh::PolyMesh(BRDF * brdf) : Object(brdf) {
	return;
}


//Read a vertices and triangles from compatible file
void PolyMesh::do_construct(char *file, Transform *transform)
{

	std::cout << file << std::endl;

	ifstream inputFile;
	inputFile.open(file);

	string currentInput;
	inputFile >> currentInput;

	if(currentInput != "kcply"){
		cout << "WARNING: Code may not support your file, expected to see 'kcply' on the first line." << endl;
	}

	inputFile >> currentInput;

	int iVertex = 0;
	int iTriangle = 0;

	float x, y, z;

	while(!inputFile.eof()){
		//Check for element lines
		if(currentInput == "element"){
			//read what kind of element it is
			inputFile >> currentInput;

			if(currentInput == "vertex"){
				//Get vertex count and allocate memory to array
				inputFile >> currentInput;
				this->vertex_count = stoi(currentInput);
				this->vertex = new Vertex[vertex_count];
			}else if(currentInput == "face"){
				//Get triangle count and allocate memory to array
				inputFile >> currentInput;
				this->triangle_count = stoi(currentInput);
				this->triangle = new TriangleIndex[triangle_count];
			}
		}else{
			//Reading vertex data
			if(iVertex < vertex_count){

				x = stof(currentInput);
				inputFile >> y;
				inputFile >> z;

				vertex[iVertex] = Vertex(x, y, z);
				transform->apply(vertex[iVertex]);


				iVertex++;
			}else if(iTriangle < triangle_count){


				int v;
				for(int i = 0; i < 3; i++){
					inputFile >> v;
					//For files with indices starting at 1:
					triangle[iTriangle][i] = v-1;

					//For files with indices starting at 0:
					//triangle[iTriangle][i] = v;
				}

				iTriangle++;
			}
		}


		inputFile >> currentInput;
	}

	inputFile.close();
}

//Test intersection with one triangle
void PolyMesh::triangle_intersection(Ray ray, TriangleIndex tri, Hit &hit){

	hit.flag = false;

	Vertex a = vertex[tri[0]];
	Vertex b = vertex[tri[1]];
	Vertex c = vertex[tri[2]];


	//Plane intersection test
	//Calculate normal
	Vector ab = Vector(a, b, true);
	Vector bc = Vector(b, c, true);
	Vector ca = Vector(c, a, true);
	Vector normal;
	ca.cross(ab, normal);
	normal.normalise();

	float nd = normal.dot(ray.direction);

	if(nd == 0){
		return; //No intersection found
	}

	//Use normal and a point on the plane to solve for d
	Vector v_a = Vector(a.x, a.y, a.z);
	float d = -1* normal.dot(v_a);

	Vector r = Vector(ray.position.x, ray.position.y, ray.position.z); //Convert vertex to vector
	float nr = normal.dot(r);

	//Ray intersects plane, but need to see if it is within the bounds of the triangle.
	float t = -1 * (nr + d)/nd;
	if(t <= 0) return;

	float x = ray.position.x + t * ray.direction.x;
	float y = ray.position.y + t * ray.direction.y;
	float z = ray.position.z + t * ray.direction.z;
	Vertex p = Vertex(x,y,z);

	Vector dirFromAB;
	Vector ap = Vector(a, p, true);
	ap.cross(ab, dirFromAB);

	Vector dirFromBC;
	Vector bp = Vector(b, p, true);
	bp.cross(bc, dirFromBC);

	Vector dirFromCA;
	Vector cp = Vector(c, p, true);
	cp.cross(ca, dirFromCA);

	bool inside = dirFromAB.dot(dirFromBC) > 0 && dirFromBC.dot(dirFromCA) > 0 && dirFromCA.dot(dirFromAB) > 0;

	if(inside){
		//Store hit information
		hit.t = t;
		hit.what = this;
		hit.position.x = p.x;
		hit.position.y = p.y;
		hit.position.z = p.z;
		hit.normal.x = normal.x;
		hit.normal.y = normal.y;
		hit.normal.z = normal.z;

		hit.flag = true;
	}




}


//Test intersection with each triangle
void PolyMesh::intersection(Ray ray, Hit &hit){

	//Try to dynamically cast bounding sphere - if fails, have to test all triangles every iteration (much slower!)
	Sphere * bound = dynamic_cast<Sphere*>(boundingSphere);
	if(bound != NULL){
		Hit boundSphereHit;
		bound->intersection(ray, boundSphereHit);
		if(!boundSphereHit.flag){
			hit.flag = false;
			return;
		}
	}

	float closestT = 9999999999999999;
	hit.flag = false;
	Hit tempHit;

	for(int tri = 0; tri < triangle_count; tri++){
		triangle_intersection(ray, triangle[tri], tempHit);

		if(tempHit.flag && tempHit.t < closestT){
			//Only store triangle with closest intersection
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


