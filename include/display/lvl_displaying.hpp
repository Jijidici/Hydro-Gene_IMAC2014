#ifndef __LVL_DISPLAYING_HPP__
#define __LVL_DISPLAYING_HPP__

#include <stdint.h>
#include <GL/glew.h>
#include "data_types.hpp"
#include "tools/MatrixStack.hpp"
#include "cameras/FreeFlyCamera.hpp"
#include "display_types.hpp"


using namespace hydrogene;

/* ENUM */
static const size_t NB_LOCATIONS = 27;

enum Locations{
	MVP,
	MODELVIEW,
	INV_VIEWMATRIX,
	LIGHTSUN,
	TIME,
	DAY,
	NIGHT,
	MODE,
	CHOICE,
	FOG,
	SKYTEX,
	GRASSTEX,
	WATERTEX,
	STONETEX,
	SNOWTEX,
	SANDTEX,
	ROCKTEX,
	PLANTTEX,
	TREETEX,
	PINETREETEX,
	SNOWTREETEX,
	MAXBENDING,
	MAXDRAIN,
	MAXGRADIENT,
	MAXSURFACE,
	MAXALTITUDE,
	DISTANCE	
};

void getLocations(GLint* locations, GLuint program);

void sendUniforms(GLint* locations, float* maxCoeffArray, float thresholdDistance);

void display_lvl2(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, GLint NbIntersectionLocation, GLint NormSumLocation, uint32_t nbIntersectionMax, uint32_t nbVertices, VoxelData* voxArray, Leaf& l, uint16_t nbSub, double cubeSize, FreeFlyCamera& ffCam, CamType camType);

void display_lvl1(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, glm::dvec3 pos, double halfLeafSize);

void display_triangle(GLuint meshVAO, MatrixStack& ms, GLuint MVPLocation, uint32_t nbVertices, GLuint* textures);

void display_vegetation(GLuint meshVAO, MatrixStack& ms, GLuint MVPLocation, uint32_t nbVertices, GLint ChoiceLocation, GLuint* textures);

bool frustumTest(Leaf& l, uint32_t i, uint32_t j, uint32_t k, double cubeSize, FreeFlyCamera& ffCam);


/******** Time management *********/
bool timePauseTrigger(bool timePause, float* timeStep);


#endif
