#include "voxel_maker/intersection_test.hpp"

#include <stdint.h>
#include <glm/glm.hpp>
#include "geom_types.hpp"
#include "voxel_maker/geometrics.hpp"

/* Calculation of intersection between a vertex of the face and the voxel */
bool processIntersectionVertexVoxel(Vertex* v, Voxel vox, double threshold){
	/* if the center of the voxel is inside a bounding sphere with a radius of threshold, turn it on */
	glm::dvec3 vertexVoxCenter = createVector(v->pos, vox.c);
	if(vecNorm(vertexVoxCenter) < threshold){
		return true;
	}
	return false;
}

/* Calculation of intersection between an edge of the face and the voxel */
bool processIntersectionEdgeVoxel(Vertex* v1, Vertex* v2, Edge edg, Voxel vox, double threshold){
	/* Projection of the voxel center on the edge */
	glm::dvec3 edgeDiff = createVector(v1->pos, vox.c);
	float t = glm::dot(edgeDiff, edg.dir);
	/* If the projected isn't on the segment */
	if(t<0. || t>1.){
		return false;
	}
	glm::dvec3 voxCProjected = glm::dvec3(v1->pos.x + t*edg.dir.x, v1->pos.y + t*edg.dir.y, v1->pos.z + t*edg.dir.z);

	/* if the center of the voxel is inside a bounding cylinder with a radius of threshold, turn it on */
	if(vecNorm(createVector(vox.c, voxCProjected)) < threshold){
		return true;
	}
	return false;
}

/* Calculation of intersections between the main plane and the voxel */
bool processIntersectionMainPlaneVoxel(Plane G, Plane H, Voxel currentVoxel){

	/* Test if the center of the voxel is between the two plane */
	double cRelativityG = relativePositionVertexFace(G, currentVoxel.c);
	double cRelativityH = relativePositionVertexFace(H, currentVoxel.c);

	//std::cout<<"//-> rel G : "<<cRelativityG<<" || rel H : "<<cRelativityH<<std::endl;

	/* If it's the case, the voxel center is in the bounding plane */
	if((cRelativityG <=0 && cRelativityH <=0) || (cRelativityG >=0 && cRelativityH >=0)){
		return true;
	}

	return false;
}

/* Calculate if the voxel center is in the Ei prism */
bool processIntersectionOtherPlanesVoxel(Plane e1, Plane e2, Plane e3, Voxel currentVoxel){

	/* Test if the center of the voxel is on the same side of the tree plan */
	double cRelativityE1 = relativePositionVertexFace(e1, currentVoxel.c);
	double cRelativityE2 = relativePositionVertexFace(e2, currentVoxel.c);
	double cRelativityE3 = relativePositionVertexFace(e3, currentVoxel.c);

	//std::cout<<"//-> rel E1 : "<<cRelativityE1<<" || rel E2 : "<<cRelativityE2<<" || rel E3 : "<<cRelativityE3<<std::endl;

	/* If it's the case, the voxel center is in the Ei prism */
	if((cRelativityE1 <=0 && cRelativityE2 <=0 && cRelativityE3 <=0) || (cRelativityE1 >=0 && cRelativityE2 >=0 && cRelativityE3 >=0)){
		return true;
	}

	return false;
}

/* Main calculation of the intersection between the face and a voxel */
bool processIntersectionPolygonVoxel(Face testedFace, Edge edg1, Edge edg2, Edge edg3, Plane H, Plane G, Plane E1, Plane E2, Plane E3, Voxel currentVoxel, double threshold, uint32_t mode){
	/* vertex Bounding sphere radius and edge bounding cylinder radius */

	if((mode == 1)||(mode==0)){
		/* Vertices tests */
		if(processIntersectionVertexVoxel(testedFace.s1, currentVoxel, threshold)){ return true;}
		if(processIntersectionVertexVoxel(testedFace.s2, currentVoxel, threshold)){ return true;}
		if(processIntersectionVertexVoxel(testedFace.s3, currentVoxel, threshold)){ return true;}
	}
	if((mode ==2)||(mode==0)){
		/* Edges tests */
		if(processIntersectionEdgeVoxel(testedFace.s1, testedFace.s2, edg1, currentVoxel, threshold)){return true;}
		if(processIntersectionEdgeVoxel(testedFace.s1, testedFace.s3, edg2, currentVoxel, threshold)){return true;}
		if(processIntersectionEdgeVoxel(testedFace.s2, testedFace.s3, edg3, currentVoxel, threshold)){return true;}
	}
	if((mode == 3)||(mode==0)){
		/* Face test */
		if(processIntersectionMainPlaneVoxel(H, G, currentVoxel) && processIntersectionOtherPlanesVoxel(E1, E2, E3, currentVoxel)){
			return true;
		}
	}
	return false;
}