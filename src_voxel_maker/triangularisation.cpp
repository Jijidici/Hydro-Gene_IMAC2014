#include "voxel_maker/triangularisation.hpp"

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
