#include <iostream>
#include <cstdlib>
#include <cmath>
#include <glm/glm.hpp>
#include <stdint.h>
#include <string.h>
#include <omp.h>
#include "geom_types.hpp"
#include "data_types.hpp"
#include "voxel_maker/geometrics.hpp"
#include "voxel_maker/intersection_test.hpp"

#include "drn/drn_writer.h"
#include "drn/drn_reader.h"

static const double APPROXIM_RANGE = 2*sqrt(3)/3.;
static const size_t GRID_3D_SIZE = 2;

/******************************************/
/*          FUNCTIONS                     */
/******************************************/

/**********************TO REMOVE**********************/
/**************EXAMPLE OF CHUNK USING*****************/
/*void tryingWritingChunk(){
	drn_writer_t f;
	int32_t t = drn_open_writer(&f, "./test/files/toto.data", "Super file !!!");
	if(t == 0){
		std::cout<<"//-> Open writer ok"<<std::endl;
	}else{
		std::cout<<"//-> Problem to open writer"<<std::endl;
	}
	
	uint32_t conf[] = {16, 128, 1, 0, 1, 0, 1}; 
	VoxelData* data = new VoxelData[3];
	
	data[0].sumNormal = glm::dvec3(1., 1., 1.5);
	data[0].sumBending = 0.4;
	data[0].sumGradient = 0.4;
	data[0].sumSurface = 0.4;
	data[0].sumDrain = 5;
	data[0].nbFaces = 10;
	
	data[1].sumNormal = glm::dvec3(1., 1., 1.5);
	data[1].sumBending = 0.3;
	data[1].sumGradient = 0.3;
	data[1].sumSurface = 0.3;
	data[1].sumDrain = 4;
	data[1].nbFaces = 8;
	
	data[2].sumNormal = glm::dvec3(1., 1., 1.5);
	data[2].sumBending = 0.6;
	data[2].sumGradient = 0.6;
	data[2].sumSurface = 0.6;
	data[2].sumDrain = 7;
	data[2].nbFaces = 9;
	
	//ecriture
	t = drn_writer_add_chunk(&f, conf, 7*sizeof(uint32_t));
	t = drn_writer_add_chunk(&f, data, 3*sizeof(VoxelData));
	
	t = drn_close_writer(&f);
	if(t == 0){
		std::cout<<"//-> Close writer ok"<<std::endl;
	}else{
		std::cout<<"//-> Problem to close writer"<<std::endl;
	}
}

void tryingReadingChunk(){
	drn_t f;
	int32_t t = drn_open(&f, "./test/files/toto.data", DRN_READ_NOLOAD);
	if(t == 0){
		std::cout<<"//-> Open reader ok"<<std::endl;
	}else{
		std::cout<<"//-> Problem to open reader"<<std::endl;
	}
	
	//lecture
	VoxelData* tab = new VoxelData[3];
	t = drn_read_chunk(&f, 1, tab);
	for(int i=0;i<3;++i){
		std::cout<<"//-> SumGradient : "<<tab[i].sumGradient<<std::endl;
	}
	
	t = drn_close(&f);
	if(t == 0){
		std::cout<<"//-> Close reader ok"<<std::endl;
	}else{
		std::cout<<"//-> Problem to close reader"<<std::endl;
	}
}
*/

