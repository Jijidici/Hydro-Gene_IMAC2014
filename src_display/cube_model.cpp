#include <iostream>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/glew.h>
#include <stdint.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "display/cube_model.hpp"
#include "geom_types.hpp"
#include "data_types.hpp"

#include "tools/MatrixStack.hpp"

#include "drn/drn_reader.h"

GLuint CreateCubeVBO(){

	//Creation of the cube
	Cube aCube = createCube(-0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f);
	
	GLdouble cubeVertices[] = {

		//near
		aCube.left, aCube.bottom, aCube.near,
		0.01f, 0.01f,
		aCube.right, aCube.bottom, aCube.near,
		0.16f, 0.01f,
		aCube.left, aCube.top, aCube.near,
		0.01f, 0.99f,

		aCube.right, aCube.bottom, aCube.near,
		0.16f, 0.01f,
		aCube.right, aCube.top, aCube.near,
		0.16f, 0.99f,
		aCube.left, aCube.top, aCube.near,
		0.01f, 0.99f,

		//right
		aCube.right, aCube.bottom, aCube.near,
		0.16f, 0.01f,
		aCube.right, aCube.bottom, aCube.far,
		0.33f, 0.01f,
		aCube.right, aCube.top, aCube.near,
		0.16f, 0.99f,

		aCube.right, aCube.bottom, aCube.far,
		0.33f, 0.01f,
		aCube.right, aCube.top, aCube.far,
		0.33f, 0.99f,
		aCube.right, aCube.top, aCube.near,
		0.16f, 0.99f,

		//left
		aCube.left, aCube.bottom, aCube.near,
		0.33f, 0.01f,
		aCube.left, aCube.bottom, aCube.far,
		0.5f, 0.01f,
		aCube.left, aCube.top, aCube.near,
		0.33f, 0.99f,

		aCube.left, aCube.bottom, aCube.far,
		0.5f, 0.01f,
		aCube.left, aCube.top, aCube.far,
		0.5f, 0.99f,
		aCube.left, aCube.top, aCube.near,
		0.33f, 0.99f,

		//far
		aCube.left, aCube.bottom, aCube.far,
		0.5f, 0.01f,
		aCube.right, aCube.bottom, aCube.far,
		0.66f, 0.01f,
		aCube.left, aCube.top, aCube.far,
		0.5f, 0.99f,

		aCube.right, aCube.bottom, aCube.far,
		0.66f, 0.01f,
		aCube.right, aCube.top, aCube.far,
		0.66f, 0.99f,
		aCube.left, aCube.top, aCube.far,
		0.5f, 0.99f,

		//down
		aCube.left, aCube.bottom, aCube.near,
		0.66f, 0.01f,
		aCube.right, aCube.bottom, aCube.near,
		0.83f, 0.01f,
		aCube.left, aCube.bottom, aCube.far,
		0.66f, 0.99f,

		aCube.right, aCube.bottom, aCube.near,
		0.83f, 0.01f,
		aCube.right, aCube.bottom, aCube.far,
		0.83f, 0.99f,
		aCube.left, aCube.bottom, aCube.far,
		0.66f, 0.99f,


		//up
		aCube.left, aCube.top, aCube.near,
		0.83f, 0.01f,
		aCube.right, aCube.top, aCube.near,
		0.99f, 0.01f,
		aCube.left, aCube.top, aCube.far,
		0.83f, 0.99f,

		aCube.right, aCube.top, aCube.near,
		0.99f, 0.01f,
		aCube.right, aCube.top, aCube.far,
		0.99f, 0.99f,
		aCube.left, aCube.top, aCube.far,
		0.83f, 0.99f,
	};

	GLuint cubeVBO = 0;
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return cubeVBO;

}

GLuint CreateCubeVAO(GLuint vbo){

	GLuint cubeVAO = 0;
	glGenVertexArrays(1, &cubeVAO);  
	glBindVertexArray(cubeVAO);  
		glEnableVertexAttribArray(POSITION_LOCATION);
		glEnableVertexAttribArray(TEXCOORDS_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glVertexAttribPointer(POSITION_LOCATION, 3, GL_DOUBLE, GL_FALSE, 5*sizeof(GLdouble), reinterpret_cast<const GLvoid*>(0));
			glVertexAttribPointer(TEXCOORDS_LOCATION, 2, GL_DOUBLE, GL_FALSE, 5*sizeof(GLdouble), reinterpret_cast<const GLvoid*>(3*sizeof(GLdouble)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return cubeVAO;

}

GLuint CreateTexture(const char* path){

	GLuint texture;
	GLuint format;

	SDL_Surface* image = IMG_Load(path);
	if(!image){
		std::cout << "Unable to load the image : " << path << std::endl;
		exit(EXIT_FAILURE);
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

    if(image->format->BytesPerPixel == 1){
			format = GL_RED;
		}
		if(image->format->BytesPerPixel == 3){
			format = GL_RGB;
		}
		if(image->format->BytesPerPixel == 4){
			format = GL_RGBA;
		}

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		image->w,
		image->h,
		0,
		format,
		GL_UNSIGNED_BYTE,
		image->pixels
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	SDL_FreeSurface(image);

	return texture;
}

void BindTexture(GLuint texture){

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

}

