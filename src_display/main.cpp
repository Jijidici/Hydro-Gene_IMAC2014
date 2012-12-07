#include <iostream>
#include <cstdlib>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <stdint.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imac2gl3/shader_tools.hpp"
#include "types.hpp"


#define FRAME_RATE 60

static const Uint32 MIN_LOOP_TIME = 1000/FRAME_RATE;
static const size_t WINDOW_WIDTH = 600, WINDOW_HEIGHT = 600;
static const size_t BYTES_PER_PIXEL = 32;
static const size_t POSITION_LOCATION = 0;
static const size_t GRID_3D_SIZE = 2;

uint32_t reduceTab(uint32_t nbSub, VoxelData *tabVoxel){

	//NB notation means the old nbSub
	uint32_t nbSubX = nbSub;	// = NBX/2
	uint32_t nbSubY = nbSub;	// = NBY/2
	uint32_t nbSubZ = nbSub;	// = NBZ/2

	uint32_t nbIntersectionMax = 0;
	//line
	uint32_t index=0;
	uint32_t* newTab = new uint32_t[nbSubX*(2*nbSubY)*(2*nbSubZ)]; //NBX is divided in 2
	for(uint32_t i=0;i<(2*nbSubX)*(2*nbSubY)*(2*nbSubZ);i=i+2){ //i runs the entire length of tabVoxel, with a treshold of 2
		newTab[index] = tabVoxel[i].nbFaces+tabVoxel[i+1].nbFaces;
		index++;
	}
	//row
	uint32_t index2=0;
	uint32_t tailleNewTab2 = nbSubX*nbSubY*(2*nbSubZ); //NBY is now divided in 2
	uint32_t* newTab2 = new uint32_t[tailleNewTab2];
	for(uint32_t j=0;j<(2*nbSubY)*(2*nbSubZ);j=j+2){
		for(uint32_t i=j*nbSubX;i<j*nbSubX+nbSubX;++i){
			newTab2[index2] = newTab[i]+newTab[i+nbSubX];
			index2++;
		}
	}
	delete[] newTab;
	//depth
	uint32_t index3=0;
	uint32_t tailleNewTab3 = nbSubX*nbSubY*nbSubZ; //NBZ is now divided in 2
	uint32_t* newTab3 = new uint32_t[tailleNewTab3];
	for(uint32_t j=0;j<2*nbSubZ;j=j+2){
		for(uint32_t i=j*nbSubX*nbSubY;i<j*nbSubX*nbSubY+nbSubX*nbSubY;++i){
			newTab3[index3] = newTab2[i]+newTab2[i+nbSubX*nbSubY];
			index3++;
		}
	}
	delete[] newTab2;
	//update of tabVoxel
	for(uint32_t i=0;i<nbSubX*nbSubY*nbSubZ;++i){
		tabVoxel[i].nbFaces = newTab3[i];
		if(tabVoxel[i].nbFaces > nbIntersectionMax) nbIntersectionMax = tabVoxel[i].nbFaces;
	}
	delete[] newTab3;
	for(uint32_t i=nbSubX*nbSubY*nbSubZ; i<8*nbSubX*nbSubY*nbSubZ; ++i){
		tabVoxel[i].nbFaces = 0;
	}
	return nbIntersectionMax;
}

uint32_t increaseTab(uint32_t nbSub, VoxelData *tabVoxel, uint32_t nbSubMax, VoxelData *tabVoxelMax, uint32_t constNbIntersectionMax){
	
	uint32_t nbSubMaxY = nbSubMax;
	uint32_t nbIntersectionMax = constNbIntersectionMax;
	
	for(uint32_t i=0;i<nbSubMax*nbSubMaxY*nbSubMax;++i){
		tabVoxel[i].nbFaces = tabVoxelMax[i].nbFaces;
	}
	
	if(nbSub != nbSubMax){
		while(nbSubMax>nbSub){
			nbSubMax /= 2;
			nbIntersectionMax = reduceTab(nbSubMax,tabVoxel);
		}
	}
	return nbIntersectionMax;
}

