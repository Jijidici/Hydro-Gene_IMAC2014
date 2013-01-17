#ifndef __CUBE_MODEL_HPP__
#define __CUBE_MODEL_HPP__

static const size_t POSITION_LOCATION = 0;
static const size_t TEXCOORDS_LOCATION = 2;

GLuint CreateCubeVBO();
GLuint CreateCubeVAO(GLuint vbo);
GLuint CreateTexture(const char* path);
void BindTexture(GLuint texture);

#endif

