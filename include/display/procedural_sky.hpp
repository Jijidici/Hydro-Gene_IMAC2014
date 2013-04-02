#ifndef __PROCEDURAL_SKY_HPP__
#define __PROCEDURAL_SKY_HPP__

#include <GL/glew.h>
#include <glm/glm.hpp>

static const size_t SKYTEX_SIZE = 1024;
static const size_t ENVMAP_SIZE = 256;
static const size_t NB_SKYLOCATIONS = 7;

enum SkyLocation{
	PLAN_OR,
	PLAN_U,
	PLAN_V,
	SUN_POS,
	SKY_TIME,
	SKY_TEX,
	IS_SKYBOX
};

/* Create FBO */
GLuint createFBO();

/* Initialize the sky location */
void getSkyLocation(GLint* skyLocations, GLuint skyProgram);

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint skyboxTexID, GLuint envmapTexID, GLuint skyProgram, GLuint quadVAO, glm::vec3 sunPos, float time, GLint* skyLocations);

#endif
