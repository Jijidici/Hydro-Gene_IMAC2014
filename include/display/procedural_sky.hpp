#ifndef __PROCEDURAL_SKY_HPP__
#define __PROCEDURAL_SKY_HPP__

#include <GL/glew.h>

static const size_t SKYTEX_SIZE = 1024;

/* Create FBO */
GLuint createFBO();

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint texID, GLuint skyProgram, GLuint quadVAO);

#endif
