#include <iostream>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdint.h>
#include <string.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "display/cube_model.hpp"
#include "geom_types.hpp"
#include "data_types.hpp"

#include "tools/MatrixStack.hpp"

#include "drn/drn_reader.h"

static const size_t SKYTEX_SIZE = 1024;

GLuint CreateCubeVBO(){

	//Creation of the cube
	Cube aCube = createCube(-0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f);
	
	GLdouble cubeVertices[] = {

		//left
		aCube.left, aCube.top, aCube.far,
		aCube.left, aCube.bottom, aCube.far,
		aCube.left, aCube.bottom, aCube.near,

		aCube.left, aCube.bottom, aCube.near,
		aCube.left, aCube.top, aCube.near,
		aCube.left, aCube.top, aCube.far,

		//top
		aCube.right, aCube.top, aCube.far,
		aCube.left, aCube.top, aCube.far,
		aCube.left, aCube.top, aCube.near,

		aCube.left, aCube.top, aCube.near,
		aCube.right, aCube.top, aCube.near,
		aCube.right, aCube.top, aCube.far,

		//far
		aCube.left, aCube.top, aCube.far,
		aCube.right, aCube.top, aCube.far,
		aCube.right, aCube.bottom, aCube.far,

		aCube.right, aCube.bottom, aCube.far,
		aCube.left, aCube.bottom, aCube.far,
		aCube.left, aCube.top, aCube.far,

		//bottom
		aCube.left, aCube.bottom, aCube.far,
		aCube.right, aCube.bottom, aCube.far,
		aCube.right, aCube.bottom, aCube.near,

		aCube.right, aCube.bottom, aCube.near,
		aCube.left, aCube.bottom, aCube.near,
		aCube.left, aCube.bottom, aCube.far,

		//right
		aCube.right, aCube.bottom, aCube.far,
		aCube.right, aCube.top, aCube.far,
		aCube.right, aCube.top, aCube.near,

		aCube.right, aCube.top, aCube.near,
		aCube.right, aCube.bottom, aCube.near,
		aCube.right, aCube.bottom, aCube.far,

		//near
		aCube.left, aCube.bottom, aCube.near,
		aCube.right, aCube.bottom, aCube.near,
		aCube.right, aCube.top, aCube.near,

		aCube.right, aCube.top, aCube.near,
		aCube.left, aCube.top, aCube.near,
		aCube.left, aCube.bottom, aCube.near
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
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glVertexAttribPointer(POSITION_LOCATION, 3, GL_DOUBLE, GL_FALSE, 3*sizeof(GLdouble), reinterpret_cast<const GLvoid*>(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return cubeVAO;

}

GLuint CreateQuadVBO(){
	//creation of the quad
	GLfloat vertices[] = {
		 -1.f, -1.f,
		  1.f, -1.f,
		  1.f,  1.f,
		  1.f,  1.f,
		 -1.f,  1.f,
		 -1.f, -1.f
	};
	
	GLuint quadVBO = 0;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	return quadVBO;
}

GLuint CreateQuadVAO(GLuint vbo){
	GLuint quadVAO = 0;
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
		glEnableVertexAttribArray(POSITION_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glVertexAttribPointer(POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	return quadVAO;
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
	
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	SDL_FreeSurface(image);

	return texture;
}


GLuint CreateCubeMap(){
	GLuint cubeMap;
	glGenTextures(1, &cubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
	
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, SKYTEX_SIZE, SKYTEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, SKYTEX_SIZE, SKYTEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, SKYTEX_SIZE, SKYTEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, SKYTEX_SIZE, SKYTEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, SKYTEX_SIZE, SKYTEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, SKYTEX_SIZE, SKYTEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
	return cubeMap;
}

void BindTexture(GLuint texture, GLenum unity){
	glActiveTexture(unity);
	glBindTexture(GL_TEXTURE_2D, texture);

}


void BindCubeMap(GLuint texture, GLenum unity){
	glActiveTexture(unity);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
}

