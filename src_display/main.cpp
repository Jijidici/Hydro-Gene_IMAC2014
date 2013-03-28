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
#include "display/procedural_sky.hpp"

#include "drn/drn_reader.h"
#include "my_imgui/imgui.h"
#include "my_imgui/imguiRenderGL.h"

#define FRAME_RATE 60
#define SKYBOX 0
#define TRIANGLES 1

#define NORMAL 2
#define BENDING 3
#define DRAIN 4
#define GRADIENT 5
#define SURFACE 6

#define VEGET 7
#define DEBUG_BOX 8
#define DEBUG_TRI 9


static const Uint32 MIN_LOOP_TIME = 1000/FRAME_RATE;
static const size_t WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720;
static const size_t BYTES_PER_PIXEL = 32;

static const size_t GRID_3D_SIZE = 2;
static const size_t TERRAIN_SCALE_PARAM = 150000;

static const size_t NB_TEXTURES_VEGET = 5;
static const size_t NB_TEXTURES_TERRAIN = 5;

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
	uint16_t total_nbSub = nbSub_lvl1*nbSub_lvl2;
	uint16_t nbLevel = arguments[7];
	
	std::cout<<"//-> Nb Subdivision lvl 1 : "<<nbSub_lvl1<<std::endl;
	std::cout<<"//-> Nb Subdivision lvl 2 : "<<nbSub_lvl2<<std::endl;
	std::cout<<"//-> Nb possible levels : "<<nbLevel<<std::endl;
	
	/* Getting the maximum hydro properties coefficients */
	float * maxCoeffArray = new float[7];
		//0 : max bending
		//1 : max drain
		//2 : max gradient
		//3 : max surface
		//4 : max altitude
		//5 : min altitude
		//6 : total nb faces
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
	
	std::cout<<"//-> NB VAO & VBO : "<<nbVao<<std::endl;
	
	/* Array which know if a leaf grid is loaded or not */
	bool* loadedLeaf = new bool[nbLeaves[0]];
	for(uint16_t idx=0;idx<nbLeaves[0];++idx){
		loadedLeaf[idx] = false;
	}
	
	/* Setting the terrain scale */
	float terrainScale = 5*(maxCoeffArray[6]/TERRAIN_SCALE_PARAM);
	std::cout<<"//-> Terrain Scale : "<<terrainScale<<std::endl;
		
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
	double voxelSize = leafSize/(double) nbSub_lvl2;
	double halfVoxelSize = voxelSize*0.5;
	
	/* ******************************** */
	/* 		Creation des VBO, VAO 		*/
	/* ******************************** */
	GLuint cubeVBO = CreateCubeVBO();
	GLuint cubeVAO = CreateCubeVAO(cubeVBO);
	GLuint quadVBO = CreateQuadVBO();
	GLuint quadVAO = CreateQuadVAO(quadVBO);
	
	/* Leaves VBOs & VAOs creation for computed triangles*/
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
	
	/* *******************************
	 *      PROGRAM CREATION
	 * ******************************* */
	GLuint terrainProgram = hydrogene::loadProgram("shaders/basic.vs.glsl", "shaders/norm.fs.glsl", "shaders/instances.gs.glsl");
	GLuint debugProgram = hydrogene::loadProgram("shaders/basic.vs.glsl", "shaders/norm.fs.glsl", "shaders/debug.gs.glsl");
	GLuint skyProgram = hydrogene::loadProgram("shaders/skybox.vs.glsl", "shaders/skybox.fs.glsl");
	if(!terrainProgram || !debugProgram || !skyProgram){
		glDeleteBuffers(1, &cubeVBO);
		glDeleteBuffers(1, &quadVBO);
		glDeleteBuffers(nbVao, l_VBOs);
		glDeleteBuffers(1, &groundVBO);
		glDeleteVertexArrays(1, &cubeVAO);
		glDeleteVertexArrays(1, &quadVAO);
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
	glUseProgram(terrainProgram);
	
	/* *******************************
	 *     TEXTURES CREATION
	 * ******************************* */
	GLuint texture_sky = CreateCubeMap();
	/* vegetation textures */
	GLuint texture_veget[NB_TEXTURES_VEGET];
	texture_veget[0] = CreateTexture("textures/rock.png");
	texture_veget[1] = CreateTexture("textures/plant.png");
	texture_veget[2] = CreateTexture("textures/tree.png");
	texture_veget[3] = CreateTexture("textures/pine_tree.png");
	texture_veget[4] = CreateTexture("textures/snow_tree.png");

	/* terrain textures */
	GLuint texture_terrain[NB_TEXTURES_TERRAIN];
	texture_terrain[0] = CreateTexture("textures/grass.jpg");
	texture_terrain[1] = CreateTexture("textures/bignormalmap.jpg");
	texture_terrain[2] = CreateTexture("textures/stone.jpg");
	texture_terrain[3] = CreateTexture("textures/snow.jpg");
	texture_terrain[4] = CreateTexture("textures/sand.jpeg");
	
	/* Bind the Cube Map */
	BindCubeMap(texture_sky, GL_TEXTURE5);
	
	/* ***************************************
	 *       DYNAMIC SKY CREATION
	 * *************************************** */
	GLuint skyFBO = createFBO();
	GLint* skyLocations = new GLint[NB_SKYLOCATIONS];
	getSkyLocation(skyLocations, skyProgram);
	
	/* MATRICES CAMERA AND LIGHTS */
	float verticalFieldOfView = 90.0;
	float nearDistance = 0.001;
	float farDistance = 100.;
	glm::mat4 P = glm::perspective(verticalFieldOfView, WINDOW_WIDTH / (float) WINDOW_HEIGHT, nearDistance, farDistance);
	
	/* Initialize the MVP matrix stack */
	MatrixStack ms;
	ms.set(P);
	
	/* Initialize the ModelView matrix stack */
	MatrixStack mvStack;
	
	//distance for changing of LOD
	float thresholdDistance = 5.f;
	
	// Time variables
	float timeStep = (2*M_PI)/1000.;
	float time = 500*timeStep;
	bool timePause = false;
	float cloudsTime = 0.;
	
	// Creation Light
	glm::vec3 lightSun = -glm::normalize(glm::vec3(cos(time), sin(time), 0.f));
	
	//Creation Cameras
	CamType currentCam = TRACK_BALL;
	hydrogene::TrackBallCamera tbCam;
	hydrogene::FreeFlyCamera ffCam(glm::vec3(0.f, 0.f, 0.f), nearDistance, farDistance, verticalFieldOfView, leafSize*terrainScale);
	
	/* Uniform Locations */
	GLint* locations = new GLint[NB_LOCATIONS];
	getLocations(locations, terrainProgram);	 
	sendUniforms(locations, maxCoeffArray, thresholdDistance, terrainScale);

	/* Memory cache - vector of voxelarray */
	std::vector<Chunk> memory;
	
	/* init memory */
	size_t freeMemory = MAX_MEMORY_SIZE;
	std::cout<<"//-> Free memory in cache : "<<freeMemory<<std::endl;
	
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
	bool displayFog = false;
	
	bool displayDebug = false;
	float camSpeed = 0.01;
	
	/* timelaps animation */
	bool timelaps = false;
	float timelapsPosOffset = 0.;
	
	/* rotation animation */
	bool rotationAnim = false;


	/* ------------- IMGUI --------------- */
	GLenum err = glewInit();
    if (GLEW_OK != err)
    {
          /* Problem: glewInit failed, something is seriously wrong. */
          fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
          exit( EXIT_FAILURE );
    }

	bool ihm = true;
	
	int timeUIHeight = 150;
	int detailsUIHeight = 170;
	int camUIHeight = 200;
	int viewUIHeight = 250;
	
	int mousex = 0;
	int mousey = 0;
	int timeUIscrollArea = 0;
	int detailsUIscrollArea = 0;
	int camUIscrollArea = 0;
	int viewUIscrollArea = 0;
	int closeScrollArea = 0;
	
	int toggle = 0;

	bool bendItem = false;
	bool drainItem = false;
	bool gradientItem = false;
	bool surfaceItem = false;
	float waterTime = 0.;
	float step = 0.01;
	float coeffStep = 1;
	
	if(arguments[BENDING]) bendItem = true;
	if(arguments[DRAIN]) drainItem = true;
	if(arguments[GRADIENT]) gradientItem = true;
	if(arguments[SURFACE]) surfaceItem = true;

	if (!imguiRenderGLInit("include/my_imgui/DroidSans.ttf"))
	{
		fprintf(stderr, "Could not init GUI renderer.\n");
		exit(EXIT_FAILURE);
	}
	
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
		
		if(waterTime > 1){
			coeffStep = -1;
		}
		if(waterTime < 0) {
			coeffStep = 1;
		}
		waterTime += coeffStep*step;			
		
		// Comupte the sky textures
		paintTheSky(skyFBO, texture_sky, skyProgram, quadVAO, -lightSun, cloudsTime, skyLocations);
		
		// Nettoyage de la fenêtre
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(terrainProgram);
		sendUniforms(locations, maxCoeffArray, thresholdDistance, terrainScale);
		
		glUniform1i(locations[MODE], TRIANGLES);
		glUniform1f(locations[TIME], cos(time));
		glUniform1f(locations[WATERTIME], waterTime);
		glUniform3fv(locations[LIGHTSUN], 1, glm::value_ptr(lightSun));
		glUniform3fv(locations[FF_FRONT_VECTOR], 1, glm::value_ptr(ffCam.getFrontVector()));

		/* Send fog */
		if(displayFog){
			glUniform1i(locations[FOG], 1);
		}else{
			glUniform1i(locations[FOG], 0);
		}

		// Choose the camera
		glm::mat4 V;
		glm::vec3 camPosition;
		if(currentCam == TRACK_BALL){
			V = tbCam.getViewMatrix();
			camPosition = tbCam.getCameraPosition();
		}else if(currentCam == FREE_FLY){
			V = ffCam.getViewMatrix();
			camPosition = ffCam.getCameraPosition();
		}
		/* Sending the ViewMatrix */
		glUniformMatrix4fv(locations[INV_VIEWMATRIX], 1, GL_FALSE, glm::value_ptr(glm::inverse(V)));
		
		mvStack.push();
			mvStack.mult(V);

			//Ground
			glUniform1i(locations[CHOICE], NORMAL);
			glUniform1i(locations[OCEAN], 1);
			BindTexture(texture_terrain[4], GL_TEXTURE4);
			mvStack.push();
				mvStack.translate(glm::vec3(0.f, maxCoeffArray[5], 0.f));
				mvStack.scale(glm::vec3(10*terrainScale));
				/* Send the model view */
				glUniformMatrix4fv(locations[MODELVIEW], 1, GL_FALSE, glm::value_ptr(mvStack.top()));
				ms.push();
					ms.mult(mvStack.top());
					glUniformMatrix4fv(locations[MVP], 1, GL_FALSE, glm::value_ptr(ms.top()));
					BindTexture(texture_terrain[1], GL_TEXTURE1);
						glBindVertexArray(groundVAO);
							glDrawArrays(GL_TRIANGLES, 0, 6);
						glBindVertexArray(0);
					BindTexture(0, GL_TEXTURE1);
				ms.pop();
			mvStack.pop();
			BindTexture(0, GL_TEXTURE4);
			glUniform1i(locations[OCEAN], 0);
			
			//Terrain
			mvStack.push();
				/* terrain scaling */
				mvStack.scale(glm::vec3(terrainScale));
				/* Send the model view */
				glUniformMatrix4fv(locations[MODELVIEW], 1, GL_FALSE, glm::value_ptr(mvStack.top()));
				ms.push();
					ms.mult(mvStack.top());
					

					uint32_t vao_idx = 0;
					//For each level
					for(uint16_t lvl=0;lvl<nbLevel;++lvl){			
						//For each leaf
						for(uint16_t idx=0;idx<nbLeaves[lvl];++idx){
							double d = computeDistanceLeafCamera(leafArrays[lvl][idx], camPosition, terrainScale);
							
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
								display_triangle(l_VAOs[vao_idx], ms, locations[MVP], leafArrays[lvl][idx].nbVertices_lvl1, texture_terrain);
							}
							
							//special case of lvl 0
							if(lvl == 0 && d < thresholdDistance){
								//Enable BackFace Culling
								glEnable(GL_CULL_FACE);
								glCullFace(GL_FRONT);
							
								if(currentCam == FREE_FLY){ //////////////////////////////////FREEFLY
									/* FRUSTUM CULLING */
									if(ffCam.leavesFrustum(leafArrays[0][idx], terrainScale)){
										/* LOADING */
										if(!loadedLeaf[idx]){
											loadInMemory(memory, loadedLeaf, leafArrays[0][idx], d, nbSub_lvl2, freeMemory);
										}
										/* DISPLAYING */
										for(std::vector<Chunk>::iterator n=memory.begin();n!=memory.end();++n){
											if(idx == n->idxLeaf){
												/* set the distance */
												n->d = d;
												display_triangle(n->vao, ms, locations[MVP], leafArrays[0][idx].nbVertices_lvl2, texture_terrain);	
												if(displayDebug){
													/* leaf cube */
													glUniform1i(locations[CHOICE], DEBUG_BOX);
													display_lvl1(cubeVAO, ms, locations[MVP], n->pos, halfLeafSize);
													
													/* Computed triangles */
													glUniform1i(locations[CHOICE], DEBUG_TRI);
													display_triangle(l_VAOs[vao_idx], ms, locations[MVP], leafArrays[lvl][idx].nbVertices_lvl1, texture_terrain);
												}else{
													if(displayVegetation){
														display_vegetation(n->vao, ms, locations[MVP], leafArrays[0][idx].nbVertices_lvl2/3, locations[CHOICE], texture_veget);
													}
												}
											}
										}
									}
								}else if(currentCam == TRACK_BALL){ /////////////////////////////TRACKBALL
									/* LOADING */
									if(!loadedLeaf[idx]){
										loadInMemory(memory, loadedLeaf, leafArrays[0][idx], d, nbSub_lvl2, freeMemory);
									}
									/* DISPLAYING */
									for(std::vector<Chunk>::iterator n=memory.begin();n!=memory.end();++n){
										if(idx == n->idxLeaf){
											/* set the distance */
											n->d = d;
											if(displayDebug){
												if(ffCam.leavesFrustum(leafArrays[0][idx], terrainScale)){
													/* real triangles */
													display_triangle(n->vao, ms, locations[MVP], leafArrays[0][idx].nbVertices_lvl2, texture_terrain);	
												}
												/* leaf cube */
												glUniform1i(locations[CHOICE], DEBUG_BOX);
												display_lvl1(cubeVAO, ms, locations[MVP], n->pos, halfLeafSize);
												/* Computed triangles */
												glUniform1i(locations[CHOICE], DEBUG_TRI);
												display_triangle(l_VAOs[vao_idx], ms, locations[MVP], leafArrays[lvl][idx].nbVertices_lvl1, texture_terrain);
											}else{
												/* real triangles */
												display_triangle(n->vao, ms, locations[MVP], leafArrays[0][idx].nbVertices_lvl2, texture_terrain);	
												if(displayVegetation){
													display_vegetation(n->vao, ms, locations[MVP], leafArrays[0][idx].nbVertices_lvl2/3, locations[CHOICE], texture_veget);
												}
											}								
											break;
										}
									}
								} //END camera
								glDisable(GL_CULL_FACE);
							}

							//DISPLAY OF THE COEFFICIENTS
							if(displayNormal) glUniform1i(locations[CHOICE], NORMAL);
							else if(displayDrain) glUniform1i(locations[CHOICE], DRAIN);
							else if(displayBending) glUniform1i(locations[CHOICE], BENDING);
							else if(displayGradient) glUniform1i(locations[CHOICE], GRADIENT);
							else if(displaySurface) glUniform1i(locations[CHOICE], SURFACE);
							
							//set the vao idx
							++vao_idx;
						}
					}
				ms.pop();
			mvStack.pop();

			//Skybox
			glUniform1i(locations[MODE], SKYBOX);
			mvStack.push();
				if(currentCam == FREE_FLY){
					mvStack.translate(ffCam.getCameraPosition());
				}else if(currentCam == TRACK_BALL){
					mvStack.scale(glm::vec3(10*terrainScale));
				}
				ms.push();
					ms.mult(mvStack.top());
					glUniformMatrix4fv(locations[MVP], 1, GL_FALSE, glm::value_ptr(ms.top()));
					glBindVertexArray(cubeVAO);
						glDrawArrays(GL_TRIANGLES, 0, 36);
					glBindVertexArray(0);
				ms.pop();
			mvStack.pop();
		
		mvStack.pop();
		
		if(ihm){
			/* ------ IHM imgui ------ */
			glActiveTexture(GL_TEXTURE0);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST);
			
			SDL_GetMouseState(&mousex, &mousey);
			mousey = WINDOW_HEIGHT - mousey;
			imguiBeginFrame(mousex, mousey, is_lClicPressed, 0);
			
			/* Time UI */
			imguiBeginScrollArea("Time", 10, WINDOW_HEIGHT - (timeUIHeight+10), WINDOW_WIDTH / 4, timeUIHeight, &timeUIscrollArea);
			imguiSeparatorLine();
			imguiSeparator();
			
			imguiLabel("Time Scroller");
			
			float tmpTime = time/(2*M_PI);
			imguiSlider("time progression", &tmpTime, 0.f, 1.f, 0.001f);
			time = tmpTime*(2*M_PI);
			
			imguiSeparator();
			if(imguiButton("Time Pause (Spacebar)")){
				timePause = timePauseTrigger(timePause);
			}
			
			imguiEndScrollArea();
			/* end Time UI */
			
			/* Details UI */
			imguiBeginScrollArea("Details", 10, WINDOW_HEIGHT - (timeUIHeight+10 + detailsUIHeight+10), WINDOW_WIDTH / 4, detailsUIHeight, &detailsUIscrollArea);
			imguiSeparatorLine();
			imguiSeparator();
			
			toggle = imguiCheck("Vegetation & details (v)", displayVegetation);
			if (toggle)	displayVegetation = !displayVegetation;
			
			toggle = imguiCheck("Fog (g)", displayFog);
			if (toggle)	displayFog = !displayFog;
			
			imguiSeparator();
			imguiLabel("Level of details distance");
			imguiSlider("threshold", &thresholdDistance, 0.f, 30.f, 0.001f);
			
			imguiEndScrollArea();
			/* end Details UI */
			
			/* Cam UI */
			imguiBeginScrollArea("Camera", 10, WINDOW_HEIGHT - (timeUIHeight+10 + detailsUIHeight+10 + camUIHeight+10), WINDOW_WIDTH / 4, camUIHeight, &camUIscrollArea);
			imguiSeparatorLine();
			imguiSeparator();
			
			float modCamSpeed = camSpeed * 100;
			imguiLabel("Camera speed (Mouse wheel)");
			imguiSlider("speed", &modCamSpeed, 0.001f, 10.f, 0.01f);
			camSpeed = modCamSpeed / 100.;
			
			imguiSeparator();
			imguiLabel("Camera type");
			if(imguiItem("TrackBall")) currentCam = TRACK_BALL;
			if(imguiItem("FreeFly")) currentCam = FREE_FLY;
			
			imguiEndScrollArea();
			/* end Cam UI */
			
			/* View UI */
			imguiBeginScrollArea("View", WINDOW_WIDTH - (10 + WINDOW_WIDTH / 4), WINDOW_HEIGHT - (viewUIHeight+10), WINDOW_WIDTH / 4, viewUIHeight, &viewUIscrollArea);
			imguiSeparator();
			imguiSeparatorLine();
			
			imguiLabel("Data view type");
			if(imguiItem("Normal view")){
				displayNormal = true;
				displayDrain = false;
				displayBending = false;
				displaySurface = false;
				displayGradient = false;
			}
			
			if(imguiItem("Bend view", bendItem)){
				displayNormal = false;
				displayDrain = false;
				displayBending = true;
				displaySurface = false;
				displayGradient = false;	
			}
			
			if(imguiItem("Drain view", drainItem)){
				displayNormal = false;
				displayDrain = true;
				displayBending = false;
				displaySurface = false;
				displayGradient = false;
			}
			
			if(imguiItem("Gradient view", gradientItem)){
				displayNormal = false;
				displayDrain = false;
				displayBending = false;
				displaySurface = false;
				displayGradient = true;
			}
			
			if(imguiItem("Surface view", surfaceItem)){
				displayNormal = false;
				displayDrain = false;
				displayBending = false;
				displaySurface = true;
				displayGradient = false;
			}
			
			imguiSeparator();
			toggle = imguiCheck("Debug Mode (f)", displayDebug);
			if(toggle){
				if(displayDebug){
					glUseProgram(terrainProgram);
					sendUniforms(locations, maxCoeffArray, thresholdDistance, terrainScale);
					displayDebug = false;
				}else{
					glUseProgram(debugProgram);
					sendUniforms(locations, maxCoeffArray, thresholdDistance, terrainScale);
					displayDebug = true;
				}
			}
			
			imguiEndScrollArea();
			/* end View UI */
			
			/* INFO */
			imguiBeginScrollArea("Press i to close UI", (WINDOW_WIDTH-130) / 2, WINDOW_HEIGHT - 45, 130, 35, &closeScrollArea);
			imguiEndScrollArea();
			/* end INFO */
			
			/* close UI */
			imguiBeginScrollArea("Quit program ?", WINDOW_WIDTH - (110 + 10), 10, 110, 60, &closeScrollArea);
			if(imguiButton("Quit")){
				done = true;
			}
			
			imguiEndScrollArea();
			/* end close UI */
			
			imguiEndFrame();
			
			imguiRenderGLDraw(WINDOW_WIDTH, WINDOW_HEIGHT);
			
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
		} // end if(ihm)
		
		// Mise à jour de l'affichage
		SDL_GL_SwapBuffers();
			
		if(ihm){
			SDL_WM_GrabInput(SDL_GRAB_OFF);
			SDL_ShowCursor(SDL_ENABLE);
		}else{
			if(currentCam == FREE_FLY){
				SDL_WM_GrabInput(SDL_GRAB_ON);
				SDL_ShowCursor(SDL_DISABLE);
			}else if(currentCam == TRACK_BALL){
				SDL_WM_GrabInput(SDL_GRAB_OFF);
				SDL_ShowCursor(SDL_ENABLE);
			}
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
							if(ihm) imguiRenderGLDestroy();
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
							//change shaders
							if(displayDebug){
								glUseProgram(terrainProgram);
								sendUniforms(locations, maxCoeffArray, thresholdDistance, terrainScale);
								displayDebug = false;
							}else{
								glUseProgram(debugProgram);
								sendUniforms(locations, maxCoeffArray, thresholdDistance, terrainScale);
								displayDebug = true;
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
							
						case SDLK_p:
							if(currentCam == FREE_FLY){
								ffCam.printInfos();
							}
							break;
							
						case SDLK_t:
							if(currentCam == FREE_FLY){
								if(timelaps == false){
									timelaps = true;
									timelapsPosOffset = 0.;
									ffCam.resetView(-0.0418879, 7.99012);
									ffCam.setCameraPosition(glm::vec3(-0.425563,0.09999,-0.230102), 0.);
								}else{
									timelaps = false;
								}
							}
							if(currentCam == TRACK_BALL){
								if(rotationAnim == false){
									rotationAnim = true;
									timelapsPosOffset = 0.;
									tbCam.setCamPos(30., 15., 0.7);
								}else{
									rotationAnim = false;
								}
							}
							break;
							
						case SDLK_i:
							ihm = !ihm;
							break;
						
						case SDLK_v:
							displayVegetation = !displayVegetation;
							break;
						
						case SDLK_g:
							displayFog = !displayFog;
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
							timePause = timePauseTrigger(timePause);
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
							if(ihm){
								is_lClicPressed = true;
							}
						
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
							if(ihm){
								is_lClicPressed = false;
							}
							if(currentCam == TRACK_BALL){
								is_lClicPressed = false;
								tbC_angleX += tbC_tmpAngleX;
								/* constrain trackball camera */
								float sum_angleY = tbC_angleY + tbC_tmpAngleY;
								if(sum_angleY >= 5 && sum_angleY <= 175){
									tbC_angleY = sum_angleY;
								}else if(sum_angleY < 5){
									tbC_angleY = 5;
								}else{
									tbC_angleY = 175;
								}
							}
							
							break;

						default:
							break;
					}
					
					break;
				
				case SDL_MOUSEMOTION:
					if(!ihm){
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
					}
					break;
				
				default:
					break;
			}
		}
		
		//IDLE			
		/* set the ffcam height */
		if(currentCam == FREE_FLY){
			/* Move Camera */
			if(!ihm){
				if(is_lKeyPressed){ ffCam.moveLeft(camSpeed); }
				if(is_rKeyPressed){ ffCam.moveLeft(-camSpeed); }
				if(is_uKeyPressed){ ffCam.moveFront(camSpeed); }
				if(is_dKeyPressed){ ffCam.moveFront(-camSpeed); }
				
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
			
			camPosition = ffCam.getCameraPosition();
			float xtremAltitude;
			float unscaleCamPosX = camPosition.x/terrainScale;
			float unscaleCamPosZ = camPosition.z/terrainScale;
			
			int32_t voxX = (unscaleCamPosX+1.f)*0.5f*total_nbSub;
			int32_t voxZ = (unscaleCamPosZ+1.f)*0.5f*total_nbSub;
			
			if(voxX < 0 || voxX >= total_nbSub || voxZ < 0 || voxZ >= total_nbSub){
				xtremAltitude = maxCoeffArray[5] + halfVoxelSize;
			}else{
				/* get the top current voxel height */
				voxX = voxX%nbSub_lvl2;
				voxZ = voxZ%nbSub_lvl2;
				uint32_t voxY = nbSub_lvl2;
				for(int32_t j=nbSub_lvl2-1;j>=0;--j){
					if(memory[0].voxels[voxX + j*nbSub_lvl2 + voxZ*nbSub_lvl2*nbSub_lvl2].nbFaces != 0){
						break;
					}
					voxY = j;
				}
				xtremAltitude = (voxY*voxelSize+memory[0].pos.y+leafSize*0.8)*terrainScale;
			}
			
			if(camPosition.y < xtremAltitude){
				camPosition.y = xtremAltitude;
			}
			ffCam.setCameraPosition(camPosition, 0);
		}	
		
		/* Sort the memory with camera position */
		std::sort(memory.begin(), memory.end(), memory.front());
		
		//Simulate time
		if(!timePause){
			time += timeStep;
			if(time >= 2*M_PI){
				time = 0.;
			}
		}
		cloudsTime += timeStep;
		
		//Manage the sun
		lightSun.x = cos(time);
		lightSun.y = sin(time);
		
		/* timelaps animation */
		if(timelaps){			
			ffCam.resetView(-0.198968, 4.23068);
			ffCam.setCameraPosition(glm::vec3(0.315615,0.266901-timelapsPosOffset/40.,0.224783), -timelapsPosOffset/20.);
			
			timelapsPosOffset+= 1./2880.;
			if(timelapsPosOffset >= 1.){
				timelaps = false;
			}
		}
		
		if(rotationAnim){
			tbCam.setCamPos(30., 15.+timelapsPosOffset*500., 0.7);
			timelapsPosOffset+= 1./2880.;
			if(timelapsPosOffset >= 1.){
				rotationAnim = false;
			}
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
	
	/* Debind the Cube Map */
	BindCubeMap(0, GL_TEXTURE5);
	
	// Destruction des ressources OpenGL
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &groundVBO);
	glDeleteBuffers(nbVao, l_VBOs);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteVertexArrays(1, &groundVAO);
	glDeleteVertexArrays(nbVao, l_VAOs);
	glDeleteTextures(1, &texture_sky);
	for(uint16_t i=0; i<NB_TEXTURES_VEGET;++i){
		glDeleteTextures(1, &texture_veget[i]);
	}
	for(uint16_t i=0; i<NB_TEXTURES_TERRAIN;++i){
		glDeleteTextures(1, &texture_terrain[i]);
	}
	
	// imgui
	imguiRenderGLDestroy();
	
	//free cache memory
	uint16_t nbLoadedLeaves = memory.size();
	for(uint16_t idx=0;idx<nbLoadedLeaves;++idx){
		freeInMemory(memory, loadedLeaf);
	}
	
	delete[] l_VAOs;
	delete[] l_VBOs;
	for(uint16_t lvl=0;lvl<nbLevel;++lvl){
		delete[] leafArrays[lvl];
	}
	delete[] loadedLeaf;
	delete[] maxCoeffArray;
	delete[] nbLeaves;
	delete[] chunkOffset;
	delete[] locations;
	delete[] skyLocations;

	return (EXIT_SUCCESS);
}
