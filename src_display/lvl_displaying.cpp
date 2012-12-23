#include "display/lvl_displaying.hpp"

#include <stdint.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "data_types.hpp"
#include  "tools/MatrixStack.hpp"

void display_lvl2(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, GLint NbIntersectionLocation, GLint NormSumLocation, uint32_t nbIntersectionMax, VoxelData* voxArray, glm::dvec3 pos, uint16_t nbSub, double cubeSize){
	for(uint32_t k=0;k<nbSub;++k){
		for(uint32_t j=0;j<nbSub;++j){
			for(uint32_t i=0;i<nbSub;++i){
				uint32_t currentIndex = k*nbSub*nbSub + j*nbSub + i;
				uint32_t currentNbIntersection = voxArray[k*nbSub*nbSub + j*nbSub + i].nbFaces;
				if(currentNbIntersection != 0){
					ms.push();
						ms.translate(glm::vec3(i*cubeSize + pos.x, j*cubeSize + pos.y, k*cubeSize + pos.z)); //PLACEMENT OF EACH GRID CUBE
						ms.scale(glm::vec3(cubeSize));// RE-SCALE EACH GRID CUBE
					
						glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
						glUniform2i(NbIntersectionLocation, currentNbIntersection, nbIntersectionMax);
						glUniform3f(NormSumLocation, voxArray[currentIndex].sumNormal.x, voxArray[currentIndex].sumNormal.y, voxArray[currentIndex].sumNormal.z);
					
						glBindVertexArray(cubeVAO);
							glDrawArrays(GL_TRIANGLES, 0, 36);
						glBindVertexArray(0);
					ms.pop();
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
