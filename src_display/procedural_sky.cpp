#include "display/procedural_sky.hpp"

#include <stdexcept>
#include <GL/glew.h>

/* Create FBO */
GLuint createFBO(){
	GLuint fboID = 0;
	glGenFramebuffers(1, &fboID);
	return fboID;
}

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint texID, GLuint skyProgram, GLuint quadVAO){
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
	
	for(uint32_t i=0;i<5;++i){
		//Attach the top of the skybox cubemap
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, types[i], texID, 0);
		//check the FBO status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE){
			throw std::runtime_error("sky framebuffer isn't complete");
		}
		
		//Clear the drawing zone
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Draw the quad
		glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
