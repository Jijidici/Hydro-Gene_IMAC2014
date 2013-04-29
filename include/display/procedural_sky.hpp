#ifndef __PROCEDURAL_SKY_HPP__
#define __PROCEDURAL_SKY_HPP__

#include <GL/glew.h>
#include <glm/glm.hpp>

static const size_t SKYTEX_SIZE = 512;
static const size_t ENVMAP_SIZE = 128;
static const size_t CLOUDTEX_SIZE = 1024;
static const size_t NB_SKYLOCATIONS = 12;

enum SkyLocation{
	PLAN_OR,
	PLAN_U,
	PLAN_V,
	SUN_POS,
	MOON_POS,
	SKY_TIME,
	SKY_TEX,
	ENVMAP_TEX,
	MOON_TEX,
	CLOUDS_TEX,
	SAMPLE_STEP,
	IS_SKYBOX,
	IS_INITIAL_BLUR
};

/* Create FBO */
GLuint createFBO();

/* Initialize the sky location */
void getSkyLocation(GLint* skyLocations, GLuint skyProgram);

/* Fill the clouds 2D texture with Simplex noise */
void paintClouds(GLuint skyFboID, GLuint cloudsTexID, GLuint cloudsProgram, GLuint quadVAO);

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint skyboxTexID, GLuint envmapTexID_main, GLuint envmapTexID_tmp, GLuint moonTexID, GLuint cloudsTexID, GLuint skyProgram, GLuint quadVAO, glm::vec3 sunPos, glm::vec3 moonPos, float time, GLint* skyLocations);

#endif
