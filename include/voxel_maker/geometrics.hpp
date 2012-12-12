#ifndef __GEOMETRICS__HPP
#define __GEOMETRICS__HPP

#include <glm/glm.hpp>
#include "geom_types.hpp"

/* Face Position Min/Max */
double getmaxX(Face testedFace);
double getminX(Face testedFace);
double getmaxY(Face testedFace);
double getminY(Face testedFace);
double getmaxZ(Face testedFace);
double getminZ(Face testedFace);

/* Determine if a point is in front of or behind a Face | >0 = in front of | <0 = behind | ==0 = on */
double relativePositionVertexFace(Plane p, glm::dvec3 vx);

double vecNorm(glm::dvec3 v);

#endif