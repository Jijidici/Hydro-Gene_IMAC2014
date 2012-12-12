#ifndef __INTERSECTION_TEST_HPP__
#define __INTERSECTION_TEST_HPP__

#include <stdint.h>
#include "geom_types.hpp"

/* Calculation of intersection between a vertex of the face and the voxel */
bool processIntersectionVertexVoxel(Vertex* v, Voxel vox, double threshold);

/* Calculation of intersection between an edge of the face and the voxel */
bool processIntersectionEdgeVoxel(Vertex* v1, Vertex* v2, Edge edg, Voxel vox, double threshold);

/* Calculation of intersections between the main plane and the voxel */
bool processIntersectionMainPlaneVoxel(Plane G, Plane H, Voxel currentVoxel);

/* Calculate if the voxel center is in the Ei prism */
bool processIntersectionOtherPlanesVoxel(Plane e1, Plane e2, Plane e3, Voxel currentVoxel);

/* Main calculation of the intersection between the face and a voxel */
bool processIntersectionPolygonVoxel(Face testedFace, Edge edg1, Edge edg2, Edge edg3, Plane H, Plane G, Plane E1, Plane E2, Plane E3, Voxel currentVoxel, double threshold, uint32_t mode);

#endif