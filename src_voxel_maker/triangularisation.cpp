#include "voxel_maker/triangularisation.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <stdint.h>
#include <Eigen/SVD>
#include "geom_types.hpp"
#include "data_types.hpp"

/* Cube Face intersection */
glm::dvec3 triangleCubefaceIntersection(glm::dvec3 optimal_current, glm::dvec3 optimal_compared, uint16_t face, glm::dvec3 position_current, double leafSize){

	double t = 0;
	glm::dvec3 normal;

	switch(face){

		case 0 : //right cube face
			normal = glm::dvec3(1.f, 0.f, 0.f);
			if(glm::dot(normal,optimal_current) != 0)	t = (position_current.x+leafSize - optimal_current.x)/(optimal_compared.x - optimal_current.x);
			break;

		case 1 : //near cube face
			normal = glm::dvec3(0.f,0.f,1.f);
			if(glm::dot(normal,optimal_current) != 0) t = (position_current.y+leafSize - optimal_current.y)/(optimal_compared.y - optimal_current.y);
			break;

		case 2 : //top cube face
			normal = glm::dvec3(0.f,1.f,0.f);
			if(glm::dot(normal,optimal_current) != 0) t = (position_current.z+leafSize - optimal_current.z)/(optimal_compared.z - optimal_current.z);
			break;

	}

	double x_inter = optimal_current.x + t*(optimal_compared.x - optimal_current.x);
	double y_inter = optimal_current.y + t*(optimal_compared.y - optimal_current.y);
	double z_inter = optimal_current.z + t*(optimal_compared.z - optimal_current.z);

	return glm::dvec3(x_inter,y_inter,z_inter);
}

