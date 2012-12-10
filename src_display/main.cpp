#include <iostream>
#include <cstdlib>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <stdint.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "types.hpp"
#include "tools/shader_tools.hpp"
#include "tools/MatrixStack.hpp"
#include "cameras/TrackBallCamera.hpp"

#define FRAME_RATE 60

static const Uint32 MIN_LOOP_TIME = 1000/FRAME_RATE;
static const size_t WINDOW_WIDTH = 600, WINDOW_HEIGHT = 600;
static const size_t BYTES_PER_PIXEL = 32;
static const size_t POSITION_LOCATION = 0;
static const size_t GRID_3D_SIZE = 2;

uint32_t reduceTab(uint32_t nbSub, VoxelData *tabVoxel, uint32_t displayMode){

	//NB notation means the old nbSub
	uint32_t nbSubX = nbSub;	// = NBX/2
	uint32_t nbSubY = nbSub;	// = NBY/2
	uint32_t nbSubZ = nbSub;	// = NBZ/2

	uint32_t nbIntersectionMax = 0;
	//line
	uint32_t index=0;
	VoxelData* newTab = new VoxelData[nbSubX*(2*nbSubY)*(2*nbSubZ)]; //NBX is divided in 2
	for(uint32_t i=0;i<(2*nbSubX)*(2*nbSubY)*(2*nbSubZ);i=i+2){ //i runs the entire length of tabVoxel, with a treshold of 2
		newTab[index].nbFaces = tabVoxel[i].nbFaces+tabVoxel[i+1].nbFaces;
		if(displayMode == 1) newTab[index].sumBending = tabVoxel[i].sumBending+tabVoxel[i+1].sumBending;
		if(displayMode == 2) newTab[index].sumDrain = tabVoxel[i].sumDrain+tabVoxel[i+1].sumDrain;
		if(displayMode == 3) newTab[index].sumGradient = tabVoxel[i].sumGradient+tabVoxel[i+1].sumGradient;
		if(displayMode == 4) newTab[index].sumNormal = tabVoxel[i].sumNormal+tabVoxel[i+1].sumNormal;
		if(displayMode == 5) newTab[index].sumSurface = tabVoxel[i].sumSurface+tabVoxel[i+1].sumSurface;
		index++;
	}
	//row
	uint32_t index2=0;
	uint32_t tailleNewTab2 = nbSubX*nbSubY*(2*nbSubZ); //NBY is now divided in 2
	VoxelData* newTab2 = new VoxelData[tailleNewTab2];
	for(uint32_t j=0;j<(2*nbSubY)*(2*nbSubZ);j=j+2){
		for(uint32_t i=j*nbSubX;i<j*nbSubX+nbSubX;++i){
			newTab2[index2].nbFaces = newTab[i].nbFaces+newTab[i+nbSubX].nbFaces;
			if(displayMode == 1) newTab2[index2].sumBending = newTab[i].sumBending+newTab[i+nbSubX].sumBending;
			if(displayMode == 2) newTab2[index2].sumDrain = newTab[i].sumDrain+newTab[i+nbSubX].sumDrain;
			if(displayMode == 3) newTab2[index2].sumGradient = newTab[i].sumGradient+newTab[i+nbSubX].sumGradient;
			if(displayMode == 4) newTab2[index2].sumNormal = newTab[i].sumNormal+newTab[i+nbSubX].sumNormal;
			if(displayMode == 5) newTab2[index2].sumSurface = newTab[i].sumSurface+newTab[i+nbSubX].sumSurface;
			index2++;
		}
	}
	delete[] newTab;
	//depth
	uint32_t index3=0;
	uint32_t tailleNewTab3 = nbSubX*nbSubY*nbSubZ; //NBZ is now divided in 2
	VoxelData* newTab3 = new VoxelData[tailleNewTab3];
	for(uint32_t j=0;j<2*nbSubZ;j=j+2){
		for(uint32_t i=j*nbSubX*nbSubY;i<j*nbSubX*nbSubY+nbSubX*nbSubY;++i){
			newTab3[index3].nbFaces = newTab2[i].nbFaces+newTab2[i+nbSubX*nbSubY].nbFaces;
			if(displayMode == 1) newTab3[index3].sumBending = newTab2[i].sumBending+newTab2[i+nbSubX*nbSubY].sumBending;
			if(displayMode == 2) newTab3[index3].sumDrain = newTab2[i].sumDrain+newTab2[i+nbSubX*nbSubY].sumDrain;
			if(displayMode == 3) newTab3[index3].sumGradient = newTab2[i].sumGradient+newTab2[i+nbSubX*nbSubY].sumGradient;
			if(displayMode == 4) newTab3[index3].sumNormal = newTab2[i].sumNormal+newTab2[i+nbSubX*nbSubY].sumNormal;
			if(displayMode == 5) newTab3[index3].sumSurface = newTab2[i].sumSurface+newTab2[i+nbSubX*nbSubY].sumSurface;
			index3++;
		}
	}
	delete[] newTab2;
	//update of tabVoxel
	for(uint32_t i=0;i<nbSubX*nbSubY*nbSubZ;++i){
		tabVoxel[i].nbFaces = newTab3[i].nbFaces;
		if(displayMode == 1) tabVoxel[i].sumBending = newTab3[i].sumBending;
		if(displayMode == 2) tabVoxel[i].sumDrain = newTab3[i].sumDrain;
		if(displayMode == 3) tabVoxel[i].sumGradient = newTab3[i].sumGradient;
		if(displayMode == 4) tabVoxel[i].sumNormal = newTab3[i].sumNormal;
		if(displayMode == 5) tabVoxel[i].sumSurface = newTab3[i].sumSurface;
		if(tabVoxel[i].nbFaces > nbIntersectionMax) nbIntersectionMax = tabVoxel[i].nbFaces;
	}
	delete[] newTab3;
	for(uint32_t i=nbSubX*nbSubY*nbSubZ; i<8*nbSubX*nbSubY*nbSubZ; ++i){
		tabVoxel[i].nbFaces = 0;
		if(displayMode == 1) tabVoxel[i].sumBending = 0;
		if(displayMode == 1) tabVoxel[i].sumDrain = 0;
		if(displayMode == 1) tabVoxel[i].sumGradient = 0;
		if(displayMode == 1) tabVoxel[i].sumNormal = glm::dvec3(0,0,0);
		if(displayMode == 1) tabVoxel[i].sumSurface = 0;
	}
	return nbIntersectionMax;
}

