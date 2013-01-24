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

/* Solve system with SVD */
glm::dvec3 useSVD(std::vector<Vertex>& vertices);

/* Build Triangularized triangle */
void buildTriangles(std::vector< std::vector<Vertex> >& l_computedVertices, Leaf* leafArray, uint32_t nbSub);

#endif
