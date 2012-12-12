#include "geom_types.hpp"

#include <glm/glm.hpp>

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

Edge createEdge(glm::dvec3 inS1, glm::dvec3 inS2){
	Edge newEdge;
	newEdge.dir = createVector(inS1, inS2);
	newEdge.length = glm::length(newEdge.dir);
	return newEdge;
}