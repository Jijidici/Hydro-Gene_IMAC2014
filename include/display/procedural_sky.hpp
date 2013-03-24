#ifndef __PROCEDURAL_SKY_HPP__
#define __PROCEDURAL_SKY_HPP__

#include <GL/glew.h>

static const size_t SKYTEX_SIZE = 1024;
static const size_t NB_SKYLOCATIONS = 3;

enum SkyLocation{
	PLAN_OR,
	PLAN_U,
	PLAN_V
};

/* Create FBO */
GLuint createFBO();

/* Initialize the sky location */
void getSkyLocation(GLint* skyLocations, GLuint skyProgram);

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint texID, GLuint skyProgram, GLuint quadVAO, GLint* skyLocations);

#endif
