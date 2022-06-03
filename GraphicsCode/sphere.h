/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2019.
 *
 * Do what you like with this code as long as you retain this comment.
 */

#pragma once

#include "vertex.h"
#include "object.h"

class Sphere : public Object {
public:
	Vertex center;
	float radius;
	Sphere(Vertex c, float r, BRDF * brdf);
	void intersection(Ray ray, Hit &hit);
};
