#ifndef CM30075_CODE_STARTING_POINT_PHYSICS_H_
#define CM30075_CODE_STARTING_POINT_PHYSICS_H_

#include "vector.h"
#include "object.h"

//Helper functions for reflect/refract
Vector* refract(Vector incident, Vector normal, Object * inside, Object * target, float *fresReflection, float *fresRefraction);
Vector reflect(Vector incident, Vector normal);

#endif
