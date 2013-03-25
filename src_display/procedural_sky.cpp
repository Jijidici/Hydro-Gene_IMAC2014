#include "display/procedural_sky.hpp"

#include <stdexcept>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/* Create FBO */
GLuint createFBO(){
	GLuint fboID = 0;
	glGenFramebuffers(1, &fboID);
	return fboID;
}

/* Initialize the sky location */
void getSkyLocation(GLint* skyLocations, GLuint skyProgram){
	skyLocations[PLAN_OR] = glGetUniformLocation(skyProgram, "uPlanOr");
	skyLocations[PLAN_U] = glGetUniformLocation(skyProgram, "uPlanU");
	skyLocations[PLAN_V] = glGetUniformLocation(skyProgram, "uPlanV");
}

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint texID, GLuint skyProgram, GLuint quadVAO, GLint* skyLocations){
	glUseProgram(skyProgram);
	
	glBindFramebuffer(GL_FRAMEBUFFER, skyFboID);
	glViewport(0, 0, SKYTEX_SIZE, SKYTEX_SIZE);
	
	GLenum types[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	
	glm::vec3 origins[] = {
		glm::vec3(0.5, 0.5, 0.5),
		glm::vec3(-0.5, 0.5, -0.5),
		glm::vec3(-0.5, 0.5, -0.5),
		glm::vec3(-0.5, 0.5, 0.5),
		glm::vec3(0.5, 0.5, -0.5)
	};
	
	glm::vec3 planU[] = {
		glm::vec3(0., 0., -1.),
		glm::vec3(0., 0., 1.),
		glm::vec3(1., 0., 0.),
		glm::vec3(1., 0., 0.),
		glm::vec3(-1., 0., 0.)
	};
	
	glm::vec3 planV[] = {
		glm::vec3(0., -1., 0.),
		glm::vec3(0., -1., 0.),
		glm::vec3(0., 0., 1.),
		glm::vec3(0., -1., 0.),
		glm::vec3(0., -1., 0.),
	};
	
	for(uint32_t i=0;i<5;++i){
		//Attach the top of the skybox cubemap
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, types[i], texID, 0);
		//check the FBO status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE){
			throw std::runtime_error("sky framebuffer isn't complete");
		}
		
		/* send uniforms */
		glUniform3fv(skyLocations[PLAN_OR], 1, glm::value_ptr(origins[i]));
		glUniform3fv(skyLocations[PLAN_U], 1, glm::value_ptr(planU[i]));
		glUniform3fv(skyLocations[PLAN_V], 1, glm::value_ptr(planV[i]));
		
		//Clear the drawing zone
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Draw the quad
		glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
