#ifndef __LVL_DISPLAYING_HPP__
#define __LVL_DISPLAYING_HPP__

#include <stdint.h>
#include <GL/glew.h>
#include "data_types.hpp"
#include  "tools/MatrixStack.hpp"

void display_lvl2(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, GLint NbIntersectionLocation, GLint NormSumLocation, uint32_t nbIntersectionMax, uint32_t nbVertices, VoxelData* voxArray, glm::dvec3 pos, uint16_t nbSub, double cubeSize);

void display_lvl1(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, glm::dvec3 pos, double cubeSize);

#endif
