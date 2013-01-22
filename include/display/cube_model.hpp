#ifndef __CUBE_MODEL_HPP__
#define __CUBE_MODEL_HPP__

#define POSITION_LOCATION 0
#define TEXCOORDS_LOCATION 2

GLuint CreateCubeVBO();
GLuint CreateCubeVAO(GLuint vbo);
GLuint CreateTexture(const char* path);
void BindTexture(GLuint texture);

#endif

