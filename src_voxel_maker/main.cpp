#include <iostream>
#include <cstdlib>
#include <cmath>
#include <stdexcept>
#include <glm/glm.hpp>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <omp.h>
#include "geom_types.hpp"
#include "data_types.hpp"
#include "voxel_maker/geometrics.hpp"
#include "voxel_maker/intersection_test.hpp"

#include "drn/drn_writer.h"

static const double APPROXIM_RANGE = 2*sqrt(3)/3;
static const size_t GRID_3D_SIZE = 2;

glm::dvec3 triangleCubefaceIntersection(glm::dvec3 average_current, glm::dvec3 average_compared, uint16_t face, glm::dvec3 position_current, double leafSize){

	double t = 0;
	glm::dvec3 normal;

	switch(face){

		case 0 : //right cube face
			normal = glm::dvec3(1.f, 0.f, 0.f);
			if(glm::dot(normal,average_current) != 0)	t = (position_current.x+leafSize - average_current.x)/(average_compared.x - average_current.x);
			break;

		case 1 : //near cube face
			normal = glm::dvec3(0.f,0.f,1.f);
			if(glm::dot(normal,average_current) != 0) t = (position_current.y+leafSize - average_current.y)/(average_compared.y - average_current.y);
			break;

		case 2 : //top cube face
			normal = glm::dvec3(0.f,1.f,0.f);
			if(glm::dot(normal,average_current) != 0) t = (position_current.z+leafSize - average_current.z)/(average_compared.z - average_current.z);
			break;

	}

	double x_inter = average_current.x + t*(average_compared.x - average_current.x);
	double y_inter = average_current.y + t*(average_compared.y - average_current.y);
	double z_inter = average_current.z + t*(average_compared.z - average_current.z);

	return glm::dvec3(x_inter,y_inter,z_inter);
}

/******************************************/
/*          FUNCTIONS                     */
/******************************************/

