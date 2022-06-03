/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2019.
 *
 * Do what you like with this code as long as you retain this comment.
 */

#pragma once

#include "vertex.h"
#include "transform.h"
#include "ray.h"
#include "hit.h"
#include "object.h"

typedef int TriangleIndex[3];

class PolyMesh : public Object {
public:
	int vertex_count;
	int triangle_count;
        Vertex *vertex;
	TriangleIndex *triangle;

	void do_construct(char *file, Transform *transform);
	void triangle_intersection(Ray ray, TriangleIndex tri, Hit &hit);
	void intersection(Ray ray, Hit &hit);
	
	PolyMesh(char *file, BRDF * brdf);
	PolyMesh(char *file, Transform *transform, BRDF * brdf);
	PolyMesh(BRDF * brdf);
};
