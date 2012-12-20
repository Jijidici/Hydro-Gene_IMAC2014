#include <iostream>
#include <cstdlib>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <stdint.h>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "geom_types.hpp"
#include "data_types.hpp"
#include "display_types.hpp"
#include "tools/shader_tools.hpp"
#include "tools/MatrixStack.hpp"
#include "cameras/TrackBallCamera.hpp"
#include "cameras/FreeFlyCamera.hpp"

#include "drn/drn_reader.h"

#define FRAME_RATE 60

static const Uint32 MIN_LOOP_TIME = 1000/FRAME_RATE;
static const size_t WINDOW_WIDTH = 600, WINDOW_HEIGHT = 600;
static const size_t BYTES_PER_PIXEL = 32;
static const size_t POSITION_LOCATION = 0;
static const size_t GRID_3D_SIZE = 2;

void resetShaderProgram(GLuint &program, GLint &MVPLocation, GLint &NbIntersectionLocation, GLint &NormSumLocation, GLint &LightVectLocation){
	glUseProgram(program);
	
	// Creation des Matrices
	MVPLocation = glGetUniformLocation(program, "uMVPMatrix");
	
	// Recuperation des variables uniformes
	NbIntersectionLocation = glGetUniformLocation(program, "uNbIntersection");
	NormSumLocation = glGetUniformLocation(program, "uNormSum");
	LightVectLocation = glGetUniformLocation(program, "uLightVect");
}