/* Compute optimal point */
Vertex computeOptimalPoint(Leaf& l, std::vector<Vertex>& l_storedVertices){
	/* compute leaf's corners */
	glm::dvec3 vert0 = l.pos;
	glm::dvec3 vert1 = glm::dvec3(vert0.x + l.size, vert0.y, vert0.z);
	glm::dvec3 vert2 = glm::dvec3(vert0.x + l.size, vert0.y + l.size, vert0.z);
	glm::dvec3 vert3 = glm::dvec3(vert0.x, vert0.y + l.size, vert0.z);
	glm::dvec3 vert4 = glm::dvec3(vert0.x, vert0.y, vert0.z + l.size);
	glm::dvec3 vert5 = glm::dvec3(vert0.x + l.size, vert0.y, vert0.z + l.size);
	glm::dvec3 vert6 = glm::dvec3(vert0.x + l.size, vert0.y + l.size, vert0.z + l.size);
	glm::dvec3 vert7 = glm::dvec3(vert0.x, vert0.y + l.size, vert0.z + l.size);
	
	/* compute leaf's edges */
	Edge e0 = createEdge(vert0, vert1);
	Edge e1 = createEdge(vert1, vert2);
	Edge e2 = createEdge(vert2, vert3);
	Edge e3 = createEdge(vert3, vert0);
	Edge e4 = createEdge(vert4, vert5);
	Edge e5 = createEdge(vert5, vert6);
	Edge e6 = createEdge(vert6, vert7);
	Edge e7 = createEdge(vert7, vert4);
	Edge e8 = createEdge(vert0, vert4);
	Edge e9 = createEdge(vert1, vert5);
	Edge e10 = createEdge(vert2, vert6);
	Edge e11 = createEdge(vert3, vert7);
	
	std::vector<Edge> leafEdges;
	leafEdges.push_back(e0);
	leafEdges.push_back(e1);
	leafEdges.push_back(e2);
	leafEdges.push_back(e3);
	leafEdges.push_back(e4);
	leafEdges.push_back(e5);
	leafEdges.push_back(e6);
	leafEdges.push_back(e7);
	leafEdges.push_back(e8);
	leafEdges.push_back(e9);
	leafEdges.push_back(e10);
	leafEdges.push_back(e11);
	
	
	/* Intersection triangle - edge */
	/* browse edges */
	
	std::vector<Vertex> intersectionPoints;
	
	/* average datas of the leaf */
	glm::dvec3 averageNormal(0., 0., 0.);
	float averageBending = 0.;
	float averageDrain = 0.;
	float averageGradient = 0.;
	float averageSurface = 0.;
	
	//~ std::cout << "---- LEAF ----" << std::endl;
	for(std::vector<Edge>::iterator e_it = leafEdges.begin(); e_it < leafEdges.end(); ++e_it){
		std::vector<Vertex> edgeIntPoints;
		bool edgeIntersected = false;
		
		/* browse saved triangles */
		//~ std::cout << "Edge : " << std::endl; 
		//~ std::cout << "Origin : " << (*e_it).origin.x << " " << (*e_it).origin.y << " " << (*e_it).origin.z << std::endl;
		//~ std::cout << "Dir : " << (*e_it).dir.x << " " << (*e_it).dir.y << " " << (*e_it).dir.z << std::endl;
		for(size_t i = 0; i < l_storedVertices.size(); i += 3){
			Face tempFace; tempFace.s1 = &l_storedVertices[i]; tempFace.s2 = &l_storedVertices[i+1]; tempFace.s3 = &l_storedVertices[i+2];
			/*** test intersection edge - tempFace ***/
				
			glm::dvec3 u = createVector(tempFace.s1->pos, tempFace.s2->pos);
			glm::dvec3 v = createVector(tempFace.s1->pos, tempFace.s3->pos);
			
			glm::dvec3 normale = glm::normalize(glm::cross(v, u));
			
			averageNormal += (tempFace.s1->normal + tempFace.s2->normal + tempFace.s3->normal);
			averageBending += (tempFace.s1->bending + tempFace.s2->bending + tempFace.s3->bending);
			averageDrain += (tempFace.s1->drain + tempFace.s2->drain + tempFace.s3->drain);
			averageGradient += (tempFace.s1->gradient + tempFace.s2->gradient + tempFace.s3->gradient);
			averageSurface += (tempFace.s1->surface + tempFace.s2->surface + tempFace.s3->surface);
			
			/* intersection point */								
			glm::dvec3 intPoint;
			if((*e_it).computeIntersectionPoint(u, v, tempFace.s1->pos, intPoint)){
				Vertex intVertex;
				intVertex.pos = intPoint;
				intVertex.normal = normale;
				
				edgeIntPoints.push_back(intVertex);
				
				edgeIntersected = true;
			}
		}
		
		averageNormal /= l_storedVertices.size();
		averageBending /= l_storedVertices.size();
		averageDrain /= l_storedVertices.size();
		averageGradient /= l_storedVertices.size();
		averageSurface /= l_storedVertices.size();
		
		if(edgeIntersected){
			/* average of the faces intersecting the edge */
			Vertex averageVertex;
			
			glm::dvec3 averagePos(0.,0.,0.);
			glm::dvec3 averageNorm(0.,0.,0.);

			for(unsigned int i = 0; i < edgeIntPoints.size(); ++i){
				averagePos += edgeIntPoints[i].pos;
				averageNorm += edgeIntPoints[i].normal;
			}
			
			averagePos /= edgeIntPoints.size();
			averageNorm /= edgeIntPoints.size();
			//~ std::cout << "average Pos : " << averagePos.x << " " << averagePos.y << " " << averagePos.z << std::endl;
			//~ std::cout << "average Norm : " << averageNorm.x << " " << averageNorm.y << " " << averageNorm.z << std::endl;

			averageVertex.pos = averagePos;
			averageVertex.normal = averageNorm;
			
			/* add the average intersection point to the vector */
			intersectionPoints.push_back(averageVertex);
		}
	}
	
	if(intersectionPoints.size() != 0){
	
		/* "mass-point" */
		glm::dvec3 massPoint(0., 0., 0.);
		for(unsigned int i = 0; i < intersectionPoints.size(); ++i){
			massPoint += intersectionPoints[i].pos;
		}
		massPoint /= intersectionPoints.size();
		
		/* Compute the "optimalPoint" (Eigen) */
		Eigen::MatrixXd MatA = Eigen::MatrixXd::Zero(intersectionPoints.size(), 3);
		Eigen::VectorXd VecB = Eigen::VectorXd::Zero(intersectionPoints.size());
		
		for(unsigned int i = 0; i < intersectionPoints.size(); ++i){
			MatA(i, 0) = intersectionPoints[i].normal.x;
			MatA(i, 1) = intersectionPoints[i].normal.y;
			MatA(i, 2) = intersectionPoints[i].normal.z;
			
			VecB(i) = (double)glm::dot(intersectionPoints[i].pos - massPoint, intersectionPoints[i].normal);
		}
		
		Eigen::JacobiSVD<Eigen::MatrixXd> MatSVD(MatA, Eigen::ComputeThinU | Eigen::ComputeThinV);
		Eigen::VectorXd eigenOptimalPoint = MatSVD.solve(VecB);
		
		glm::dvec3 dvec_optimalPoint(eigenOptimalPoint(0), eigenOptimalPoint(1), eigenOptimalPoint(2));
		dvec_optimalPoint += massPoint;
		
		Vertex optimalPoint;
		optimalPoint.pos = dvec_optimalPoint;
		optimalPoint.normal = averageNormal;
		optimalPoint.bending = averageBending;
		optimalPoint.drain = averageDrain;
		optimalPoint.gradient = averageGradient;
		optimalPoint.surface = averageSurface;
		
		//~ std::cout << std::endl << "optimal point : " << optimalPoint.pos.x << " " << optimalPoint.pos.y << " " << optimalPoint.pos.z << std::endl;
		
		//cap the average Point
		if(optimalPoint.pos.x < l.pos.x){ optimalPoint.pos.x = l.pos.x; }
		if(optimalPoint.pos.y < l.pos.y){ optimalPoint.pos.y = l.pos.y; }
		if(optimalPoint.pos.z < l.pos.z){ optimalPoint.pos.z = l.pos.z; }
		if(optimalPoint.pos.x > l.pos.x + l.size){ optimalPoint.pos.x = l.pos.x + l.size; }
		if(optimalPoint.pos.y > l.pos.y + l.size){ optimalPoint.pos.y = l.pos.y + l.size; }
		if(optimalPoint.pos.z > l.pos.z + l.size){ optimalPoint.pos.z = l.pos.z + l.size; }
		
		return optimalPoint;

		//~ std::cout << std::endl << "average point : " << optimalPoint.pos.x << " " << optimalPoint.pos.y << " " << optimalPoint.pos.z << std::endl;
		
	}else{
		//~ std::cout << std::endl << "--- No edge intersection ---" << std::endl;
		glm::dvec3 dvec_optimalPoint((l.pos.x + l.size)/2., (l.pos.y + l.size)/2., (l.pos.z + l.size)/2.);
		
		Vertex optimalPoint;
		optimalPoint.pos = dvec_optimalPoint;
		optimalPoint.normal = averageNormal;
		optimalPoint.bending = averageBending;
		optimalPoint.drain = averageDrain;
		optimalPoint.gradient = averageGradient;
		optimalPoint.surface = averageSurface;
		
		return optimalPoint;
		//~ std::cout << std::endl << "average point : " << optimalPoint.pos.x << " " << optimalPoint.pos.y << " " << optimalPoint.pos.z << std::endl;
	}
	return createVertex(glm::dvec3(0., 0., 0.), glm::dvec3(0., 0., 0.), 0.f, 0.f, 0.f, 0.f);
}
