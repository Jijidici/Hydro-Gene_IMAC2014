#include <iostream>
#include <cstdlib>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/glew.h>
#include <stdint.h>
#include <stdexcept>
#include <vector>
#include <sstream>
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
#include "display/cube_model.hpp"

#include "drn/drn_reader.h"

#define FRAME_RATE 60
#define SKYBOX 0
#define TRIANGLES 1

static const Uint32 MIN_LOOP_TIME = 1000/FRAME_RATE;
static const size_t WINDOW_WIDTH = 1060, WINDOW_HEIGHT = 600;
static const size_t BYTES_PER_PIXEL = 32;
static const size_t NORMAL_LOCATION = 1;
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

	/* Open DATA file */
	drn_t cache;
	int32_t test_cache = drn_open(&cache, "./voxels_data/voxel_intersec_1.data", DRN_READ_NOLOAD);
	if(test_cache < 0){ throw std::runtime_error("unable to open data file"); }

	/* Getting the number of chunks saved in the voxel_data file */
	uint64_t nbChunks = drn_get_chunk_count(&cache) - 1;
	std::cout<<"//-> Nb chunks :"<<nbChunks<<std::endl;

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
	uint16_t nbSub_lvl2 = arguments[1];
	
	std::cout<<"//-> Nb Subdivision lvl 1 : "<<nbSub_lvl1<<std::endl;
	std::cout<<"//-> Nb Subdivision lvl 2 : "<<nbSub_lvl2<<std::endl;
	
	uint32_t lengthTabVoxel = nbSub_lvl2*nbSub_lvl2*nbSub_lvl2;
	
	size_t chunkBytesSize = lengthTabVoxel*VOXELDATA_BYTES_SIZE;
	std::cout<<"//-> Chunk bytes size : "<<chunkBytesSize<<std::endl;
	
	/* Getting the leaf chunk (last chunk) */
	uint32_t nbLeaves = (nbChunks-2)/3;
	uint32_t lvl2_dataOffset = nbLeaves*2;
	std::cout<<"//-> Nb Leaves saved : "<<nbLeaves<<std::endl;

	Leaf* leafArray = new Leaf[nbLeaves];
	test_cache = drn_read_chunk(&cache, nbChunks-1, leafArray);
	
	/* Array which know if a leaf grid is loaded or not */
	bool* loadedLeaf = new bool[nbLeaves];
	for(uint16_t idx=0;idx<nbLeaves;++idx){
		loadedLeaf[idx] = false;
	}
	
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
	
	/* ******************************** */
	/* 		Creation des VBO, VAO 		*/
	/* ******************************** */
	GLuint cubeVBO = CreateCubeVBO();
	GLuint cubeVAO = CreateCubeVAO(cubeVBO);

	GLuint texture_test = CreateTexture("textures/sky.jpg");
	
	/* Leaves VBOs & VAOs creation */
	GLuint l_VBOs = 0;
	glGenBuffers(1, &l_VBOs);
	
	GLuint l_VAOs = 0;
	glGenVertexArrays(1, &l_VAOs);
	//load the vertices		
	Vertex* trVertices = new Vertex[leafArray[0].nbVertices_lvl1];
	test_cache = drn_read_chunk(&cache, leafArray[0].id+lvl2_dataOffset+CONFIGCHUNK_OFFSET, trVertices);
	std::cout<<"//-> Vertices Computed : "<<std::endl;
	std::cout<<"//-> NB vertices : "<<leafArray[0].nbVertices_lvl1<<std::endl;
	std::cout<<"\tx: "<<trVertices[0].pos.x<<"\ty: "<<trVertices[0].pos.y<<"\tz: "<<trVertices[0].pos.z<<std::endl;
	std::cout<<"\tx: "<<trVertices[1].pos.x<<"\ty: "<<trVertices[1].pos.y<<"\tz: "<<trVertices[1].pos.z<<std::endl;
	std::cout<<"\tx: "<<trVertices[2].pos.x<<"\ty: "<<trVertices[2].pos.y<<"\tz: "<<trVertices[2].pos.z<<std::endl<<std::endl;
	std::cout<<"\tx: "<<trVertices[3].pos.x<<"\ty: "<<trVertices[3].pos.y<<"\tz: "<<trVertices[3].pos.z<<std::endl;
	std::cout<<"\tx: "<<trVertices[4].pos.x<<"\ty: "<<trVertices[4].pos.y<<"\tz: "<<trVertices[4].pos.z<<std::endl;
	std::cout<<"\tx: "<<trVertices[5].pos.x<<"\ty: "<<trVertices[5].pos.y<<"\tz: "<<trVertices[5].pos.z<<std::endl;


	glBindBuffer(GL_ARRAY_BUFFER, l_VBOs);
		glBufferData(GL_ARRAY_BUFFER, leafArray[0].nbVertices_lvl1*sizeof(Vertex), trVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(l_VAOs);
		glEnableVertexAttribArray(POSITION_LOCATION);
		glEnableVertexAttribArray(NORMAL_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, l_VBOs);
			glVertexAttribPointer(POSITION_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(0));
			glVertexAttribPointer(NORMAL_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(3*sizeof(GLdouble)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	delete[] trVertices;
	
	test_cache = drn_close(&cache);
	
	// Creation des Shaders
	GLuint programNorm = hydrogene::loadProgram("shaders/basic.vs.glsl", "shaders/norm.fs.glsl");
	if(!programNorm){
		glDeleteBuffers(1, &cubeVBO);
		glDeleteBuffers(1, &l_VBOs);
		glDeleteVertexArrays(1, &cubeVAO);
		glDeleteVertexArrays(1, &l_VAOs);
		delete[] leafArray;
		delete[] loadedLeaf;
		return (EXIT_FAILURE);
	}
	
	GLuint program = programNorm;	
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
	GLint LightVectLocation = glGetUniformLocation(program, "uLightVect");
	GLint TextureLocation = glGetUniformLocation(program, "uTexture");
	GLint ModeLocation = glGetUniformLocation(program, "uMode");

	// Creation Light
	glm::vec3 light(-1.f,-1.f,0.f);
	
	//Creation Cameras
	CamType currentCam = TRACK_BALL;
	hydrogene::TrackBallCamera tbCam;

	/* Memory cache - vector of voxelarray */
	std::vector<Chunk> memory;
	
	/* init memory */
	size_t currentMemCache = initMemory(memory, leafArray, loadedLeaf, nbLeaves, nbSub_lvl2,  chunkBytesSize, tbCam.getViewMatrix(), halfLeafSize);
	std::cout<<"//-> Chunks loaded : "<<memory.size()<<std::endl;
	std::cout<<"//-> free memory : "<<MAX_MEMORY_SIZE - currentMemCache<<" bytes"<<std::endl; 
	
	hydrogene::FreeFlyCamera ffCam(nearDistance, farDistance, verticalFieldOfView, leafSize);

	// Creation des ressources OpenGL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);
	
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
	float ffC_angleY = 0;
	float old_positionX = 0.;
	float new_positionX = 0.;
	float new_positionY = 0.;

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
			glUniform3fv(LightVectLocation, 1, glm::value_ptr(light));
			glUniform1i(ModeLocation, TRIANGLES);

			//For each leaf
			for(uint16_t idx=0;idx<nbLeaves;++idx){
				double d = computeDistanceLeafCamera(leafArray[idx], V, halfLeafSize);
				if(false){ //d<THRESHOLD_DISTANCE
					glUniform3fv(glGetUniformLocation(program, "uColor"), 1, glm::value_ptr(glm::vec3(1.f, 0.f, 0.f)));
					if(!loadedLeaf[idx]){
						Chunk voidChunk = freeInMemory(memory, loadedLeaf);
						loadInMemory(memory, leafArray[idx], idx, d, nbSub_lvl2, voidChunk.vao, voidChunk.vbo);
						loadedLeaf[idx] = true;
						std::sort(memory.begin(), memory.end(), memory.front());
					}
					for(std::vector<Chunk>::iterator n=memory.begin();n!=memory.end();++n){
						if(idx == n->idxLeaf){
							if(currentCam == FREE_FLY){
								//FRUSTUM CULLING
								if(ffCam.leavesFrustum(leafArray[idx])){
									display_triangle(n->vao, ms, MVPLocation, leafArray[idx].nbVertices_lvl2);
									break;
								}
							}else{
								display_triangle(n->vao, ms, MVPLocation, leafArray[idx].nbVertices_lvl2);
								break;
							}
						}
					}
				}else{
					//display_lvl1(cubeVAO, ms, MVPLocation, leafArray[idx].pos, halfLeafSize);
					/* display the triangularized leaf */
					glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
					glUniform3fv(glGetUniformLocation(program, "uColor"), 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));
					glBindVertexArray(l_VAOs);
						glDrawArrays(GL_TRIANGLES, 0, leafArray[0].nbVertices_lvl1);
					glBindVertexArray(0);
				}
			}
		ms.pop();

		//Skybox
		ms.push();
			if(currentCam == FREE_FLY){
				ms.mult(ffCam.getViewMatrix());
				ms.translate(ffCam.getCameraPosition());
			}
				ms.scale(glm::vec3(2.f, 2.f, 2.f));
				glUniform1i(ModeLocation, SKYBOX);
				glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
				glUniform1i(TextureLocation,0);
				BindTexture(texture_test);
					glBindVertexArray(cubeVAO);
						glDrawArrays(GL_TRIANGLES, 0, 36);
					glBindVertexArray(0);
				BindTexture(0);
		ms.pop();

		// Mise à jour de l'affichage
		SDL_GL_SwapBuffers();
		if(currentCam == FREE_FLY){
			SDL_WM_GrabInput(SDL_GRAB_ON);
			SDL_ShowCursor(SDL_DISABLE);
		}else if(currentCam == TRACK_BALL){
			SDL_WM_GrabInput(SDL_GRAB_OFF);
			SDL_ShowCursor(SDL_ENABLE);
		}
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
						new_positionX = e.motion.x;
						new_positionY = e.motion.y;
						ffC_angleY = 0.6f*(WINDOW_HEIGHT/2. - e.motion.y);
						if(ffC_angleY >= 90) ffC_angleY = 90;
						if(ffC_angleY <= -90) ffC_angleY = -90;
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

		if(currentCam == FREE_FLY){
			if(new_positionX >= WINDOW_WIDTH-1){
				SDL_WarpMouse(0, new_positionY);
				old_positionX = 0-(old_positionX - new_positionX);
				new_positionX = 0;
			}else if(new_positionX <= 0){
				SDL_WarpMouse(WINDOW_WIDTH, new_positionY);
				old_positionX = WINDOW_WIDTH+(old_positionX - new_positionX);
				new_positionX = WINDOW_WIDTH;
			}
			ffCam.rotateLeft((old_positionX - new_positionX)*0.6);
		}
		
		// Gestion compteur
		end = SDL_GetTicks();
		ellapsedTime = end - start;
		
		/* Compute the framerate */
		float frameRate = 0;
		if(ellapsedTime != 0){
			frameRate = 1000.f/ellapsedTime;
		}
		std::stringstream title;
		title << "Hydro-Gene Project | FPS: " <<frameRate;
		SDL_WM_SetCaption(title.str().c_str(),  NULL);
		
		if(ellapsedTime < MIN_LOOP_TIME){
			SDL_Delay(MIN_LOOP_TIME - ellapsedTime);
		}	
	}

	// Destruction des ressources OpenGL
	glDeleteBuffers(1, &cubeVBO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteTextures(1, &texture_test);
	
	//free cache memory
	uint16_t nbLoadedLeaves = memory.size();
	for(uint16_t idx=0;idx<nbLoadedLeaves;++idx){
		Chunk tmpChunk = freeInMemory(memory, loadedLeaf);
		glDeleteVertexArrays(1, &(tmpChunk.vao));
		glDeleteBuffers(1, &(tmpChunk.vbo));
	}
	
	glDeleteBuffers(1, &l_VBOs);
	glDeleteVertexArrays(1, &l_VAOs);
	delete[] leafArray;
	delete[] loadedLeaf;

	return (EXIT_SUCCESS);
}