void resetShaderProgram(GLuint &program, GLint &MVPLocation, glm::mat4 &P, glm::mat4 &V, glm::mat4 &VP, GLint &NbIntersectionLocation, GLint &NormSumLocation, GLint &LightVectLocation){
	glUseProgram(program);
	
	// Creation des Matrices
	MVPLocation = glGetUniformLocation(program, "uMVPMatrix");
	
	P = glm::perspective(90.f, WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, 1000.f);
	V = glm::lookAt(glm::vec3(0.f,0.f,0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f,1.f,0.f));
	VP = P*V;
	
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

	uint32_t nbSubMax = 1;
	
	test_fic = fread(&nbSubMax, sizeof(uint32_t), 1, voxelFile);

	uint32_t nbSubMaxY = nbSubMax; //number of subdivisions on Y

	uint32_t lengthTabVoxel = nbSubMax*nbSubMaxY*nbSubMax;
	VoxelData* tabVoxelMax = new VoxelData[lengthTabVoxel];
	test_fic = fread(tabVoxelMax, lengthTabVoxel*sizeof(VoxelData), 1, voxelFile);
	
	uint32_t nbSub = nbSubMax;
	uint32_t nbSubY = nbSubMaxY;
	uint32_t nbSubExpected = nbSub;
	
	
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
		nbIntersectionMax = increaseTab(nbSub, tabVoxel, nbSubMax, tabVoxelMax, constNbIntersectionMax);
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
	GLuint programInter = imac2gl3::loadProgram("shaders/basic.vs.glsl", "shaders/basic.fs.glsl");
	GLuint programNorm = imac2gl3::loadProgram("shaders/basic.vs.glsl", "shaders/norm.fs.glsl");
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
	glm::mat4 V = glm::lookAt(glm::vec3(0.f,0.f,0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f,1.f,0.f));
	glm::mat4 VP = P*V;
	
	// Recuperation des variables uniformes
	GLint NbIntersectionLocation = glGetUniformLocation(program, "uNbIntersection");
	GLint NormSumLocation = glGetUniformLocation(program, "uNormSum");
	GLint LightVectLocation = glGetUniformLocation(program, "uLightVect");
	
	glm::vec3 light(-1.f,0.5f,0.f);
	float lightAngle = 0.05;
	
	//light = glm::translate(light, glm::vec3(0.f,1.f,0.f));
	
	// Creation des ressources OpenGL
	glEnable(GL_DEPTH_TEST);
	
	//Creation des ressources d'evenements
	float offsetViewX = 0.;
	float offsetViewY = 0.;
	float offsetViewZ = 0.;
	float angleViewY = 0.;
	float tmpAngleViewY = 0.;
	float angleViewX = 0.;
	float tmpAngleViewX = 0.;
	uint8_t isArrowKeyUpPressed = 0;
	uint8_t isArrowKeyDownPressed = 0;
	uint8_t isArrowKeyLeftPressed = 0;
	uint8_t isArrowKeyRightPressed = 0;
	uint8_t isLeftClicPressed = 0;
	uint16_t savedClicX = -1;
	uint16_t savedClicY = -1;
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

		// Nettoyage de la fenêtre
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
		if(changeNbSubPlus){ //si on a appuyé sur +
			std::cout <<"-> New number of subdivisions : "<<nbSub<<std::endl;
			nbIntersectionMax = increaseTab(nbSub,tabVoxel,nbSubMax,tabVoxelMax, constNbIntersectionMax);
			changeNbSubPlus = false;
		}
		
		if(changeNbSubMinus){ //si on a appuyé sur -
			std::cout << " > New number of subdivisions : " << nbSub << std::endl;
			nbIntersectionMax = reduceTab(nbSub,tabVoxel);
			changeNbSubMinus = false;
		}
			
		double cubeSize = GRID_3D_SIZE/(double)nbSub;

		glm::mat4 MVP = glm::translate(VP, glm::vec3(offsetViewX, offsetViewY, offsetViewZ)); //MOVE WITH ARROWKEYS & ZOOM WITH SCROLL
		MVP = glm::translate(MVP, glm::vec3(0.f, 0.f, -5.f)); //MOVE AWWAY FROM THE CAMERA
		MVP = glm::rotate(MVP, angleViewX + tmpAngleViewX,  glm::vec3(0.f, 1.f, 0.f)); //ROTATE WITH XCOORDS CLIC
		MVP = glm::rotate(MVP, angleViewY + tmpAngleViewY,  glm::vec3(1.f, 0.f, 0.f)); //ROTATE WITH YCOORDS CLIC
		
		nbSubY = nbSub;
		
		
		
		light.x += lightAngle;
		//light.z += lightAngle;
		
		glUniform3f(LightVectLocation, light.x, light.y, light.z);
		
		// Affichage de la grille
		for(uint32_t k=0;k<nbSub;++k){
			for(uint32_t j=0;j<nbSubY;++j){
				for(uint32_t i=0;i<nbSub;++i){
					uint32_t currentIndex = k*nbSub*nbSubY + j*nbSub + i;
					uint32_t currentNbIntersection = tabVoxel[k*nbSub*nbSubY + j*nbSub + i].nbFaces;
					if(currentNbIntersection != 0){
						//std::cout << "somme normales cube " << currentIndex << " : " << tabVoxel[currentIndex].sumNormal.x << " " << tabVoxel[currentIndex].sumNormal.y << " " <<tabVoxel[currentIndex].sumNormal.z << " " << std::endl;
						//std::cout << "nombre d'intersection cube " << currentIndex << " : " << tabVoxel[currentIndex].nbFaces << std::endl;
						glm::mat4 aCubeMVP = glm::translate(MVP, glm::vec3(i*cubeSize-(GRID_3D_SIZE-cubeSize)/2, j*cubeSize-(GRID_3D_SIZE-cubeSize)/2, k*cubeSize-(GRID_3D_SIZE-cubeSize)/2)); //PLACEMENT OF EACH GRID CUBE
						aCubeMVP = glm::scale(aCubeMVP, glm::vec3(cubeSize)); // RE-SCALE EACH GRID CUBE
						
						glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(aCubeMVP));
						glUniform2i(NbIntersectionLocation, currentNbIntersection, nbIntersectionMax);
						glUniform3f(NormSumLocation, tabVoxel[currentIndex].sumNormal.x, tabVoxel[currentIndex].sumNormal.y, tabVoxel[currentIndex].sumNormal.z);
					
						glBindVertexArray(cubeVAO);
							glDrawArrays(GL_TRIANGLES, 0, aCube.nbVertices);
						glBindVertexArray(0);
					}
				}
			}
		}	
		
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
						case SDLK_q:
							done=true;
						break;
						
						case SDLK_LEFT:
							isArrowKeyLeftPressed = 1;
						break;
						
						case SDLK_RIGHT:
							isArrowKeyRightPressed = 1;
						break;
						
						case SDLK_UP:
							isArrowKeyUpPressed = 1;
						break;
						
						case SDLK_DOWN:
							isArrowKeyDownPressed = 1;
						break;
						
						case SDLK_KP_PLUS:
							if(nbSub != nbSubMax){
								nbSub *= 2;
								changeNbSubPlus = true;
							}else std::cout << "You reached the maximum number of subdivisions." << std::endl;						
						break;
						
						case SDLK_KP_MINUS:
							if(nbSub != 1){
								nbSub /= 2;
								changeNbSubMinus = true;
							}else std::cout << "You reached the minimum number of subdivisions." << std::endl;						
						break;
						
						case SDLK_p:
							if(nbSub != nbSubMax){
								nbSub *= 2;
								changeNbSubPlus = true;
							}else std::cout << "You reached the maximum number of subdivisions." << std::endl;						
						break;
						
						case SDLK_m:
							if(nbSub != 1){
								nbSub /= 2;
								changeNbSubMinus = true;
							}else std::cout << "You reached the minimum number of subdivisions." << std::endl;						
						break;
						case SDLK_SPACE:
						break;
						
						case SDLK_n:
							resetShaderProgram(programNorm, MVPLocation, P, V, VP, NbIntersectionLocation, NormSumLocation, LightVectLocation);
							break;
							
						case SDLK_i:
							resetShaderProgram(programInter, MVPLocation, P, V, VP, NbIntersectionLocation, NormSumLocation, LightVectLocation);
							break;
						
						default:
						break;
					}
				break;
				
				case SDL_KEYUP:
					switch(e.key.keysym.sym){
						case SDLK_LEFT:
							isArrowKeyLeftPressed = 0;
						break;
						
						case SDLK_RIGHT:
							isArrowKeyRightPressed = 0;
						break;
						
						case SDLK_UP:
							isArrowKeyUpPressed = 0;
						break;
						
						case SDLK_DOWN:
							isArrowKeyDownPressed = 0;
						break;
						
						default:
						break;
					}
				break;
				
				case SDL_MOUSEBUTTONDOWN:
					switch(e.button.button){
						case SDL_BUTTON_WHEELUP:
							offsetViewZ += 0.3;
						break;
						
						case SDL_BUTTON_WHEELDOWN:
							offsetViewZ -= 0.3;
						break;
						
						case SDL_BUTTON_LEFT:
							isLeftClicPressed = 1;
							savedClicX = e.button.x;
							savedClicY = e.button.y;
						break;
						
						default:
						break;
					}
				break;
				
				case SDL_MOUSEBUTTONUP:
					isLeftClicPressed = 0;
					savedClicX = -1;
					savedClicY = -1;
					angleViewX += tmpAngleViewX;
					angleViewY += tmpAngleViewY;
					tmpAngleViewX=0;
					tmpAngleViewY=0;
				break;
				
				case SDL_MOUSEMOTION:
					if(isLeftClicPressed){
						tmpAngleViewX =  0.25f*(e.motion.x - savedClicX);
						tmpAngleViewY =  0.25f*(e.motion.y - savedClicY);
					}
				break;
				
				default:
				break;
			}
		}

		//Idle
		if(isArrowKeyLeftPressed){
			offsetViewX += 0.05;
		}
		
		if(isArrowKeyRightPressed){
			offsetViewX -= 0.05;
		}
		
		if(isArrowKeyUpPressed){
			offsetViewY -= 0.05;
		}
		
		if(isArrowKeyDownPressed){
			offsetViewY += 0.05;
		}
		
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
