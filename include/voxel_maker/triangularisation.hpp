#ifndef __TRIANGULARISATION_HPP__
#define __TRIANGULARISATION_HPP__

#include <vector>
#include <glm/glm.hpp>
#include <stdint.h>
#include "geom_types.hpp"
#include "data_types.hpp"

/* Cube Face intersection */
glm::dvec3 triangleCubefaceIntersection(glm::dvec3 optimal_current, glm::dvec3 optimal_compared, uint16_t face, glm::dvec3 position_current, double leafSize);

/* Compute optimal point */
Vertex computeOptimalPoint(Leaf& l, std::vector<Vertex>& l_storedVertices);

#endif
