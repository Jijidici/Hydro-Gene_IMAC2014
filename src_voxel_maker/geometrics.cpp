#include "voxel_maker/geometrics.hpp"

#include <glm/glm.hpp>
#include "geom_types.hpp"

/* Face Position Min/Max */
double getmaxX(Face testedFace){
	double maxX = testedFace.s1->pos.x;
	if(testedFace.s2->pos.x > maxX) maxX = testedFace.s2->pos.x;
	if(testedFace.s3->pos.x > maxX) maxX = testedFace.s3->pos.x;
	
	return maxX;
}

double getminX(Face testedFace){
	double minX = testedFace.s1->pos.x;
	if(testedFace.s2->pos.x < minX) minX = testedFace.s2->pos.x;
	if(testedFace.s3->pos.x < minX) minX = testedFace.s3->pos.x;
	
	return minX;
}

double getmaxY(Face testedFace){
	double maxY = testedFace.s1->pos.y;
	if(testedFace.s2->pos.y > maxY) maxY = testedFace.s2->pos.y;
	if(testedFace.s3->pos.y > maxY) maxY = testedFace.s3->pos.y;
	
	return maxY;
}

double getminY(Face testedFace){
	double minY = testedFace.s1->pos.y;
	if(testedFace.s2->pos.y < minY) minY = testedFace.s2->pos.y;
	if(testedFace.s3->pos.y < minY) minY = testedFace.s3->pos.y;
	
	return minY;
}

double getmaxZ(Face testedFace){
	double maxZ = testedFace.s1->pos.z;
	if(testedFace.s2->pos.z > maxZ) maxZ = testedFace.s2->pos.z;
	if(testedFace.s3->pos.z > maxZ) maxZ = testedFace.s3->pos.z;
	
	return maxZ;
}

double getminZ(Face testedFace){
	double minZ = testedFace.s1->pos.z;
	if(testedFace.s2->pos.z < minZ) minZ = testedFace.s2->pos.z;
	if(testedFace.s3->pos.z < minZ) minZ = testedFace.s3->pos.z;
	
	return minZ;
}

/* Determine if a point is in front of or behind a Face | >0 = in front of | <0 = behind | ==0 = on */
double relativePositionVertexFace(Plane p, glm::dvec3 vx){
	glm::dvec3 referentVector = createVector(p.s1.pos, vx);
	return glm::dot(referentVector, p.normal);
}

double vecNorm(glm::dvec3 v){
	return glm::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}