int main(int argc, char** argv){
	
	// OPEN AND READ THE VOXEL-INTERSECTION FILE
	FILE* voxelFile = NULL;
	size_t test_fic = 0;
	voxelFile = fopen("voxels_data/voxel_intersec_1.data", "rb");
	if(NULL == voxelFile){
		std::cout << "[!]-> Unable to load the file voxelFile" << std::endl;
		return EXIT_FAILURE;
	}

	/* Open DATA file */
	drn_t cache;
	int32_t test_cache = drn_open(&cache, "./voxels_data/voxel_intersec_1.data", DRN_READ_NOLOAD);
	if(test_cache < 0){ throw std::runtime_error("unable to open data file"); }

	/* Getting the config data */
	uint16_t arguments[7];
		//0 : nbSub_lvl1
		//1 : nbSub_lvl2
		//2 : bending
		//3 : drain
		//4 : gradient
		//5 : normal
		//6 : surface

	test_cache = drn_read_chunk(&cache, 0, arguments);

	uint16_t nbSub_lvl1 = arguments[0];
	uint16_t nbSubMaxLeaf = arguments[1];

	uint32_t lengthTabVoxel = nbSubMaxLeaf*nbSubMaxLeaf*nbSubMaxLeaf;
	VoxelData* tabVoxelMax = new VoxelData[lengthTabVoxel];

/* Getting the first leafArray */
	test_cache = drn_read_chunk(&cache, 1, tabVoxelMax);

	uint32_t nbSub = nbSubMaxLeaf;
	uint32_t nbSubExpected = nbSub;
	int displayMode = 0; // = 0 to display the faces, = 1 to display the normals	
	
	VoxelData* tabVoxel = new VoxelData[lengthTabVoxel];
	for(uint32_t i = 0; i<lengthTabVoxel; ++i){
		tabVoxel[i].nbFaces = tabVoxelMax[i].nbFaces;
		tabVoxel[i].sumNormal = tabVoxelMax[i].sumNormal;
		tabVoxel[i].sumDrain = tabVoxelMax[i].sumDrain;
		tabVoxel[i].sumBending = tabVoxelMax[i].sumBending;
		tabVoxel[i].sumGradient = tabVoxelMax[i].sumGradient;
		tabVoxel[i].sumSurface = tabVoxelMax[i].sumSurface;
	}
		
	std::cout << "-> nbSub : " << nbSubExpected << std::endl;

	test_cache = drn_close(&cache);

	uint32_t nbIntersectionMax = 0;
	for(uint32_t i=0; i<lengthTabVoxel;++i){
		if(tabVoxel[i].nbFaces>nbIntersectionMax){
			nbIntersectionMax = tabVoxel[i].nbFaces;
		}
	}
	uint32_t constNbIntersectionMax = nbIntersectionMax;
	
	
	/*while(nbSub > nbSubExpected){
		nbSub /= 2;
		nbSubY /=2;
		nbIntersectionMax = increaseTab(nbSub, tabVoxel, nbSubLeaf, tabVoxelMax, constNbIntersectionMax, displayMode);
	}
	*/
	

	/* ************************************************************* */
	/* *************INITIALISATION OPENGL/SDL*********************** */
	/* ************************************************************* */

	// Initialisation de la SDL
	SDL_Init(SDL_INIT_VIDEO);

	// Creation de la fenêtre et d'un contexte OpenGL
	SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, BYTES_PER_PIXEL, SDL_OPENGL);

	// Initialisation de GLEW
	GLenum error;
	if(GLEW_OK != (error = glewInit())) {
		std::cerr << "Impossible d'initialiser GLEW: " << glewGetErrorString(error) << std::endl;
		return EXIT_FAILURE;
	}
	
	/* *********************************** */
	/* ****** CREATION DES FORMES ******** */
	/* *********************************** */

	// CREATION DU CUBE 
	Cube aCube = createCube(-0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f);
	
	GLdouble cubeVertices[] = {//
		aCube.left, aCube.bottom, aCube.near,
		aCube.right, aCube.bottom, aCube.near,
		aCube.left, aCube.top, aCube.near,

		aCube.right, aCube.bottom, aCube.near,
		aCube.right, aCube.top, aCube.near,
		aCube.left, aCube.top, aCube.near,

		//
		aCube.right, aCube.bottom, aCube.near,
		aCube.right, aCube.bottom, aCube.far,
		aCube.right, aCube.top, aCube.far,

		aCube.right, aCube.bottom, aCube.near,
		aCube.right, aCube.top, aCube.far,
		aCube.right, aCube.top, aCube.near,

		//
		aCube.left, aCube.top, aCube.near,
		aCube.right, aCube.top, aCube.near,
		aCube.right, aCube.top, aCube.far,

		aCube.left, aCube.top, aCube.near,
		aCube.right, aCube.top, aCube.far,
		aCube.left, aCube.top, aCube.far,

		////
		aCube.left, aCube.bottom, aCube.far,
		aCube.right, aCube.bottom, aCube.far,
		aCube.left, aCube.top, aCube.far,

		aCube.right, aCube.bottom, aCube.far,
		aCube.right, aCube.top, aCube.far,
		aCube.left, aCube.top, aCube.far,

		//
		aCube.left, aCube.bottom, aCube.near,
		aCube.left, aCube.bottom, aCube.far,
		aCube.left, aCube.top, aCube.far,

		aCube.left, aCube.bottom, aCube.near,
		aCube.left, aCube.top, aCube.far,
		aCube.left, aCube.top, aCube.near,

		//
		aCube.left, aCube.bottom, aCube.near,
		aCube.right, aCube.bottom, aCube.near,
		aCube.right, aCube.bottom, aCube.far,

		aCube.left, aCube.bottom, aCube.near,
		aCube.right, aCube.bottom, aCube.far,
		aCube.left, aCube.bottom, aCube.far
	};

	/* ******************************** */
	/* 		Creation des VBO, VAO 		*/
	/* ******************************** */

	GLuint cubeVBO = 0;
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint cubeVAO = 0;
	glGenVertexArrays(1, &cubeVAO);  
	glBindVertexArray(cubeVAO);  
		glEnableVertexAttribArray(POSITION_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			glVertexAttribPointer(POSITION_LOCATION, 3, GL_DOUBLE, GL_FALSE, 0, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Creation des Shaders
	GLuint programInter = hydrogene::loadProgram("shaders/basic.vs.glsl", "shaders/basic.fs.glsl");
	GLuint programNorm = hydrogene::loadProgram("shaders/basic.vs.glsl", "shaders/norm.fs.glsl");
	if(!programInter || !programNorm){
		glDeleteBuffers(1, &cubeVBO);
		glDeleteVertexArrays(1, &cubeVAO);
		delete[] tabVoxel;
		delete[] tabVoxelMax;
		return (EXIT_FAILURE);
	}
	
	GLuint program = programInter;
	//program = programNorm;
	
	glUseProgram(program);

	// Creation des Matrices
	GLint MVPLocation = glGetUniformLocation(program, "uMVPMatrix");
	
	glm::mat4 P = glm::perspective(90.f, WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, 1000.f);
	
	MatrixStack ms;
	ms.set(P);

	// Recuperation des variables uniformes
	GLint NbIntersectionLocation = glGetUniformLocation(program, "uNbIntersection");
	GLint NormSumLocation = glGetUniformLocation(program, "uNormSum");
	GLint LightVectLocation = glGetUniformLocation(program, "uLightVect");
	
	// Creation Light
	glm::vec3 light(-1.f,0.5f,0.f);
	float lightAngle = 0.05;
	
	//Creation Cameras
	CamType currentCam = TRACK_BALL;
	hydrogene::TrackBallCamera tbCam;
	hydrogene::FreeFlyCamera ffCam;

	// Creation des ressources OpenGL
	glEnable(GL_DEPTH_TEST);
	
	//Creation des ressources d'evenements
	bool is_lClicPressed = false;
	bool is_lKeyPressed = false;
	bool is_rKeyPressed = false;
	bool is_uKeyPressed = false;
	bool is_dKeyPressed = false;
	uint32_t tbC_savedClicX = 0;
	uint32_t tbC_savedClicY = 0;
	float tbC_angleX = 0;
	float tbC_angleY = 0;
	float tbC_tmpAngleX = 0;
	float tbC_tmpAngleY = 0;
	float ffC_angleX = 0;
	float ffC_angleY = 0;
	bool changeNbSubPlus = false;
	bool changeNbSubMinus = false;

	/* ************************************************************* */
	/* ********************DISPLAY LOOP***************************** */
	/* ************************************************************* */

	bool done = false;
	while(!done) {
		// Initilisation compteur
		Uint32 start = 0;
		Uint32 end = 0;
		Uint32 ellapsedTime = 0;
		start = SDL_GetTicks();

		//PRE_IDLE
		double cubeSize = GRID_3D_SIZE/(double)nbSub;

		// Nettoyage de la fenêtre
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
		if(changeNbSubPlus){ //si on a appuyé sur +
			std::cout <<"-> New number of subdivisions : "<<nbSub<<std::endl;
			//nbIntersectionMax = increaseTab(nbSub,tabVoxel,nbSubLeaf,tabVoxelMax, constNbIntersectionMax, displayMode);
			changeNbSubPlus = false;
		}
		
		if(changeNbSubMinus){ //si on a appuyé sur -
			std::cout << " > New number of subdivisions : " << nbSub << std::endl;
			//nbIntersectionMax = reduceTab(nbSub,tabVoxel, displayMode);
			changeNbSubMinus = false;
		}



		ms.push();
			// Choose the camera
			glm::mat4 V;
			if(currentCam == TRACK_BALL){
				V = tbCam.getViewMatrix();
			}else if(currentCam == FREE_FLY){
				V = ffCam.getViewMatrix();
			}
			ms.mult(V);
			ms.translate(glm::vec3(-1.f, -1.f, -1.f)); //CENTER TO THE ORIGIN
			glUniform3f(LightVectLocation, light.x, light.y, light.z);
			
			// Affichage de la grille
			for(uint32_t k=0;k<nbSub;++k){
				for(uint32_t j=0;j<nbSub;++j){
					for(uint32_t i=0;i<nbSub;++i){
						uint32_t currentIndex = k*nbSub*nbSub + j*nbSub + i;
						uint32_t currentNbIntersection = tabVoxel[k*nbSub*nbSub + j*nbSub + i].nbFaces;
						if(currentNbIntersection != 0){
							ms.push();
								ms.translate(glm::vec3(i*cubeSize, j*cubeSize, k*cubeSize)); //PLACEMENT OF EACH GRID CUBE
								ms.scale(glm::vec3(cubeSize));// RE-SCALE EACH GRID CUBE
							
								glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
								glUniform2i(NbIntersectionLocation, currentNbIntersection, nbIntersectionMax);
								glUniform3f(NormSumLocation, tabVoxel[currentIndex].sumNormal.x, tabVoxel[currentIndex].sumNormal.y, tabVoxel[currentIndex].sumNormal.z);
							
								glBindVertexArray(cubeVAO);
									glDrawArrays(GL_TRIANGLES, 0, aCube.nbVertices);
								glBindVertexArray(0);
							ms.pop();
						}
					}
				}
			}
		ms.pop();

		// Mise à jour de l'affichage
		SDL_GL_SwapBuffers();

		// Boucle de gestion des évenements
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			switch(e.type){
				case SDL_QUIT:
					done=true;
					break;
				
				case SDL_KEYDOWN:
					switch(e.key.keysym.sym){
						case SDLK_ESCAPE:
							done=true;
							break;
						
						case SDLK_KP_PLUS:
						case SDLK_p:
							if(nbSub != nbSubMaxLeaf){
								nbSub *= 2;
								changeNbSubPlus = true;
							}else std::cout << "You reached the maximum number of subdivisions." << std::endl;						
							break;
						
						case SDLK_KP_MINUS:
						case SDLK_m:
							if(nbSub != 1){
								nbSub /= 2;
								changeNbSubMinus = true;
							}else std::cout << "You reached the minimum number of subdivisions." << std::endl;						
							break;
						
						//Relative to the cameras
						case SDLK_TAB:
							if(currentCam == TRACK_BALL){
								currentCam = FREE_FLY;
							}else if(currentCam == FREE_FLY){
								currentCam = TRACK_BALL;
							}
							break;

						case SDLK_n:
							if(arguments[5]){
								resetShaderProgram(programNorm, MVPLocation, NbIntersectionLocation, NormSumLocation, LightVectLocation);
								displayMode = 4;
							}
							break;
							
						case SDLK_i:
							resetShaderProgram(programInter, MVPLocation, NbIntersectionLocation, NormSumLocation, LightVectLocation);
							displayMode = 0;
							break;
						
						case SDLK_q:
							if(currentCam == FREE_FLY){
								is_lKeyPressed = true;
							}
							break;

						case SDLK_d:
							if(currentCam == FREE_FLY){
								is_rKeyPressed = true;
							}
							break;

						case SDLK_z:
							if(currentCam == FREE_FLY){
								is_uKeyPressed = true;
							}
							break;

						case SDLK_s:
							if(currentCam == FREE_FLY){
								is_dKeyPressed = true;
							}
							break;

						default:
							break;
					}
					break;
				
				case SDL_KEYUP:
					switch(e.key.keysym.sym){
						case SDLK_q:
							if(currentCam == FREE_FLY){
								is_lKeyPressed = false;
							}
							break;

						case SDLK_d:
							if(currentCam == FREE_FLY){
								is_rKeyPressed = false;
							}
							break;

						case SDLK_z:
							if(currentCam == FREE_FLY){
								is_uKeyPressed = false;
							}
							break;

						case SDLK_s:
							if(currentCam == FREE_FLY){
								is_dKeyPressed = false;
							}
							break;

						default:
							break;
					}
					break;
				
				case SDL_MOUSEBUTTONDOWN:
					switch(e.button.button){
						case SDL_BUTTON_WHEELUP:
							if(currentCam == TRACK_BALL){
								tbCam.moveFront(-0.08f);
							}
							break;
						
						case SDL_BUTTON_WHEELDOWN:
							if(currentCam == TRACK_BALL){
								tbCam.moveFront(0.08f);
							}
							break;
						
						case SDL_BUTTON_LEFT:
							if(currentCam == TRACK_BALL){
								is_lClicPressed = true;
								tbC_savedClicX = e.button.x;
								tbC_savedClicY = e.button.y;
							}
							break;
						
						default:
							break;
					}
					break;
				
				case SDL_MOUSEBUTTONUP:
					switch(e.button.button){
						case SDL_BUTTON_LEFT:
							if(currentCam == TRACK_BALL){
								is_lClicPressed = false;
								tbC_angleX += tbC_tmpAngleX;
								tbC_angleY += tbC_tmpAngleY;
							}
							break;

						default:
							break;
					}
					
					break;
				
				case SDL_MOUSEMOTION:
					if(currentCam == TRACK_BALL){
						if(is_lClicPressed){
							tbC_tmpAngleX = 0.25*((int)e.motion.x - (int)tbC_savedClicX);
							tbC_tmpAngleY = 0.25*((int)e.motion.y - (int)tbC_savedClicY);
							tbCam.rotateLeft(tbC_angleX + tbC_tmpAngleX);
							tbCam.rotateUp(tbC_angleY + tbC_tmpAngleY);
						}
					}else if(currentCam == FREE_FLY){
						ffC_angleX = 0.6f*(WINDOW_WIDTH/2. - e.motion.x);
	                    ffC_angleY = 0.6f*(WINDOW_HEIGHT/2. - e.motion.y);
	                    ffCam.rotateLeft(ffC_angleX);
	                    ffCam.rotateUp(ffC_angleY);
					}
					break;
				
				default:
					break;
			}
		}

		//IDLE
		if(is_lKeyPressed){ ffCam.moveLeft(0.01); }
		if(is_rKeyPressed){ ffCam.moveLeft(-0.01); }
		if(is_uKeyPressed){ ffCam.moveFront(0.01); }
		if(is_dKeyPressed){ ffCam.moveFront(-0.01); }

		//ILLUMINATION
		//light.x += lightAngle;

		// Gestion compteur
		end = SDL_GetTicks();
		ellapsedTime = end - start;
		if(ellapsedTime < MIN_LOOP_TIME){
			SDL_Delay(MIN_LOOP_TIME - ellapsedTime);
		}
	}

	// Destruction des ressources OpenGL
	glDeleteBuffers(1, &cubeVBO);
	glDeleteVertexArrays(1, &cubeVAO);
	
	delete[] tabVoxel;
	delete[] tabVoxelMax;

	return (EXIT_SUCCESS);
}
