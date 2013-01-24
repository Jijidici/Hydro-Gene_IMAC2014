#ifndef __TRIANGULARISATION_HPP__
#define __TRIANGULARISATION_HPP__

#include <glm/glm.hpp>
#include <stdint.h>

/* Cube Face intersection */
glm::dvec3 triangleCubefaceIntersection(glm::dvec3 optimal_current, glm::dvec3 optimal_compared, uint16_t face, glm::dvec3 position_current, double leafSize);


#endif
