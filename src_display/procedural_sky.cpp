#include "display/procedural_sky.hpp"

#include <iostream>
#include <stdexcept>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "display/cube_model.hpp"

static const size_t WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720;
static const size_t BLUR_PRECISION = 9; //Must be impar and greater than 1 (like 2*blur+1, with blur != 0)

/* Create FBO */
GLuint createFBO(){
	GLuint fboID = 0;
	glGenFramebuffers(1, &fboID);
	return fboID;
}

/* Initialize the sky location */
void getSkyLocation(GLint* skyLocations, GLuint skyProgram){
	skyLocations[PLAN_OR] = glGetUniformLocation(skyProgram, "uPlanOr");
	skyLocations[PLAN_U] = glGetUniformLocation(skyProgram, "uPlanU");
	skyLocations[PLAN_V] = glGetUniformLocation(skyProgram, "uPlanV");
	skyLocations[SUN_POS] = glGetUniformLocation(skyProgram, "uSunPos");
	skyLocations[MOON_POS] = glGetUniformLocation(skyProgram, "uMoonPos");
	skyLocations[SKY_TIME] = glGetUniformLocation(skyProgram, "uTime");
	skyLocations[SKY_TEX] = glGetUniformLocation(skyProgram, "uSkyTex");
	skyLocations[ENVMAP_TEX] = glGetUniformLocation(skyProgram, "uEnvmapTex");
	skyLocations[SAMPLE_STEP] = glGetUniformLocation(skyProgram, "uSampleStep");
	skyLocations[IS_SKYBOX] = glGetUniformLocation(skyProgram, "uIsSkybox");
	skyLocations[IS_INITIAL_BLUR] = glGetUniformLocation(skyProgram, "uIsInitialBlur");
}

/* Test for dynamique texturing the sky */
void paintTheSky(GLuint skyFboID, GLuint skyboxTexID, GLuint envmapTexID_main, GLuint envmapTexID_tmp, GLuint skyProgram, GLuint quadVAO, glm::vec3 sunPos, glm::vec3 moonPos, float time, GLint* skyLocations){
	glUseProgram(skyProgram);
	
	//send uniforms
	glUniform3fv(skyLocations[SUN_POS], 1, glm::value_ptr(sunPos));
	glUniform3fv(skyLocations[MOON_POS], 1, glm::value_ptr(moonPos));
	glUniform1f(skyLocations[SKY_TIME], time);
	glUniform1i(skyLocations[IS_SKYBOX], 1);
	
	//Define cube properties
	GLenum types[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	
	glm::vec3 origins[] = {
		glm::vec3(0.5, 0.5, 0.5),
		glm::vec3(-0.5, 0.5, -0.5),
		glm::vec3(-0.5, 0.5, -0.5),
		glm::vec3(-0.5, 0.5, 0.5),
		glm::vec3(0.5, 0.5, -0.5)
	};
	
	glm::vec3 planU[] = {
		glm::vec3(0., 0., -1.),
		glm::vec3(0., 0., 1.),
		glm::vec3(1., 0., 0.),
		glm::vec3(1., 0., 0.),
		glm::vec3(-1., 0., 0.)
	};
	
	glm::vec3 planV[] = {
		glm::vec3(0., -1., 0.),
		glm::vec3(0., -1., 0.),
		glm::vec3(0., 0., 1.),
		glm::vec3(0., -1., 0.),
		glm::vec3(0., -1., 0.),
	};
	
	//DRAW THE SKYBOX
	glViewport(0, 0, SKYTEX_SIZE, SKYTEX_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, skyFboID);		
		//for each planes of the cubemap
		for(uint8_t i=0;i<5;++i){
			//Attach the face of the skybox cubemap
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, types[i], skyboxTexID, 0);
			//check the FBO status
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if(status != GL_FRAMEBUFFER_COMPLETE){
				throw std::runtime_error("sky framebuffer isn't complete");
			}
			
			/* send uniforms */
			glUniform3fv(skyLocations[PLAN_OR], 1, glm::value_ptr(origins[i]));
			glUniform3fv(skyLocations[PLAN_U], 1, glm::value_ptr(planU[i]));
			glUniform3fv(skyLocations[PLAN_V], 1, glm::value_ptr(planV[i]));
			
			//Clear the drawing zone
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			//Draw the quad
			glBindVertexArray(quadVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
			
			//detach the skybox face
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, types[i], 0, 0);
		}
	
	
		//DRAW THE ENVMAP
		glViewport(0, 0, ENVMAP_SIZE, ENVMAP_SIZE);	
		glUniform1i(skyLocations[IS_SKYBOX], 0);
		glUniform1f(skyLocations[SAMPLE_STEP], 5*(1./(float)ENVMAP_SIZE));
		glUniform1i(skyLocations[SKY_TEX], 0);
		glUniform1i(skyLocations[ENVMAP_TEX], 1);
		
		//Use skybox as base for the blur
		BindCubeMap(skyboxTexID, GL_TEXTURE0);
		
		//for each blur iterations
		for(uint8_t blur=0;blur<BLUR_PRECISION;++blur){
			//determine the written envmap
			GLuint writtenEnvMap = 0;
			if(blur <= 0){
				//case of initial blur - just copy the skybox with a lower resolution
				glUniform1i(skyLocations[IS_INITIAL_BLUR], 1);
				writtenEnvMap = envmapTexID_main;
			}else{
				glUniform1i(skyLocations[IS_INITIAL_BLUR], 0);
				//determine which blur step we are
				if(blur%2 == 1){
					writtenEnvMap = envmapTexID_tmp;
					BindCubeMap(envmapTexID_main, GL_TEXTURE1);
				}else{
					writtenEnvMap = envmapTexID_main;
					BindCubeMap(envmapTexID_tmp, GL_TEXTURE1);
				}
			}
		
			//for each planes of the cubemap
			for(uint8_t i=0;i<5;++i){
				//Attache the envmap face
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, types[i], writtenEnvMap, 0);
				//check the FBO status
				GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				if(status != GL_FRAMEBUFFER_COMPLETE){
					throw std::runtime_error("envmap framebuffer isn't complete");
				}
				
				/* send uniforms */
				glUniform3fv(skyLocations[PLAN_OR], 1, glm::value_ptr(origins[i]));
				glUniform3fv(skyLocations[PLAN_U], 1, glm::value_ptr(planU[i]));
				glUniform3fv(skyLocations[PLAN_V], 1, glm::value_ptr(planV[i]));
				
				//Clear the drawing zone
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
				//Draw the quad
				glBindVertexArray(quadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);
				
				//dettach the envmap face
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, types[i], 0, 0);
			}
		}
		
		BindCubeMap(0, GL_TEXTURE1);
		BindCubeMap(0, GL_TEXTURE0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}
