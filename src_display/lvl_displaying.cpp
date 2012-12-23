#include "display/lvl_displaying.hpp"

#include <stdint.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "data_types.hpp"
#include "tools/MatrixStack.hpp"

void display_lvl2(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, GLint NbIntersectionLocation, GLint NormSumLocation, uint32_t nbIntersectionMax, uint32_t nbVertices, VoxelData* voxArray, Leaf& l, uint16_t nbSub, double cubeSize){
	for(uint32_t k=0;k<nbSub;++k){
		for(uint32_t j=0;j<nbSub;++j){
			for(uint32_t i=0;i<nbSub;++i){
				uint32_t currentIndex = k*nbSub*nbSub + j*nbSub + i;
				uint32_t currentNbIntersection = voxArray[k*nbSub*nbSub + j*nbSub + i].nbFaces;
				if(frustumTest(l, i, j, k, cubeSize)){
					if(currentNbIntersection != 0){
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


bool frustumTest(Leaf& l, uint32_t i, uint32_t j, uint32_t k, double cubeSize){
	bool result = true;
	//~ if(l.pos.x + (float)i*cubeSize <= 0.8){
		//~ result = false;
	//~ }
	
	return result;
}