void printHelp(){
	std::cout << std::endl << "############## HELP ##############" << std::endl;
	std::cout << "usage : ./bin/hg_voxel_maker <number of subdivisions> -<option>" << std::endl << std::endl;
	std::cout << "You can put several options at the same time. Options available :" << std::endl<<std::endl;
	std::cout << "-vtx : testing the intersections regarding the vertices of the faces" << std::endl;	
	std::cout << "-edg : testing the intersections regarding the edges of the faces" << std::endl;	
	std::cout << "-pln : testing the intersections regarding the planes of the faces" << std::endl;	
	std::cout << "#DEFAULT : intersections regarding the vertices, the edges and the planes." << std::endl<<std::endl;	
	std::cout << "-n : saving the faces normals in the voxel_data file" << std::endl;
	std::cout << "-b : saving the bending coefficients in the voxel_data file" << std::endl;
	std::cout << "-g : saving the faces gradients in the voxel_data file" << std::endl;
	std::cout << "-s : saving the surfaces in the voxel_data file" << std::endl;
	std::cout << "-d : saving the drain coefficients in the voxel_data file" << std::endl;
	std::cout << "##################################" << std::endl << std::endl;
}
/*************************************/
/*             MAIN                  */
/*************************************/
int main(int argc, char** argv) {
	/* ************************************************************* */
	/* **************PRE - TRAITEMENT DES VOXELS******************** */
	/* ************************************************************* */
	
	std::cout << std::endl << "############################# INITIALISATION #############################" << std::endl;
	
	//getting the arguments

	uint16_t nbSub_lvl1 = 0;
	uint16_t nbSub_lvl2 = 0;
	uint16_t p4Requested = 0;
	uint16_t drain = 0;
	uint16_t gradient = 0;
	uint16_t surface = 0;
	uint16_t bending = 0;
	uint16_t normal = 0;
	uint16_t mode = 0;
	
	//Managing of the console arguments
	if(argc > 1){
		char** tabArguments = new char*[argc];

		for(uint16_t i = 0; i<argc-1; ++i){

			tabArguments[i] = argv[i+1];

			//if a number of subdivisions has been entered
			if(atoi(tabArguments[i])){
				if(!nbSub_lvl1) nbSub_lvl1 = atoi(tabArguments[i]);
				else nbSub_lvl2 = atoi(tabArguments[i]);
			}
			else if(strcmp(tabArguments[i],"help") == 0){
				printHelp();
				return(EXIT_SUCCESS);
			}

			//intersection with vertices
			else if (strcmp(tabArguments[i], "-vtx") == 0){
				if(!normal && !p4Requested && mode==0) std::cout << "############ REQUESTS ############" << std::endl;
				std::cout << "-> Requested : intersections regarding the vertices" << std::endl;
				mode = 1;
			}
			//intersection with edges
			else if (strcmp(tabArguments[i], "-edg") == 0){
				if(!normal && !p4Requested && mode==0) std::cout << "############ REQUESTS ############" << std::endl;
				std::cout << "-> Requested : intersections regarding the edges" << std::endl;
				mode = 2;
			}
			//intersection with planes
			else if (strcmp(tabArguments[i], "-pln") == 0){
				if(!normal && !p4Requested && mode==0) std::cout << "############ REQUESTS ############" << std::endl;
				std::cout << "-> Requested : intersections regarding the planes" << std::endl;
				mode = 3;
			}

			//normals requested
			else if(strcmp(tabArguments[i],"-n") == 0){
				if(!normal && !p4Requested && mode==0) std::cout << std::endl << "############ REQUESTS ############" << std::endl;
				std::cout << "-> Requested : loading of the normals" << std::endl;
				normal = 1;
			}

			else if ((strcmp(tabArguments[i],"-d") == 0)||(strcmp(tabArguments[i],"-b") == 0)||(strcmp(tabArguments[i],"-g") == 0)||(strcmp(tabArguments[i],"-s") == 0)) {
				if(!normal && !p4Requested && mode==0) std::cout << "############ REQUESTS ############" << std::endl;
				p4Requested = 1;
				//drain coeff requested
				if (strcmp(tabArguments[i],"-d") == 0){
					std::cout << "-> Requested : loading of the drain coefficients" << std::endl;
					drain = 1;
				}
				//bending coeff requested
				else if (strcmp(tabArguments[i],"-b") == 0){
					std::cout << "-> Requested : loading of the bending coefficients" << std::endl;
					bending = 1;
				}
				//surfaces requested
				else if (strcmp(tabArguments[i],"-s") == 0){
					std::cout << "-> Requested : loading of the surfaces" << std::endl;
					surface = 1;
				}
				//gradients requested
				else{
					std::cout << "-> Requested : loading of the gradient coefficients" << std::endl;
					gradient = 1;
				}
			}
			else{
				std::cout << std::endl << "[!] -> Warning : the request \"" << tabArguments[i] << "\" does not exist.";
				printHelp();
			}
		}
		delete[] tabArguments;
		if(p4Requested||normal||mode!=0) std::cout << "##################################" << std::endl << std::endl;
	}else {
		printHelp();
		return EXIT_FAILURE;
	}
	
	//BEGIN LOADING DATA
	//CHARGEMENT PAGE1.DATA

	size_t test_fic = 0;

	FILE *dataFile = NULL;
	dataFile = fopen("terrain_data/page_1.data", "rb");
	if(NULL == dataFile){
		std::cout << "[!] > Unable to load the file dataFile" << std::endl;
		return EXIT_FAILURE;
	}
	
	//lecture du nombre de vertex et de faces, puis affichage ds la console
	uint32_t nbVertice = 0, nbFace = 0;	
	test_fic = fread(&nbVertice, sizeof(nbVertice), 1, dataFile);
	test_fic =fread(&nbFace, sizeof(nbFace), 1, dataFile);

	// altitudes min et max de la carte
	double altMin = 0;
	double altMax = 0;
	
	double * positionsData = new double[3*nbVertice];
	test_fic = fread(positionsData, sizeof(double), 3*nbVertice, dataFile); // to read the positions of the vertices
	
	uint32_t * facesData = new uint32_t[3*nbFace];
	test_fic = fread(facesData, sizeof(uint32_t), 3*nbFace, dataFile); // to read the indexes of the vertices which compose each face
	
	fclose(dataFile);
	
	//CHARGEMENT PAGE2.DATA
	FILE* normalFile = NULL;
	normalFile = fopen("terrain_data/page_2.data", "rb");
	if(NULL == normalFile){
		std::cout << "[!] -> Unable to load the second data page" << std::endl;
		return EXIT_FAILURE;
	}

	double * normalData = new double[3*nbFace+3*nbVertice];
	test_fic = fread(normalData, sizeof(double), 3*nbFace+3*nbVertice, normalFile);
	
	fclose(normalFile);
	
	//CHARGEMENT PAGE4.DATA
	int* drainData = NULL;
	double* otherData = NULL;
	if(p4Requested){
		drainData = new int[nbFace];
		otherData = new double[3*nbFace];

		FILE* page4File = NULL;
		page4File = fopen("terrain_data/page_4.data", "rb");
		if(NULL == page4File){
			std::cout << "[!] -> Unable to load the file fourth data page" << std::endl;
			return EXIT_FAILURE;
		}

		//moving to the beginning of the other data in the file
		test_fic = fseek(page4File, nbVertice*(sizeof(int)+3*sizeof(double)), SEEK_SET);
		if(test_fic != 0){
			std::cout<<"[!] -> Unable to move inside the fourth data page"<<std::endl;
			return EXIT_FAILURE;
		}
	
		for(uint32_t n=0;n<nbFace;++n){
			test_fic = fread(&(drainData[n]), sizeof(int), 1, page4File);
			test_fic = fread(&(otherData[3*n]), sizeof(double), 3, page4File);
		}
		
		fclose(page4File);
	}
	
	//CONSTRUCTION OF THE DATA STRUCTURES
	uint32_t cptNormals = 0;
	
	//VERTICES
	Vertex * tabV = new Vertex[nbVertice];
	
	for(uint32_t n=0;n<nbVertice;++n){ // to create the vertices tab
		tabV[n].pos.x = positionsData[3*n];
		tabV[n].pos.z = positionsData[3*n+1];
		tabV[n].pos.y = positionsData[3*n+2];
		tabV[n].normal.x = normalData[3*cptNormals];
		tabV[n].normal.z = normalData[3*cptNormals+1];
		tabV[n].normal.y = normalData[3*cptNormals+2];
		cptNormals++;
		
		// on récupère les altitudes extremes
		if(tabV[n].pos.y > altMax){
			altMax = tabV[n].pos.y;
		}else{
			if(tabV[n].pos.y < altMin){
				altMin = tabV[n].pos.y;
			}
		}
	}
	
	//FACES
	Face * tabF = new Face[nbFace];
	uint32_t vertexCoordsOffset[3];
	
	for(uint32_t n=0;n<nbFace;++n){
		for(size_t i = 0; i < 3; ++i){
			vertexCoordsOffset[i] = facesData[3*n+i];
		}
		tabF[n].s1 = tabV + vertexCoordsOffset[0] -1;
		tabF[n].s2 = tabV + vertexCoordsOffset[1] -1;
		tabF[n].s3 = tabV + vertexCoordsOffset[2] -1;
		tabF[n].normal.x = normalData[3*cptNormals];
		tabF[n].normal.z = normalData[3*cptNormals+1];
		tabF[n].normal.y = normalData[3*cptNormals+2];
		cptNormals++;
		
		if(p4Requested && drain){
			tabF[n].drain = drainData[n];
		}else{
			tabF[n].drain = 0;
		}
		
		if(p4Requested && bending){	
			tabF[n].bending = otherData[n*3];
		}else{
			tabF[n].bending = 0;
		}
		
		if(p4Requested && surface){
			tabF[n].surface = otherData[n*3+1];
		}else{
			tabF[n].surface = 0;
		}
		
		if(p4Requested && gradient){
			tabF[n].gradient = otherData[n*3+2];
		}else{
			tabF[n].gradient = 0;
		}
	}

	delete[] positionsData;
	delete[] facesData;
	delete[] normalData;
	if(p4Requested){
		delete[] drainData;
		delete[] otherData;
	}
	//END LOADING DATA
	
	uint16_t test_lvl1 = nbSub_lvl1;
	uint16_t test_lvl2 = nbSub_lvl2;
	uint16_t power_lvl1 = 0;
	uint16_t power_lvl2 = 0;
	
	while(test_lvl1 > 1){
		test_lvl1 = test_lvl1/2;
		++power_lvl1;
	}
	while(test_lvl2 > 1){
		test_lvl2 = test_lvl2/2;
		++power_lvl2;
	}
	
	std::cout << "-> Number of vertices : " << nbVertice << std::endl;
	std::cout << "-> Number of faces : " << nbFace << std::endl;
	std::cout <<"-> Altitude max : "<< altMax<< " - Altitude min : "<< altMin << std::endl << std::endl;

	if(nbSub_lvl1 == 0){
		nbSub_lvl1 = 16;
		std::cout << "[!] -> nbSub_lvl1 = 0, Number of subdivisions initialized to 16" << std::endl;
	}else{
		uint16_t nbLow = pow(2,power_lvl1);
		uint16_t nbUp = pow(2,power_lvl1+1);
	
		if(nbSub_lvl1 - nbLow < nbUp - nbSub_lvl1){
			nbSub_lvl1 = nbLow;
		}else{
			nbSub_lvl1 = nbUp;
		}
		std::cout << "-> Number of subdivisions rounded to the closest power of two" << std::endl;
	}

	if(nbSub_lvl2 == 0){
		nbSub_lvl2 = 16;
		std::cout << "[!] -> nbSub_lvl2 = 0, Number of subdivisions initialized to 16" << std::endl;
	}else{
		uint16_t nbLow = pow(2,power_lvl2);
		uint16_t nbUp = pow(2,power_lvl2+1);
	
		if(nbSub_lvl2 - nbLow < nbUp - nbSub_lvl2){
			nbSub_lvl2 = nbLow;
		}else{
			nbSub_lvl2 = nbUp;
		}
		std::cout << "-> Number of subdivisions rounded to the closest power of two" << std::endl;
	}
	
	std::cout << "-> Number of subdivisions : " << nbSub_lvl1 << std::endl;
	std::cout << "-> Number of leave subdivisions : " << nbSub_lvl2 << std::endl;
	std::cout << std::endl << "##########################################################################" << std::endl << std::endl;
	
	glm::dvec3 inter =  triangleCubefaceIntersection(glm::dvec3(0.4f,0.f,1.f), glm::dvec3(2.7f,1.f,0.3f), 0, glm::dvec3(0.f,0.f,0.f), 2.0);

	/* Grid from level 1 : grid of leaves */
	double l_size = GRID_3D_SIZE/(double)nbSub_lvl1;
	
	/* Grid from level 2 : grid of voxel inside a leaf */
	double voxelSize = l_size/(double)nbSub_lvl2;
	
	size_t const l_voxArrLength = nbSub_lvl2*nbSub_lvl2*nbSub_lvl2;
	VoxelData* l_voxelArray = NULL;
	l_voxelArray = new VoxelData[l_voxArrLength];
	if(NULL == l_voxelArray){
		std::cout<<"[!] -> Allocation failure for l_voxelArray"<<std::endl;
		return EXIT_FAILURE;
	}
	
	/* Setting the config data */
	uint16_t arguments[7];
	arguments[0] = nbSub_lvl1;
	arguments[1] = nbSub_lvl2;
	for(int i = 2; i<7; ++i){
		arguments[i] = 0;
	}
	if(bending) arguments[2] = 1;
	if(drain) arguments[3] = 1;
	if(gradient) arguments[4] = 1;
	if(normal) arguments[5] = 1;
	if(surface) arguments[6] = 1;
	
	/* Open DATA file */
	drn_writer_t cache;
	int32_t test_cache = drn_open_writer(&cache, "./voxels_data/voxel_intersec_1.data", "Terrain voxelisation.");
	if(test_cache < 0){ throw std::runtime_error("unable to open data file"); }
	
	/* Saving the config data */
	test_cache = drn_writer_add_chunk(&cache, arguments, 7*sizeof(uint16_t));
	if(test_cache < 0){ throw std::runtime_error("unable to write in the data file"); }
	
	/* Intersection flag to know if a leaf have at least one intersection - if it is not the case, we do not save the leaf */
	bool is_intersec = false;
	/* Triangle flag to know if we have to save the triangle inside the leaf */
	bool is_triangle_in = false;
	
	/* Initialize the Leaves vector */
	std::vector<Leaf> l_queue;
	Leaf currentLeaf;
	currentLeaf.id = 1;
	
	/* Initialize the vertices per leaf vector */
	std::vector<Vertex> l_storedVertices;
	
	//INTERSECTION PROCESSING
	/* Range approximation for voxelisation */
	double Rc = voxelSize * APPROXIM_RANGE;

	//For each leaf
	//#pragma omp parallel for
	for(uint16_t l_i=0;l_i<nbSub_lvl1;++l_i){
		for(uint16_t l_j=0;l_j<nbSub_lvl1;++l_j){
			for(uint16_t l_k=0;l_k<nbSub_lvl1;++l_k){
				
				/* Init leaf voxel array */	
				for(uint32_t n=0;n<l_voxArrLength;++n){
					l_voxelArray[n].nbFaces=0;
					l_voxelArray[n].sumNormal = glm::dvec3(0,0,0);
					l_voxelArray[n].sumDrain = 0;
					l_voxelArray[n].sumGradient = 0;
					l_voxelArray[n].sumSurface = 0;
					l_voxelArray[n].sumBending = 0;
				}
				
				/* Fix the current leaf */
				currentLeaf.pos = glm::dvec3((l_i*l_size)-1., (l_j*l_size)-1., (l_k*l_size)-1.);
				currentLeaf.nbIntersection = 0;
				currentLeaf.nbVertices = 0;
				
				//For each Face
				for(uint32_t n=0;n<nbFace;++n){
					/* Face properties */
					/* min and max */
					uint16_t minVoxelX = ((getminX(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_i*nbSub_lvl2;
					uint16_t maxVoxelX = ((getminX(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_i*nbSub_lvl2;
					uint16_t minVoxelY = ((getminY(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_j*nbSub_lvl2;
					uint16_t maxVoxelY = ((getminY(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_j*nbSub_lvl2;
					uint16_t minVoxelZ = ((getminZ(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_k*nbSub_lvl2;
					uint16_t maxVoxelZ = ((getminZ(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_k*nbSub_lvl2;

					//Test if the triangle is inside the current 
					if(maxVoxelX >= 0 && minVoxelX < nbSub_lvl2 &&
					   maxVoxelY >= 0 && minVoxelY < nbSub_lvl2 &&
					   maxVoxelZ >= 0 && minVoxelZ < nbSub_lvl2){
					   
						//case where triangle overlap 2 leaves
						if(maxVoxelX > nbSub_lvl2){ maxVoxelX = nbSub_lvl2 - 1; } 
						if(minVoxelX < 0){ maxVoxelX = 0; }
						if(maxVoxelY > nbSub_lvl2){ maxVoxelY = nbSub_lvl2 - 1; } 
						if(minVoxelY < 0){ maxVoxelY = 0; }
						if(maxVoxelZ > nbSub_lvl2){ maxVoxelZ = nbSub_lvl2 - 1; } 
						if(minVoxelZ < 0){ maxVoxelZ = 0; }
						
						/* Edges */
						Edge edgS1S2 = createEdge(tabF[n].s1->pos, tabF[n].s2->pos);
						Edge edgS1S3 = createEdge(tabF[n].s1->pos, tabF[n].s3->pos);
						Edge edgS2S3 = createEdge(tabF[n].s2->pos, tabF[n].s3->pos);
						/* Planes */
						/* Calculate the threshold normals and its inverse */
						glm::dvec3 thresholdNormal = Rc * glm::normalize(tabF[n].normal);
						/* Define the upper and lower plane which surround the triangle face */
						Plane upperPlane = createPlane(tabF[n].s3->pos + thresholdNormal, tabF[n].s2->pos + thresholdNormal, tabF[n].s1->pos + thresholdNormal);
						Plane lowerPlane = createPlane(tabF[n].s1->pos - thresholdNormal, tabF[n].s2->pos - thresholdNormal, tabF[n].s3->pos - thresholdNormal);
						/* Define the three perpendicular planes to the trangle Face passing by each edge */
						Plane e1 = createPlane(tabF[n].s1->pos, tabF[n].s2->pos, tabF[n].s2->pos + tabF[n].normal);
						Plane e2 = createPlane(tabF[n].s2->pos, tabF[n].s3->pos, tabF[n].s3->pos + tabF[n].normal);
						Plane e3 = createPlane(tabF[n].s3->pos, tabF[n].s1->pos, tabF[n].s1->pos + tabF[n].normal);

						//For each cube of the face bounding box
						for(uint16_t k=minVoxelZ; k<=maxVoxelZ; ++k){
							for(uint16_t j=minVoxelY;j<=maxVoxelY; ++j){
								for(uint16_t i=minVoxelX;i<=maxVoxelX;++i){
									// Voxel Properties 
									Voxel vox = createVoxel(i*voxelSize + currentLeaf.pos.x + voxelSize*0.5, j*voxelSize + currentLeaf.pos.y + voxelSize*0.5, k*voxelSize + currentLeaf.pos.z + voxelSize*0.5, voxelSize);
									if(processIntersectionPolygonVoxel(tabF[n], edgS1S2, edgS1S3, edgS2S3, upperPlane, lowerPlane, e1, e2, e3, vox, Rc, mode)){
										is_intersec = true;
										is_triangle_in = true;
										/* update the voxel array */
										uint32_t currentIndex = i + nbSub_lvl2*j + k*nbSub_lvl2*nbSub_lvl2;
										l_voxelArray[currentIndex].nbFaces++;
										if(normal) 	l_voxelArray[currentIndex].sumNormal = glm::dvec3(l_voxelArray[currentIndex].sumNormal.x + tabF[n].normal.x, l_voxelArray[currentIndex].sumNormal.y + tabF[n].normal.y, l_voxelArray[currentIndex].sumNormal.z + tabF[n].normal.z);
										if(drain) 	l_voxelArray[currentIndex].sumDrain = l_voxelArray[currentIndex].sumDrain + tabF[n].drain;
										if(gradient)l_voxelArray[currentIndex].sumGradient = l_voxelArray[currentIndex].sumGradient + tabF[n].gradient;
										if(surface) l_voxelArray[currentIndex].sumSurface = l_voxelArray[currentIndex].sumSurface + tabF[n].surface;
										if(bending) l_voxelArray[currentIndex].sumBending = l_voxelArray[currentIndex].sumBending + tabF[n].bending;
										
										/* update the leaf info */
										currentLeaf.nbIntersection++;
									}
								}
							}
						}//end foreach voxel
						/* add the triangle to the chunck */
						if(is_triangle_in){
							l_storedVertices.push_back(*(tabF[n].s1));
							l_storedVertices.push_back(*(tabF[n].s2));
							l_storedVertices.push_back(*(tabF[n].s3));
							currentLeaf.nbVertices+=3;
							is_triangle_in = false;
						}			
					}
				}//end foreach face
				/* if the leaf is not empty, save its voxels */
				if(is_intersec){
					/*** TRIANGULARISATION HERE ***/
					
					/* compute leaf's corners */
					glm::dvec3 vert0 = currentLeaf.pos;
					glm::dvec3 vert1 = glm::dvec3(vert0.x + l_size, vert0.y, vert0.z);
					glm::dvec3 vert2 = glm::dvec3(vert0.x + l_size, vert0.y + l_size, vert0.z);
					glm::dvec3 vert3 = glm::dvec3(vert0.x, vert0.y + l_size, vert0.z);
					glm::dvec3 vert4 = glm::dvec3(vert0.x, vert0.y, vert0.z + l_size);
					glm::dvec3 vert5 = glm::dvec3(vert0.x + l_size, vert0.y, vert0.z + l_size);
					glm::dvec3 vert6 = glm::dvec3(vert0.x + l_size, vert0.y + l_size, vert0.z + l_size);
					glm::dvec3 vert7 = glm::dvec3(vert0.x, vert0.y + l_size, vert0.z + l_size);
					
					/* compute leaf's edges */
					Edge e0 = createEdge(vert0, vert1);
					Edge e1 = createEdge(vert1, vert2);
					Edge e2 = createEdge(vert2, vert3);
					Edge e3 = createEdge(vert3, vert0);
					Edge e4 = createEdge(vert4, vert5);
					Edge e5 = createEdge(vert5, vert6);
					Edge e6 = createEdge(vert6, vert7);
					Edge e7 = createEdge(vert7, vert4);
					Edge e8 = createEdge(vert0, vert4);
					Edge e9 = createEdge(vert1, vert5);
					Edge e10 = createEdge(vert2, vert6);
					Edge e11 = createEdge(vert3, vert7);
					
					std::vector<Edge> leafEdges;
					leafEdges.push_back(e0);
					leafEdges.push_back(e1);
					leafEdges.push_back(e2);
					leafEdges.push_back(e3);
					leafEdges.push_back(e4);
					leafEdges.push_back(e5);
					leafEdges.push_back(e6);
					leafEdges.push_back(e7);
					leafEdges.push_back(e8);
					leafEdges.push_back(e9);
					leafEdges.push_back(e10);
					leafEdges.push_back(e11);
					
					
					/* Intersection triangle - edge */
					/* browse edges */
					
					std::vector<Vertex> intersectionPoints;
					
					std::cout << "---- LEAF ----" << std::endl;
					for(std::vector<Edge>::iterator e_it = leafEdges.begin(); e_it < leafEdges.end(); ++e_it){
						/* browse saved triangles */
						std::cout << "Edge : " << std::endl; 
						std::cout << "Origin : " << (*e_it).origin.x << " " << (*e_it).origin.y << " " << (*e_it).origin.z << std::endl;
						std::cout << "Dir : " << (*e_it).dir.x << " " << (*e_it).dir.y << " " << (*e_it).dir.z << std::endl;
						for(size_t i = 0; i < l_storedVertices.size(); i += 3){
							Face tempFace; tempFace.s1 = &l_storedVertices[i]; tempFace.s2 = &l_storedVertices[i+1]; tempFace.s3 = &l_storedVertices[i+2];
							/*** test intersection edge - tempFace ***/
							if((*e_it).faceIntersectionTest(tempFace)){
								std::cout << "Face : " << std::endl;
								std::cout << "s1 : " << tempFace.s1->pos.x << " " << tempFace.s1->pos.y << " " << tempFace.s1->pos.z << std::endl;
								std::cout << "s2 : " << tempFace.s2->pos.x << " " << tempFace.s2->pos.y << " " << tempFace.s2->pos.z << std::endl;
								std::cout << "s3 : " << tempFace.s3->pos.x << " " << tempFace.s3->pos.y << " " << tempFace.s3->pos.z << std::endl;
								
								glm::dvec3 u = createVector(tempFace.s1->pos, tempFace.s2->pos);
								glm::dvec3 v = createVector(tempFace.s1->pos, tempFace.s3->pos);
								
								glm::dvec3 normale = glm::normalize(glm::cross(u, v));
								std::cout << "Normale : " << normale.x << " " << normale.y << " " << normale.z << std::endl;
								
								/* intersection point */								
								glm::dvec3 intPoint = (*e_it).computeIntersectionPoint(u, v, tempFace.s1->pos);
								std::cout << "Intersection Point : " << intPoint.x << " " << intPoint.y << " " << intPoint.z << std::endl;
								
								Vertex intVertex;
								intVertex.pos = intPoint;
								intVertex.normal = normale;
								
								/* add the intersection point to the vector */
								intersectionPoints.push_back(intVertex);
								
								break;
							}
						}
						std::cout << std::endl;
					}
					
					/* Compute the "dual-vertex" */
					
					
					
					/* Save the VoxelData array */
					test_cache = drn_writer_add_chunk(&cache, l_voxelArray, l_voxArrLength*sizeof(VoxelData));
					if(test_cache < 0){ throw std::runtime_error("unable to write in the data file"); }
					
					/* Save the vertices */
					test_cache = drn_writer_add_chunk(&cache, l_storedVertices.data(), currentLeaf.nbVertices*sizeof(Vertex));
					
					/* updade the Leaf indexation */
					currentLeaf.nbIntersection /= l_voxArrLength;
					l_queue.push_back(currentLeaf);
					
					/* init for next leaf */
					currentLeaf.id+=2;
					currentLeaf.nbIntersection = 0;
					currentLeaf.nbVertices = 0;
					l_storedVertices.clear();
					is_intersec = false;
				}
			}
		}
	}//end foreach leaf
	std::cout<<"-> Voxelisation finished !"<<std::endl;

	std::cout << "Number of leaves saved : "<< l_queue.size() << std::endl;

	/* writing the Leaf chunck */
	test_cache = drn_writer_add_chunk(&cache, l_queue.data(), l_queue.size()*sizeof(Leaf));
	
	/* Close the DATA file */
	test_cache = drn_close_writer(&cache);
	if(test_cache < 0){ throw std::runtime_error("unable to close the data file"); }

	delete[] tabV;
	delete[] tabF;
	delete[] l_voxelArray;

	return EXIT_SUCCESS;
}
