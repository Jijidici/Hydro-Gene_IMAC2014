#ifndef __CUBE_MODEL_HPP__
#define __CUBE_MODEL_HPP__

#define POSITION_LOCATION 0
#define TEXCOORDS_LOCATION 2

GLuint CreateCubeVBO();
GLuint CreateCubeVAO(GLuint vbo);
GLuint CreateTexture(const char* path);
GLuint CreateCubeMap();
void BindTexture(GLuint texture, GLenum unity);
void BindCubeMap(GLuint texture, GLenum unity);

#endif

