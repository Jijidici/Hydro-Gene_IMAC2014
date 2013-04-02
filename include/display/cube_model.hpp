#ifndef __CUBE_MODEL_HPP__
#define __CUBE_MODEL_HPP__

#define POSITION_LOCATION 0
#define TEXCOORDS_LOCATION 2

#include <stdint.h>

GLuint CreateCubeVBO();
GLuint CreateCubeVAO(GLuint vbo);
GLuint CreateQuadVBO();
GLuint CreateQuadVAO(GLuint vbo);

GLuint CreateTexture(const char* path);
GLuint CreateCubeMap(uint16_t size);
void BindTexture(GLuint texture, GLenum unity);
void BindCubeMap(GLuint texture, GLenum unity);

#endif

