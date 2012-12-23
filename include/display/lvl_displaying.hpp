#ifndef __LVL_DISPLAYING__
#define __LVL_DISPLAYING__

#include <stdint.h>
#include <GL/glew.h>
#include "data_types.hpp"
#include  "tools/MatrixStack.hpp"

void display_lvl2(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, GLint NbIntersectionLocation, GLint NormSumLocation, uint32_t nbIntersectionMax, uint32_t nbVertices, VoxelData* voxArray, Leaf& l, uint16_t nbSub, double cubeSize);

void display_lvl1(GLuint cubeVAO, MatrixStack& ms, GLuint MVPLocation, Leaf& l, double cubeSize);

#endif
