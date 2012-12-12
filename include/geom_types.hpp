#ifndef __GEOM_TYPES_HPP__
#define __GEOM_TYPES_HPP__

#include <stdint.h>
#include <glm/glm.hpp>

typedef struct s_vertex{
	glm::dvec3 pos;
}Vertex;

typedef struct s_face{
	Vertex *s1, *s2, *s3;
	glm::dvec3 normal;
	double bending;
	double gradient;
	double surface;
	int drain;
}Face;

typedef struct s_cube{
	double left;
	double right;
	double top;
	double bottom;
	double far;
	double near;
	uint8_t nbVertices;
}Cube;

typedef struct s_voxel{
	glm::dvec3 c;
	double size;
}Voxel;

typedef struct s_voxelData{ //voxels dans tabVoxel
	glm::dvec3 sumNormal;
	double sumBending;
	double sumGradient;
	double sumSurface;
	int sumDrain;
	uint32_t nbFaces;
}VoxelData;

typedef struct s_plane{
	Vertex s1, s2, s3;
	glm::dvec3 normal;
}Plane;

typedef struct s_edge{
	glm::dvec3 dir;
	double length;
}Edge;

/******************************************/
/*          FUNCTIONS                     */
/******************************************/

Voxel createVoxel(double inX, double inY, double inZ, double inSize);
Cube createCube(double inLeft, double inRight, double inTop, double inBottom, double inFar, double inNear);
glm::dvec3 createVector(glm::dvec3 begin, glm::dvec3 end);
Plane createPlane(glm::dvec3 inS1, glm::dvec3 inS2, glm::dvec3 inS3);
Edge createEdge(glm::dvec3 inS1, glm::dvec3 inS2);

#endif
