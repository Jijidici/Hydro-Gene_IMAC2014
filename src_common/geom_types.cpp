#include <iostream>

#include "geom_types.hpp"
#include "voxel_maker/geometrics.hpp"

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
	newEdge.origin = inS1;
	return newEdge;
}

bool Edge::faceIntersectionTest(Face &face){
	if(dir.x != 0){
		double minY = face.s1->pos.y;
		double maxY = face.s1->pos.y;
		
		if(face.s2->pos.y < minY) minY = face.s2->pos.y;
		if(face.s3->pos.y < minY) minY = face.s3->pos.y;
		if(face.s2->pos.y > maxY) maxY = face.s3->pos.y;
		if(face.s3->pos.y > maxY) maxY = face.s3->pos.y;
		
		if(minY <= origin.y && maxY > origin.y){
			double minZ = face.s1->pos.z;
			double maxZ = face.s1->pos.z;
			
			if(face.s2->pos.z < minZ) minZ = face.s2->pos.z;
			if(face.s3->pos.z < minZ) minZ = face.s3->pos.z;
			if(face.s2->pos.z > maxZ) maxZ = face.s3->pos.z;
			if(face.s3->pos.z > maxZ) maxZ = face.s3->pos.z;
			
			if(minZ <= origin.z && maxZ > origin.z){
				return true;
			}
		}
	}
	if(dir.y != 0){
		double minX = face.s1->pos.x;
		double maxX = face.s1->pos.x;
		
		if(face.s2->pos.x < minX) minX = face.s2->pos.x;
		if(face.s3->pos.x < minX) minX = face.s3->pos.x;
		if(face.s2->pos.x > maxX) maxX = face.s3->pos.x;
		if(face.s3->pos.x > maxX) maxX = face.s3->pos.x;
		
		if(minX <= origin.x && maxX > origin.x){
			double minZ = face.s1->pos.z;
			double maxZ = face.s1->pos.z;
			
			if(face.s2->pos.z < minZ) minZ = face.s2->pos.z;
			if(face.s3->pos.z < minZ) minZ = face.s3->pos.z;
			if(face.s2->pos.z > maxZ) maxZ = face.s3->pos.z;
			if(face.s3->pos.z > maxZ) maxZ = face.s3->pos.z;
			
			if(minZ <= origin.z && maxZ > origin.z){
				return true;
			}
		}
	}
	if(dir.z != 0){
		double minY = face.s1->pos.y;
		double maxY = face.s1->pos.y;
		
		if(face.s2->pos.y < minY) minY = face.s2->pos.y;
		if(face.s3->pos.y < minY) minY = face.s3->pos.y;
		if(face.s2->pos.y > maxY) maxY = face.s3->pos.y;
		if(face.s3->pos.y > maxY) maxY = face.s3->pos.y;
		
		if(minY <= origin.y && maxY > origin.y){
			double minX = face.s1->pos.x;
			double maxX = face.s1->pos.x;
			
			if(face.s2->pos.x < minX) minX = face.s2->pos.x;
			if(face.s3->pos.x < minX) minX = face.s3->pos.x;
			if(face.s2->pos.x > maxX) maxX = face.s3->pos.x;
			if(face.s3->pos.x > maxX) maxX = face.s3->pos.x;
			
			if(minX <= origin.x && maxX > origin.x){
				return true;
			}
		}
	}
	
	return false;
}
