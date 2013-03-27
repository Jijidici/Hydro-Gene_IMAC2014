#ifndef __PROCEDURAL_SKY_HPP__
#define __PROCEDURAL_SKY_HPP__

#include <GL/glew.h>
#include <glm/glm.hpp>

static const size_t SKYTEX_SIZE = 1024;
static const size_t NB_SKYLOCATIONS = 5;

enum SkyLocation{
	PLAN_OR,
	PLAN_U,
	PLAN_V,
	SUN_POS,
	SKY_TIME
};

/* Create FBO */
GLuint createFBO();

/* Initialize the sky location */
void getSkyLocation(GLint* skyLocations, GLuint skyProgram);

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint texID, GLuint skyProgram, GLuint quadVAO, glm::vec3 sunPos, float time, GLint* skyLocations);

#endif
