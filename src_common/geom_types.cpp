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


bool Edge::computeIntersectionPoint(glm::dvec3& u, glm::dvec3& v, glm::dvec3& s1, glm::dvec3& result){
	if(u.x == 0 || u.y == 0 || u.z == 0 || v.x == 0 || v.y == 0 || v.z == 0){
		return false;
	}
	
	if(dir.x != 0){
		glm::dvec2 row1(u.y, u.z);
		glm::dvec2 row2(v.y, v.z);
		glm::dvec2 rowSol(origin.y - s1.y, origin.z - s1.z);
		
		/* Gauss algorithm */
		row2.y = row2.y - row2.x*(row1.y/row1.x);
		rowSol.y = rowSol.y - rowSol.x*(row1.y/row1.x);
		
		double b = rowSol.y / row2.y;
		double a = (rowSol.x - b*row2.x)/row1.x;
		
		if(a < 0 || b < 0 || a + b > 1){
			return false;
		}
		
		result = (s1 + a*u + b*v);
		
		return true;
	}
	
	if(dir.y != 0){
		glm::dvec2 row1(u.x, u.z);
		glm::dvec2 row2(v.x, v.z);
		glm::dvec2 rowSol(origin.x - s1.x, origin.z - s1.z);
		
		/* Gauss algorithm */
		row2.y = row2.y - row2.x*(row1.y/row1.x);
		rowSol.y = rowSol.y - rowSol.x*(row1.y/row1.x);
		
		double b = rowSol.y / row2.y;
		double a = (rowSol.x - b*row2.x)/row1.x;
		
		if(a < 0 || b < 0 || a + b > 1){
			return false;
		}
		
		result = (s1 + a*u + b*v);
		
		return true;
	}
	
	if(dir.z != 0){
		glm::dvec2 row1(u.x, u.y);
		glm::dvec2 row2(v.x, v.y);
		glm::dvec2 rowSol(origin.x - s1.x, origin.y - s1.y);
		
		/* Gauss algorithm */
		row2.y = row2.y - row2.x*(row1.y/row1.x);
		rowSol.y = rowSol.y - rowSol.x*(row1.y/row1.x);
		
		double b = rowSol.y / row2.y;
		double a = (rowSol.x - b*row2.x)/row1.x;
		
		if(a < 0 || b < 0 || a + b > 1){
			return false;
		}
		
		result = (s1 + a*u + b*v);
		
		return true;
	}
	
	return false;
}