uint32_t increaseTab(uint32_t nbSub, VoxelData *tabVoxel, uint32_t nbSubMax, VoxelData *tabVoxelMax, uint32_t constNbIntersectionMax, uint32_t displayMode){
	
	uint32_t nbSubMaxY = nbSubMax;
	uint32_t nbIntersectionMax = constNbIntersectionMax;
	
	for(uint32_t i=0;i<nbSubMax*nbSubMaxY*nbSubMax;++i){
		tabVoxel[i].nbFaces = tabVoxelMax[i].nbFaces;
		tabVoxel[i].sumNormal = tabVoxelMax[i].sumNormal;
	}
	
	if(nbSub != nbSubMax){
		while(nbSubMax>nbSub){
			nbSubMax /= 2;
			nbIntersectionMax = reduceTab(nbSubMax,tabVoxel,displayMode);
		}
	}
	return nbIntersectionMax;
}

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

	uint32_t arguments[6];
		//0 : nbSub
		//1 : bending
		//2 : drain
		//3 : gradient
		//4 : normal
		//5 : surface

	test_fic = fread(arguments, sizeof(uint32_t), 6, voxelFile);
	uint32_t nbSubMax = arguments[0];
	uint32_t nbSubMaxY = nbSubMax; //number of subdivisions on Y

	uint32_t lengthTabVoxel = nbSubMax*nbSubMaxY*nbSubMax;
	VoxelData* tabVoxelMax = new VoxelData[lengthTabVoxel];
	test_fic = fread(tabVoxelMax, lengthTabVoxel*sizeof(VoxelData), 1, voxelFile);
	
	uint32_t nbSub = nbSubMax;
	uint32_t nbSubY = nbSubMaxY;
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
	
	if(argc > 1){
		if(atoi(argv[1]) <= (int) nbSubMax){
			nbSubExpected = atoi(argv[1]);
			
			uint32_t test = nbSubExpected;
			uint32_t power = 0;
			
			while(test > 1){
				test = test/2;
				++power;
			}
			
			uint32_t nbLow = pow(2,power);
			uint32_t nbUp = pow(2,power+1);
			
			if(nbSubExpected - nbLow < nbUp - nbSubExpected){
				nbSubExpected = nbLow;
			}else{
				nbSubExpected = nbUp;
			}
			
			if(nbSubExpected == 0){
				nbSubExpected = nbSubMax;
				std::cout << "-> ! nbSub = 0, nbSub initialisé à " << nbSubMax << std::endl;
			}else{
				std::cout << "-> Nombre de subdivisions arrondi à la puissance de 2 la plus proche" << std::endl;
			}
		}else{
			std::cout << "précision demandée supérieure au nb de subdivs max" << std::endl;
		}
	}
	
	std::cout << "-> nbSub : " << nbSubExpected << std::endl;
	
	
	fclose(voxelFile);

	uint32_t nbIntersectionMax = 0;
	for(uint32_t i=0; i<lengthTabVoxel;++i){
		if(tabVoxel[i].nbFaces>nbIntersectionMax){
			nbIntersectionMax = tabVoxel[i].nbFaces;
		}
	}
	uint32_t constNbIntersectionMax = nbIntersectionMax;
	
	
	while(nbSub > nbSubExpected){
		nbSub /= 2;
		nbSubY /=2;
		nbIntersectionMax = increaseTab(nbSub, tabVoxel, nbSubMax, tabVoxelMax, constNbIntersectionMax, displayMode);
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
	hydrogene::TrackBallCamera tbCam;

	// Creation des ressources OpenGL
	glEnable(GL_DEPTH_TEST);
	
	//Creation des ressources d'evenements
	bool is_lClicPressed = false;
	uint32_t savedClicX = 0;
	uint32_t savedClicY = 0;
	float angleX = 0;
	float angleY = 0;
	float tmpAngleX = 0;
	float tmpAngleY = 0;
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
		nbSubY = nbSub;
		double cubeSize = GRID_3D_SIZE/(double)nbSub;

		// Nettoyage de la fenêtre
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
		if(changeNbSubPlus){ //si on a appuyé sur +
			std::cout <<"-> New number of subdivisions : "<<nbSub<<std::endl;
			nbIntersectionMax = increaseTab(nbSub,tabVoxel,nbSubMax,tabVoxelMax, constNbIntersectionMax, displayMode);
			changeNbSubPlus = false;
		}
		
		if(changeNbSubMinus){ //si on a appuyé sur -
			std::cout << " > New number of subdivisions : " << nbSub << std::endl;
			nbIntersectionMax = reduceTab(nbSub,tabVoxel, displayMode);
			changeNbSubMinus = false;
		}



		ms.push();
			glm::mat4 V = tbCam.getViewMatrix();
			ms.mult(V);
			ms.translate(glm::vec3(-1.f, -1.f, -1.f)); //CENTER TO THE ORIGIN
			glUniform3f(LightVectLocation, light.x, light.y, light.z);
			
			// Affichage de la grille
			for(uint32_t k=0;k<nbSub;++k){
				for(uint32_t j=0;j<nbSubY;++j){
					for(uint32_t i=0;i<nbSub;++i){
						uint32_t currentIndex = k*nbSub*nbSubY + j*nbSub + i;
						uint32_t currentNbIntersection = tabVoxel[k*nbSub*nbSubY + j*nbSub + i].nbFaces;
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
							if(nbSub != nbSubMax){
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
						
						case SDLK_n:
							if(arguments[4]){
								resetShaderProgram(programNorm, MVPLocation, NbIntersectionLocation, NormSumLocation, LightVectLocation);
								displayMode = 4;
							}
							break;
							
						case SDLK_i:
							resetShaderProgram(programInter, MVPLocation, NbIntersectionLocation, NormSumLocation, LightVectLocation);
							displayMode = 0;
							break;
						
						default:
						break;
					}
				break;
				
				case SDL_KEYUP:
					switch(e.key.keysym.sym){						
						default:
						break;
					}
				break;
				
				case SDL_MOUSEBUTTONDOWN:
					switch(e.button.button){
						case SDL_BUTTON_WHEELUP:
							tbCam.moveFront(-0.08f);
						break;
						
						case SDL_BUTTON_WHEELDOWN:
							tbCam.moveFront(0.08f);
						break;
						
						case SDL_BUTTON_LEFT:
							is_lClicPressed = true;
							savedClicX = e.button.x;
							savedClicY = e.button.y;
						break;
						
						default:
						break;
					}
				break;
				
				case SDL_MOUSEBUTTONUP:
					switch(e.button.button){
						case SDL_BUTTON_LEFT:
							is_lClicPressed = false;
							angleX += tmpAngleX;
							angleY += tmpAngleY;
						break;

						default:
						break;
					}
					
				break;
				
				case SDL_MOUSEMOTION:
					if(is_lClicPressed){
						tmpAngleX = 0.25*((int)e.motion.x - (int)savedClicX);
						tmpAngleY = 0.25*((int)e.motion.y - (int)savedClicY);
						tbCam.rotateLeft(angleX + tmpAngleX);
						tbCam.rotateUp(angleY + tmpAngleY);
					}
				break;
				
				default:
				break;
			}
		}

		//IDLE
		
		//ILLUMINATION
		light.x += lightAngle;

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
