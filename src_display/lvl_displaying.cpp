#include "display/lvl_displaying.hpp"

#include <stdint.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "data_types.hpp"
#include "tools/MatrixStack.hpp"
#include "cameras/FreeFlyCamera.hpp"
#include "display/cube_model.hpp"

#define TRIANGLES 1
#define NORMAL 2
#define BENDING 3
#define DRAIN 4
#define GRADIENT 5
#define SURFACE 6
#define VEGET 7

void display_lvl2(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, GLint NbIntersectionLocation, GLint NormSumLocation, uint32_t nbIntersectionMax, uint32_t nbVertices, VoxelData* voxArray, Leaf& l, uint16_t nbSub, double cubeSize, FreeFlyCamera& ffCam, CamType camType){
	for(uint32_t k=0;k<nbSub;++k){
		for(uint32_t j=0;j<nbSub;++j){
			for(uint32_t i=0;i<nbSub;++i){
				uint32_t currentIndex = k*nbSub*nbSub + j*nbSub + i;
				uint32_t currentNbIntersection = voxArray[k*nbSub*nbSub + j*nbSub + i].nbFaces;
				if(currentNbIntersection != 0){
					if(camType == FREE_FLY){
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
					}else{
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

void display_lvl1(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, glm::dvec3 pos, double halfLeafSize){
	ms.push();
		ms.translate(glm::vec3(pos.x + halfLeafSize, pos.y + halfLeafSize, pos.z + halfLeafSize)); //PLACEMENT OF THE LEAF
		ms.scale(glm::vec3(2*halfLeafSize));// RE-SCALE EACH GRID CUBE
	
		glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
	
		glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	ms.pop();
}

void display_triangle(GLuint meshVAO, MatrixStack& ms, GLuint MVPLocation, uint32_t nbVertices){
	glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
	
	glBindVertexArray(meshVAO);
		glDrawArrays(GL_TRIANGLES, 0, nbVertices);
	glBindVertexArray(0);
}

void display_vegetation(GLuint meshVAO, MatrixStack& ms, GLuint MVPLocation, uint32_t nbVertices, GLint ChoiceLocation, GLint TextureLocation, GLuint texture){
	glUniform1i(ChoiceLocation, VEGET);
	glUniform1i(TextureLocation,0);
	BindTexture(texture, GL_TEXTURE0);
		glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
	
		glBindVertexArray(meshVAO);
			glDrawArrays(GL_TRIANGLES, 0, nbVertices);
		glBindVertexArray(0);
	BindTexture(0, GL_TEXTURE0);
}

bool frustumTest(Leaf& l, uint32_t i, uint32_t j, uint32_t k, double cubeSize, FreeFlyCamera& ffCam){
	
	if(glm::dot(ffCam.m_frustumNearPlaneNormal, (ffCam.m_frustumNearPlanePoint - glm::vec3(l.pos.x + i*cubeSize + cubeSize/2., l.pos.y + j*cubeSize + cubeSize/2., l.pos.z + k*cubeSize + cubeSize/2.))) < 0.){
		return false;
	}
	if(glm::dot(ffCam.m_frustumRightPlaneNormal, (glm::vec3(l.pos.x + i*cubeSize + cubeSize/2., l.pos.y + j*cubeSize + cubeSize/2., l.pos.z + k*cubeSize + cubeSize/2.) - ffCam.m_frustumRightPlanePoint)) < 0.){
		return false;
	}
	if(glm::dot(ffCam.m_frustumLeftPlaneNormal, (glm::vec3(l.pos.x + i*cubeSize + cubeSize/2., l.pos.y + j*cubeSize + cubeSize/2., l.pos.z + k*cubeSize + cubeSize/2.) - ffCam.m_frustumLeftPlanePoint)) < 0.){
		return false;
	}
	if(glm::dot(ffCam.m_frustumTopPlaneNormal, (glm::vec3(l.pos.x + i*cubeSize + cubeSize/2., l.pos.y + j*cubeSize + cubeSize/2., l.pos.z + k*cubeSize + cubeSize/2.) - ffCam.m_frustumTopPlanePoint)) < 0.){
		return false;
	}
	if(glm::dot(ffCam.m_frustumBottomPlaneNormal, (glm::vec3(l.pos.x + i*cubeSize + cubeSize/2., l.pos.y + j*cubeSize + cubeSize/2., l.pos.z + k*cubeSize + cubeSize/2.) - ffCam.m_frustumBottomPlanePoint)) < 0.){
		return false;
	}
	
	return true;
}
