#ifndef __TYPE_HPP__
#define __TYPE_HPP__

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

/******************************************/
/*          FUNCTIONS                     */
/******************************************/

Voxel createVoxel(double inX, double inY, double inZ, double inSize){
	Voxel newVoxel;
	newVoxel.c.x = inX;
	newVoxel.c.y = inY;
	newVoxel.c.z = inZ;
	newVoxel.size = inSize;
	return newVoxel;
}

Cube createCube(double inLeft, double inRight, double inTop, double inBottom, double inFar, double inNear){
	Cube newCube;
	newCube.left = inLeft;
	newCube.right = inRight;
	newCube.top = inTop;
	newCube.bottom = inBottom;
	newCube.far = inFar;
	newCube.near = inNear;
	newCube.nbVertices = 36;
	
	return newCube;
}

glm::dvec3 createVector(glm::dvec3 begin, glm::dvec3 end){
	return glm::dvec3(end.x - begin.x, end.y - begin.y, end.z - begin.z);
}

Plane createPlane(glm::dvec3 inS1, glm::dvec3 inS2, glm::dvec3 inS3){
	Plane newPlane;
	newPlane.normal = glm::cross(createVector(inS1, inS2), createVector(inS1, inS3));
	newPlane.s1.pos = inS1;
	newPlane.s2.pos = inS2;
	newPlane.s3.pos = inS3;
	return newPlane;
}

#endif
