#include "display/procedural_sky.hpp"

#include <GL/glew.h>

/* Create FBO */
GLuint createFBO(){
	GLuint fboID = 0;
	glGenFramebuffers(1, &fboID);
	return fboID;
}

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint texID, GLuint skyProgram, GLuint quadVAO){
}
