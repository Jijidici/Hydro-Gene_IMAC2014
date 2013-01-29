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

#define NORMAL 2
#define BENDING 3
#define DRAIN 4
#define GRADIENT 5
#define SURFACE 6

#define VEGET 7
#define DEBUG 8


static const Uint32 MIN_LOOP_TIME = 1000/FRAME_RATE;
static const size_t WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720;
static const size_t BYTES_PER_PIXEL = 32;

static const size_t GRID_3D_SIZE = 2;

int main(int argc, char** argv){

	/* Open DATA file */
	drn_t cache;
	int32_t test_cache = drn_open(&cache, "./voxels_data/voxel_intersec_1.data", DRN_READ_NOLOAD);
	if(test_cache < 0){ throw std::runtime_error("unable to open data file"); }

	/* Getting the number of chunks saved in the voxel_data file */
	uint64_t nbChunks = drn_get_chunk_count(&cache) - 1;
	std::cout<<"//-> Nb chunks :"<<nbChunks<<std::endl;

	/* Getting the config data */
	uint16_t arguments[8];
		//0 : nbSub_lvl1
		//1 : nbSub_lvl2
		//2 : normal
		//3 : bending
		//4 : drain
		//5 : gradient
		//6 : surface
		//7 : nbLevel
	
	test_cache = drn_read_chunk(&cache, 0, arguments);

	uint16_t nbSub_lvl1 = arguments[0];
	uint16_t nbSub_lvl2 = arguments[1];
	uint16_t nbLevel = arguments[7];
	
	std::cout<<"//-> Nb Subdivision lvl 1 : "<<nbSub_lvl1<<std::endl;
	std::cout<<"//-> Nb Subdivision lvl 2 : "<<nbSub_lvl2<<std::endl;
	std::cout<<"//-> Nb possible levels : "<<nbLevel<<std::endl;
	
	uint32_t lengthTabVoxel = nbSub_lvl2*nbSub_lvl2*nbSub_lvl2;
	
	size_t chunkBytesSize = lengthTabVoxel*VOXELDATA_BYTES_SIZE;
	std::cout<<"//-> Chunk bytes size : "<<chunkBytesSize<<std::endl;
	
	/* Getting the maximum hydro properties coefficients */
	float * maxCoeffArray = new float[7];
	test_cache = drn_read_chunk(&cache, 1, maxCoeffArray);
	
	/* Getting the nb leaves chunk (last chunk) */
	uint32_t* nbLeaves = new uint32_t[nbLevel];
	test_cache = drn_read_chunk(&cache, nbChunks-1, nbLeaves);
	
	/* create chunk offset array - to getting the right triangle chunk in one specific level */
	uint32_t* chunkOffset = new uint32_t[nbLevel];
	chunkOffset[0] = CONFIGCHUNK_OFFSET + 2*nbLeaves[0];
	for(uint16_t lvl=1;lvl<nbLevel;++lvl){
		chunkOffset[lvl] = chunkOffset[lvl-1] + nbLeaves[lvl-1];
	}
	
	/* Getting the leaf arrays */
	uint32_t nbVao = 0;
	std::vector<Leaf*> leafArrays(nbLevel);
	for(uint16_t lvl=0;lvl<nbLevel;++lvl){
		leafArrays[lvl] = new Leaf[nbLeaves[lvl]];
		test_cache = drn_read_chunk(&cache, nbChunks-1-nbLevel+lvl, leafArrays[lvl]);
		
		nbVao+=nbLeaves[lvl];
	}
	
	/* Array which know if a leaf grid is loaded or not */
	bool* loadedLeaf = new bool[nbLeaves[0]];
	for(uint16_t idx=0;idx<nbLeaves[0];++idx){
		loadedLeaf[idx] = false;
	}
	
	std::cout<<"//-> NB VAO & VBO : "<<nbVao<<std::endl;
	
	/* ************************************************************* */
	/* *************INITIALISATION OPENGL/SDL*********************** */
	/* ************************************************************* */

	// Initialisation de la SDL
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

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

	GLuint texture_sky = CreateTexture("textures/sky.jpg");
	GLuint texture_night = CreateTexture("textures/night.jpg");
	/* vegetation textures */
	GLuint texture_veget[5];
	texture_veget[0] = CreateTexture("textures/rock.png");
	texture_veget[1] = CreateTexture("textures/plant.png");
	texture_veget[2] = CreateTexture("textures/tree.png");
	texture_veget[3] = CreateTexture("textures/pine_tree.png");
	texture_veget[4] = CreateTexture("textures/snow_tree.png");

	/* terrain textures */
	GLuint texture_terrain[6];
	texture_terrain[0] = CreateTexture("textures/grass.jpg");
	texture_terrain[1] = CreateTexture("textures/water.png");
	texture_terrain[2] = CreateTexture("textures/stone.jpg");
	texture_terrain[3] = CreateTexture("textures/snow.jpg");
	texture_terrain[4] = CreateTexture("textures/sand.jpeg");
	texture_terrain[5] = CreateTexture("textures/clouds2.jpg");
	
	
	/* Leaves VBOs & VAOs creation */
	GLuint* l_VBOs = new GLuint[nbVao];
	glGenBuffers(nbVao, l_VBOs);
	
	GLuint* l_VAOs = new GLuint[nbVao];
	glGenVertexArrays(nbVao, l_VAOs);
	
	uint16_t currentLevel=0;
	uint16_t levelFloor = nbLeaves[0];
	uint16_t offset_idx = 0;
	for(uint32_t idx=0;idx<nbVao;++idx){
		//check the level
		if(idx >= levelFloor){
			currentLevel++;
			offset_idx = levelFloor;
			levelFloor += nbLeaves[currentLevel];
		}
	
		//load the vertices
		Vertex* trVertices = new Vertex[leafArrays[currentLevel][idx-offset_idx].nbVertices_lvl1];
		test_cache = drn_read_chunk(&cache, leafArrays[currentLevel][idx-offset_idx].id+chunkOffset[currentLevel], trVertices);
		
		glBindBuffer(GL_ARRAY_BUFFER, l_VBOs[idx]);
			glBufferData(GL_ARRAY_BUFFER, leafArrays[currentLevel][idx-offset_idx].nbVertices_lvl1*sizeof(Vertex), trVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glBindVertexArray(l_VAOs[idx]);
			glEnableVertexAttribArray(POSITION_LOCATION);
			glEnableVertexAttribArray(NORMAL_LOCATION);
			glEnableVertexAttribArray(BENDING_LOCATION);
			glEnableVertexAttribArray(DRAIN_LOCATION);
			glEnableVertexAttribArray(GRADIENT_LOCATION);
			glEnableVertexAttribArray(SURFACE_LOCATION);
			glBindBuffer(GL_ARRAY_BUFFER, l_VBOs[idx]);
				glVertexAttribPointer(POSITION_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(0));
				glVertexAttribPointer(NORMAL_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(3*sizeof(GLdouble)));
				glVertexAttribPointer(BENDING_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)));
				glVertexAttribPointer(DRAIN_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+sizeof(GLfloat)));
				glVertexAttribPointer(GRADIENT_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+2*sizeof(GLfloat)));
				glVertexAttribPointer(SURFACE_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+3*sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
		delete[] trVertices;
	}
	
	test_cache = drn_close(&cache);
	
	//Infinite ground creation
	Vertex groundVertices[6];
	groundVertices[0] = createVertex(glm::dvec3(-1., 0, 1.), glm::dvec3(0., 1., 0.), 0.f, maxCoeffArray[1], 0.f, maxCoeffArray[3]);
	groundVertices[1] = createVertex(glm::dvec3(1., 0, 1.), glm::dvec3(0., 1., 0.), 0.f, maxCoeffArray[1], 0.f, maxCoeffArray[3]);
	groundVertices[2] = createVertex(glm::dvec3(1., 0, -1.), glm::dvec3(0., 1., 0.), 0.f, maxCoeffArray[1], 0.f, maxCoeffArray[3]);
	groundVertices[3] = createVertex(glm::dvec3(1., 0, -1.), glm::dvec3(0., 1., 0.), 0.f, maxCoeffArray[1], 0.f, maxCoeffArray[3]);
	groundVertices[4] = createVertex(glm::dvec3(-1., 0, -1.), glm::dvec3(0., 1., 0.), 0.f, maxCoeffArray[1], 0.f, maxCoeffArray[3]);
	groundVertices[5] = createVertex(glm::dvec3(-1., 0, 1.), glm::dvec3(0., 1., 0.), 0.f, maxCoeffArray[1], 0.f, maxCoeffArray[3]);
	
	GLuint groundVBO = 0;
	glGenBuffers(1, &groundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
		glBufferData(GL_ARRAY_BUFFER, 6*sizeof(Vertex), groundVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GLuint groundVAO = 0;
	glGenVertexArrays(1, &groundVAO);
	glBindVertexArray(groundVAO);
		glEnableVertexAttribArray(POSITION_LOCATION);
		glEnableVertexAttribArray(NORMAL_LOCATION);
		glEnableVertexAttribArray(BENDING_LOCATION);
		glEnableVertexAttribArray(DRAIN_LOCATION);
		glEnableVertexAttribArray(GRADIENT_LOCATION);
		glEnableVertexAttribArray(SURFACE_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
			glVertexAttribPointer(POSITION_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(0));
			glVertexAttribPointer(NORMAL_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(3*sizeof(GLdouble)));
			glVertexAttribPointer(BENDING_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)));
			glVertexAttribPointer(DRAIN_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+sizeof(GLfloat)));
			glVertexAttribPointer(GRADIENT_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+2*sizeof(GLfloat)));
			glVertexAttribPointer(SURFACE_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+3*sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	// Creation des Shaders
	GLuint program = hydrogene::loadProgram("shaders/basic.vs.glsl", "shaders/norm.fs.glsl", "shaders/instances.gs.glsl");
	if(!program){
		glDeleteBuffers(1, &cubeVBO);
		glDeleteBuffers(nbVao, l_VBOs);
		glDeleteBuffers(1, &groundVBO);
		glDeleteVertexArrays(1, &cubeVAO);
		glDeleteVertexArrays(nbVao, l_VAOs);
		glDeleteVertexArrays(1, &groundVAO);
		delete[] l_VAOs;
		delete[] l_VBOs;
		delete[] loadedLeaf;
		for(uint16_t lvl=0;lvl<nbLevel;++lvl){
			delete[] leafArrays[lvl];
		}
		delete[] nbLeaves;
		delete[] chunkOffset;
		return (EXIT_FAILURE);
	}
	glUseProgram(program);

	// Creation des Matrices
	GLint MVPLocation = glGetUniformLocation(program, "uMVPMatrix");
	GLint ViewMatrixLocation = glGetUniformLocation(program, "uViewMatrix");
	
	float verticalFieldOfView = 90.0;
	float nearDistance = 0.001;
	float farDistance = 100.;
	glm::mat4 P = glm::perspective(verticalFieldOfView, WINDOW_WIDTH / (float) WINDOW_HEIGHT, nearDistance, farDistance);
	
	MatrixStack ms;
	ms.set(P);

	// Recuperation des variables uniformes
	/* Light */
	GLint LightSunVectLocation = glGetUniformLocation(program, "uLightSunVect");
	GLint LightMoonVectLocation = glGetUniformLocation(program, "uLightMoonVect");
	GLint TimeLocation = glGetUniformLocation(program, "uTime");
	GLint DayLocation = glGetUniformLocation(program, "uDay");
	GLint NightLocation = glGetUniformLocation(program, "uNight");
	/* Textures */
	GLint SkyTexLocation = glGetUniformLocation(program, "uSkyTex");
	GLint NightTexLocation = glGetUniformLocation(program, "uNightTex");
	GLint GrassTexLocation = glGetUniformLocation(program, "uGrassTex");
	GLint WaterTexLocation = glGetUniformLocation(program, "uWaterTex");
	GLint StoneTexLocation = glGetUniformLocation(program, "uStoneTex");
	GLint SnowTexLocation = glGetUniformLocation(program, "uSnowTex");
	GLint SandTexLocation = glGetUniformLocation(program, "uSandTex");
	GLint CloudsTexLocation = glGetUniformLocation(program, "uCloudsShadows");

	GLint RockTexLocation = glGetUniformLocation(program, "uRockTex");
	GLint PlantTexLocation = glGetUniformLocation(program, "uPlantTex");
	GLint TreeTexLocation = glGetUniformLocation(program, "uTreeTex");
	GLint PineTreeTexLocation = glGetUniformLocation(program, "uPineTreeTex");
	GLint SnowTreeTexLocation = glGetUniformLocation(program, "uSnowTreeTex");

	/* Shaders modes */
	GLint ModeLocation = glGetUniformLocation(program, "uMode");
	GLint ChoiceLocation = glGetUniformLocation(program, "uChoice");
	/* Max properties */
	GLint MaxBendingLocation = glGetUniformLocation(program, "uMaxBending");
	GLint MaxDrainLocation = glGetUniformLocation(program, "uMaxDrain");
	GLint MaxGradientLocation = glGetUniformLocation(program, "uMaxGradient");
	GLint MaxSurfaceLocation = glGetUniformLocation(program, "uMaxSurface");
	GLint MaxAltitudeLocation = glGetUniformLocation(program, "uMaxAltitude");

	/* Vegetation */
	GLint VegetSizeLocation = glGetUniformLocation(program, "uVegetSizeCoef");
	GLint DistanceVegetLocation = glGetUniformLocation(program, "uDistance");
	
	/* Controlers  */
	GLint FogLocation = glGetUniformLocation(program, "uFog");
	
	glUniform1f(MaxBendingLocation, maxCoeffArray[0]);	
	glUniform1f(MaxDrainLocation, maxCoeffArray[1]);
	glUniform1f(MaxGradientLocation, maxCoeffArray[2]);
	glUniform1f(MaxSurfaceLocation, maxCoeffArray[3]);
	glUniform1f(MaxAltitudeLocation, maxCoeffArray[4]);
	
	// Send terrain textures
	glUniform1i(GrassTexLocation, 0);
	glUniform1i(WaterTexLocation, 1);
	glUniform1i(StoneTexLocation, 2);
	glUniform1i(SnowTexLocation, 3);
	glUniform1i(SandTexLocation, 4);
	glUniform1i(CloudsTexLocation, 5);
	glUniform1i(NightTexLocation, 6);
	glUniform1i(SkyTexLocation, 7);

	glUniform1i(RockTexLocation, 0);
	glUniform1i(PlantTexLocation, 1);
	glUniform1i(TreeTexLocation, 2);
	glUniform1i(PineTreeTexLocation, 3);
	glUniform1i(SnowTreeTexLocation, 4);
	
	// Creation Light
	float coefLight = 0.;
	glm::vec3 lightSun(glm::cos(coefLight),glm::sin(coefLight),0.f);
	glm::vec3 lightMoon(0.f,glm::sin(coefLight-2.5),glm::cos(coefLight-2.5));
	float time = -1.;
	float day = 0.;
	float night = 0.;
	float timeStep = 1./720.;
	float dayStep = 1./720.;
	float tempDayStep = 0.;
	float coefLightStep = 0.00218166156f;
	bool timePause = false;
	
	// send vegetation size coefficient
	std::cout << "test : " << maxCoeffArray[6] << std::endl;
	
	float vegetSizeCoef = maxCoeffArray[6];
	glUniform1f(VegetSizeLocation, vegetSizeCoef);
	
	//Creation Cameras
	CamType currentCam = TRACK_BALL;
	hydrogene::TrackBallCamera tbCam;
	hydrogene::FreeFlyCamera ffCam(glm::vec3(0.f, maxCoeffArray[4], 0.f), nearDistance, farDistance, verticalFieldOfView, leafSize);

	/* Memory cache - vector of voxelarray */
	std::vector<Chunk> memory;
	
	/* init memory */
	size_t currentMemCache = initMemory(memory, leafArrays[0], loadedLeaf, nbLeaves[0], nbSub_lvl2,  chunkBytesSize, tbCam.getViewMatrix(), halfLeafSize);
	std::cout<<"//-> Chunks loaded : "<<memory.size()<<std::endl;
	std::cout<<"//-> free memory : "<<MAX_MEMORY_SIZE - currentMemCache<<" bytes"<<std::endl; 

	// Creation des ressources OpenGL
	glEnable(GL_DEPTH_TEST);
	//~ //glEnable(GL_CULL_FACE); /* not so cool */
	//~ //glCullFace(GL_FRONT);
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
	float tbC_angleY = 60;
	float tbC_tmpAngleX = 0;
	float tbC_tmpAngleY = 0;
	float ffC_angleY = 0;
	float old_positionX = 0.;
	float new_positionX = 0.;
	float new_positionY = 0.;
	bool displayNormal = true;	
	bool displayDrain = false;
	bool displayBending = false;
	bool displaySurface = false;
	bool displayGradient = false;
	bool displayVegetation = true;
	bool displayFog = true;
	float thresholdDistance = 0.1f;
	
	bool displayDebug = false;
	float camSpeed = 0.01;

	glUniform1i(DistanceVegetLocation, thresholdDistance);

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
		
		glUniform1i(ModeLocation, TRIANGLES);
		glUniform1f(TimeLocation, time);
		glUniform1f(DayLocation, day);
		glUniform1f(NightLocation, night);
		glUniform3fv(LightSunVectLocation, 1, glm::value_ptr(lightSun));
		glUniform3fv(LightMoonVectLocation, 1, glm::value_ptr(lightMoon));
		/* Send fog */
		if(displayFog){
			glUniform1i(FogLocation, 1);
		}else{
			glUniform1i(FogLocation, 0);
		}
		
		
		//Ground
		ms.push();
			if(currentCam == FREE_FLY){
				ms.mult(ffCam.getViewMatrix());
			}else if(currentCam == TRACK_BALL){
				ms.mult(tbCam.getViewMatrix());
			}
			ms.translate(glm::vec3(0.f, maxCoeffArray[5], 0.f));
			ms.scale(glm::vec3(100.f, 100.f, 100.f));
			glUniform1i(ChoiceLocation, NORMAL);
			glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
			BindTexture(texture_terrain[0], GL_TEXTURE0);
			BindTexture(texture_terrain[1], GL_TEXTURE1);
				glBindVertexArray(groundVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);
			BindTexture(0, GL_TEXTURE0);
			BindTexture(0, GL_TEXTURE1);
		ms.pop();
		
		//Terrain
		ms.push();
			// Choose the camera
			glm::mat4 V;
			if(currentCam == TRACK_BALL){
				V = tbCam.getViewMatrix();
			}else if(currentCam == FREE_FLY){
				V = ffCam.getViewMatrix();
			}
			ms.mult(V);

			glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(V));

			uint32_t vao_idx = 0;
			//For each level
			for(uint16_t lvl=0;lvl<nbLevel;++lvl){			
				//For each leaf
				for(uint16_t idx=0;idx<nbLeaves[lvl];++idx){
					double d = computeDistanceLeafCamera(leafArrays[lvl][idx], V);
					double crt_lvlTD = thresholdDistance*(lvl+1);
					double nxt_lvlTD = 0;
					/* special case of uppest level */
					if(lvl == nbLevel-1){
						nxt_lvlTD = 1000;
					}else{
						nxt_lvlTD = crt_lvlTD+thresholdDistance;
					}
					
					//display the leaf of this level if it is in the distance fork
					if(crt_lvlTD <= d && d < nxt_lvlTD){
						display_triangle(l_VAOs[vao_idx], ms, MVPLocation, leafArrays[lvl][idx].nbVertices_lvl1, texture_terrain);
					}
					
					//special case of lvl 0
					if(lvl == 0 && d < thresholdDistance){
						if(!loadedLeaf[idx]){
							Chunk voidChunk = freeInMemory(memory, loadedLeaf);
							loadInMemory(memory, leafArrays[0][idx], idx, d, nbSub_lvl2, voidChunk.vao, voidChunk.vbo);
							loadedLeaf[idx] = true;
							std::sort(memory.begin(), memory.end(), memory.front());
						}
						for(std::vector<Chunk>::iterator n=memory.begin();n!=memory.end();++n){
							if(idx == n->idxLeaf){
								if(currentCam == FREE_FLY){
									//FRUSTUM CULLING
									if(ffCam.leavesFrustum(leafArrays[0][idx])){
											display_triangle(n->vao, ms, MVPLocation, leafArrays[0][idx].nbVertices_lvl2, texture_terrain);
										if(displayVegetation){
											display_vegetation(n->vao, ms, MVPLocation, leafArrays[0][idx].nbVertices_lvl2/3, ChoiceLocation, texture_veget);
										}
										break;
									}
								}else{
									if(displayDebug){
										if(ffCam.leavesFrustum(leafArrays[0][idx])){ // wrong frustum - comment this line to display every leaf with TrackBallCam
											display_triangle(n->vao, ms, MVPLocation, leafArrays[0][idx].nbVertices_lvl2, texture_terrain);
											glUniform1i(ChoiceLocation, DEBUG);
											display_lvl1(cubeVAO, ms, MVPLocation, n->pos, halfLeafSize);
											if(displayVegetation){
												display_vegetation(n->vao, ms, MVPLocation, leafArrays[0][idx].nbVertices_lvl2/3, ChoiceLocation, texture_veget);
											}
											break;
										} // comment too
									}else{
										display_triangle(n->vao, ms, MVPLocation, leafArrays[0][idx].nbVertices_lvl2, texture_terrain);
										if(displayVegetation){
											display_vegetation(n->vao, ms, MVPLocation, leafArrays[0][idx].nbVertices_lvl2/3, ChoiceLocation, texture_veget);
										}
										break;
									}
								}
							}
						}
					}

					//DISPLAY OF THE COEFFICIENTS
					if(displayNormal) glUniform1i(ChoiceLocation, NORMAL);
					else if(displayDrain) glUniform1i(ChoiceLocation, DRAIN);
					else if(displayBending) glUniform1i(ChoiceLocation, BENDING);
					else if(displayGradient) glUniform1i(ChoiceLocation, GRADIENT);
					else if(displaySurface) glUniform1i(ChoiceLocation, SURFACE);
					
					//set the vao idx
					++vao_idx;
				}
			}
		ms.pop();

		//Skybox
		glUniform1i(ModeLocation, SKYBOX);
		ms.push();
			if(currentCam == FREE_FLY){
				ms.mult(ffCam.getViewMatrix());
				ms.translate(ffCam.getCameraPosition());
				ms.scale(glm::vec3(2.f, 2.f, 2.f));
			}else if(currentCam == TRACK_BALL){
				ms.mult(tbCam.getViewMatrix());
				ms.scale(glm::vec3(100.f, 100.f, 100.f));
			}
			glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(ms.top()));
			//~ glUniform1i(SkyTexLocation,0);
			BindTexture(texture_sky, GL_TEXTURE7);
			BindTexture(texture_night, GL_TEXTURE6);
				glBindVertexArray(cubeVAO);
					glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
			BindTexture(0, GL_TEXTURE6);
			BindTexture(0, GL_TEXTURE5);
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
							
						case SDLK_s:
							if(currentCam == FREE_FLY){
								is_dKeyPressed = true;
							}
							break;
							
						case SDLK_f:
							if(currentCam == TRACK_BALL){
								displayDebug = !displayDebug;
							}
							break;
							
						case SDLK_F1:
							displayNormal = true;
	 						displayDrain = false;
							displayBending = false;
							displaySurface = false;
							displayGradient = false;
							break;

						case SDLK_F2:
							if(arguments[BENDING]){
							displayNormal = false;
	 						displayDrain = false;
							displayBending = true;
							displaySurface = false;
							displayGradient = false;
							}
							break;

						case SDLK_F3:
							if(arguments[DRAIN]){
							displayNormal = false;
	 						displayDrain = true;
							displayBending = false;
							displaySurface = false;
							displayGradient = false;
							}
							break;

						case SDLK_F4:
							if(arguments[GRADIENT]){
							displayNormal = false;
	 						displayDrain = false;
							displayBending = false;
							displaySurface = false;
							displayGradient = true;
							}
							break;

						case SDLK_F5:
							if(arguments[SURFACE]){
							displayNormal = false;
	 						displayDrain = false;
							displayBending = false;
							displaySurface = true;
							displayGradient = false;
							}
							break;
						
						case SDLK_UP:
							thresholdDistance += 0.01f;
							break;
							
						case SDLK_DOWN:
							thresholdDistance -= 0.01f;
							break;
						
						case SDLK_v:
							if(displayVegetation){
								displayVegetation=false;
							}else{
								displayVegetation=true;							
							}
							break;
						
						case SDLK_g:
							if(displayFog){
								displayFog = false;
							}else{
								displayFog = true;
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
							if(!timePause){
								coefLightStep = 0.;
								tempDayStep = dayStep;
								dayStep = 0.;
								timeStep = 0.;
								timePause = true;
							}else{
								timeStep = 1./720.;
								dayStep = tempDayStep;
								coefLightStep = 0.00218166156f;
								timePause = false;
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
							if(currentCam == FREE_FLY){
								camSpeed += 0.001;
								if(camSpeed >= 1.){
									camSpeed = 1.;
								}
							}
							break;
						
						case SDL_BUTTON_WHEELDOWN:
							if(currentCam == TRACK_BALL){
								tbCam.moveFront(0.08f);
							}
							if(currentCam == FREE_FLY){
								camSpeed -= 0.001;
								if(camSpeed <= 0.){
									camSpeed = 0.001;
								}
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
		if(is_lKeyPressed){ ffCam.moveLeft(camSpeed); }
		if(is_rKeyPressed){ ffCam.moveLeft(-camSpeed); }
		if(is_uKeyPressed){ ffCam.moveFront(camSpeed); }
		if(is_dKeyPressed){ ffCam.moveFront(-camSpeed); }

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
		
		//Manage the sun
		coefLight -= coefLightStep;
		lightSun.x = glm::cos(coefLight);
		lightSun.y = glm::sin(coefLight);
		
		lightMoon.y = glm::sin(coefLight-2.5);
		lightMoon.z = glm::cos(coefLight-2.5);
		
		//~ std::cout << "coefLight : " << coefLight << std::endl;
		
		if(coefLight < -4.71238898){ coefLight = 1.57079633; }
		
		time += timeStep;
		day += dayStep;
		night -= dayStep;
		
		if(time > 1){time = -time;}
		if(day > 1){dayStep = -dayStep;}
		if(day < -1){dayStep = -dayStep;}
		
		//~ std::cout << "time : " << time << std::endl;
		//~ std::cout << "day : " << 0.5 - fabs(day) << std::endl;
		//~ std::cout << "night : " << night << std::endl;
		
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
	glDeleteBuffers(1, &groundVBO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &groundVAO);
	glDeleteTextures(1, &texture_sky);
	
	//free cache memory
	uint16_t nbLoadedLeaves = memory.size();
	for(uint16_t idx=0;idx<nbLoadedLeaves;++idx){
		Chunk tmpChunk = freeInMemory(memory, loadedLeaf);
		glDeleteVertexArrays(1, &(tmpChunk.vao));
		glDeleteBuffers(1, &(tmpChunk.vbo));
	}
	
	glDeleteBuffers(nbVao, l_VBOs);
	glDeleteVertexArrays(nbVao, l_VAOs);
	delete[] l_VAOs;
	delete[] l_VBOs;
	for(uint16_t lvl=0;lvl<nbLevel;++lvl){
		delete[] leafArrays[lvl];
	}
	delete[] loadedLeaf;
	delete[] maxCoeffArray;
	delete[] nbLeaves;
	delete[] chunkOffset;

	return (EXIT_SUCCESS);
}
