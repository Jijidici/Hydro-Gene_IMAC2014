#ifndef __PROCEDURAL_SKY_HPP__
#define __PROCEDURAL_SKY_HPP__

#include <GL/glew.h>
#include <glm/glm.hpp>

static const size_t SKYTEX_SIZE = 512;
static const size_t ENVMAP_SIZE = 128;
static const size_t NB_SKYLOCATIONS = 11;

enum SkyLocation{
	PLAN_OR,
	PLAN_U,
	PLAN_V,
	SUN_POS,
	MOON_POS,
	SKY_TIME,
	ENVMAP_TEX,
	SKY_TEX,
	SAMPLE_STEP,
	IS_SKYBOX,
	IS_INITIAL_BLUR
};

/* Create FBO */
GLuint createFBO();

/* Initialize the sky location */
void getSkyLocation(GLint* skyLocations, GLuint skyProgram);

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint skyboxTexID, GLuint envmapTexID_main, GLuint envmapTexID_tmp, GLuint skyProgram, GLuint quadVAO, glm::vec3 sunPos, glm::vec3 moonPos, float time, GLint* skyLocations);

#endif
