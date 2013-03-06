#include <iostream>
#include <cstdlib>
#include <cmath>
#include <stdexcept>
#include <glm/glm.hpp>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <omp.h>
#include "geom_types.hpp"
#include "data_types.hpp"
#include "voxel_maker/geometrics.hpp"
#include "voxel_maker/intersection_test.hpp"
#include "voxel_maker/triangularisation.hpp"

#include "drn/drn_writer.h"

static const double APPROXIM_RANGE = sqrt(3)/2;
static const size_t GRID_3D_SIZE = 2;
static const size_t LEAF_STEP = 2;
static const size_t NB_LEAVES_IN_GROUP = LEAF_STEP*LEAF_STEP*LEAF_STEP;

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
	std::cout << "-d : saving the drain coefficients in the voxel_data file" << std::endl << std::endl;
	std::cout << "-a : saving all these datas" << std::endl << std::endl;
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
	
	//Managing the console arguments
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
			// everything requested
			else if(strcmp(tabArguments[i],"-a") == 0){
				if(!normal && !p4Requested && mode==0) std::cout << "############ REQUESTS ############" << std::endl;
				p4Requested = 1;
				
				std::cout << "-> Requested : everything" << std::endl;
				std::cout << "-> loading of the normals" << std::endl;
				normal = 1;
				std::cout << "-> loading of the drain coefficients" << std::endl;
				drain = 1;
				std::cout << "-> loading of the bending coefficients" << std::endl;
				bending = 1;
				std::cout << "-> loading of the surfaces" << std::endl;
				surface = 1;
				std::cout << "-> loading of the gradient coefficients" << std::endl;
				gradient = 1;
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
	if(test_fic == 0){ throw std::runtime_error("unable to read the original data file"); }
	test_fic =fread(&nbFace, sizeof(nbFace), 1, dataFile);
	if(test_fic == 0){ throw std::runtime_error("unable to read the original data file"); }

	// terrain minimal and maximum
	double terrainMinX = 1.;
	double terrainMaxX = -1.;
	double terrainMinY = 1.;
	double terrainMaxY = -1.;
	double terrainMinZ = 1.;
	double terrainMaxZ = -1.;
	
	double * positionsData = new double[3*nbVertice];
	test_fic = fread(positionsData, sizeof(double), 3*nbVertice, dataFile); // to read the positions of the vertices
	if(test_fic == 0){ throw std::runtime_error("unable to read the original data file"); }
	
	uint32_t * facesData = new uint32_t[3*nbFace];
	test_fic = fread(facesData, sizeof(uint32_t), 3*nbFace, dataFile); // to read the indexes of the vertices which compose each face
	if(test_fic == 0){ throw std::runtime_error("unable to read the original data file"); }
	
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
	if(test_fic == 0){ throw std::runtime_error("unable to read the original data file"); }
	
	fclose(normalFile);
	
	//CHARGEMENT PAGE4.DATA
	int* drainData = NULL;
	double* otherData = NULL;
	if(p4Requested){
		drainData = new int[nbVertice+nbFace];
		otherData = new double[3*nbVertice+3*nbFace];

		FILE* page4File = NULL;
		page4File = fopen("terrain_data/page_4.data", "rb");
		if(NULL == page4File){
			std::cout << "[!] -> Unable to load the file fourth data page" << std::endl;
			return EXIT_FAILURE;
		}

		for(uint32_t n=0;n<nbVertice+nbFace;++n){
			test_fic = fread(&(drainData[n]), sizeof(int), 1, page4File);
			if(test_fic == 0){ throw std::runtime_error("unable to read the original data file"); }
			test_fic = fread(&(otherData[3*n]), sizeof(double), 3, page4File);
			if(test_fic == 0){ throw std::runtime_error("unable to read the original data file"); }
		}
		
		fclose(page4File);
	}
	
	//CONSTRUCTION OF THE DATA STRUCTURES
	uint32_t cptNormals = 0;
	
	//VERTICES
	Vertex * tabV = new Vertex[nbVertice];

	//Min & Max values
	float *maxCoeffArray = new float[7];
	for(int n = 0; n<7; ++n){
		maxCoeffArray[n] = 0;
	}
	
	for(uint32_t n=0;n<nbVertice;++n){ // to create the vertices tab
		tabV[n].pos.x = positionsData[3*n];
		tabV[n].pos.z = positionsData[3*n+1];
		tabV[n].pos.y = positionsData[3*n+2];
		tabV[n].normal.x = normalData[3*cptNormals];
		tabV[n].normal.z = normalData[3*cptNormals+1];
		tabV[n].normal.y = normalData[3*cptNormals+2];
		if(bending){
			tabV[n].bending = otherData[3*n];
			if(tabV[n].bending>maxCoeffArray[0]) maxCoeffArray[0] = tabV[n].bending;
		}
		if(drain){
			tabV[n].drain = drainData[n];
			if(tabV[n].drain>maxCoeffArray[1]) maxCoeffArray[1] = tabV[n].drain;
		}
		if(surface){
			tabV[n].surface = otherData[3*n+1];
			if(tabV[n].surface>maxCoeffArray[3]) maxCoeffArray[3] = tabV[n].surface;
		}
		if(gradient){
			tabV[n].gradient = otherData[3*n+2];
			if(tabV[n].gradient>maxCoeffArray[2]) maxCoeffArray[2] = tabV[n].gradient;
		}
		cptNormals++;
		
		// Compute the terrain min and max
		if(tabV[n].pos.x < terrainMinX){ terrainMinX = tabV[n].pos.x; }
		if(tabV[n].pos.y < terrainMinY){ terrainMinY = tabV[n].pos.y; }
		maxCoeffArray[5] = terrainMinY;
		if(tabV[n].pos.z < terrainMinZ){ terrainMinZ = tabV[n].pos.z; }
		if(tabV[n].pos.x > terrainMaxX){ terrainMaxX = tabV[n].pos.x; }
		if(tabV[n].pos.y > terrainMaxY){ terrainMaxY = tabV[n].pos.y; }
		maxCoeffArray[4] = terrainMaxY;
		if(tabV[n].pos.z > terrainMaxZ){ terrainMaxZ = tabV[n].pos.z; }
	}
	
	maxCoeffArray[6] = nbFace; // nbFaces for terrain scaling

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
			tabF[n].drain = drainData[nbVertice+n];
		}else{
			tabF[n].drain = 0;
		}
		
		if(p4Requested && bending){	
			tabF[n].bending = otherData[3*nbVertice+n*3];
		}else{
			tabF[n].bending = 0;
		}
		
		if(p4Requested && surface){
			tabF[n].surface = otherData[3*nbVertice+n*3+1];
		}else{
			tabF[n].surface = 0;
		}
		
		if(p4Requested && gradient){
			tabF[n].gradient = otherData[3*nbVertice+n*3+2];
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
	
	/* Manage demanded levels of subdivision */
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

	if(nbSub_lvl1 < LEAF_STEP){
		nbSub_lvl1 = LEAF_STEP;
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
	
	/* Grid from level 1 : grid of leaves */
	double l_size = GRID_3D_SIZE/(double)nbSub_lvl1;
	
	/* Grid from level 2 : grid of voxel inside a leaf */
	double voxelSize = l_size/(double)nbSub_lvl2;
	
	size_t const l_voxArrLength = nbSub_lvl2*nbSub_lvl2*nbSub_lvl2;
	VoxelData* l_voxelArray[NB_LEAVES_IN_GROUP];
	for(unsigned int n = 0; n < NB_LEAVES_IN_GROUP; ++n){
		l_voxelArray[n] = new VoxelData[l_voxArrLength];
		if(NULL == l_voxelArray[n]){
			std::cout<<"[!] -> Allocation failure for l_voxelArray"<<std::endl;
			return EXIT_FAILURE;
		}
	}
	
	/* Setting the config data */
	/* Compute the possible number of resolution level on leaves */
	uint16_t nbLevel = 1;
	uint16_t tmp_nbSub_lvl1 = nbSub_lvl1;
	while(tmp_nbSub_lvl1 > 4){
		tmp_nbSub_lvl1 /= 2;
		nbLevel++;
	}
	
	std::cout<<"//-> NB possible level : "<<nbLevel<<std::endl;
	
	uint16_t arguments[8];
	arguments[0] = nbSub_lvl1;
	arguments[1] = nbSub_lvl2;
	arguments[7] = nbLevel;
	for(int i = 2; i<7; ++i){
		arguments[i] = 0;
	}
	if(normal) arguments[2] = 1;
	if(bending) arguments[3] = 1;
	if(drain) arguments[4] = 1;
	if(gradient) arguments[5] = 1;
	if(surface) arguments[6] = 1;

	/* Open DATA file */
	drn_writer_t cache;
	int32_t test_cache = drn_open_writer(&cache, "./voxels_data/voxel_intersec_1.data", "Terrain voxelisation.");
	if(test_cache < 0){ throw std::runtime_error("unable to open data file"); }
	
	/* Saving the config data */
	test_cache = drn_writer_add_chunk(&cache, arguments, 8*sizeof(uint16_t));
	if(test_cache < 0){ throw std::runtime_error("unable to write in the data file"); }
	
	/* Saving the maximum data */
	test_cache = drn_writer_add_chunk(&cache, maxCoeffArray, 7*sizeof(float));
	if(test_cache < 0){ throw std::runtime_error("unable to write in the data file"); }
		
	/* Initialize the Leaves array for each level */
	std::vector<Leaf*> leafArrays;
	for(uint32_t n=0;n<nbLevel;++n){
		uint16_t tmp_nbSub = nbSub_lvl1 / glm::pow(2, int(n));
		uint32_t sizeLeafArray = tmp_nbSub*tmp_nbSub*tmp_nbSub;
		Leaf* l_arr = new Leaf[sizeLeafArray];
		leafArrays.push_back(l_arr);
	}
	
	//INTERSECTION PROCESSING
	/* Range approximation for voxelisation */
	double Rc = voxelSize * APPROXIM_RANGE;

	//For each group of leaves
	#pragma omp parallel for
	for(uint16_t l_j=0;l_j<nbSub_lvl1;l_j+=LEAF_STEP){
		for(uint16_t l_k=0;l_k<nbSub_lvl1;l_k+=LEAF_STEP){
			for(uint16_t l_i=0;l_i<nbSub_lvl1;l_i+=LEAF_STEP){
				uint32_t leafIndex[NB_LEAVES_IN_GROUP];
				
				// indexes of the 8 leaves of the group
				leafIndex[0] = l_i 		+ 	nbSub_lvl1*l_k 		+ 	l_j*nbSub_lvl1*nbSub_lvl1;
				leafIndex[1] = l_i+1 	+ 	nbSub_lvl1*l_k 		+ 	l_j*nbSub_lvl1*nbSub_lvl1;
				leafIndex[2] = l_i 		+ 	nbSub_lvl1*(l_k+1) 	+ 	l_j*nbSub_lvl1*nbSub_lvl1;
				leafIndex[3] = l_i+1 	+ 	nbSub_lvl1*(l_k+1) 	+	l_j*nbSub_lvl1*nbSub_lvl1;
				leafIndex[4] = l_i 		+ 	nbSub_lvl1*l_k 		+	(l_j+1)*nbSub_lvl1*nbSub_lvl1;
				leafIndex[5] = l_i+1 	+ 	nbSub_lvl1*l_k		+ 	(l_j+1)*nbSub_lvl1*nbSub_lvl1;
				leafIndex[6] = l_i 		+ 	nbSub_lvl1*(l_k+1)	+ 	(l_j+1)*nbSub_lvl1*nbSub_lvl1;
				leafIndex[7] = l_i+1 	+ 	nbSub_lvl1*(l_k+1) 	+ 	(l_j+1)*nbSub_lvl1*nbSub_lvl1;
				
				/* Initialize the vertices per leaf vector */
				std::vector<Vertex> l_storedVertices[NB_LEAVES_IN_GROUP];
				
				/* Init leaves voxel arrays */
				for(unsigned int i = 0; i < NB_LEAVES_IN_GROUP; ++i){
					for(uint32_t n=0;n<l_voxArrLength;++n){
						l_voxelArray[i][n].nbFaces=0;
						l_voxelArray[i][n].sumNormal = glm::dvec3(0,0,0);
						l_voxelArray[i][n].sumDrain = 0;
						l_voxelArray[i][n].sumGradient = 0;
						l_voxelArray[i][n].sumSurface = 0;
						l_voxelArray[i][n].sumBending = 0;
					}
				}
				
				/* Fix the current leaf */
				for(unsigned int n = 0; n < NB_LEAVES_IN_GROUP; ++n){
					leafArrays[0][leafIndex[n]].size = l_size;
					leafArrays[0][leafIndex[n]].nbIntersection = 0;
					leafArrays[0][leafIndex[n]].nbVertices_lvl1 = 0;
					leafArrays[0][leafIndex[n]].nbVertices_lvl2 = 0;
				}
				
				leafArrays[0][leafIndex[0]].pos = glm::dvec3((l_i*l_size)-1., 		(l_j*l_size)-1., 		(l_k*l_size)-1.);
				leafArrays[0][leafIndex[1]].pos = glm::dvec3(((l_i+1)*l_size)-1., 	(l_j*l_size)-1., 		(l_k*l_size)-1.);
				leafArrays[0][leafIndex[2]].pos = glm::dvec3((l_i*l_size)-1., 		(l_j*l_size)-1., 		((l_k+1)*l_size)-1.);
				leafArrays[0][leafIndex[3]].pos = glm::dvec3(((l_i+1)*l_size)-1., 	(l_j*l_size)-1., 		((l_k+1)*l_size)-1.);
				leafArrays[0][leafIndex[4]].pos = glm::dvec3((l_i*l_size)-1., 		((l_j+1)*l_size)-1., 	(l_k*l_size)-1.);
				leafArrays[0][leafIndex[5]].pos = glm::dvec3(((l_i+1)*l_size)-1., 	((l_j+1)*l_size)-1., 	(l_k*l_size)-1.);
				leafArrays[0][leafIndex[6]].pos = glm::dvec3((l_i*l_size)-1., 		((l_j+1)*l_size)-1., 	((l_k+1)*l_size)-1.);
				leafArrays[0][leafIndex[7]].pos = glm::dvec3(((l_i+1)*l_size)-1., 	((l_j+1)*l_size)-1., 	((l_k+1)*l_size)-1.);
				
				//For each Face
				for(uint32_t n=0;n<nbFace;++n){
					/* Face properties */
					/* min and max */
					int minVoxelX = ((getminX(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_i*nbSub_lvl2;
					int maxVoxelX = ((getmaxX(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_i*nbSub_lvl2;
					int minVoxelY = ((getminY(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_j*nbSub_lvl2;
					int maxVoxelY = ((getmaxY(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_j*nbSub_lvl2;
					int minVoxelZ = ((getminZ(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_k*nbSub_lvl2;
					int maxVoxelZ = ((getmaxZ(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_k*nbSub_lvl2;
					
					//Test if the triangle is inside the current group of leaves
					if(maxVoxelX >= 0 && minVoxelX < nbSub_lvl2*2 &&
					   maxVoxelY >= 0 && minVoxelY < nbSub_lvl2*2 &&
					   maxVoxelZ >= 0 && minVoxelZ < nbSub_lvl2*2){
					   
						//case where triangle overlaps 2 groups
						if(maxVoxelX >= nbSub_lvl2*2){ maxVoxelX = nbSub_lvl2*2 - 1; } 
						if(minVoxelX < 0){ minVoxelX = 0; }
						if(maxVoxelY >= nbSub_lvl2*2){ maxVoxelY = nbSub_lvl2*2 - 1; } 
						if(minVoxelY < 0){ minVoxelY = 0; }
						if(maxVoxelZ >= nbSub_lvl2*2){ maxVoxelZ = nbSub_lvl2*2 - 1; } 
						if(minVoxelZ < 0){ minVoxelZ = 0; }
						
						uint32_t insideLeafIndex = 0;
						uint32_t insideArrayIndex = 0;
						
						/* put triangles in the right leaf */
						if(	maxVoxelX >= 0 && minVoxelX < nbSub_lvl2 &&
							maxVoxelY >= 0 && minVoxelY < nbSub_lvl2 &&
							maxVoxelZ >= 0 && minVoxelZ < nbSub_lvl2){		// first leaf
							insideLeafIndex = leafIndex[0];
							insideArrayIndex = 0;
						}
						
						if(	maxVoxelX >= nbSub_lvl2 && minVoxelX < nbSub_lvl2*2 &&
							maxVoxelY >= 0 && minVoxelY < nbSub_lvl2 &&
							maxVoxelZ >= 0 && minVoxelZ < nbSub_lvl2){
							insideLeafIndex = leafIndex[1];
							insideArrayIndex = 1;
						}
						
						if(	maxVoxelX >= 0 && minVoxelX < nbSub_lvl2 &&
							maxVoxelY >= 0 && minVoxelY < nbSub_lvl2 &&
							maxVoxelZ >= nbSub_lvl2 && minVoxelZ < nbSub_lvl2*2){
							insideLeafIndex = leafIndex[2];
							insideArrayIndex = 2;
						}
						
						if(	maxVoxelX >= nbSub_lvl2 && minVoxelX < nbSub_lvl2*2 &&
							maxVoxelY >= 0 && minVoxelY < nbSub_lvl2 &&
							maxVoxelZ >= nbSub_lvl2 && minVoxelZ < nbSub_lvl2*2){
							insideLeafIndex = leafIndex[3];
							insideArrayIndex = 3;
						}
						
						if(	maxVoxelX >= 0 && minVoxelX < nbSub_lvl2 &&
							maxVoxelY >= nbSub_lvl2 && minVoxelY < nbSub_lvl2*2 &&
							maxVoxelZ >= 0 && minVoxelZ < nbSub_lvl2){
							insideLeafIndex = leafIndex[4];
							insideArrayIndex = 4;
						}
						
						if(	maxVoxelX >= nbSub_lvl2 && minVoxelX < nbSub_lvl2*2 &&
							maxVoxelY >= nbSub_lvl2 && minVoxelY < nbSub_lvl2*2 &&
							maxVoxelZ >= 0 && minVoxelZ < nbSub_lvl2){
							insideLeafIndex = leafIndex[5];
							insideArrayIndex = 5;
						}
						
						if(	maxVoxelX >= 0 && minVoxelX < nbSub_lvl2 &&
							maxVoxelY >= nbSub_lvl2 && minVoxelY < nbSub_lvl2*2 &&
							maxVoxelZ >= nbSub_lvl2 && minVoxelZ < nbSub_lvl2*2){
							insideLeafIndex = leafIndex[6];
							insideArrayIndex = 6;
						}
						
						if(	maxVoxelX >= nbSub_lvl2 && minVoxelX < nbSub_lvl2*2 &&
							maxVoxelY >= nbSub_lvl2 && minVoxelY < nbSub_lvl2*2 &&
							maxVoxelZ >= nbSub_lvl2 && minVoxelZ < nbSub_lvl2*2){
							insideLeafIndex = leafIndex[7];
							insideArrayIndex = 7;
						}
							
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
						/* Define the three perpendicular planes to the triangle Face passing by each edge */
						Plane e1 = createPlane(tabF[n].s1->pos, tabF[n].s2->pos, tabF[n].s2->pos + tabF[n].normal);
						Plane e2 = createPlane(tabF[n].s2->pos, tabF[n].s3->pos, tabF[n].s3->pos + tabF[n].normal);
						Plane e3 = createPlane(tabF[n].s3->pos, tabF[n].s1->pos, tabF[n].s1->pos + tabF[n].normal);

						//For each cube of the face bounding box
						for(int J=minVoxelY;J<=maxVoxelY; ++J){	
							for(int K=minVoxelZ; K<=maxVoxelZ; ++K){
								for(int I=minVoxelX;I<=maxVoxelX; ++I){
									int i = I;
									int k = K;
									int j = J;
									
									if(i >= nbSub_lvl2){ i -= nbSub_lvl2;}
									if(j >= nbSub_lvl2){ j -= nbSub_lvl2;}
									if(k >= nbSub_lvl2){ k -= nbSub_lvl2;}
									
									// Voxel Properties 
									Voxel vox = createVoxel(i*voxelSize + leafArrays[0][insideLeafIndex].pos.x + voxelSize*0.5, j*voxelSize + leafArrays[0][insideLeafIndex].pos.y + voxelSize*0.5, k*voxelSize + leafArrays[0][insideLeafIndex].pos.z + voxelSize*0.5, voxelSize);

									if(processIntersectionPolygonVoxel(tabF[n], edgS1S2, edgS1S3, edgS2S3, upperPlane, lowerPlane, e1, e2, e3, vox, Rc, mode)){
										/* update the voxel array */
										uint32_t currentIndex = i + nbSub_lvl2*k + j*nbSub_lvl2*nbSub_lvl2;
										l_voxelArray[insideArrayIndex][currentIndex].nbFaces++;
										if(normal) 	l_voxelArray[insideArrayIndex][currentIndex].sumNormal = glm::dvec3(l_voxelArray[insideArrayIndex][currentIndex].sumNormal.x + tabF[n].normal.x, l_voxelArray[insideArrayIndex][currentIndex].sumNormal.y + tabF[n].normal.y, l_voxelArray[insideArrayIndex][currentIndex].sumNormal.z + tabF[n].normal.z);
										if(drain) 	l_voxelArray[insideArrayIndex][currentIndex].sumDrain = l_voxelArray[insideArrayIndex][currentIndex].sumDrain + tabF[n].drain;
										if(gradient)l_voxelArray[insideArrayIndex][currentIndex].sumGradient = l_voxelArray[insideArrayIndex][currentIndex].sumGradient + tabF[n].gradient;
										if(surface) l_voxelArray[insideArrayIndex][currentIndex].sumSurface = l_voxelArray[insideArrayIndex][currentIndex].sumSurface + tabF[n].surface;
										if(bending) l_voxelArray[insideArrayIndex][currentIndex].sumBending = l_voxelArray[insideArrayIndex][currentIndex].sumBending + tabF[n].bending;
										
										/* update the leaf info */
										++leafArrays[0][insideLeafIndex].nbIntersection;
									}
								}
							}
						}//end foreach voxel
						
						/* add the triangle to the chunck */
						l_storedVertices[insideArrayIndex].push_back(*(tabF[n].s1));
						l_storedVertices[insideArrayIndex].push_back(*(tabF[n].s2));
						l_storedVertices[insideArrayIndex].push_back(*(tabF[n].s3));
						leafArrays[0][insideLeafIndex].nbVertices_lvl2+=3;
					}
				}//end foreach face
				
				/* if the leaf is not empty, save its voxels */
				for(unsigned int n = 0; n < NB_LEAVES_IN_GROUP; ++n){
					if(leafArrays[0][leafIndex[n]].nbIntersection != 0){ // is_intersect
						/* Save the VoxelData array */
						#pragma omp critical
						{
						test_cache = drn_writer_add_chunk(&cache, l_voxelArray[n], l_voxArrLength*sizeof(VoxelData));
						if(test_cache < 0){ throw std::runtime_error("unable to write in the data file"); }
						
						/* Save the vertices lvl2 */
						test_cache = drn_writer_add_chunk(&cache, l_storedVertices[n].data(), leafArrays[0][leafIndex[n]].nbVertices_lvl2*sizeof(Vertex));
						if(test_cache < 0){ throw std::runtime_error("unable to write in the data file"); }
						
						/* Set Leaf id */
						leafArrays[0][leafIndex[n]].id = (drn_writer_get_last_chunk_id(&cache)-2)/2;
						std::cout<<"//-> Leaf Id : "<<leafArrays[0][leafIndex[n]].id<<std::endl;
						}
						
						/*** TRIANGULARISATION HERE ***/					
						leafArrays[0][leafIndex[n]].optimal = computeOptimalPoint(leafArrays[0][leafIndex[n]], l_storedVertices[n]);
					}
				}
			} //end
		} // 	foreach
	} // 		leaf
	std::cout<<"-> Voxelisation finished !"<<std::endl;
	
	
	// BUILD THE TRIANGLES
	
	/* Save the number of saved Leaves */
	uint32_t* nbSavedLeaves = new uint32_t[nbLevel];
	for(uint16_t idx=1;idx<nbLevel;++idx){ nbSavedLeaves[idx] = 0; }
	nbSavedLeaves[0] = (drn_writer_get_last_chunk_id(&cache)-1)/2; //finest level of resolution - real number of saved leaves
	
	/* Set the computed vertices to the upper leafArrays */
	for(uint32_t lvl=1;lvl<nbLevel;++lvl){
		uint16_t crt_nbSub = nbSub_lvl1 / pow(2, int(lvl));
		uint32_t l_id = 0;
		/* Fill the other LeafArray */
		uint32_t l_index[8];
		for(uint16_t l_i=0;l_i<crt_nbSub;++l_i){
			for(uint16_t l_j=0;l_j<crt_nbSub;++l_j){
				for(uint16_t l_k=0;l_k<crt_nbSub;++l_k){
					uint32_t crt_l = l_i + l_j*crt_nbSub + l_k*crt_nbSub*crt_nbSub;
					
					l_index[0] = l_i*2 + l_j*2*crt_nbSub*2 + l_k*2*crt_nbSub*crt_nbSub*4;
					l_index[1] = l_i*2+1 + l_j*2*crt_nbSub*2 + l_k*2*crt_nbSub*crt_nbSub*4;
					l_index[2] = l_i*2 + l_j*2*crt_nbSub*2 + (l_k*2+1)*crt_nbSub*crt_nbSub*4;
					l_index[3] = l_i*2+1 + l_j*2*crt_nbSub*2 + (l_k*2+1)*crt_nbSub*crt_nbSub*4;
					l_index[4] = l_i*2 + (l_j*2+1)*crt_nbSub*2 + l_k*2*crt_nbSub*crt_nbSub*4;
					l_index[5] = l_i*2+1 + (l_j*2+1)*crt_nbSub*2 + l_k*2*crt_nbSub*crt_nbSub*4;
					l_index[6] = l_i*2 + (l_j*2+1)*crt_nbSub*2 + (l_k*2+1)*crt_nbSub*crt_nbSub*4;
					l_index[7] = l_i*2+1 + (l_j*2+1)*crt_nbSub*2 + (l_k*2+1)*crt_nbSub*crt_nbSub*4;
					
					double l_size = leafArrays[lvl-1][crt_l].size*2;
					leafArrays[lvl][crt_l].size = l_size;
					leafArrays[lvl][crt_l].pos = glm::dvec3(l_i*l_size -1., l_j*l_size -1., l_k*l_size -1.);
					std::vector<Vertex> optPoints;
					for(uint32_t idx=0;idx<8;idx++){
						leafArrays[lvl][crt_l].nbIntersection += leafArrays[lvl-1][l_index[idx]].nbIntersection;
						if(leafArrays[lvl-1][l_index[idx]].nbIntersection != 0){
							optPoints.push_back(leafArrays[lvl-1][l_index[idx]].optimal);
						}
					}
					if(leafArrays[lvl][crt_l].nbIntersection != 0){
						leafArrays[lvl][crt_l].optimal = computeAvrOptimalPoint(optPoints);
						leafArrays[lvl][crt_l].id = l_id;
						l_id++;
						nbSavedLeaves[lvl]++;
					}
				}
			}
		}
		std::cout<<std::endl;
	}
	
	/* Create queue of leaves */
	std::vector<std::vector<Leaf> > l_queues(nbLevel);
	
	for(uint16_t lvl=0;lvl<nbLevel;++lvl){
		uint32_t crt_nbSub = nbSub_lvl1/pow(2, lvl);
		std::vector< std::vector<Vertex> > l_computedVertices(nbSavedLeaves[lvl]);
		
		buildTriangles(l_computedVertices, leafArrays[lvl], crt_nbSub);
		
		/* Fill the leaves queue */
		for(uint16_t l_j=0;l_j<crt_nbSub;++l_j){
			for(uint16_t l_k=0;l_k<crt_nbSub;++l_k){
				for(uint16_t l_i=0;l_i<crt_nbSub;++l_i){
					uint32_t currentLeafIndex = l_i + crt_nbSub*l_k + l_j*crt_nbSub*crt_nbSub;
					if(leafArrays[lvl][currentLeafIndex].nbIntersection != 0){
						std::cout<<"//-> NB Vertices saved in leaf : "<<l_computedVertices[leafArrays[0][currentLeafIndex].id].size()<<std::endl;
						leafArrays[lvl][currentLeafIndex].nbVertices_lvl1 = l_computedVertices[leafArrays[lvl][currentLeafIndex].id].size();
						l_queues[lvl].push_back(leafArrays[lvl][currentLeafIndex]);
					}
				}
			}
		}
		
		//sort the leaves by chunk_id
		std::sort(l_queues[lvl].begin(), l_queues[lvl].end(), l_queues[lvl].front());
		
		/* Save the computed triangle */	
		for(unsigned int n = 0; n < l_queues[lvl].size(); ++n){
			test_cache = drn_writer_add_chunk(&cache, l_computedVertices[l_queues[lvl][n].id].data(), l_computedVertices[l_queues[lvl][n].id].size()*sizeof(Vertex));
		}
		
		std::cout << "Number of leaves saved : "<< l_queues[lvl].size() << std::endl;
	}
	
	/* Save the leaf queues */
	for(uint16_t lvl=0;lvl<nbLevel;++lvl){
		/* writing the Leaf chunck */
		test_cache = drn_writer_add_chunk(&cache, l_queues[lvl].data(), l_queues[lvl].size()*sizeof(Leaf));
	}
	
	/* Save the number of saved leaves per level */
	test_cache = drn_writer_add_chunk(&cache, nbSavedLeaves, nbLevel*sizeof(uint32_t));
	if(test_cache < 0){ throw std::runtime_error("unable to close the data file"); }		
	
	/* Close the DATA file */
	test_cache = drn_close_writer(&cache);
	if(test_cache < 0){ throw std::runtime_error("unable to close the data file"); }

	delete[] tabV;
	delete[] tabF;
	for(uint32_t n = 0; n < NB_LEAVES_IN_GROUP; ++n){
		delete[] l_voxelArray[n];
	}
	for(uint32_t n = 0; n < nbLevel; ++n){
		delete[] leafArrays[n];
	}
	delete[] maxCoeffArray;
	delete[] nbSavedLeaves;

	return EXIT_SUCCESS;
}