/*******************************************************/

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

	//CONSTRUCTION OF THE DATA STRUCTURES
	Vertex * tabV = new Vertex[nbVertice];
	
	for(uint32_t n=0;n<nbVertice;++n){ // to create the vertices tab
		tabV[n].pos.x = positionsData[3*n];
		tabV[n].pos.z = positionsData[3*n+1];
		tabV[n].pos.y = positionsData[3*n+2];
		
		// on récupère les altitudes extremes
		if(tabV[n].pos.y > altMax){
			altMax = tabV[n].pos.y;
		}else{
			if(tabV[n].pos.y < altMin){
				altMin = tabV[n].pos.y;
			}
		}
	}

	Face * tabF = new Face[nbFace];
	uint32_t vertexCoordsOffset[3];
	
	// creation of the faces
	for(uint32_t n=0;n<nbFace;++n){
		for(size_t i = 0; i < 3; ++i){
			vertexCoordsOffset[i] = facesData[3*n+i];
		}
		tabF[n].s1 = tabV + vertexCoordsOffset[0] -1;
		tabF[n].s2 = tabV + vertexCoordsOffset[1] -1;
		tabF[n].s3 = tabV + vertexCoordsOffset[2] -1;
		tabF[n].normal = glm::dvec3(0,0,0);
		tabF[n].bending = 0;
		tabF[n].drain = 0;
		tabF[n].gradient = 0;
		tabF[n].surface = 0;
	}

	//VOXELS ARRAY CREATION
	
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
	}else printHelp();

	FILE* normalFile = NULL;
	normalFile = fopen("terrain_data/page_2.data", "rb");
	if(NULL == normalFile){
		std::cout << "[!] -> Unable to load the second data page" << std::endl;
		return EXIT_FAILURE;
	}

	//moving to the beginning of face normals in the file
	test_fic = fseek(normalFile, nbVertice*3*sizeof(double), SEEK_SET);
	if(test_fic != 0){
		std::cout<<"[!] -> Unable to move inside the second data page"<<std::endl;
		return EXIT_FAILURE;
	}

	double * normalData = new double[3*nbFace];
	test_fic = fread(normalData, sizeof(double), 3*nbFace, normalFile);

	for(uint32_t n=0;n<nbFace;++n){
		tabF[n].normal = glm::dvec3(normalData[3*n], normalData[3*n+2], normalData[3*n+1]);
	}

	fclose(normalFile);
	delete[] normalData;

	if(p4Requested){
		int* drainData = new int[nbFace];
		double* otherData = new double[3*nbFace];

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

		if(drain){
			for(uint32_t n=0;n<nbFace;++n) tabF[n].drain = drainData[n];
		}
		if(bending){	
			for(uint32_t n=0;n<nbFace;++n) tabF[n].bending = otherData[n*3];
		}
		if(surface){
			for(uint32_t n=0;n<nbFace;++n) tabF[n].surface = otherData[n*3+1];
		}
		if(gradient){
			for(uint32_t n=0;n<nbFace;++n) tabF[n].gradient = otherData[n*3+2];
		}
	}

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
	
	
	/* Grid from level 1 : grid of leaves */
	double l_size = GRID_3D_SIZE/(double)nbSub_lvl1;
	
	/* Grid from level 2 : grid of voxel inside a leaf */
	double voxelSize = l_size/(double)nbSub_lvl2;
	
	size_t const l_voxArrLength = nbSub_lvl2*nbSub_lvl2*nbSub_lvl2;
	VoxelData* l_voxelArray = NULL;
	l_voxelArray = new VoxelData[l_voxArrLength];
	if(NULL == l_voxelArray){
		std::cout<<"[!] -> Allocation failure for tabVoxel"<<std::endl;
		return EXIT_FAILURE;
	}
	
	for(uint32_t n=0;n<l_voxArrLength;++n){
		l_voxelArray[n].nbFaces=0;
		l_voxelArray[n].sumNormal = glm::dvec3(0,0,0);
		l_voxelArray[n].sumDrain = 0;
		l_voxelArray[n].sumGradient = 0;
		l_voxelArray[n].sumSurface = 0;
		l_voxelArray[n].sumBending = 0;
	}
	
	//INTERSECTION PROCESSING
	/* Range approximation for voxelisation */
	double Rc = voxelSize * APPROXIM_RANGE;
	
	//For each leaf
	for(uint16_t l_i=0;l_i<nbSub_lvl1;++l_i){
		for(uint16_t l_j=0;l_j<nbSub_lvl1;++l_j){
			for(uint16_t l_k=0;l_k<nbSub_lvl1;++l_k){
				
				glm::dvec3 l_origin = glm::dvec3((l_i*l_size)-1., (l_j*l_size)-1., (l_k*l_size)-1.);
				std::cout<<"//-> Leaf origin : ["<<l_origin.x<<"|"<<l_origin.y<<"|"<<l_origin.z<<"]"<<std::endl;
				
				//For each Face
				//#pragma omp parallel for
				for(uint32_t n=0;n<nbFace;++n){
					/* Face properties */
					/* min and max */
					uint16_t minVoxelX = ((getminX(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_i*nbSub_lvl2;
					uint16_t maxVoxelX = ((getminX(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_i*nbSub_lvl2;
					uint16_t minVoxelY = ((getminY(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_j*nbSub_lvl2;
					uint16_t maxVoxelY = ((getminY(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_j*nbSub_lvl2;
					uint16_t minVoxelZ = ((getminZ(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_k*nbSub_lvl2;
					uint16_t maxVoxelZ = ((getminZ(tabF[n])+1)*0.5)*nbSub_lvl1*nbSub_lvl2 - l_k*nbSub_lvl2;
					
					
					/*std::cout<<"//-> X bounds : ["<<minVoxelX<<"] ["<<maxVoxelX<<"]"<<std::endl;
					std::cout<<"//-> Y bounds : ["<<minVoxelY<<"] ["<<maxVoxelY<<"]"<<std::endl;
					std::cout<<"//-> Z bounds : ["<<minVoxelZ<<"] ["<<maxVoxelZ<<"]"<<std::endl;
					std::cout<<std::endl;*/
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
									Voxel vox = createVoxel(i*voxelSize + l_origin.x + voxelSize*0.5, j*voxelSize + l_origin.y + voxelSize*0.5, k*voxelSize + l_origin.z + voxelSize*0.5, voxelSize);
									if(processIntersectionPolygonVoxel(tabF[n], edgS1S2, edgS1S3, edgS2S3, upperPlane, lowerPlane, e1, e2, e3, vox, Rc, mode)){
										std::cout<<"intersec"<<std::endl;
										uint32_t currentIndex = i + nbSub_lvl2*j + k*nbSub_lvl2*nbSub_lvl2;
										l_voxelArray[currentIndex].nbFaces++;
										if(normal) 	l_voxelArray[currentIndex].sumNormal = glm::dvec3(l_voxelArray[currentIndex].sumNormal.x + tabF[n].normal.x, l_voxelArray[currentIndex].sumNormal.y + tabF[n].normal.y, l_voxelArray[currentIndex].sumNormal.z + tabF[n].normal.z);
										if(drain) 	l_voxelArray[currentIndex].sumDrain = l_voxelArray[currentIndex].sumDrain + tabF[n].drain;
										if(gradient)l_voxelArray[currentIndex].sumGradient = l_voxelArray[currentIndex].sumGradient + tabF[n].gradient;
										if(surface) l_voxelArray[currentIndex].sumSurface = l_voxelArray[currentIndex].sumSurface + tabF[n].surface;
										if(bending) l_voxelArray[currentIndex].sumBending = l_voxelArray[currentIndex].sumBending + tabF[n].bending;	
									} 						
								}
							}
						}
					}
				}
			}
		}
	}
	std::cout<<"-> Voxelisation finished !"<<std::endl;

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

	//WRITTING THE VOXEL-INTERSECTION FILE
	FILE* voxelFile = NULL;
	voxelFile = fopen("voxels_data/voxel_intersec_1.data", "wb");
	if(NULL == voxelFile){
		std::cout << "[!] > Unable to load the file voxelFile" << std::endl;
		return EXIT_FAILURE;
	}

	test_fic = fwrite(arguments, 7*sizeof(uint16_t), 1, voxelFile);
	test_fic = fwrite(l_voxelArray, l_voxArrLength*sizeof(VoxelData), 1, voxelFile);

	fclose(voxelFile);
	
	delete[] positionsData;
	delete[] facesData;

	delete[] tabV;
	delete[] tabF;
	delete[] l_voxelArray;

	return EXIT_SUCCESS;
}
