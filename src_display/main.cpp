#include <iostream>
#include <cstdlib>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <stdint.h>
#include <stdexcept>
#include <vector>
#include <algorithm>

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
#include "display/lvl_displaying.hpp"
#include "display/memory_cache.hpp"

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

	/* Getting the number of chunks saved in the voxel_data file */
	uint64_t nbChunks = drn_get_chunk_count(&cache);
	std::cout << "number of chunks :" << nbChunks-1 << std::endl;

	/* Getting the config data */
	uint16_t arguments[7];
		//0 : nbSub_lvl1
		//1 : nbSub_lvl2
		//2 : bending
		//3 : drain
		//4 : gradient
		//5 : normal
		//6 : surface
	
	std::cout<<"//-> NB_SUB1 : "<<arguments[0]<<" | "<<arguments[1]<<std::endl;
	
	test_cache = drn_read_chunk(&cache, 0, arguments);

	uint16_t nbSub_lvl1 = arguments[0];
	uint16_t nbSub_lvl2 = arguments[1];
	
	uint32_t lengthTabVoxel = nbSub_lvl2*nbSub_lvl2*nbSub_lvl2;
	
	size_t chunkBytesSize = lengthTabVoxel*VOXELDATA_BYTES_SIZE;
	std::cout<<"//-> Chunk bytes size : "<<chunkBytesSize<<std::endl;
	
	/* Getting the leaf chunk (last chunk) */
	uint32_t nbLeaves = nbChunks - 3;
	std::cout<<"number of leaves saved : "<<nbLeaves << std::endl;

	Leaf* leafArray = new Leaf[nbLeaves];
	test_cache = drn_read_chunk(&cache, nbChunks-2, leafArray);
	
	/* Array which know if a leaf grid is loaded or not */
	bool* loadedLeaf = new bool[nbLeaves];
	for(uint16_t idx=0;idx<nbLeaves;++idx){
		loadedLeaf[idx] = false;
	}
	
	uint16_t displayMode = 0; // = 0 to display the faces, = 1 to display the normals	
	
	test_cache = drn_close(&cache);
		
	uint32_t nbIntersectionMax = 50;
	
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
	
	/* Differents cube size */
	double leafSize = GRID_3D_SIZE/(double)nbSub_lvl1;
	double halfLeafSize = leafSize*0.5;
	double cubeSize = leafSize/(double)nbSub_lvl2;
	
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
		return (EXIT_FAILURE);
	}
	
	GLuint program = programInter;
	//program = programNorm;
	
	glUseProgram(program);

	// Creation des Matrices
	GLint MVPLocation = glGetUniformLocation(program, "uMVPMatrix");
	
	float verticalFieldOfView = 90.0;
	float nearDistance = 0.001;
	float farDistance = 100.;
	glm::mat4 P = glm::perspective(verticalFieldOfView, WINDOW_WIDTH / (float) WINDOW_HEIGHT, nearDistance, farDistance);
	
	MatrixStack ms;
	ms.set(P);

	// Recuperation des variables uniformes
	GLint NbIntersectionLocation = glGetUniformLocation(program, "uNbIntersection");
	GLint NormSumLocation = glGetUniformLocation(program, "uNormSum");
	GLint LightVectLocation = glGetUniformLocation(program, "uLightVect");
	
	// Creation Light
	glm::vec3 light(-1.f,0.5f,0.f);
	
	//Creation Cameras
	CamType currentCam = TRACK_BALL;
	hydrogene::TrackBallCamera tbCam;

	/* Memory cache - vector of voxelarray */
	std::vector<Chunk> memory;
	
	size_t currentMemCache = initMemory(memory, leafArray, loadedLeaf, nbSub_lvl2,  chunkBytesSize, tbCam.getViewMatrix(), halfLeafSize);
	std::cout<<"//-> Chunks loaded : "<<memory.size()<<std::endl;
	std::cout<<"//-> free memory : "<<MAX_MEMORY_SIZE - currentMemCache<<" bytes"<<std::endl; 
	
	hydrogene::FreeFlyCamera ffCam(nearDistance, farDistance, verticalFieldOfView);


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

		// Nettoyage de la fenêtre
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ms.push();
			// Choose the camera
			glm::mat4 V;
			if(currentCam == TRACK_BALL){
				V = tbCam.getViewMatrix();
			}else if(currentCam == FREE_FLY){
				V = ffCam.getViewMatrix();
			}
			ms.mult(V);
			glUniform3f(LightVectLocation, light.x, light.y, light.z);
			
			//For each leaf
			for(uint16_t idx=0;idx<nbLeaves;++idx){
				double d = computeDistanceLeafCamera(leafArray[idx], V, halfLeafSize);
				if(d<THRESHOLD_DISTANCE){
					if(!loadedLeaf[idx]){
						freeInMemory(memory, loadedLeaf);
						loadInMemory(memory, leafArray[idx], idx, d, nbSub_lvl2);
						loadedLeaf[idx] = true;
						std::sort(memory.begin(), memory.end(), memory.front());
					}
					for(std::vector<Chunk>::iterator n=memory.begin();n!=memory.end();++n){
						if(idx == n->idxLeaf){
							display_lvl2(cubeVAO, ms, MVPLocation, NbIntersectionLocation, NormSumLocation, nbIntersectionMax, aCube.nbVertices, n->voxels, leafArray[idx], nbSub_lvl2, cubeSize, ffCam, currentCam);
							break;
						}
					}
				}else{
					display_lvl1(cubeVAO, ms, MVPLocation, leafArray[idx].pos, halfLeafSize);
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
						
						case SDLK_g:
								std::cout << "near normal : " << std::endl;
								std::cout << ffCam.m_frustumNearPlaneNormal.x << std::endl;
								std::cout << ffCam.m_frustumNearPlaneNormal.y << std::endl;
								std::cout << ffCam.m_frustumNearPlaneNormal.z << std::endl << std::endl;
								
								std::cout << "far normal : " << std::endl;
								std::cout << ffCam.m_frustumFarPlaneNormal.x << std::endl;
								std::cout << ffCam.m_frustumFarPlaneNormal.y << std::endl;
								std::cout << ffCam.m_frustumFarPlaneNormal.z << std::endl << std::endl;
								
								std::cout << "left normal : " << std::endl;
								std::cout << ffCam.m_frustumLeftPlaneNormal.x << std::endl;
								std::cout << ffCam.m_frustumLeftPlaneNormal.y << std::endl;
								std::cout << ffCam.m_frustumLeftPlaneNormal.z << std::endl << std::endl;
								
								std::cout << "right normal : " << std::endl;
								std::cout << ffCam.m_frustumRightPlaneNormal.x << std::endl;
								std::cout << ffCam.m_frustumRightPlaneNormal.y << std::endl;
								std::cout << ffCam.m_frustumRightPlaneNormal.z << std::endl << std::endl;
								
								std::cout << "top normal : " << std::endl;
								std::cout << ffCam.m_frustumTopPlaneNormal.x << std::endl;
								std::cout << ffCam.m_frustumTopPlaneNormal.y << std::endl;
								std::cout << ffCam.m_frustumTopPlaneNormal.z << std::endl << std::endl;
								
								std::cout << "bottom normal : " << std::endl;
								std::cout << ffCam.m_frustumBottomPlaneNormal.x << std::endl;
								std::cout << ffCam.m_frustumBottomPlaneNormal.y << std::endl;
								std::cout << ffCam.m_frustumBottomPlaneNormal.z << std::endl << std::endl;
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

						case SDLK_SPACE:
							std::cout << "top plane normal : " << std::endl << "x : " << ffCam.m_frustumTopPlaneNormal.x << ", y : " << ffCam.m_frustumTopPlaneNormal.y << ", z : " << ffCam.m_frustumTopPlaneNormal.z << std::endl;
							std::cout << "right plane normal : " << std::endl << "x : " << ffCam.m_frustumRightPlaneNormal.x << ", y : " << ffCam.m_frustumRightPlaneNormal.y << ", z : " << ffCam.m_frustumRightPlaneNormal.z << std::endl;
							std::cout << "bottom plane normal : " << std::endl << "x : " << ffCam.m_frustumBottomPlaneNormal.x << ", y : " << ffCam.m_frustumBottomPlaneNormal.y << ", z : " << ffCam.m_frustumBottomPlaneNormal.z << std::endl;
							std::cout << "left plane normal : " << std::endl << "x : " << ffCam.m_frustumLeftPlaneNormal.x << ", y : " << ffCam.m_frustumLeftPlaneNormal.y << ", z : " << ffCam.m_frustumLeftPlaneNormal.z << std::endl;
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
	
	//free cache memory
	uint16_t nbLoadedLeaves = memory.size();
	for(uint16_t idx=0;idx<nbLoadedLeaves;++idx){
		freeInMemory(memory, loadedLeaf);
	}
	
	delete[] leafArray;
	delete[] loadedLeaf;

	return (EXIT_SUCCESS);
}
