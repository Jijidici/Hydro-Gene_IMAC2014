#include "display/lvl_displaying.hpp"

#include <stdint.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "data_types.hpp"
#include "tools/MatrixStack.hpp"
#include "cameras/FreeFlyCamera.hpp"

void display_lvl2(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, GLint NbIntersectionLocation, GLint NormSumLocation, uint32_t nbIntersectionMax, uint32_t nbVertices, VoxelData* voxArray, Leaf& l, uint16_t nbSub, double cubeSize, FreeFlyCamera& ffCam){
	for(uint32_t k=0;k<nbSub;++k){
		for(uint32_t j=0;j<nbSub;++j){
			for(uint32_t i=0;i<nbSub;++i){
				uint32_t currentIndex = k*nbSub*nbSub + j*nbSub + i;
				uint32_t currentNbIntersection = voxArray[k*nbSub*nbSub + j*nbSub + i].nbFaces;
				if(currentNbIntersection != 0){
					if(frustumTest(l, i, j, k, cubeSize, ffCam)){
						ms.push();
							ms.translate(glm::vec3(i*cubeSize + l.pos.x, j*cubeSize + l.pos.y, k*cubeSize + l.pos.z)); //PLACEMENT OF EACH GRID CUBE
							ms.scale(glm::vec3(cubeSize));// RE-SCALE EACH GRID CUBE
						
							glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
							glUniform2i(NbIntersectionLocation, currentNbIntersection, nbIntersectionMax);
							glUniform3f(NormSumLocation, voxArray[currentIndex].sumNormal.x, voxArray[currentIndex].sumNormal.y, voxArray[currentIndex].sumNormal.z);
							glBindVertexArray(cubeVAO);
								glDrawArrays(GL_TRIANGLES, 0, nbVertices);
							glBindVertexArray(0);
						ms.pop();
					}
				}
			}
		}
	}
}

void display_lvl1(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, Leaf& l, double cubeSize){
	double halfCubeSize = 0.5*cubeSize;
	ms.push();
		ms.translate(glm::vec3(l.pos.x + halfCubeSize, l.pos.y + halfCubeSize, l.pos.z + halfCubeSize)); //PLACEMENT OF CUBE
		ms.scale(glm::vec3(cubeSize));// RE-SCALE EACH GRID CUBE
	
		glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
	
		glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	ms.pop();
}


bool frustumTest(Leaf& l, uint32_t i, uint32_t j, uint32_t k, double cubeSize, FreeFlyCamera& ffCam){
	//~ bool result = true;
	//~ int in = 0;
	//~ int out = 0;
	
	//~ if(ffCam.m_frustumNearPlaneNormal * (l.pos - ffCam.m_frustumNearPlanePoint) < 0){
	if(glm::dot(ffCam.m_frustumRightPlaneNormal, (glm::vec3(l.pos.x + i*cubeSize + cubeSize/2., l.pos.y + j*cubeSize + cubeSize/2., l.pos.z + k*cubeSize + cubeSize/2.) - ffCam.m_frustumRightPlanePoint)) < 0){
		return false;
	}
	if(glm::dot(ffCam.m_frustumLeftPlaneNormal, (glm::vec3(l.pos.x + i*cubeSize + cubeSize/2., l.pos.y + j*cubeSize + cubeSize/2., l.pos.z + k*cubeSize + cubeSize/2.) - ffCam.m_frustumLeftPlanePoint)) < 0){
		return false;
	}
	if(glm::dot(ffCam.m_frustumTopPlaneNormal, (glm::vec3(l.pos.x + i*cubeSize + cubeSize/2., l.pos.y + j*cubeSize + cubeSize/2., l.pos.z + k*cubeSize + cubeSize/2.) - ffCam.m_frustumTopPlanePoint)) < 0){
		return false;
	}
	if(glm::dot(ffCam.m_frustumBottomPlaneNormal, (glm::vec3(l.pos.x + i*cubeSize + cubeSize/2., l.pos.y + j*cubeSize + cubeSize/2., l.pos.z + k*cubeSize + cubeSize/2.) - ffCam.m_frustumBottomPlanePoint)) < 0){
		return false;
	}
	
	return true;
}
